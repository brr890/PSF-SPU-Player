/***************************************************************************
                            spu.c  -  description
                             -------------------
    begin                : Wed May 15 2002
    copyright            : (C) 2002 by Pete Bernert
    email                : BlackDove@addcom.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version. See also the license.txt file for *
 *   additional informations.                                              *
 *                                                                         *
 ***************************************************************************/

//*************************************************************************//
// History of changes:
//
// 2003/03/01 - linuzappz
// - libraryName changes using ALSA
//
// 2003/02/28 - Pete
// - added option for type of interpolation
// - adjusted spu irqs again (Thousant Arms, Valkyrie Profile)
// - added MONO support for MSWindows DirectSound
//
// 2003/02/20 - kode54
// - amended interpolation code, goto GOON could skip initialization of gpos and cause segfault
//
// 2003/02/19 - kode54
// - moved SPU IRQ handler and changed sample flag processing
//
// 2003/02/18 - kode54
// - moved ADSR calculation outside of the sample decode loop, somehow I doubt that
//   ADSR timing is relative to the frequency at which a sample is played... I guess
//   this remains to be seen, and I don't know whether ADSR is applied to noise channels...
//
// 2003/02/09 - kode54
// - one-shot samples now process the end block before stopping
// - in light of removing fmod hack, now processing ADSR on frequency channel as well
//
// 2003/02/08 - kode54
// - replaced easy interpolation with gaussian
// - removed fmod averaging hack
// - changed .sinc to be updated from .iRawPitch, no idea why it wasn't done this way already (<- Pete: because I sometimes fail to see the obvious, haharhar :)
//
// 2003/02/08 - linuzappz
// - small bugfix for one usleep that was 1 instead of 1000
// - added iDisStereo for no stereo (Linux)
//
// 2003/01/22 - Pete
// - added easy interpolation & small noise adjustments
//
// 2003/01/19 - Pete
// - added Neill's reverb
//
// 2003/01/12 - Pete
// - added recording window handlers
//
// 2003/01/06 - Pete
// - added Neill's ADSR timings
//
// 2002/12/28 - Pete
// - adjusted spu irq handling, fmod handling and loop handling
//
// 2002/08/14 - Pete
// - added extra reverb
//
// 2002/06/08 - linuzappz
// - SPUupdate changed for SPUasync
//
// 2002/05/15 - Pete
// - generic cleanup for the Peops release
//
//*************************************************************************//

#define _IN_SPU

#include "../peops/stdafx.h"
#include "../peops/externals.h"
#include "../peops/registers.h"
#include "../peops/spu.h"
#include "spu2log.h"

// Enable experimental silence skipping
// Currently it is too aggressive, destroying the rhythm of some songs
// See https://github.com/audacious-media-player/audacious/issues/335
// #define ENABLE_SILENCE_SKIPPING

//#include "PsxMem.h"
//#include "driver.h"

////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////

// psx buffer / addresses

static u16  regArea[0x200];
static u16  spuMem[256*1024];
static u8 * spuMemC;
static u8 * pSpuIrq=0;
static u8 * pSpuBuffer;

extern "C" unsigned int psf2log_peops_copy_sample(
 unsigned int start_addr,
 unsigned int loop_addr,
 unsigned char *out_data,
 unsigned int capacity,
 unsigned int *out_loop_offset,
 unsigned int *out_end_flags)
{
 unsigned int start_byte,loop_byte,offset,length,copy_length,end_flags=0;

 if(out_loop_offset!=nullptr) *out_loop_offset=0;
 if(out_end_flags!=nullptr) *out_end_flags=0;
 if(spuMemC==nullptr || start_addr==0 || start_addr>=0x10000u) return 0;
 start_byte=(start_addr&0xffffu)<<3;
 loop_byte=(loop_addr&0xffffu)<<3;
 offset=start_byte;
 while(offset+16u<=sizeof(spuMem))
  {
   end_flags=spuMemC[offset+1u];
   offset+=16u;
   if(end_flags&1u) break;
  }
 if(offset<=start_byte || offset>sizeof(spuMem)) return 0;
 length=offset-start_byte;
 copy_length=length<capacity?length:capacity;
 if(out_data!=nullptr && copy_length!=0) memcpy(out_data,spuMemC+start_byte,copy_length);
 if(out_loop_offset!=nullptr && loop_byte>=start_byte && loop_byte<offset)
  *out_loop_offset=loop_byte-start_byte;
 if(out_end_flags!=nullptr) *out_end_flags=end_flags;
 return length;
}

// user settings
static int             iVolume;

// MAIN infos struct for each channel

static SPUCHAN         s_chan[MAXCHAN+1];                     // channel + 1 infos (1 is security for fmod handling)
static REVERBInfo      rvb;

static u32   dwNoiseVal=1;                          // global noise generator

static u16  spuCtrl=0;                             // some vars to store psx reg infos
static u16  spuStat=0;
static u16  spuIrq=0;
static u32  spuAddr=0xffffffff;                    // address into spu mem
static int  bSPUIsOpen=0;
static u32  psf2log_ps1_endx_mask=0;

static const int f[5][2] = {
			{    0,  0  },
                        {   60,  0  },
                        {  115, -52 },
                        {   98, -55 },
                        {  122, -60 } };
static s16 * pS;

static unsigned int psf2log_ps1_mute_mask;
static unsigned int psf2log_ps1_reverb_force_on_mask;
static unsigned int psf2log_ps1_reverb_force_off_mask;
static unsigned int psf2log_ps1_noise_force_on_mask;
static unsigned int psf2log_ps1_noise_force_off_mask;
static unsigned int psf2log_ps1_pmod_force_on_mask;
static unsigned int psf2log_ps1_pmod_force_off_mask;
static unsigned int psf2log_ps1_adsr_force_mask;
static int psf2log_ps1_adsr_freeze;
static unsigned int psf2log_ps1_pitch_lock_mask;
static unsigned int psf2log_ps1_pitch_lock_values[MAXCHAN];
static unsigned int psf2log_ps1_volume_lock_masks[2];
static unsigned int psf2log_ps1_volume_lock_values[2][MAXCHAN];
static int psf2log_ps1_main_enabled=1;
static int psf2log_ps1_reverb_enabled=1;
static int psf2log_ps1_mute_gain[MAXCHAN];
static int psf2log_ps1_mute_gain_init;
static unsigned int psf2log_ps1_timbre_solo_enabled;
static unsigned int psf2log_ps1_timbre_solo_starts[64];
static unsigned int psf2log_ps1_timbre_solo_loops[64];
static unsigned int psf2log_ps1_timbre_solo_flags[64];
static unsigned int psf2log_ps1_timbre_solo_key_count;
static int psf2log_ps1_timbre_gain[MAXCHAN];
static unsigned int psf2log_ps1_timbre_keyed_start[MAXCHAN];
static unsigned int psf2log_ps1_timbre_keyed_flags[MAXCHAN];
static unsigned char psf2log_ps1_timbre_allowed_latched[MAXCHAN];
static int psf2log_ps1_timbre_gain_init;

static int psf2log_ps1_effective_noise(int ch);
static int psf2log_ps1_effective_pmod_target(int ch);
static int psf2log_ps1_timbre_start_allowed(unsigned int start_addr, unsigned int flags);

static void psf2log_ps1_init_mute_gains(void)
{
 unsigned int ch;
 if(psf2log_ps1_mute_gain_init) return;
 for(ch=0;ch<MAXCHAN;ch++) psf2log_ps1_mute_gain[ch]=4096;
 psf2log_ps1_mute_gain_init=1;
}

static int psf2log_ps1_next_mute_gain(unsigned int ch, int muted)
{
 const int step=32;
 int target=muted?0:4096;
 int gain;
 psf2log_ps1_init_mute_gains();
 if(ch>=MAXCHAN) return target;
 gain=psf2log_ps1_mute_gain[ch];
 if(gain<target)
  {
   gain+=step;
   if(gain>target) gain=target;
  }
 else if(gain>target)
  {
   gain-=step;
   if(gain<target) gain=target;
  }
 psf2log_ps1_mute_gain[ch]=gain;
 return gain;
}

extern "C" void psf2log_peops_set_mute_mask(unsigned int mask)
{
 psf2log_ps1_init_mute_gains();
 psf2log_ps1_mute_mask=mask;
}

extern "C" void psf2log_peops_set_timbre_solo(int enabled, const unsigned int *starts, const unsigned int *loops, const unsigned int *flags, unsigned int count)
{
 unsigned int ch,i;
 if(count>64) count=64;
 psf2log_ps1_timbre_solo_enabled=enabled?1u:0u;
 psf2log_ps1_timbre_solo_key_count=count;
 for(i=0;i<count;i++)
  {
    psf2log_ps1_timbre_solo_starts[i]=starts!=nullptr?(starts[i]&0x000fffffu):0u;
    psf2log_ps1_timbre_solo_loops[i]=loops!=nullptr?(loops[i]&0x000fffffu):0u;
    psf2log_ps1_timbre_solo_flags[i]=flags!=nullptr?(flags[i]&0x0cu):0u;
  }
 for(;i<64;i++)
  {
   psf2log_ps1_timbre_solo_starts[i]=0u;
    psf2log_ps1_timbre_solo_loops[i]=0u;
    psf2log_ps1_timbre_solo_flags[i]=0u;
  }
 for(ch=0;ch<MAXCHAN;ch++)
  psf2log_ps1_timbre_allowed_latched[ch]=
   psf2log_ps1_timbre_start_allowed(
    psf2log_ps1_timbre_keyed_start[ch],
    psf2log_ps1_timbre_keyed_flags[ch])?1u:0u;
}

static unsigned int psf2log_ps1_ptr_addr(const u8 *ptr)
{
 if(ptr==nullptr || ptr<spuMemC) return 0xffffffffu;
 return (unsigned int)(((ptr-spuMemC)>>3)&0x000fffffu);
}

static unsigned int psf2log_ps1_timbre_channel_flags(int ch)
{
 unsigned int flags=0;
 if(psf2log_ps1_effective_noise(ch)) flags|=0x04u;
 if(psf2log_ps1_effective_pmod_target(ch)) flags|=0x08u;
 return flags;
}

static int psf2log_ps1_timbre_start_allowed(unsigned int start_addr, unsigned int flags)
{
 unsigned int i;
 if(!psf2log_ps1_timbre_solo_enabled || psf2log_ps1_timbre_solo_key_count==0) return 1;
 if(start_addr==0xffffffffu) return 0;
 for(i=0;i<psf2log_ps1_timbre_solo_key_count;i++)
  {
   unsigned int selected_start=psf2log_ps1_timbre_solo_starts[i];
   if(selected_start==start_addr && psf2log_ps1_timbre_solo_flags[i]==flags) return 1;
  }
 return 0;
}

static void psf2log_ps1_init_timbre_gains(void)
{
 unsigned int ch;
 if(psf2log_ps1_timbre_gain_init) return;
 for(ch=0;ch<MAXCHAN;ch++)
  {
   psf2log_ps1_timbre_gain[ch]=4096;
   psf2log_ps1_timbre_keyed_start[ch]=0xffffffffu;
   psf2log_ps1_timbre_keyed_flags[ch]=0u;
   psf2log_ps1_timbre_allowed_latched[ch]=1u;
  }
 psf2log_ps1_timbre_gain_init=1;
}

static int psf2log_ps1_next_timbre_gain(int ch)
{
 const int step=64;
 int target;
 int gain;
 psf2log_ps1_init_timbre_gains();
 if(ch<0 || ch>=MAXCHAN) return 0;
 target=psf2log_ps1_timbre_allowed_latched[ch]?4096:0;
 gain=psf2log_ps1_timbre_gain[ch];
 if(gain<target)
  {
   gain+=step;
   if(gain>target) gain=target;
  }
 else if(gain>target)
  {
   gain-=step;
   if(gain<target) gain=target;
  }
 psf2log_ps1_timbre_gain[ch]=gain;
 return gain;
}

extern "C" void psf2log_peops_set_main_enabled(int enabled)
{
 psf2log_ps1_main_enabled=enabled?1:0;
}

extern "C" void psf2log_peops_set_reverb_enabled(int enabled)
{
 psf2log_ps1_reverb_enabled=enabled?1:0;
}

extern "C" void psf2log_peops_set_reverb_override_masks(unsigned int force_on_mask, unsigned int force_off_mask)
{
 psf2log_ps1_reverb_force_on_mask=force_on_mask&0x00ffffffu;
 psf2log_ps1_reverb_force_off_mask=force_off_mask&0x00ffffffu;
}

extern "C" void psf2log_peops_set_noise_override_masks(unsigned int force_on_mask, unsigned int force_off_mask)
{
 psf2log_ps1_noise_force_on_mask=force_on_mask&0x00ffffffu;
 psf2log_ps1_noise_force_off_mask=force_off_mask&0x00ffffffu;
}

extern "C" void psf2log_peops_set_pmod_override_masks(unsigned int force_on_mask, unsigned int force_off_mask)
{
 psf2log_ps1_pmod_force_on_mask=force_on_mask&0x00ffffffu;
 psf2log_ps1_pmod_force_off_mask=force_off_mask&0x00ffffffu;
}

extern "C" void psf2log_peops_set_adsr_force_mask(unsigned int mask)
{
 unsigned int ch;
 psf2log_ps1_adsr_force_mask=mask&0x00ffffffu;
 for(ch=0;ch<MAXCHAN;ch++)
  {
   if((psf2log_ps1_adsr_force_mask&(1u<<ch))!=0)
    {
     s_chan[ch].ADSRX.EnvelopeVol=0x7FFFFFFF;
     s_chan[ch].ADSRX.lVolume=s_chan[ch].ADSRX.EnvelopeVol>>21;
     s_chan[ch].ADSRX.State=2;
     s_chan[ch].bStop=0;
    }
  }
}

extern "C" int psf2log_peops_get_adsr_force(unsigned int voice)
{
 if(voice>=MAXCHAN) return 0;
 return (psf2log_ps1_adsr_force_mask&(1u<<voice))!=0;
}

extern "C" void psf2log_peops_set_adsr_freeze(int enabled)
{
 psf2log_ps1_adsr_freeze=enabled?1:0;
}

extern "C" int psf2log_peops_get_adsr_freeze(void)
{
 return psf2log_ps1_adsr_freeze;
}

static void psf2log_ps1_restore_pitch_from_raw(unsigned int ch)
{
 int NP;
 if(ch>=MAXCHAN) return;
 NP=(44100L*s_chan[ch].iRawPitch)/4096L;
 if(NP<1) NP=1;
 s_chan[ch].iActFreq=NP;
 s_chan[ch].iUsedFreq=NP;
 s_chan[ch].sinc=s_chan[ch].iRawPitch<<4;
 if(!s_chan[ch].sinc) s_chan[ch].sinc=1;
}

extern "C" void psf2log_peops_restore_pitch(unsigned int voice)
{
 psf2log_ps1_restore_pitch_from_raw(voice);
}

extern "C" void psf2log_peops_set_pitch(unsigned int voice, unsigned int value)
{
 if(voice>=MAXCHAN) return;
 if(value>0x3fffu) value=0x3fffu;
 s_chan[voice].iRawPitch=(int)value;
 psf2log_ps1_restore_pitch_from_raw(voice);
}

extern "C" void psf2log_peops_set_pitch_lock(unsigned int voice, int enabled, unsigned int value)
{
 unsigned int bit;
 if(voice>=MAXCHAN) return;
 if(value>0x3fffu) value=0x3fffu;
 bit=1u<<voice;
 if(enabled)
  {
   psf2log_ps1_pitch_lock_values[voice]=value;
   psf2log_ps1_pitch_lock_mask|=bit;
   psf2log_peops_set_pitch(voice,value);
  }
 else
  {
   psf2log_ps1_pitch_lock_mask&=~bit;
  }
}

extern "C" int psf2log_peops_get_pitch_lock_value(unsigned int voice, unsigned int *out_value)
{
 if(voice>=MAXCHAN || out_value==NULL) return 0;
 if((psf2log_ps1_pitch_lock_mask&(1u<<voice))==0) return 0;
 *out_value=psf2log_ps1_pitch_lock_values[voice]&0x3fffu;
 return 1;
}

static int psf2log_ps1_effective_volume_from_raw(unsigned int value)
{
 s16 vol=(s16)(value&0xffffu);
 if(vol&0x8000)
  {
   s16 sInc=1;
   if(vol&0x2000) sInc=-1;
   if(vol&0x1000) vol^=0xffff;
   vol=((vol&0x7f)+1)/2;
   vol+=vol/(2*sInc);
   vol*=128;
   vol&=0x3fff;
  }
 else
  {
   if(vol&0x4000) vol=(vol&0x3fff)-0x4000;
   else vol&=0x3fff;
  }
 return (int)vol;
}

extern "C" void psf2log_peops_set_volume(unsigned int voice, unsigned int side, unsigned int value)
{
 int effective;
 if(voice>=MAXCHAN || side>=2) return;
 value&=0xffffu;
 effective=psf2log_ps1_effective_volume_from_raw(value);
 if(side==0)
  {
   s_chan[voice].iLeftVolRaw=(int)(s16)value;
   s_chan[voice].iLeftVolume=effective;
  }
 else
  {
   s_chan[voice].iRightVolRaw=(int)(s16)value;
   s_chan[voice].iRightVolume=effective;
  }
}

extern "C" void psf2log_peops_set_volume_lock(unsigned int voice, unsigned int side, int enabled, unsigned int value)
{
 unsigned int bit;
 if(voice>=MAXCHAN || side>=2) return;
 bit=1u<<voice;
 if(enabled)
  {
   psf2log_ps1_volume_lock_values[side][voice]=value&0xffffu;
   psf2log_ps1_volume_lock_masks[side]|=bit;
   psf2log_peops_set_volume(voice,side,value);
  }
 else
  {
   psf2log_ps1_volume_lock_masks[side]&=~bit;
  }
}

extern "C" int psf2log_peops_get_volume_lock_value(unsigned int voice, unsigned int side, unsigned int *out_value)
{
 if(voice>=MAXCHAN || side>=2 || out_value==NULL) return 0;
 if((psf2log_ps1_volume_lock_masks[side]&(1u<<voice))==0) return 0;
 *out_value=psf2log_ps1_volume_lock_values[side][voice]&0xffffu;
 return 1;
}

extern "C" void psf2log_peops_set_adsr_value(unsigned int voice, unsigned int field, unsigned int value)
{
 if(voice>=MAXCHAN) return;
 switch(field)
 {
  case 0: s_chan[voice].ADSRX.AttackRate=(int)(value&0x7fu); break;
  case 1: s_chan[voice].ADSRX.DecayRate=(int)(value&0x0fu); break;
  case 2: s_chan[voice].ADSRX.SustainLevel=(int)(value&0x0fu); break;
  case 3: s_chan[voice].ADSRX.SustainRate=(int)(value&0x7fu); break;
  case 4: s_chan[voice].ADSRX.ReleaseRate=(int)(value&0x1fu); break;
  default: break;
 }
}

extern "C" void psf2log_peops_set_noise_clock(unsigned int value)
{
 if(value>0x3fu) value=0x3fu;
 spuCtrl=(u16)((spuCtrl&~0x3f00u)|(value<<8));
}

extern "C" void psf2log_peops_set_reverb_value(unsigned int side, unsigned int value)
{
 value&=0xffffu;
 if(side==0) rvb.VolLeft=(s16)value;
 else if(side==1) rvb.VolRight=(s16)value;
}

static int psf2log_ps1_effective_noise(int ch)
{
 unsigned int bit=1u<<ch;
 if((psf2log_ps1_noise_force_off_mask&bit)!=0) return 0;
 if((psf2log_ps1_noise_force_on_mask&bit)!=0) return 1;
 return s_chan[ch].bNoise!=0;
}

static int psf2log_ps1_effective_pmod_target(int ch)
{
 unsigned int bit=1u<<ch;
 if((psf2log_ps1_pmod_force_off_mask&bit)!=0) return 0;
 if((psf2log_ps1_pmod_force_on_mask&bit)!=0) return 1;
 return s_chan[ch].bFMod==1;
}

static int psf2log_ps1_effective_pmod_source(int ch)
{
 if(ch+1>=MAXCHAN) return 0;
 return psf2log_ps1_effective_pmod_target(ch+1);
}
static s32 ttemp;

////////////////////////////////////////////////////////////////////////
// CODE AREA
////////////////////////////////////////////////////////////////////////

// dirty inline func includes

#include "../peops/reverb.cc"

static void psf2log_ps1_set_endx(int ch)
{
 if(ch>=0 && ch<MAXCHAN)
 {
  psf2log_ps1_endx_mask|=(1u<<ch);
 }
}

static u32 sampcount;
static u32 decaybegin;
static u32 decayend;
static u32 seektime;

#include "../peops/adsr.cc"

// Try this to increase speed.
#include "../peops/registers.cc"
#include "../peops/dma.cc"

////////////////////////////////////////////////////////////////////////
// helpers for so-called "gauss interpolation"

#define gval0 (((int *)(&s_chan[ch].SB[29]))[gpos])
#define gval(x) (((int *)(&s_chan[ch].SB[29]))[(gpos+x)&3])

#include "gauss_i.h"

////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// START SOUND... called by main thread to setup a new sound on a channel
////////////////////////////////////////////////////////////////////////

static inline void StartSound(int ch)
{
 unsigned int timbre_start;
 unsigned int timbre_flags;
 int timbre_allowed;

 StartADSR(ch);
 psf2log_ps1_init_timbre_gains();
 timbre_start=psf2log_ps1_ptr_addr(s_chan[ch].pStart);
 timbre_flags=psf2log_ps1_timbre_channel_flags(ch);
 timbre_allowed=psf2log_ps1_timbre_start_allowed(timbre_start,timbre_flags);
 psf2log_ps1_timbre_keyed_start[ch]=timbre_start;
 psf2log_ps1_timbre_keyed_flags[ch]=timbre_flags;
 psf2log_ps1_timbre_allowed_latched[ch]=timbre_allowed?1u:0u;
 psf2log_ps1_timbre_gain[ch]=timbre_allowed?4096:0;

 s_chan[ch].pCurr=s_chan[ch].pStart;                   // set sample start

 s_chan[ch].s_1=0;                                     // init mixing vars
 s_chan[ch].s_2=0;
 s_chan[ch].iSBPos=28;

 s_chan[ch].bNew=0;                                    // init channel flags
 s_chan[ch].bStop=0;
 s_chan[ch].bOn=1;

 s_chan[ch].SB[29]=0;                                  // init our interpolation helpers
 s_chan[ch].SB[30]=0;

 s_chan[ch].spos=0x30000L;s_chan[ch].SB[28]=0;  // -> start with more decoding
}

////////////////////////////////////////////////////////////////////////
// MAIN SPU FUNCTION
// here is the main job handler... thread, timer or direct func call
// basically the whole sound processing is done in this fat func!
////////////////////////////////////////////////////////////////////////

int psf_seek(u32 t)
{
 seektime=t*441/10;
 if(seektime>=sampcount) return(1);
 return(0);
}

static int endless;
void setendless(int e)
{
 endless=e;
}

// Counting to 65536 results in full volume offage.
void setlength(s32 stop, s32 fade)
{
 if(stop==~0 || endless)
 {
  decaybegin=~0;
 }
 else
 {
  stop=(stop*441)/10;
  fade=(fade*441)/10;

  decaybegin=stop;
  decayend=stop+fade;
 }
}

#define CLIP(_x) {if(_x>32767) _x=32767; if(_x<-32767) _x=-32767;}
int SPUasync(u32 cycles, void (*update)(const void *, int))
{
 int volmul=iVolume;
 static s32 dosampies;
 s32 temp;

 ttemp+=cycles;
 dosampies=ttemp/384;
 if(!dosampies) return(1);
 ttemp-=dosampies*384;
 temp=dosampies;

 while(temp)
 {
   s32 revLeft=0, revRight=0;
   s32 sl=0, sr=0;
   int ch,fa;

   temp--;
   //--------------------------------------------------//
   //- main channel loop                              -//
   //--------------------------------------------------//
    {
     for(ch=0;ch<MAXCHAN;ch++)                         // loop em all.
      {
       if(s_chan[ch].bNew) StartSound(ch);             // start new sound
       if(!s_chan[ch].bOn) continue;                   // channel not playing? next


       if(s_chan[ch].iActFreq!=s_chan[ch].iUsedFreq)   // new psx frequency?
        {
         s_chan[ch].iUsedFreq=s_chan[ch].iActFreq;     // -> take it and calc steps
         s_chan[ch].sinc=s_chan[ch].iRawPitch<<4;
         if(!s_chan[ch].sinc) s_chan[ch].sinc=1;
        }

         while(s_chan[ch].spos>=0x10000L)
          {
           if(s_chan[ch].iSBPos==28)                   // 28 reached?
            {
	     int predict_nr,shift_factor,flags,d,s;
	     u8* start;unsigned int nSample;
	     int s_1,s_2;

             start=s_chan[ch].pCurr;                   // set up the current pos

             if (start == (u8*)-1)          // special "stop" sign
             {
               s_chan[ch].bOn=0;                       // -> turn everything off
               if(psf2log_peops_get_adsr_force((unsigned int)ch))
                {
                 s_chan[ch].ADSRX.EnvelopeVol=0x7FFFFFFF;
                 s_chan[ch].ADSRX.lVolume=s_chan[ch].ADSRX.EnvelopeVol>>21;
                }
               else
                {
                 s_chan[ch].ADSRX.lVolume=0;
                 s_chan[ch].ADSRX.EnvelopeVol=0;
                }
               psf2log_ps1_set_endx(ch);
               goto ENDX;                              // -> and done for this channel
              }

             s_chan[ch].iSBPos=0;	// Reset buffer play index.

             //////////////////////////////////////////// spu irq handler here? mmm... do it later

             s_1=s_chan[ch].s_1;
             s_2=s_chan[ch].s_2;

             predict_nr=(int)*start;start++;
             shift_factor=predict_nr&0xf;
             predict_nr >>= 4;
             flags=(int)*start;start++;

             // -------------------------------------- //
	     // Decode new samples into s_chan[ch].SB[0 through 27]
             for (nSample=0;nSample<28;start++)
              {
               d=(int)*start;
               s=((d&0xf)<<12);
               if(s&0x8000) s|=0xffff0000;

               fa=(s >> shift_factor);
               fa=fa + ((s_1 * f[predict_nr][0])>>6) + ((s_2 * f[predict_nr][1])>>6);
               s_2=s_1;s_1=fa;
               s=((d & 0xf0) << 8);

               s_chan[ch].SB[nSample++]=fa;

               if(s&0x8000) s|=0xffff0000;
               fa=(s>>shift_factor);
               fa=fa + ((s_1 * f[predict_nr][0])>>6) + ((s_2 * f[predict_nr][1])>>6);
               s_2=s_1;s_1=fa;

               s_chan[ch].SB[nSample++]=fa;
              }

             //////////////////////////////////////////// irq check

             if(spuCtrl&0x40)         			// irq active?
              {
               if((pSpuIrq >  start-16 &&              // irq address reached?
                   pSpuIrq <= start) ||
                  ((flags&1) &&                        // special: irq on looping addr, when stop/loop flag is set
                   (pSpuIrq >  s_chan[ch].pLoop-16 &&
                    pSpuIrq <= s_chan[ch].pLoop)))
               {
		 //extern s32 spuirqvoodoo;
                 s_chan[ch].iIrqDone=1;                // -> debug flag
		 SPUirq();
		//puts("IRQ");
		 //if(spuirqvoodoo!=-1)
		 //{
		 // spuirqvoodoo=temp*384;
		 // temp=0;
		 //}
                }
              }

             //////////////////////////////////////////// flag handler

             if((flags&4) && (!s_chan[ch].bIgnoreLoop))
              s_chan[ch].pLoop=start-16;               // loop adress

             if(flags&1)                               // 1: stop/loop
              {
               // We play this block out first...
               //if(!(flags&2))                          // 1+2: do loop... otherwise: stop
               if(flags!=3 || s_chan[ch].pLoop==nullptr)  // PETE: if we don't check exactly for 3, loop hang ups will happen (DQ4, for example)
                {                                      // and checking if pLoop is set avoids crashes, yeah
                 start = (u8*)-1;
                }
               else
                {
                 start = s_chan[ch].pLoop;
                }
              }

             s_chan[ch].pCurr=start;                   // store values for next cycle
             s_chan[ch].s_1=s_1;
             s_chan[ch].s_2=s_2;

             ////////////////////////////////////////////
            }

           fa=s_chan[ch].SB[s_chan[ch].iSBPos++];      // get sample data

           if((spuCtrl&0x4000)==0) fa=0;               // muted?
	   else CLIP(fa);

	    {
	     int gpos;
             gpos = s_chan[ch].SB[28];
             gval0 = fa;
             gpos = (gpos+1) & 3;
             s_chan[ch].SB[28] = gpos;
	    }
           s_chan[ch].spos -= 0x10000L;
          }

         ////////////////////////////////////////////////
         // noise handler... just produces some noise data
         // surely wrong... and no noise frequency (spuCtrl&0x3f00) will be used...
         // and sometimes the noise will be used as fmod modulation... pfff

         if(psf2log_ps1_effective_noise(ch))
          {
	   //puts("Noise");
           if((dwNoiseVal<<=1)&0x80000000L)
            {
             dwNoiseVal^=0x0040001L;
             fa=((dwNoiseVal>>2)&0x7fff);
             fa=-fa;
            }
           else fa=(dwNoiseVal>>2)&0x7fff;

           // mmm... depending on the noise freq we allow bigger/smaller changes to the previous val
           fa=s_chan[ch].iOldNoise+((fa-s_chan[ch].iOldNoise)/((0x003f-((spuCtrl&0x3f00)>>8))+1));
           if(fa>32767L)  fa=32767L;
           if(fa<-32767L) fa=-32767L;
           s_chan[ch].iOldNoise=fa;

          }                                            //----------------------------------------
         else                                         // NO NOISE (NORMAL SAMPLE DATA) HERE
          {
             int vl, vr, gpos;
             vl = (s_chan[ch].spos >> 6) & ~3;
             gpos = s_chan[ch].SB[28];
             vr=(gauss[vl]*gval0)>>9;
             vr+=(gauss[vl+1]*gval(1))>>9;
             vr+=(gauss[vl+2]*gval(2))>>9;
             vr+=(gauss[vl+3]*gval(3))>>9;
             fa = vr>>2;
          }

         s_chan[ch].sval = (MixADSR(ch) * fa)>>10;     // / 1023;  // add adsr
         {
         int pmod_source=psf2log_ps1_effective_pmod_source(ch);
         if(pmod_source)                               // fmod freq channel
         {
           int NP=s_chan[ch+1].iRawPitch;
           NP=((32768L+s_chan[ch].sval)*NP)>>15; ///32768L;

           if(NP>0x3fff) NP=0x3fff;
           if(NP<0x1)    NP=0x1;

	   // mmmm... if I do this, all is screwed
	  //           s_chan[ch+1].iRawPitch=NP;

           NP=(44100L*NP)/(4096L);                     // calc frequency

           s_chan[ch+1].iActFreq=NP;
           s_chan[ch+1].iUsedFreq=NP;
           s_chan[ch+1].sinc=(((NP/10)<<16)/4410);
           if(!s_chan[ch+1].sinc) s_chan[ch+1].sinc=1;

		// mmmm... set up freq decoding positions?
		//           s_chan[ch+1].iSBPos=28;
		//           s_chan[ch+1].spos=0x10000L;
          }
         if(1)
          {
           //////////////////////////////////////////////
           // ok, left/right sound volume (psx volume goes from 0 ... 0x3fff)
	   int tmpl,tmpr;

	   int mute_gain=psf2log_ps1_next_mute_gain(ch,(psf2log_ps1_mute_mask&(1u<<ch))!=0);
	   int timbre_gain=psf2log_ps1_next_timbre_gain(ch);
	   tmpl=(s_chan[ch].sval*s_chan[ch].iLeftVolume)>>14;
	   tmpr=(s_chan[ch].sval*s_chan[ch].iRightVolume)>>14;
	   tmpl=(tmpl*mute_gain)>>12;
	   tmpr=(tmpr*mute_gain)>>12;
	   tmpl=(tmpl*timbre_gain)>>12;
	   tmpr=(tmpr*timbre_gain)>>12;
	   if(psf2log_ps1_main_enabled)
	   {
	    sl+=tmpl;
	    sr+=tmpr;
	   }

	   if(psf2log_ps1_reverb_enabled &&
          (psf2log_ps1_reverb_force_off_mask&(1u<<ch))==0 &&
          ((((rvb.Enabled>>ch)&1) && (spuCtrl&0x80)) ||
           (psf2log_ps1_reverb_force_on_mask&(1u<<ch))!=0))
	   {
	    revLeft+=tmpl;
	    revRight+=tmpr;
           }
	  }
         }

         s_chan[ch].spos += s_chan[ch].sinc;
 ENDX:   ;
      }
    }

  ///////////////////////////////////////////////////////
  // mix all channels (including reverb) into one buffer
  if(psf2log_ps1_reverb_enabled)
   MixREVERBLeftRight(&sl,&sr,revLeft,revRight);
//  printf("sampcount %d decaybegin %d decayend %d\n", sampcount, decaybegin, decayend);
  if(sampcount>=decaybegin)
  {
   s32 dmul;
   if(decaybegin!=~0U) // Is anyone REALLY going to be playing a song
		      // for 13 hours?
   {
    if(sampcount>=decayend)
    {
      update(nullptr, 0);
      return(0);
    }
    dmul=256-(256*(sampcount-decaybegin)/(decayend-decaybegin));
    sl=(sl*dmul)>>8;
    sr=(sr*dmul)>>8;
   }
  }

  sampcount++;
  sl=(sl*volmul)>>8;
  sr=(sr*volmul)>>8;

  //{
  // static double asl=0;
  // static double asr=0;

  // asl+=(sl-asl)/5;
  // asr+=(sl-asr)/5;

   //sl-=asl;
   //sr-=asr;

  // if(sl>32767 || sl < -32767) printf("Left: %d, %f\n",sl,asl);
  // if(sr>32767 || sr < -32767) printf("Right: %d, %f\n",sl,asl);
  //}

  if(sl>32767) sl=32767;
  if(sl<-32767) sl=-32767;
  if(sr>32767) sr=32767;
  if(sr<-32767) sr=-32767;

  *pS++=sl;
  *pS++=sr;
 }

 if (seektime != 0 && sampcount < seektime)
 {
   pS=(short *)pSpuBuffer;
 }
 else if ((((unsigned char *)pS)-((unsigned char *)pSpuBuffer)) == (735*4))
 {
#ifdef ENABLE_SILENCE_SKIPPING
   short *pSilenceIter = (short *)pSpuBuffer;
   int iSilenceCount = 0;

   for (; pSilenceIter < pS; pSilenceIter++)
   {
      if (*pSilenceIter == 0)
        iSilenceCount++;

      if (iSilenceCount > 20)
        break;
   }

   if (iSilenceCount < 20)
#endif
     update((u8*)pSpuBuffer,(u8*)pS-(u8*)pSpuBuffer);

   pS=(short *)pSpuBuffer;
 }

 return(1);
}

#ifdef TIMEO
static u64 begintime;
static u64 gettime64(void)
{
 struct timeval tv;
 u64 ret;

 gettimeofday(&tv,0);
 ret=tv.tv_sec;
 ret*=1000000;
 ret+=tv.tv_usec;
 return(ret);
}
#endif
////////////////////////////////////////////////////////////////////////
// INIT/EXIT STUFF
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// SPUINIT: this func will be called first by the main emu
////////////////////////////////////////////////////////////////////////

int SPUinit(void)
{
 spuMemC=(u8*)spuMem;                      // just small setup
 memset((void *)s_chan,0,MAXCHAN*sizeof(SPUCHAN));
 memset((void *)&rvb,0,sizeof(REVERBInfo));
 memset(regArea,0,sizeof(regArea));
 memset(spuMem,0,sizeof(spuMem));
 psf2log_ps1_endx_mask=0;
 InitADSR();
 sampcount=ttemp=0;
 seektime=0;
 #ifdef TIMEO
 begintime=gettime64();
 #endif
 return 0;
}

////////////////////////////////////////////////////////////////////////
// SETUPSTREAMS: init most of the spu buffers
////////////////////////////////////////////////////////////////////////

static void SetupStreams(void)
{
 int i;

 pSpuBuffer=(u8*)malloc(32768);            // alloc mixing buffer
 pS=(s16 *)pSpuBuffer;

 for(i=0;i<MAXCHAN;i++)                                // loop sound channels
  {
   s_chan[i].ADSRX.SustainLevel = 1024;                // -> init sustain
   psf2log_ps1_mute_gain[i]=4096;
   psf2log_ps1_timbre_gain[i]=4096;
   psf2log_ps1_timbre_keyed_start[i]=0xffffffffu;
   psf2log_ps1_timbre_keyed_flags[i]=0u;
   psf2log_ps1_timbre_allowed_latched[i]=1u;
   s_chan[i].iIrqDone=0;
   s_chan[i].pLoop=spuMemC;
   s_chan[i].pStart=spuMemC;
   s_chan[i].pCurr=spuMemC;
  }
 psf2log_ps1_mute_gain_init=1;
 psf2log_ps1_timbre_gain_init=1;
}

////////////////////////////////////////////////////////////////////////
// REMOVESTREAMS: free most buffer
////////////////////////////////////////////////////////////////////////

static void RemoveStreams(void)
{
 free(pSpuBuffer);                                     // free mixing buffer
 pSpuBuffer=nullptr;

 #ifdef TIMEO
 {
  u64 tmp;
  tmp=gettime64();
  tmp-=begintime;
  if(tmp)
   tmp=(u64)sampcount*1000000/tmp;
  printf("%lld samples per second\n",tmp);
 }
 #endif
}


////////////////////////////////////////////////////////////////////////
// SPUOPEN: called by main emu after init
////////////////////////////////////////////////////////////////////////

int SPUopen(void)
{
 if(bSPUIsOpen) return 0;                              // security for some stupid main emus
 spuIrq=0;

 spuStat=spuCtrl=0;
 spuAddr=0xffffffff;
 dwNoiseVal=1;
 psf2log_ps1_endx_mask=0;

 spuMemC=(u8*)spuMem;
 memset((void *)s_chan,0,(MAXCHAN+1)*sizeof(SPUCHAN));
 pSpuIrq=0;

 iVolume=255; //85;
 SetupStreams();                                       // prepare streaming

 bSPUIsOpen=1;

 return 1;
}

////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// SPUCLOSE: called before shutdown
////////////////////////////////////////////////////////////////////////

int SPUclose(void)
{
 if(!bSPUIsOpen) return 0;                             // some security

 bSPUIsOpen=0;                                         // no more open

 RemoveStreams();                                      // no more streaming

 return 0;
}

////////////////////////////////////////////////////////////////////////
// SPUSHUTDOWN: called by main emu on final exit
////////////////////////////////////////////////////////////////////////

int SPUshutdown(void)
{
 return 0;
}

void SPUinjectRAMImage(u16 *pIncoming)
{
	int i;

	for (i = 0; i < (256*1024); i++)
	{
		spuMem[i] = pIncoming[i];
	}
}

static u16 psf2log_ps1_clamp_u16_int(int value)
{
 if(value<=0) return 0;
 if(value>=0xffff) return 0xffff;
 return (u16)value;
}

static u16 psf2log_ps1_voice_volume_for_display(int raw_value,int effective_value)
{
 s16 raw=(s16)raw_value;
 (void)effective_value;
 if(raw<0 && raw>=-0x100)
 {
  return psf2log_ps1_clamp_u16_int(-raw);
 }
 if((raw&0x8000)==0)
 {
  int value;
  if(raw&0x4000) value=(raw&0x3fff)-0x4000;
  else value=raw&0x3fff;
  return psf2log_ps1_clamp_u16_int(value);
 }

 /*
  * Volume sweep registers describe a programmed step/rate, not a continuously
  * rewritten volume. For display, keep the bar tied to the written register so
  * mid-note volume changes appear as discrete setting changes.
  */
 return psf2log_ps1_clamp_u16_int((((int)raw&0x7f)+1)*128);
}

static u16 psf2log_ps1_swap16(u16 value)
{
 return (u16)((value<<8)|(value>>8));
}

static u32 psf2log_ps1_spu_ptr_to_addr(const u8 *ptr)
{
 const u8 *spu_end;
 if(ptr==nullptr || spuMemC==nullptr || ptr<spuMemC) return 0;
 spu_end=spuMemC+(256u*1024u*sizeof(u16));
 if(ptr>=spu_end) return 0;
 return (u32)((ptr-spuMemC)>>3);
}

static u8 psf2log_ps1_adsr_phase(const SPUCHAN *channel)
{
 if(channel==nullptr) return SPU2LOG_ADSR_OFF;
 if(channel->bStop) return SPU2LOG_ADSR_RELEASE;
 if(!channel->bOn && !channel->bNew && channel->ADSRX.EnvelopeVol<=0) return SPU2LOG_ADSR_OFF;
 switch(channel->ADSRX.State)
 {
  case 0: return SPU2LOG_ADSR_ATTACK;
  case 1: return SPU2LOG_ADSR_DECAY;
  case 2: return SPU2LOG_ADSR_SUSTAIN;
  default: return SPU2LOG_ADSR_SUSTAIN;
 }
}

extern "C" void psf2log_peops_emit_snapshots(
 void *user,
 uint64_t sample_pos,
 Spu2LogResult (*core_snapshot)(void *user, uint64_t sample_pos, const Spu2LogCoreSnapshot *snapshot),
 Spu2LogResult (*voice_snapshot)(void *user, uint64_t sample_pos, const Spu2LogVoiceSnapshot *snapshot))
{
 Spu2LogCoreSnapshot core;
 u32 active_mask=0;
 u32 noise_mask=0;
 u32 pmod_mask=0;
 u32 reverb_mask=0;
 int ch;

 memset(&core,0,sizeof(core));
 core.core=0;
 core.master_l=psf2log_ps1_swap16(regArea[(H_SPUmvolL-0xc00)>>1]);
 core.master_r=psf2log_ps1_swap16(regArea[(H_SPUmvolR-0xc00)>>1]);
 core.reverb_l=psf2log_ps1_swap16((u16)rvb.VolLeft);
 core.reverb_r=psf2log_ps1_swap16((u16)rvb.VolRight);
 core.flags=spuCtrl;

 for(ch=0;ch<MAXCHAN;ch++)
 {
  const SPUCHAN *channel=&s_chan[ch];
  u32 bit=1u<<ch;
  int adsr_forced=psf2log_peops_get_adsr_force(ch);
  int keyed=(channel->bNew || (channel->bOn && !channel->bStop) || adsr_forced);
  if(keyed) active_mask|=bit;
  if(psf2log_ps1_effective_noise(ch)) noise_mask|=bit;
  if(psf2log_ps1_effective_pmod_target(ch)) pmod_mask|=bit;
  if((rvb.Enabled&bit)!=0) reverb_mask|=bit;
 }

 core.key_on_mask=active_mask;
 core.noise_mask=noise_mask;
 core.pmod_mask=pmod_mask;
 core.reverb_mask=reverb_mask;
 if(core_snapshot!=nullptr) core_snapshot(user,sample_pos,&core);

 for(ch=0;ch<MAXCHAN;ch++)
 {
 const SPUCHAN *channel=&s_chan[ch];
 Spu2LogVoiceSnapshot voice;
  int adsr_forced=psf2log_peops_get_adsr_force(ch);
  int active=(channel->bOn || channel->bNew || channel->ADSRX.EnvelopeVol>0 || adsr_forced);
  int has_state=(active || channel->bStop || adsr_forced || channel->iRawPitch || channel->iLeftVolume || channel->iRightVolume ||
   channel->ADSRX.AttackRate || channel->ADSRX.DecayRate || channel->ADSRX.SustainRate || channel->ADSRX.ReleaseRate);
  memset(&voice,0,sizeof(voice));
  voice.core=0;
  voice.voice=(u8)ch;
  voice.active=(u8)(active?1:0);
  voice.adsr_phase=adsr_forced ? (u8)SPU2LOG_ADSR_SUSTAIN : (has_state ? psf2log_ps1_adsr_phase(channel) : (u8)SPU2LOG_ADSR_OFF);
  if(has_state && voice.adsr_phase == SPU2LOG_ADSR_OFF && channel->bStop)
  {
   voice.adsr_phase=SPU2LOG_ADSR_RELEASE;
  }
  if(has_state)
  {
   u16 adsr1=0;
   u16 adsr2=0;
   if(channel->ADSRX.AttackModeExp) adsr1|=0x8000u;
   adsr1|=(u16)((channel->ADSRX.AttackRate&0x7f)<<8);
   adsr1|=(u16)((channel->ADSRX.DecayRate&0x0f)<<4);
   adsr1|=(u16)(channel->ADSRX.SustainLevel&0x0f);
   if(channel->ADSRX.SustainModeExp) adsr2|=0x8000u;
   if(!channel->ADSRX.SustainIncrease) adsr2|=0x4000u;
   adsr2|=(u16)((channel->ADSRX.SustainRate&0x7f)<<6);
   if(channel->ADSRX.ReleaseModeExp) adsr2|=0x0020u;
   adsr2|=(u16)(channel->ADSRX.ReleaseRate&0x1f);
   voice.vol_l=psf2log_ps1_voice_volume_for_display(channel->iLeftVolRaw,channel->iLeftVolume);
   voice.vol_r=psf2log_ps1_voice_volume_for_display(channel->iRightVolRaw,channel->iRightVolume);
   voice.pitch=psf2log_ps1_clamp_u16_int(channel->iRawPitch);
   voice.adsr1=adsr1;
   voice.adsr2=adsr2;
   voice.envx=adsr_forced ? 0x7fffu : psf2log_ps1_clamp_u16_int(channel->ADSRX.EnvelopeVol>>16);
   voice.ssa=psf2log_ps1_spu_ptr_to_addr(channel->pStart);
   voice.lsa=psf2log_ps1_spu_ptr_to_addr(channel->pLoop);
   voice.nax=psf2log_ps1_spu_ptr_to_addr(channel->pCurr);
  }
  if(channel->bNew || (channel->bOn && !channel->bStop) || adsr_forced) voice.flags|=SPU2LOG_VOICE_KEY_ON;
  if(channel->bStop && !adsr_forced) voice.flags|=SPU2LOG_VOICE_RELEASE;
  voice.noise_clock=(u8)(((spuCtrl&0x3f00u)>>8)&0x3fu);
  if(has_state && psf2log_ps1_effective_noise(ch)) voice.flags|=SPU2LOG_VOICE_NOISE;
  if(has_state && psf2log_ps1_effective_pmod_target(ch)) voice.flags|=SPU2LOG_VOICE_PMOD;
  if((rvb.Enabled&(1u<<ch))!=0) voice.flags|=SPU2LOG_VOICE_REVERB;
  if(has_state && channel->iLeftVolume) voice.flags|=SPU2LOG_VOICE_DRY_L;
  if(has_state && channel->iRightVolume) voice.flags|=SPU2LOG_VOICE_DRY_R;
  if(voice_snapshot!=nullptr) voice_snapshot(user,sample_pos,&voice);
 }
}
/* Modified by brr890 for PSF SPU Player through 2026-07-21. */

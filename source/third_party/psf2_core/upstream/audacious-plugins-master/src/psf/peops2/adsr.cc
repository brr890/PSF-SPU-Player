/***************************************************************************
                          adsr.c  -  description
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

#include "stdafx.h"

#define _IN_ADSR

// will be included from spu.c
#ifdef _IN_SPU

////////////////////////////////////////////////////////////////////////
// ADSR func
////////////////////////////////////////////////////////////////////////

static void InitADSR(void)
{
}

// SPU2 uses the same 15-bit envelope and rate counter as SPU.
static inline int RunADSRStep(ADSRInfoEx2 *adsr,int shift,int stepValue,
                              int exponential,int decreasing,int infinite)
{
 int level=adsr->EnvelopeVol>>16;
 int levelStep=7-stepValue;
 u32 counterIncrement;

 if(infinite) return level;
 if(decreasing) levelStep=~levelStep;

 if(shift<11) levelStep<<=(11-shift);
 counterIncrement=(shift>11) ? (0x8000u>>(shift-11)) : 0x8000u;

 if(exponential && !decreasing && level>0x6000)
  {
   if(shift<10) levelStep>>=2;
   else if(shift>10) counterIncrement>>=2;
   else
    {
     levelStep>>=1;
     counterIncrement>>=1;
    }
  }
 else if(exponential && decreasing)
  {
   levelStep=(levelStep*level)>>15;
  }

 if(counterIncrement==0) counterIncrement=1;
 adsr->Counter+=counterIncrement;
 if(adsr->Counter>=0x8000u)
  {
   adsr->Counter=0;
   level+=levelStep;
   if(level<0) level=0;
   if(level>0x7fff) level=0x7fff;
   adsr->EnvelopeVol=level<<16;
  }

 return level;
}

static void StartADSR(int ch)
{
 s_chan[ch].ADSRX.lVolume=1;
 s_chan[ch].ADSRX.State=0;
 s_chan[ch].ADSRX.EnvelopeVol=0;
 s_chan[ch].ADSRX.Counter=0;
 s_chan[ch].ADSRX.ClockCounter=0;
}

static inline void RunADSRTick(int ch)
{
 ADSRInfoEx2 *adsr=&s_chan[ch].ADSRX;
 int level;

 if(s_chan[ch].bStop)
  {
   level=RunADSRStep(adsr,adsr->ReleaseRate,0,
                     adsr->ReleaseModeExp,1,adsr->ReleaseRate==0x1f);
   if(level<=0)
    {
     adsr->EnvelopeVol=0;
     s_chan[ch].bOn=0;
    }
  }
 else if(adsr->State==0)
  {
   level=RunADSRStep(adsr,adsr->AttackRate>>2,adsr->AttackRate&3,
                     adsr->AttackModeExp,0,adsr->AttackRate==0x7f);
   if(level>=0x7fff)
    {
     adsr->EnvelopeVol=0x7fff0000;
     adsr->Counter=0;
     adsr->State=1;
    }
  }
 else if(adsr->State==1)
  {
   int sustainTarget=(adsr->SustainLevel+1)<<11;
   if(sustainTarget>0x7fff) sustainTarget=0x7fff;
   level=RunADSRStep(adsr,adsr->DecayRate,0,1,1,0);
   if(level<=sustainTarget)
    {
     adsr->Counter=0;
     adsr->State=2;
    }
  }
 else
  {
   RunADSRStep(adsr,adsr->SustainRate>>2,adsr->SustainRate&3,
               adsr->SustainModeExp,!adsr->SustainIncrease,
               adsr->SustainRate==0x7f);
  }

 adsr->lVolume=adsr->EnvelopeVol>>21;
}

static int MixADSR(int ch)
{
 ADSRInfoEx2 *adsr=&s_chan[ch].ADSRX;

 if(psf2log_peops2_get_adsr_freeze())
  {
   adsr->lVolume=adsr->EnvelopeVol>>21;
   return adsr->lVolume;
  }

 if(psf2log_peops2_get_adsr_force((unsigned int)ch))
  {
   adsr->EnvelopeVol=0x7fff0000;
   adsr->lVolume=adsr->EnvelopeVol>>21;
   return adsr->lVolume;
  }

 // The player outputs 44.1 kHz; distribute the SPU2's 48 kHz ADSR ticks.
 adsr->ClockCounter+=48000u;
 while(adsr->ClockCounter>=44100u)
  {
   adsr->ClockCounter-=44100u;
   RunADSRTick(ch);
   if(!s_chan[ch].bOn) break;
  }

 return adsr->lVolume;
}

#endif

/* Modified by brr890 for PSF SPU Player through 2026-08-01. */

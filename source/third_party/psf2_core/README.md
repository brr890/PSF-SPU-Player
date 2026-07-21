# Imported PSF Playback Core

This directory contains the PSF input core imported from Audacious Plugins and
adapted for PSF SPU Player.

Only the source required by the PSF player is retained:

```text
upstream/audacious-plugins-master/
  COPYING
  src/psf/
```

The PSF code contains components under different licenses. Audio Overload SDK
files use BSD terms, while PeOPS and PeOPS2 use GNU GPL version 2 or later. The
combined player is therefore distributed as GPL-2.0-or-later.

See `IMPORT_NOTES.md`, the source-file headers, the original license files, and
the repository-level `THIRD_PARTY_NOTICES.md` for details.


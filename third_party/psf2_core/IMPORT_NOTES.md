# PSF2 Core Import Notes

## Imported Core

```text
Name: Audacious Plugins PSF input plugin
Source URL: https://github.com/audacious-media-player/audacious-plugins
Upstream version / commit: master snapshot; exact commit was not recorded
Imported date: 2026-06-28
Local source: upstream/audacious-plugins-master/src/psf
```

The repository contains the complete imported PSF source used by the build.
Unused Audacious plugins are intentionally omitted.

License summary:

- Audio Overload SDK PSF files: BSD 3-Clause
- PeOPS / PeOPS2 SPU files: GNU GPL version 2 or later
- Audacious framework notice: BSD 2-Clause; individual source terms apply

## License Review

Checklist:

- [x] License files included
- [x] Source modification allowed
- [x] Source and binary redistribution allowed under the stated licenses
- [x] Attribution requirements recorded
- [x] GPL/static-linking impact recorded
- [x] Combined player licensed as GPL-2.0-or-later

## Required Patches

Implemented integration points:

- PSF2 file open / library resolution
- PCM render loop
- SPU2 register write function
- sample position tracking
- SPU/SPU2 voice state extraction

## Bridge Mapping

The imported core should be exposed as:

```c
const Psf2CoreProvider *psf2log_get_imported_provider(void);
```

The provider must implement:

- `open`
- `render`
- `close`

The SPU2 write path must call:

```c
callbacks.spu2_write16(callbacks.user, sample_pos, address, value);
```

If accurate voice state is available:

```c
callbacks.voice_snapshot(callbacks.user, sample_pos, &snapshot);
```

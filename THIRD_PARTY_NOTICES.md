# Third-Party Notices

PSF SPU Player is distributed as a combined work under the GNU General Public
License, version 2 or (at your option) any later version. See `LICENSE`.

Copyright (C) 2026 brr890

## PeOPS / PeOPS2

Copyright (C) 2002 Pete Bernert and other contributors.

The PS1 SPU and PS2 SPU2 implementations under
`third_party/psf2_core/upstream/audacious-plugins-master/src/psf/peops` and
`peops2` are licensed under GNU GPL version 2 or later. Their original notices
and license files are retained in those directories.

## Audio Overload SDK PSF Code

Copyright (c) 2007-2008 R. Belmont and Richard Bannister.

The PSF loader, PSF engines, and PSX/IOP hardware code imported through the
Audacious PSF plugin are distributed under a BSD 3-Clause license. See
`dist/PSF SPU Player/licenses/LICENSE_AUDIO_OVERLOAD.txt` and the notices in the
individual source files.

## Audacious

Copyright (C) 2001-2026 Audacious developers and contributors.

The Audacious plugin build/framework license is BSD 2-Clause. Individual
plugins retain their own terms. See
`third_party/psf2_core/upstream/audacious-plugins-master/COPYING` and the source
file notices. In particular, the imported PSF code includes the GPL-covered
PeOPS components identified above.

## zlib

Copyright (C) 1995-2026 Jean-loup Gailly and Mark Adler.

zlib is distributed under the zlib License. See
`dist/PSF SPU Player/licenses/LICENSE_ZLIB.txt`.

## MinGW-w64 Runtime Libraries

The Windows player statically links eligible GCC runtime code and
`libwinpthread`. The GCC runtime libraries are covered by GNU GPL version 3 plus
the GCC Runtime Library Exception, version 3.1. `libwinpthread` is covered by
permissive MIT and BSD terms. See the corresponding files under
`dist/PSF SPU Player/licenses`.

## Archive Extraction Runtime

Binary packages may include a separate `archive` folder containing the
MinGW32 build of bsdtar and its dynamic libraries. This runtime is used only
to extract supported archives and is not loaded into the player process.

The runtime includes libarchive, bzip2, Expat, libb2, GNU libiconv, LZ4,
PCRE2, XZ Utils/liblzma, zlib, Zstandard, and MinGW runtime libraries. These
components retain their respective BSD, MIT, 0BSD, zlib, GNU LGPL, and GCC
Runtime Library Exception terms. License texts are under
`dist/PSF SPU Player/licenses/archive-runtime` and the existing MinGW license
files. Corresponding source is available from the upstream projects and the
matching MSYS2 MinGW32 source packages.

## Binary Releases

A binary release of PSF SPU Player must be accompanied by equivalent access to
the complete corresponding source code for that exact release, including the
modified third-party source and the scripts needed to build it. A GitHub
Release should therefore be created from the same tagged commit as its binary
package. Do not include PSF/PSF2 music, game assets, or BIOS files.

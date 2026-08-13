# PSF SPU Player Production Source

This folder contains the source required to build PSF SPU Player Ver 1.53.

Included:

- Windows player UI and resources
- PSF/miniPSF/PSF2/miniPSF2 loading and playback integration
- SPU/SPU2 state and logging code used by the player
- Required public headers
- Required Audacious PSF, Audio Overload, PeOPS, and PeOPS2 source
- Build configuration and license documents

Not included:

- Test songs or BIOS files
- Diagnostic analysis scripts
- Demo programs
- Test probes and mock providers
- Test-only command-line tools
- Generated build files, logs, settings, or debug output

## Build

Install CMake, MSYS2 MinGW32, Ninja, GCC, and zlib. Then run:

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
```

Output:

```text
build\mingw32-release\psf_spu_player.exe
```

The resulting 32-bit executable runs on Windows 10 32-bit and through WOW64 on
Windows 10 64-bit. GCC runtime code, zlib, and the logger are statically linked.

See `LICENSE`, `THIRD_PARTY_NOTICES.md`, and `licenses/` for license terms.

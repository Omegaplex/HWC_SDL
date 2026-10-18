# Homeworld: Cataclysm

This project aims to provide a human-readable, editable decompilation of the
Homeworld: Cataclysm source code. It cleans up compiler and decompiler artifacts
where possible and reuses applicable code from the publicly released Homeworld 1
source. Cataclysm-specific code is reconstructed from the original executable.

## Current status

- Single-player mission scripts are not fully ported from the original binary
  and will likely break on most levels.
- Movie playback is not supported.
- Skirmish games against the CPU are stable and should have only minimal bugs.
- Online multiplayer is not supported.
- Includes fixes for compatibility problems between the original Cataclysm
  binary and modern PCs, including failure to initialize the OpenGL renderer.
- The D3D renderer has been upgraded to D3D9; Glide rendering has been removed.
- Ship LOD ranges have been increased.

## Requirements to run

- A legal copy of the original Homeworld: Cataclysm game. Game data is not
  included in this repository.
- The rebuilt `cataclysm_debug.exe` or `cataclysm_release.exe`, `rgl.dll`, and
  `rgld3d.dll` placed in the game
  installation directory.
- Set `CATACLYSM_PATH` to that directory when using the development launch
  script.

## Build

Build on Windows with Visual Studio, the MSVC v143 C++ toolset or newer, a
Windows SDK, and vcpkg. The renderer uses the D3D9 headers and import library
provided by the Windows SDK.

Install vcpkg, set `VCPKG_ROOT` to its directory, and bootstrap and integrate
it once:

```powershell
$env:VCPKG_ROOT = '<path-to-vcpkg>'
& "$env:VCPKG_ROOT\bootstrap-vcpkg.bat" -disableMetrics
& "$env:VCPKG_ROOT\vcpkg.exe" integrate install
```

From a Visual Studio Developer PowerShell at the repository root, rebuild the
full solution:

```powershell
msbuild Cataclysm.sln /t:Rebuild /p:Configuration=Release /p:Platform=x86 /m /nologo /v:m
```

The full solution writes all project outputs to `Debug\` or `Release\` at the
repository root. For a Release build, copy `Release\cataclysm_release.exe`,
`Release\rgl.dll`, and `Release\rgld3d.dll` to the game installation directory.

To launch the built game from the repository, set `CATACLYSM_PATH` and use the
helper script, which runs the build output with the game directory as its
working directory:

```powershell
$env:CATACLYSM_PATH = '<path-to-game-installation>'
.\scripts\run_game.ps1 -Configuration Release
```

## Legal

No ownership over the source code is claimed and is the property of the original rights holders. The source code is provided for educational purposes only.
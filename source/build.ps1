param(
    [string]$Msys2Root = "C:\msys64"
)

$ErrorActionPreference = "Stop"

$mingwBin = Join-Path $Msys2Root "mingw32\bin"
$msysBin = Join-Path $Msys2Root "usr\bin"
$env:Path = "$mingwBin;$msysBin;$env:Path"
$build = Join-Path $PSScriptRoot "build\mingw32-release"

cmake -S $PSScriptRoot -B $build -G Ninja -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) {
    throw "CMake configuration failed with exit code $LASTEXITCODE"
}

cmake --build $build --target psf_spu_player
if ($LASTEXITCODE -ne 0) {
    throw "Build failed with exit code $LASTEXITCODE"
}

Write-Output "Built: $(Join-Path $build 'psf_spu_player.exe')"


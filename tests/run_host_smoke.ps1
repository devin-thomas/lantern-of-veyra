$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$gcc = (Get-Command gcc -ErrorAction Stop).Source
$exe = Join-Path $PSScriptRoot 'host_smoke.exe'
# The Genesis call recorder is the sgdk-host-stub package from uppercut-labs/game-kit, taken at this release tag.
# SGDK_HOST_STUB_DIR (the package folder of a game-kit checkout or the unpacked release) is used instead when set.
$stubTag = 'sgdk-host-stub@0.1.0'
$stubClone = $null
$stubDir = $env:SGDK_HOST_STUB_DIR
if (-not $stubDir) {
    $stubClone = Join-Path ([System.IO.Path]::GetTempPath()) ("sgdk-host-stub-" + [System.Guid]::NewGuid().ToString('N'))
    & git clone --quiet --depth 1 --branch $stubTag https://github.com/uppercut-labs/game-kit.git $stubClone
    if ($LASTEXITCODE -ne 0) { throw "Cannot fetch $stubTag; set SGDK_HOST_STUB_DIR to the sgdk-host-stub folder." }
    $stubDir = Join-Path $stubClone 'packages/sgdk-host-stub'
}
$resourceText = Get-Content -Raw (Join-Path $root 'res\resources.res')

if ($resourceText -notmatch '(?m)^TILESET bg_tileset "tileset\.png" FAST NONE$') {
    throw 'The map tileset must preserve source tile indices.'
}

try {
    & $gcc -std=c99 -Wall -Wextra `
        "-I$stubDir/include" "-I$root\inc" `
        "$PSScriptRoot\host_smoke.c" "$stubDir/src/sgdkstub.c" `
        "$root\src\map.c" "$root\src\map_data.c" `
        "$root\src\battle.c" "$root\src\ui.c" "$root\src\audio.c" "$root\src\save.c" "$root\src\settings.c" "$root\src\pause.c" "$root\src\town.c" "$root\src\inventory.c" "$root\src\economy.c" "$root\src\quest.c" "$root\src\dungeon.c" `
        -o $exe
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    & $exe
    exit $LASTEXITCODE
}
finally {
    if (Test-Path -LiteralPath $exe) {
        Remove-Item -LiteralPath $exe -Force
    }
    if ($stubClone -and (Test-Path -LiteralPath $stubClone)) {
        Remove-Item -LiteralPath $stubClone -Recurse -Force
    }
}

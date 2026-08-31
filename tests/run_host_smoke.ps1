$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$gcc = (Get-Command gcc -ErrorAction Stop).Source
$exe = Join-Path $PSScriptRoot 'host_smoke.exe'
$resourceText = Get-Content -Raw (Join-Path $root 'res\resources.res')

if ($resourceText -notmatch '(?m)^TILESET bg_tileset "tileset\.png" FAST NONE$') {
    throw 'The map tileset must preserve source tile indices.'
}

try {
    & $gcc -std=c99 -Wall -Wextra `
        "-I$PSScriptRoot\stub" "-I$root\inc" `
        "$PSScriptRoot\host_smoke.c" `
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
}

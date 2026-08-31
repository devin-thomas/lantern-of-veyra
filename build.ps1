param(
    [ValidateSet("release", "debug")]
    [string]$Configuration = "release"
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($env:GDK)) {
    throw "Set the GDK environment variable to your SGDK folder before building."
}

$make = Join-Path $env:GDK "bin\make.exe"
$makefile = Join-Path $env:GDK "makefile.gen"
if (!(Test-Path -LiteralPath $make)) {
    throw "SGDK make was not found at $make"
}
if (!(Test-Path -LiteralPath $makefile)) {
    throw "SGDK makefile.gen was not found at $makefile"
}

& $make -f $makefile $Configuration
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Host "Built out\rom.bin"

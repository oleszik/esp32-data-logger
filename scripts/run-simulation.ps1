$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Push-Location $projectRoot
try {
    pio run -e native
    $outputDir = Join-Path $projectRoot 'simulation-output'
    if (Test-Path -LiteralPath $outputDir) {
        Remove-Item -Recurse -Force -LiteralPath $outputDir
    }
    $program = Join-Path $projectRoot '.pio/build/native/program.exe'
    if (-not (Test-Path -LiteralPath $program)) {
        $program = Join-Path $projectRoot '.pio/build/native/program'
    }
    & $program $outputDir | Tee-Object -FilePath (Join-Path $projectRoot 'docs/example-output.txt')
    Copy-Item -LiteralPath (Join-Path $outputDir 'data/2026-10-03.csv') `
              -Destination (Join-Path $projectRoot 'docs/example-data.csv') -Force
} finally {
    Pop-Location
}


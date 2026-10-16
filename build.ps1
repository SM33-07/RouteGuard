[CmdletBinding()]
param(
    [ValidateSet('all', 'strict', 'test', 'demo', 'demo-check', 'clean')]
    [string]$Target = 'all',
    [string]$OptLevel = ''
)

$ErrorActionPreference = 'Stop'

# Precedence for compiler discovery:
# 1. RG_GCC_DIR, if explicitly set and valid. Prepend to PATH for this process and child processes.
# 2. gcc executable discoverable on PATH.
# 3. Clear actionable error if neither is available.
$resolvedGcc = $null
if ($env:RG_GCC_DIR) {
    $dir = $env:RG_GCC_DIR.Trim()
    $candidate = Join-Path $dir "gcc.exe"
    if (Test-Path $candidate) {
        $resolvedGcc = $candidate
        # Always ensure directory is prepended to PATH for all child processes
        $env:PATH = "$dir;$env:PATH"
    } else {
        Write-Error "RG_GCC_DIR is set to '$dir' but gcc.exe was not found there."
        exit 1
    }
} else {
    $gccCmd = Get-Command gcc -ErrorAction SilentlyContinue
    if ($gccCmd) {
        $resolvedGcc = $gccCmd.Source
    } else {
        Write-Error "ERROR: gcc compiler not found on PATH. Please add gcc to PATH or set RG_GCC_DIR to the directory containing gcc (e.g. `$env:RG_GCC_DIR = 'C:\msys64\ucrt64\bin')."
        exit 1
    }
}

# Determine optimization level: explicit -OptLevel parameter, or RG_OPT environment variable, or OPT alias, or -O2
if (-not $OptLevel) {
    if ($env:RG_OPT) {
        $OptLevel = $env:RG_OPT.Trim()
    } elseif ($env:OPT) {
        $OptLevel = $env:OPT.Trim()
    } else {
        $OptLevel = '-O2'
    }
}

$cflags = @('-std=c11', '-Wall', '-Wextra', '-pedantic', '-Wshadow', $OptLevel, '-Iinclude')
if ($Target -eq 'strict') {
    $cflags += '-Werror'
}

function Ensure-Dirs {
    if (-not (Test-Path 'bin')) { New-Item -ItemType Directory -Path 'bin' -Force | Out-Null }
    if (-not (Test-Path 'obj')) { New-Item -ItemType Directory -Path 'obj' -Force | Out-Null }
    if (-not (Test-Path 'out')) { New-Item -ItemType Directory -Path 'out' -Force | Out-Null }
}

function Clean-Build {
    Write-Host "Cleaning build artifacts..." -ForegroundColor Yellow
    if (Test-Path 'bin') { Remove-Item -Path 'bin' -Recurse -Force }
    if (Test-Path 'obj') { Remove-Item -Path 'obj' -Recurse -Force }
    if (Test-Path 'out\demo_run1.txt') { Remove-Item -Path 'out\demo_run1.txt' -Force }
    if (Test-Path 'out\demo_run2.txt') { Remove-Item -Path 'out\demo_run2.txt' -Force }
    if (Test-Path 'out\*.svg') { Remove-Item -Path 'out\*.svg' -Force }
}

if ($Target -eq 'clean') {
    Clean-Build
    exit 0
}

Ensure-Dirs

$srcFiles = @(
    'src/rg_alloc.c',
    'src/rg_graph.c',
    'src/rg_queue.c',
    'src/rg_stack.c',
    'src/rg_traverse.c',
    'src/rg_analysis.c',
    'src/rg_fixture.c',
    'src/rg_svg.c'
)

$testFiles = @(
    'tests/test_main.c',
    'tests/test_graph.c',
    'tests/test_traverse.c',
    'tests/test_analysis.c',
    'tests/test_fixture_ref.c'
)

Write-Host "Using compiler: $resolvedGcc" -ForegroundColor DarkGray
Write-Host "Compiling common modules with flags: $cflags" -ForegroundColor Cyan
$commonObjs = @()
foreach ($src in $srcFiles) {
    $obj = "obj/" + [System.IO.Path]::GetFileNameWithoutExtension($src) + ".o"
    & gcc $cflags -c $src -o $obj
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $commonObjs += $obj
}

Write-Host "Compiling main application..." -ForegroundColor Cyan
& gcc $cflags -c src/main.c -o obj/main.o
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& gcc $cflags -o bin/routeguard.exe ($commonObjs + 'obj/main.o')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Compiling test suite..." -ForegroundColor Cyan
$testObjs = @()
foreach ($t in $testFiles) {
    $obj = "obj/" + [System.IO.Path]::GetFileNameWithoutExtension($t) + ".o"
    & gcc $cflags -c $t -o $obj
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $testObjs += $obj
}

& gcc $cflags -o bin/rg_tests.exe ($commonObjs + $testObjs)
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Build complete: bin/routeguard.exe, bin/rg_tests.exe" -ForegroundColor Green

if ($Target -eq 'test') {
    Write-Host "`n=== Running Test Suite ===" -ForegroundColor Cyan
    & .\bin\rg_tests.exe
    exit $LASTEXITCODE
}

if ($Target -eq 'demo') {
    Write-Host "`n=== Running Demo ===" -ForegroundColor Cyan
    & .\bin\routeguard.exe --demo
    exit $LASTEXITCODE
}

if ($Target -eq 'demo-check') {
    Write-Host "`n=== Checking Demo Determinism ===" -ForegroundColor Cyan
    & .\bin\routeguard.exe --demo > out/demo_run1.txt
    & .\bin\routeguard.exe --demo > out/demo_run2.txt
    
    $bytes1 = [System.IO.File]::ReadAllBytes((Resolve-Path out/demo_run1.txt))
    $bytes2 = [System.IO.File]::ReadAllBytes((Resolve-Path out/demo_run2.txt))
    
    $match = $true
    if ($bytes1.Length -ne $bytes2.Length) {
        $match = $false
    } else {
        for ($i = 0; $i -lt $bytes1.Length; $i++) {
            if ($bytes1[$i] -ne $bytes2[$i]) {
                $match = $false
                break
            }
        }
    }
    
    if ($match) {
        Write-Host "DEMO DETERMINISM VERIFIED: Both runs are byte-identical ($($bytes1.Length) bytes)!" -ForegroundColor Green
    } else {
        Write-Error "Demo determinism failed! Binary mismatch detected between runs."
        exit 1
    }
}

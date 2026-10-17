[CmdletBinding()]
param(
    [string]$OutputDir = "frontend/src/wasm"
)

$ErrorActionPreference = 'Stop'

Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "RouteGuard WebAssembly Build Driver (Emscripten)" -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan

# 1. Discover Emscripten environment
$emccFound = Get-Command emcc -ErrorAction SilentlyContinue
if (-not $emccFound) {
    $emsdkDir = $null
    if ($env:RG_EMSDK_DIR -and (Test-Path $env:RG_EMSDK_DIR)) {
        $emsdkDir = $env:RG_EMSDK_DIR.Trim()
    } elseif (Test-Path "C:\emsdk") {
        $emsdkDir = "C:\emsdk"
    }

    if ($emsdkDir) {
        $envScript = Join-Path $emsdkDir "emsdk_env.ps1"
        if (Test-Path $envScript) {
            Write-Host "Activating Emscripten from: $emsdkDir" -ForegroundColor DarkGray
            $env:EMSDK_QUIET = "1"
            & $envScript
        } else {
            $emccPath = Join-Path $emsdkDir "upstream\emscripten"
            if (Test-Path $emccPath) {
                $env:PATH = "$emccPath;$emsdkDir;$env:PATH"
            }
        }
    }
}

# Verify emcc is available
$emccCmd = Get-Command emcc -ErrorAction SilentlyContinue
if (-not $emccCmd) {
    Write-Error "ERROR: Emscripten (emcc) was not found on PATH and could not be detected. Please add emcc to PATH or set RG_EMSDK_DIR to your EMSDK installation directory (e.g. `$env:RG_EMSDK_DIR = 'C:\emsdk')."
    exit 1
}

Write-Host "Using Emscripten: $($emccCmd.Source)" -ForegroundColor DarkGray
& emcc --version | Select-Object -First 1 | Write-Host -ForegroundColor DarkGray

# 2. Ensure output directory exists
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
}

$targetJs = Join-Path $OutputDir "routeguard.js"

# 3. Source files to compile (EXCLUDES main.c)
$sources = @(
    "src/rg_alloc.c",
    "src/rg_graph.c",
    "src/rg_queue.c",
    "src/rg_stack.c",
    "src/rg_traverse.c",
    "src/rg_analysis.c",
    "src/rg_fixture.c",
    "src/rg_svg.c",
    "wasm/rg_wasm_bridge.c"
)

$emccArgs = @(
    "-std=c11",
    "-Wall",
    "-Wextra",
    "-pedantic",
    "-Wshadow",
    "-O2",
    "-Iinclude",
    "-Iwasm",
    "-s", "WASM=1",
    "-s", "SINGLE_FILE=1",
    "-s", "EXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString','FS']",
    "-s", "EXPORTED_FUNCTIONS=['_rg_wasm_init','_rg_wasm_reset','_rg_wasm_get_graph_json','_rg_wasm_set_edge_active','_rg_wasm_run_bfs','_rg_wasm_run_dfs','_rg_wasm_find_route','_rg_wasm_get_components','_rg_wasm_get_network_state','_rg_wasm_get_failure_table','_rg_wasm_export_svg','_malloc','_free']",
    "-s", "ALLOW_MEMORY_GROWTH=1",
    "-s", "MODULARIZE=1",
    "-s", "EXPORT_ES6=1",
    "-o", $targetJs
) + $sources

Write-Host "Compiling C11 engine to WebAssembly module..." -ForegroundColor Yellow
& emcc @emccArgs

if ($LASTEXITCODE -ne 0) {
    Write-Error "Emscripten compilation failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

# 4. Generate TypeScript declarations for the generated module
$targetDts = Join-Path $OutputDir "routeguard.d.ts"
$dtsContent = @"
export interface RouteGuardModule {
    _rg_wasm_init(): number;
    _rg_wasm_reset(): number;
    _rg_wasm_get_graph_json(): number;
    _rg_wasm_set_edge_active(edge_id: number, active: number): number;
    _rg_wasm_run_bfs(src: number, dst: number, use_matrix: number): number;
    _rg_wasm_run_dfs(src: number, dst: number, use_matrix: number): number;
    _rg_wasm_find_route(src: number, dst: number, use_matrix: number): number;
    _rg_wasm_get_components(use_matrix: number): number;
    _rg_wasm_get_network_state(src: number, dst: number, use_matrix: number): number;
    _rg_wasm_get_failure_table(src: number, dst: number): number;
    _rg_wasm_export_svg(src: number, dst: number, use_matrix: number): number;
    UTF8ToString(ptr: number): string;
}

export default function createRouteGuardModule(): Promise<RouteGuardModule>;
"@
Set-Content -Path $targetDts -Value $dtsContent -Encoding UTF8

Write-Host "WASM Build complete:" -ForegroundColor Green
Get-Item $targetJs | Select-Object Name, Length, LastWriteTime | Format-Table -AutoSize
Write-Host "TypeScript definitions created: $targetDts" -ForegroundColor Green

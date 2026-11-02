$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot

cmake -S $Root -B (Join-Path $Root "build") -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed" }

cmake --build (Join-Path $Root "build") --config Release --parallel
if ($LASTEXITCODE -ne 0) { throw "Build failed" }

ctest --test-dir (Join-Path $Root "build") -C Release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "Automated tests failed" }

& (Join-Path $Root "build/Release/ecu_can_bench.exe") --demo
if ($LASTEXITCODE -ne 0) { throw "Demo failed" }

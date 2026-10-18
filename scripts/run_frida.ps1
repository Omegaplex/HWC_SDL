<#
.SYNOPSIS
    Convenience wrapper for the Cataclysm Frida parity runtime.

.DESCRIPTION
    Resolves the repo-local Frida virtualenv and forwards all remaining
    arguments to tools\frida\cataclysm_runtime.py. The runtime spawns the
    *retail* cataclysm.exe under Frida, injects CataclysmEngineTweaks.dll so
    retail behaves on modern Windows, resolves symbols from the IDA map, and
    emits ParityLog-style output.

    Wrapper-owned parameters use PowerShell's normal '-' style; everything else
    is passed straight through to the Python CLI (which uses '--' options and
    subcommands such as ping / resolve-symbol / memwrite-watch / function-trace /
    call-log).

.PARAMETER Setup
    Create the virtualenv (tools\frida\.venv) and pip-install requirements, then
    exit unless a runtime command was also supplied.

.PARAMETER PythonVersion
    Python launcher version used by -Setup (default: 3.11).

.PARAMETER RuntimeArgs
    Remaining arguments, forwarded verbatim to cataclysm_runtime.py.

.EXAMPLE
    .\scripts\run_frida.ps1 -Setup
    .\scripts\run_frida.ps1 ping
    .\scripts\run_frida.ps1 resolve-symbol --symbol univUpdate
    .\scripts\run_frida.ps1 memwrite-watch --field-addr 0xB62BAC --size 4 --duration 30
    .\scripts\run_frida.ps1 call-log --symbol taskExecuteAllPending --no-leave --duration 10
#>
[CmdletBinding(PositionalBinding = $false)]
param(
    [switch]$Setup,

    [string]$PythonVersion = '3.11',

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$RuntimeArgs
)

$ErrorActionPreference = 'Stop'

$RepoRoot = Split-Path -Parent $PSScriptRoot
$FridaDir = Join-Path $RepoRoot 'tools\frida'
$VenvPython = Join-Path $FridaDir '.venv\Scripts\python.exe'
$RuntimeScript = Join-Path $FridaDir 'cataclysm_runtime.py'
$Requirements = Join-Path $FridaDir 'requirements.txt'

if (-not (Test-Path $RuntimeScript)) {
    Write-Error "Cannot find cataclysm_runtime.py at $RuntimeScript"
    exit 1
}

if ($Setup) {
    Write-Host "Creating virtualenv at $FridaDir\.venv (python $PythonVersion)..."
    & py "-$PythonVersion" -m venv (Join-Path $FridaDir '.venv')
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to create virtualenv with 'py -$PythonVersion'."
        exit 1
    }

    & $VenvPython -m pip install --quiet --upgrade pip
    & $VenvPython -m pip install --quiet -r $Requirements
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Failed to install requirements from $Requirements."
        exit 1
    }

    & $VenvPython -c "import frida; print('frida', frida.__version__, 'ready')"
    Write-Host "Setup complete."

    if (-not $RuntimeArgs -or $RuntimeArgs.Count -eq 0) {
        exit 0
    }
}

if (-not (Test-Path $VenvPython)) {
    Write-Error "Frida venv not found at $VenvPython. Run: .\scripts\run_frida.ps1 -Setup"
    exit 1
}

if (-not $RuntimeArgs -or $RuntimeArgs.Count -eq 0) {
    Write-Host "No runtime command given. Examples:"
    Write-Host "  .\scripts\run_frida.ps1 ping"
    Write-Host "  .\scripts\run_frida.ps1 memwrite-watch --field-addr 0xB62BAC --size 4 --duration 30"
    Write-Host "Run with -Setup to (re)create the venv."
    exit 1
}

& $VenvPython $RuntimeScript @RuntimeArgs
exit $LASTEXITCODE

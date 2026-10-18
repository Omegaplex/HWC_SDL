<#
.SYNOPSIS
    Launches the built Cataclysm executable from the correct game-data directory.

.DESCRIPTION
    Resolves the exe path from the current repo root or worktree, sets the
    working directory to CATACLYSM_PATH, and passes through game arguments
    unchanged.

    Wrapper-owned parameters use PowerShell's normal '-' style.
    Game parameters use Cataclysm's native '/' style.

.PARAMETER Wait
    Wait N seconds after launch, then dump the tail of cataclysm.log.

.PARAMETER Configuration
    Build configuration to launch (`Debug` or `Release`).

.PARAMETER RecordDemo
    Record a demo to the specified path. Relative paths are resolved by trying
    caller working directory, repo root, then CATACLYSM_PATH.

.PARAMETER PlayDemo
    Play a demo from the specified path. Relative paths are resolved by trying
    caller working directory, repo root, then CATACLYSM_PATH.

.PARAMETER GameArgs
    Remaining arguments are passed directly to the game. Relative paths passed
    to /load and demo path arguments are resolved by trying caller working
    directory, repo root, then CATACLYSM_PATH.

.EXAMPLE
    .\scripts\run_game.ps1 -Wait 15
    .\scripts\run_game.ps1 /load .\temp\save.gam
    .\scripts\run_game.ps1 -PlayDemo .\temp\demo-smoke /new 1
    .\scripts\run_game.ps1 -Configuration Release -NoBackground
#>
[CmdletBinding(PositionalBinding = $false)]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',

    [int]$Wait = 15,

    [string]$RecordDemo,

    [string]$PlayDemo,

    [switch]$NoBackground,

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$GameArgs
)

$ErrorActionPreference = 'Stop'

function Resolve-RelativePath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$PathValue,
        [string[]]$SearchRoots = @(),
        [string[]]$OptionalExtensions = @(),
        [switch]$MustExist
    )

    if ([string]::IsNullOrWhiteSpace($PathValue)) {
        return $PathValue
    }

    if ([System.IO.Path]::IsPathRooted($PathValue)) {
        if ($MustExist -and -not (Test-Path $PathValue)) {
            Write-Error "Path not found: $PathValue"
            exit 1
        }
        return $PathValue
    }

    $Candidates = New-Object System.Collections.Generic.List[string]
    foreach ($Root in $SearchRoots) {
        if ([string]::IsNullOrWhiteSpace($Root)) {
            continue
        }

        $Candidates.Add((Join-Path $Root $PathValue))
        foreach ($Extension in $OptionalExtensions) {
            if ([string]::IsNullOrWhiteSpace($Extension)) {
                continue
            }

            if (-not $PathValue.EndsWith($Extension, [System.StringComparison]::OrdinalIgnoreCase)) {
                $Candidates.Add((Join-Path $Root ($PathValue + $Extension)))
            }
        }
    }

    foreach ($Candidate in ($Candidates | Select-Object -Unique)) {
        if (Test-Path $Candidate) {
            return $Candidate
        }
    }

    if ($MustExist) {
        if ($Candidates.Count -gt 0) {
            Write-Error ("Path not found: " + (($Candidates | Select-Object -Unique) -join ', '))
        }
        else {
            Write-Error "Path not found: $PathValue"
        }
        exit 1
    }

    if ($Candidates.Count -gt 0) {
        return $Candidates[0]
    }

    return $PathValue
}

function Resolve-GamePathArguments {
    param(
        [string[]]$GameArgs,
        [string]$Option,
        [string[]]$SearchRoots = @(),
        [string[]]$OptionalExtensions = @(),
        [switch]$MustExist
    )

    for ($i = 0; $i -lt $GameArgs.Count; $i++) {
        if ($GameArgs[$i] -ne $Option) {
            continue
        }

        if ($i + 1 -ge $GameArgs.Count) {
            Write-Error "Missing path after $Option"
            exit 1
        }

        $GameArgs[$i + 1] = Resolve-RelativePath -PathValue $GameArgs[$i + 1] -SearchRoots $SearchRoots -OptionalExtensions $OptionalExtensions -MustExist:$MustExist
        $i++
    }

    return $GameArgs
}

function Get-LogDelta {
    param(
        [string]$Path,
        [ref]$Offset
    )

    if (-not (Test-Path $Path)) {
        return ''
    }

    $Item = Get-Item $Path
    if ($Item.Length -lt $Offset.Value) {
        $Offset.Value = 0L
    }

    $FileStream = [System.IO.File]::Open($Path, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::ReadWrite)
    try {
        $FileStream.Seek($Offset.Value, [System.IO.SeekOrigin]::Begin) | Out-Null
        $Reader = [System.IO.StreamReader]::new($FileStream)
        try {
            $Text = $Reader.ReadToEnd()
            $Offset.Value = $FileStream.Position
            return $Text
        }
        finally {
            $Reader.Dispose()
        }
    }
    finally {
        $FileStream.Dispose()
    }
}

function Get-EnvironmentVariableValue {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    foreach ($Scope in 'Process', 'User', 'Machine') {
        $Value = [Environment]::GetEnvironmentVariable($Name, $Scope)
        if (-not [string]::IsNullOrWhiteSpace($Value)) {
            return $Value
        }
    }

    return $null
}

function Get-ActiveLogFileName {
    param(
        [string]$RepoRoot
    )

    $Worktree = 'main'
    $Match = [System.Text.RegularExpressions.Regex]::Match(
        ([System.IO.Path]::GetFullPath($RepoRoot)),
        '[\\/](?:worktrees)[\\/](?<name>[^\\/]+)$',
        [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)

    if ($Match.Success) {
        $Worktree = $Match.Groups['name'].Value
    }
    else {
        $Leaf = [System.IO.Path]::GetFileName([System.IO.Path]::GetFullPath($RepoRoot).TrimEnd('\', '/'))
        if (-not [string]::IsNullOrWhiteSpace($Leaf) -and $Leaf -ne 'Cataclysm') {
            $Worktree = $Leaf
        }
    }

    if ($Worktree -eq 'main') {
        return 'cataclysm.log'
    }

    $SanitizedWorktree = ($Worktree -replace '[^A-Za-z0-9._-]', '_')
    return "cataclysm_$SanitizedWorktree.log"
}

$RepoRoot = Split-Path -Parent $PSScriptRoot
if (-not (Test-Path (Join-Path $RepoRoot 'Cataclysm.vcxproj'))) {
    Write-Error "Cannot find Cataclysm.vcxproj at $RepoRoot"
    exit 1
}

if ($env:CATACLYSM_RUN_GAME_CHILD -ne '1') {
    $ScriptPath = $PSCommandPath.Replace("'", "''")
    $NestedRepoRoot = $RepoRoot.Replace("'", "''")
    $NestedCommand = "& { Set-Location '" + $NestedRepoRoot + "'; & '" + $ScriptPath + "' }"

    $NestedPsi = [System.Diagnostics.ProcessStartInfo]::new()
    $NestedPsi.FileName = (Get-Command powershell.exe -CommandType Application | Select-Object -First 1 -ExpandProperty Source)
    $NestedPsi.WorkingDirectory = $RepoRoot
    $NestedPsi.CreateNoWindow = $false
    $NestedPsi.UseShellExecute = $false
    $NestedPsi.Arguments = '-NoProfile -ExecutionPolicy Bypass -Command "{0}"' -f $NestedCommand
    $NestedPsi.EnvironmentVariables['CATACLYSM_RUN_GAME_CHILD'] = '1'
    $NestedPsi.EnvironmentVariables['CATACLYSM_RUN_GAME_CONFIGURATION'] = [System.Management.Automation.PSSerializer]::Serialize($Configuration)
    $NestedPsi.EnvironmentVariables['CATACLYSM_RUN_GAME_WAIT'] = [System.Management.Automation.PSSerializer]::Serialize($Wait)
    $NestedPsi.EnvironmentVariables['CATACLYSM_RUN_GAME_RECORD_DEMO'] = [System.Management.Automation.PSSerializer]::Serialize($RecordDemo)
    $NestedPsi.EnvironmentVariables['CATACLYSM_RUN_GAME_PLAY_DEMO'] = [System.Management.Automation.PSSerializer]::Serialize($PlayDemo)
    $NestedPsi.EnvironmentVariables['CATACLYSM_RUN_GAME_NO_BACKGROUND'] = [System.Management.Automation.PSSerializer]::Serialize([bool]$NoBackground)
    $NestedGameArgs = if ($null -eq $GameArgs) { [string[]]@() } else { [string[]]$GameArgs }
    $NestedPsi.EnvironmentVariables['CATACLYSM_RUN_GAME_ARGS'] = [System.Management.Automation.PSSerializer]::Serialize($NestedGameArgs)
    $NestedPsi.EnvironmentVariables['CATACLYSM_RUN_GAME_CALLER_CWD'] = [System.Management.Automation.PSSerializer]::Serialize((Get-Location).Path)

    $NestedProc = [System.Diagnostics.Process]::Start($NestedPsi)
    $NestedProc.WaitForExit()
    exit $NestedProc.ExitCode
}

if (-not [string]::IsNullOrEmpty($env:CATACLYSM_RUN_GAME_CONFIGURATION)) {
    $Configuration = [string][System.Management.Automation.PSSerializer]::Deserialize($env:CATACLYSM_RUN_GAME_CONFIGURATION)
}
if (-not [string]::IsNullOrEmpty($env:CATACLYSM_RUN_GAME_WAIT)) {
    $Wait = [int][System.Management.Automation.PSSerializer]::Deserialize($env:CATACLYSM_RUN_GAME_WAIT)
}
if (-not [string]::IsNullOrEmpty($env:CATACLYSM_RUN_GAME_RECORD_DEMO)) {
    $RecordDemo = [string][System.Management.Automation.PSSerializer]::Deserialize($env:CATACLYSM_RUN_GAME_RECORD_DEMO)
}
if (-not [string]::IsNullOrEmpty($env:CATACLYSM_RUN_GAME_PLAY_DEMO)) {
    $PlayDemo = [string][System.Management.Automation.PSSerializer]::Deserialize($env:CATACLYSM_RUN_GAME_PLAY_DEMO)
}
if (-not [string]::IsNullOrEmpty($env:CATACLYSM_RUN_GAME_NO_BACKGROUND)) {
    $NoBackground = [bool][System.Management.Automation.PSSerializer]::Deserialize($env:CATACLYSM_RUN_GAME_NO_BACKGROUND)
}
if (-not [string]::IsNullOrEmpty($env:CATACLYSM_RUN_GAME_ARGS)) {
    $GameArgs = [string[]][System.Management.Automation.PSSerializer]::Deserialize($env:CATACLYSM_RUN_GAME_ARGS)
}
else {
    $GameArgs = @()
}

$CallerWorkingDirectory = $RepoRoot
if (-not [string]::IsNullOrEmpty($env:CATACLYSM_RUN_GAME_CALLER_CWD)) {
    $CallerWorkingDirectory = [string][System.Management.Automation.PSSerializer]::Deserialize($env:CATACLYSM_RUN_GAME_CALLER_CWD)
}

$GameDataPath = Get-EnvironmentVariableValue -Name 'CATACLYSM_PATH'
if (-not $GameDataPath) {
    Write-Error "CATACLYSM_PATH environment variable must be set to the Cataclysm install root."
    exit 1
}

$OutputDir = Join-Path $RepoRoot $Configuration
$ExeName = 'cataclysm_{0}.exe' -f $Configuration.ToLowerInvariant()
$ExePath = Join-Path $OutputDir $ExeName
$LogFile = Join-Path $GameDataPath (Get-ActiveLogFileName -RepoRoot $RepoRoot)

if (-not (Test-Path $ExePath)) {
    Write-Error "Expected $ExeName in $OutputDir -- did you build $Configuration first?"
    exit 1
}

if (-not (Test-Path $GameDataPath)) {
    Write-Error "CATACLYSM_PATH not found at $GameDataPath"
    exit 1
}

if (-not $GameArgs) {
    $GameArgs = @()
}

if (-not [string]::IsNullOrWhiteSpace($RecordDemo) -and -not [string]::IsNullOrWhiteSpace($PlayDemo)) {
    Write-Error "Use only one of -RecordDemo or -PlayDemo."
    exit 1
}

if (-not [string]::IsNullOrWhiteSpace($RecordDemo) -and $GameArgs -contains '/demoRecord') {
    Write-Error "Use either -RecordDemo or a raw /demoRecord argument, not both."
    exit 1
}

if (-not [string]::IsNullOrWhiteSpace($PlayDemo) -and $GameArgs -contains '/demoPlay') {
    Write-Error "Use either -PlayDemo or a raw /demoPlay argument, not both."
    exit 1
}

$PathSearchRoots = @($CallerWorkingDirectory, $RepoRoot, $GameDataPath)

if (-not [string]::IsNullOrWhiteSpace($RecordDemo)) {
    $GameArgs += @('/demoRecord', (Resolve-RelativePath -PathValue $RecordDemo -SearchRoots $PathSearchRoots -OptionalExtensions @('.dem')))
}

if (-not [string]::IsNullOrWhiteSpace($PlayDemo)) {
    $GameArgs += @('/demoPlay', (Resolve-RelativePath -PathValue $PlayDemo -SearchRoots $PathSearchRoots -OptionalExtensions @('.dem')))
}

$GameArgs = Resolve-GamePathArguments -GameArgs $GameArgs -Option '/load' -SearchRoots $PathSearchRoots -MustExist
$GameArgs = Resolve-GamePathArguments -GameArgs $GameArgs -Option '/demoRecord' -SearchRoots $PathSearchRoots -OptionalExtensions @('.dem')
$GameArgs = Resolve-GamePathArguments -GameArgs $GameArgs -Option '/demoPlay' -SearchRoots $PathSearchRoots -OptionalExtensions @('.dem')

if (-not $NoBackground) {
    if ($GameArgs -notcontains '/background') {
        $GameArgs = @('/background') + $GameArgs
    }
}

Write-Host "Exe:       $ExePath"
Write-Host "Config:    $Configuration"
Write-Host "WorkDir:   $GameDataPath"
Write-Host "Log:       $LogFile"
Write-Host "Args:      $($GameArgs -join ' ')"
Write-Host ''

$Psi = [System.Diagnostics.ProcessStartInfo]::new()
$Psi.FileName = $ExePath
$Psi.WorkingDirectory = $GameDataPath
$Psi.CreateNoWindow = $true
$Psi.UseShellExecute = $false
if ($GameArgs) {
    $Psi.Arguments = ($GameArgs | ForEach-Object {
            if ($_ -match '\s') { "`"$_`"" } else { $_ }
        }) -join ' '
}

$Proc = [System.Diagnostics.Process]::Start($Psi)

Write-Host "Launched PID $($Proc.Id)"

if ($Wait -gt 0) {
    $WatchingDemoCompletion = ($GameArgs -contains '/demoPlay')
    $LogOffset = 0L
    if (Test-Path $LogFile) {
        $LogOffset = (Get-Item $LogFile).Length
    }

    Write-Host "Waiting up to ${Wait}s (exits early if process dies)..."
    $Deadline = (Get-Date).AddSeconds($Wait)
    while ((Get-Date) -lt $Deadline) {
        Start-Sleep -Milliseconds 500
        $Proc.Refresh()
        if ($Proc.HasExited) {
            Write-Host "Process exited early (exit code $($Proc.ExitCode))"
            break
        }

        if ($WatchingDemoCompletion) {
            $LogDelta = Get-LogDelta -Path $LogFile -Offset ([ref]$LogOffset)
            if ($LogDelta -match 'Quit to main menu!') {
                Write-Host "Demo playback finished; stopping process."
                Stop-Process -Id $Proc.Id
                $Proc.WaitForExit()
                $Proc.Refresh()
                break
            }
        }
    }

    if (Test-Path $LogFile) {
        Write-Host "`n--- Last 40 lines of $LogFile ---"
        Get-Content $LogFile -Tail 40
    }
    else {
        Write-Host "Log file not found yet: $LogFile"
    }

    if (-not $Proc.HasExited) {
        Stop-Process -Id $Proc.Id
        $Proc.WaitForExit()
        $Proc.Refresh()
        Write-Host "Process terminated after ${Wait}s (exit code $($Proc.ExitCode))"
    }

    if ($Proc.HasExited) {
        exit $Proc.ExitCode
    }
}

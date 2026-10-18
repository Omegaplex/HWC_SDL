param(
    [Parameter(Mandatory = $true)]
    [string]$RepoDir,

    [Parameter(Mandatory = $true)]
    [string]$OutputPath
)

$ErrorActionPreference = 'Stop'

function Escape-CString {
    param([string]$Value)

    return $Value.Replace('\', '\\').Replace('"', '\"')
}

function Get-GitValue {
    param(
        [string[]]$Arguments,
        [string]$Fallback
    )

    try {
        $value = & git -C $RepoDir @Arguments 2>$null
        if ($LASTEXITCODE -ne 0) {
            return $Fallback
        }

        $text = ($value | Out-String).Trim()
        if ([string]::IsNullOrWhiteSpace($text)) {
            return $Fallback
        }

        return $text
    } catch {
        return $Fallback
    }
}

$repoRoot = Get-GitValue -Arguments @('rev-parse', '--show-toplevel') -Fallback $RepoDir
$branch = Get-GitValue -Arguments @('branch', '--show-current') -Fallback 'detached'
$repoRoot = [System.IO.Path]::GetFullPath($repoRoot)

$worktree = 'main'
$match = [System.Text.RegularExpressions.Regex]::Match(
    $repoRoot,
    '[\\/](?:worktrees)[\\/](?<name>[^\\/]+)$',
    [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)

if ($match.Success) {
    $worktree = $match.Groups['name'].Value
} else {
    $leaf = [System.IO.Path]::GetFileName($repoRoot)
    if (-not [string]::IsNullOrWhiteSpace($leaf) -and $leaf -ne 'Cataclysm') {
        $worktree = $leaf
    }
}

$sanitizedWorktree = ($worktree -replace '[^A-Za-z0-9._-]', '_')

$logFile = 'cataclysm.log'
if ($worktree -ne 'main') {
    $logFile = "cataclysm_$sanitizedWorktree.log"
}

$outputDir = Split-Path -Parent $OutputPath
if (-not [string]::IsNullOrWhiteSpace($outputDir)) {
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
}

$escapedBranch = Escape-CString $branch
$escapedWorktree = Escape-CString $worktree
$escapedLogFile = Escape-CString $logFile

$header = @"
#pragma once
#define CATACLYSM_BUILD_BRANCH "$escapedBranch"
#define CATACLYSM_BUILD_WORKTREE "$escapedWorktree"
#define CATACLYSM_BUILD_LOG_FILE "$escapedLogFile"
"@

$existingContent = $null
if (Test-Path $OutputPath) {
    $existingContent = [System.IO.File]::ReadAllText($OutputPath, [System.Text.Encoding]::ASCII)
    $existingContent = $existingContent -replace "`r`n", "`n" -replace "`r", "`n"
}

$newContent = $header -replace "`r`n", "`n" -replace "`r", "`n"

if ($existingContent -ne $newContent) {
    [System.IO.File]::WriteAllText($OutputPath, $header, [System.Text.Encoding]::ASCII)
}

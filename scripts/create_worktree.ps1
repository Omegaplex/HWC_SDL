<#
.SYNOPSIS
    Creates a git worktree with pre-warmed Cataclysm build caches.

.DESCRIPTION
    Creates a new worktree under worktrees\<name>, copies the Debug\ and Release\
    build caches from the current repo for faster incremental builds, and clears
    tracking logs that would otherwise keep absolute paths from the source tree.

.PARAMETER Name
    Worktree directory name. Created under worktrees\<Name>.

.PARAMETER BranchName
    Optional branch name. Defaults to <Name>.

.PARAMETER NoCopy
    Skip copying the Debug\ and Release\ build caches.

.EXAMPLE
    .\scripts\create_worktree.ps1 naming-pass
    .\scripts\create_worktree.ps1 -Name naming-pass -BranchName feature/naming-pass
#>
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Name,

    [Parameter()]
    [string]$BranchName,

    [switch]$NoCopy
)

$ErrorActionPreference = 'Stop'

$RepoRoot = Split-Path -Parent $PSScriptRoot
if (-not (Test-Path (Join-Path $RepoRoot 'Cataclysm.vcxproj'))) {
    Write-Error "Cannot find Cataclysm.vcxproj at $RepoRoot"
    exit 1
}

$WorktreeRoot = Join-Path $RepoRoot 'worktrees'
$WorktreeDir = Join-Path $WorktreeRoot $Name
$BuildCacheDirs = @(
    @{ Name = 'Debug'; Path = (Join-Path $RepoRoot 'Debug') },
    @{ Name = 'Release'; Path = (Join-Path $RepoRoot 'Release') }
)

if (-not $BranchName) {
    $BranchName = $Name
}

if (Test-Path $WorktreeDir) {
    Write-Error "Worktree directory already exists: $WorktreeDir"
    exit 1
}

$ExistingBranch = git -C $RepoRoot branch --list $BranchName 2>$null
if (-not [string]::IsNullOrWhiteSpace($ExistingBranch)) {
    Write-Error "Branch already exists: $BranchName"
    exit 1
}

if (-not ($BuildCacheDirs | Where-Object { Test-Path $_.Path })) {
    Write-Warning 'No Debug\ or Release\ directories found in the current tree -- skipping build cache copy.'
    $NoCopy = $true
}

function Copy-BuildCacheDirectory {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SourceDir,

        [Parameter(Mandatory = $true)]
        [string]$DestinationDir,

        [Parameter(Mandatory = $true)]
        [string]$Label
    )

    Write-Host "Copying $Label build cache..." -ForegroundColor Cyan

    $null = robocopy $SourceDir $DestinationDir /E /NFL /NDL /NJH /NJS /nc /ns /np
    if ($LASTEXITCODE -gt 7) {
        Write-Warning "robocopy returned $LASTEXITCODE for $Label -- build cache may be incomplete"
    }

    Get-ChildItem -Path $DestinationDir -Directory -Filter '*.tlog' -Recurse -ErrorAction SilentlyContinue |
        ForEach-Object {
            Remove-Item -Recurse -Force $_.FullName
            Write-Host "  Cleared $($_.FullName.Substring($DestinationDir.Length + 1))" -ForegroundColor DarkGray
        }

    Get-ChildItem -Path $DestinationDir -Filter '*.FileListAbsolute.txt' -Recurse -ErrorAction SilentlyContinue |
        Remove-Item -Force

    $ObjCount = (Get-ChildItem -Path $DestinationDir -Filter '*.obj' -File -Recurse -ErrorAction SilentlyContinue).Count
    Write-Host "  Copied $ObjCount .obj files" -ForegroundColor DarkGray
}

New-Item -ItemType Directory -Force -Path $WorktreeRoot | Out-Null

Write-Host "Creating worktree: worktrees\$Name (branch: $BranchName)" -ForegroundColor Cyan
git -C $RepoRoot worktree add "worktrees\$Name" -b $BranchName
if ($LASTEXITCODE -ne 0) {
    Write-Error 'git worktree add failed'
    exit 1
}

if (-not $NoCopy) {
    foreach ($BuildCache in $BuildCacheDirs) {
        if (-not (Test-Path $BuildCache.Path)) {
            Write-Host "Skipping missing $($BuildCache.Name)\ build cache." -ForegroundColor DarkGray
            continue
        }

        $WorktreeBuildCacheDir = Join-Path $WorktreeDir $BuildCache.Name
        Copy-BuildCacheDirectory -SourceDir $BuildCache.Path `
                                 -DestinationDir $WorktreeBuildCacheDir `
                                 -Label $BuildCache.Name
    }
}
# Precompiled codec obj files live in lib\ — copy them to the new worktree
$LibSource = Join-Path $RepoRoot 'lib'
$LibDest   = Join-Path $WorktreeDir 'lib'
if (Test-Path $LibSource) {
    if (-not (Test-Path $LibDest)) {
        New-Item -ItemType Directory -Path $LibDest | Out-Null
    }
    Copy-Item (Join-Path $LibSource '*.obj') $LibDest -Force
    Write-Host "  Copied lib\ codec obj files" -ForegroundColor DarkGray
}

Write-Host "Done. Worktree ready at: $WorktreeDir" -ForegroundColor Green
Write-Host "  Set-Location $WorktreeDir" -ForegroundColor DarkGray

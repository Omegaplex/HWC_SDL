<#
.SYNOPSIS
    Packages cataclysm.i64 and uploads it to Azure Blob Storage using Azure CLI login.

.DESCRIPTION
    Creates a zip snapshot of the IDA database, uploads it with `az storage blob upload`
    using `--auth-mode login`, and optionally prunes older automatic snapshots so the
    remote history stays bounded. The script never stores credentials; the caller must
    have already run `az login` locally and have data-plane access to the target container.

.PARAMETER AccountName
    Azure storage account name. You may also pass the full blob endpoint URL, such as
    https://mystorage.blob.core.windows.net, and the script will extract the account name.

.PARAMETER ContainerName
    Azure Blob container name that will store the snapshots.

.PARAMETER SourcePath
    Path to the IDA database to snapshot. Defaults to <repo>\cataclysm.i64.

.PARAMETER Prefix
    Blob path prefix inside the container. Defaults to cataclysm-ida.

.PARAMETER KeepLast
    Number of automatic snapshots to retain per branch. Older automatic snapshots are
    deleted after a successful upload. Milestone snapshots are never pruned.

.PARAMETER MilestoneName
    Optional milestone label. When provided, the snapshot is stored under a milestone
    path and excluded from automatic retention pruning.

.PARAMETER TempDir
    Local staging directory for the zip archive. Defaults to <repo>\temp\ida-snapshots.

.PARAMETER KeepArchive
    Keep the local zip archive after upload. By default it is deleted.

.EXAMPLE
    .\scripts\upload_ida_snapshot.ps1 -AccountName mystorage -ContainerName ida-snapshots

.EXAMPLE
    .\scripts\upload_ida_snapshot.ps1 -AccountName mystorage -ContainerName ida-snapshots -MilestoneName pre-batch-12
#>
[CmdletBinding(SupportsShouldProcess = $true, ConfirmImpact = 'Medium', PositionalBinding = $false)]
param(
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$AccountName,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string]$ContainerName,

    [string]$SourcePath,

    [ValidateNotNullOrEmpty()]
    [string]$Prefix = 'cataclysm-ida',

    [ValidateRange(0, 1000)]
    [int]$KeepLast = 7,

    [string]$MilestoneName,

    [string]$TempDir,

    [switch]$KeepArchive
)

$ErrorActionPreference = 'Stop'

function Invoke-CommandChecked {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath,

        [Parameter(Mandatory = $true)]
        [string[]]$Arguments
    )

    $Output = & $FilePath @Arguments 2>&1
    if ($LASTEXITCODE -ne 0) {
        $Message = ($Output | Out-String).Trim()
        if ([string]::IsNullOrWhiteSpace($Message)) {
            $Message = "$FilePath exited with code $LASTEXITCODE"
        }
        throw $Message
    }

    return $Output
}

function Invoke-AzJson {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Arguments
    )

    $Output = Invoke-CommandChecked -FilePath 'az' -Arguments ($Arguments + @('--only-show-errors', '--output', 'json'))
    $Text = ($Output | Out-String).Trim()
    if ([string]::IsNullOrWhiteSpace($Text)) {
        return $null
    }

    return $Text | ConvertFrom-Json
}

function Invoke-AzTsv {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Arguments
    )

    $Output = Invoke-CommandChecked -FilePath 'az' -Arguments ($Arguments + @('--only-show-errors', '--output', 'tsv'))
    return ($Output | Out-String).Trim()
}

function Invoke-AzNone {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Arguments
    )

    Invoke-CommandChecked -FilePath 'az' -Arguments ($Arguments + @('--only-show-errors', '--output', 'none')) | Out-Null
}

function Get-GitValue {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Arguments,

        [switch]$AllowFailure
    )

    $Output = & git -C $RepoRoot @Arguments 2>$null
    if ($LASTEXITCODE -ne 0) {
        if ($AllowFailure) {
            return $null
        }

        throw "git $($Arguments -join ' ') failed"
    }

    return ($Output | Out-String).Trim()
}

function Convert-ToBlobSegment {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Value,

        [switch]$AllowSlash
    )

    $Pattern = if ($AllowSlash) { '[^0-9A-Za-z._/-]+' } else { '[^0-9A-Za-z._-]+' }
    $Sanitized = ($Value -replace $Pattern, '-').Trim('-')
    if ([string]::IsNullOrWhiteSpace($Sanitized)) {
        return 'unnamed'
    }

    return $Sanitized
}

function Resolve-StorageAccountName {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Value
    )

    $Trimmed = $Value.Trim()
    if ([string]::IsNullOrWhiteSpace($Trimmed)) {
        throw 'Storage account name cannot be empty.'
    }

    if ($Trimmed -match '^(?i)https?://') {
        try {
            $Uri = [Uri]$Trimmed
        }
        catch {
            throw "Invalid storage account URL: $Trimmed"
        }

        if ([string]::IsNullOrWhiteSpace($Uri.Host)) {
            throw "Invalid storage account URL: $Trimmed"
        }

        $Trimmed = $Uri.Host
    }

    if ($Trimmed -match '^(?<account>[a-z0-9]+)\.blob\.core\.windows\.net$') {
        return $Matches['account']
    }

    if ($Trimmed -notmatch '^[a-z0-9]{3,24}$') {
        throw "Invalid storage account name '$Value'. Pass either the raw account name or the blob endpoint URL."
    }

    return $Trimmed
}

function New-ZipArchiveFromFile {
    param(
        [Parameter(Mandatory = $true)]
        [string]$InputPath,

        [Parameter(Mandatory = $true)]
        [string]$ArchivePath
    )

    Add-Type -AssemblyName System.IO.Compression.FileSystem

    if (Test-Path $ArchivePath) {
        Remove-Item $ArchivePath -Force
    }

    $ArchiveStream = [System.IO.File]::Open($ArchivePath, [System.IO.FileMode]::CreateNew)
    try {
        $ZipArchive = [System.IO.Compression.ZipArchive]::new($ArchiveStream, [System.IO.Compression.ZipArchiveMode]::Create, $false)
        try {
            [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile(
                $ZipArchive,
                $InputPath,
                (Split-Path $InputPath -Leaf),
                [System.IO.Compression.CompressionLevel]::Fastest
            ) | Out-Null
        }
        finally {
            $ZipArchive.Dispose()
        }
    }
    finally {
        $ArchiveStream.Dispose()
    }
}

function Remove-OldAutomaticSnapshots {
    param(
        [Parameter(Mandatory = $true)]
        [string]$AutoPrefix
    )

    if ($KeepLast -lt 1) {
        return @()
    }

    $BlobList = Invoke-AzJson -Arguments @(
        'storage', 'blob', 'list',
        '--auth-mode', 'login',
        '--account-name', $AccountName,
        '--container-name', $ContainerName,
        '--prefix', $AutoPrefix,
        '--num-results', '5000'
    )

    if (-not $BlobList) {
        return @()
    }

    $Sorted = @($BlobList | Sort-Object { [DateTimeOffset]$_.properties.lastModified })
    if ($Sorted.Count -le $KeepLast) {
        return @()
    }

    $DeleteCount = $Sorted.Count - $KeepLast
    $ToDelete = $Sorted | Select-Object -First $DeleteCount
    $Deleted = @()

    foreach ($Blob in $ToDelete) {
        if ($PSCmdlet.ShouldProcess($Blob.name, 'Delete old automatic snapshot')) {
            Invoke-AzNone -Arguments @(
                'storage', 'blob', 'delete',
                '--auth-mode', 'login',
                '--account-name', $AccountName,
                '--container-name', $ContainerName,
                '--name', $Blob.name
            )

            $Deleted += $Blob.name
        }
    }

    return $Deleted
}

$RepoRoot = Split-Path -Parent $PSScriptRoot
if (-not (Test-Path (Join-Path $RepoRoot 'Cataclysm.vcxproj'))) {
    Write-Error "Cannot find Cataclysm.vcxproj at $RepoRoot"
    exit 1
}

if (-not $SourcePath) {
    $SourcePath = Join-Path $RepoRoot 'cataclysm.i64'
}

if (-not $TempDir) {
    $TempDir = Join-Path $RepoRoot 'temp\ida-snapshots'
}

try {
    $AccountName = Resolve-StorageAccountName -Value $AccountName
}
catch {
    Write-Error $_.Exception.Message
    exit 1
}

$AzCommand = Get-Command az -CommandType Application -ErrorAction SilentlyContinue
if (-not $AzCommand) {
    Write-Error 'Azure CLI was not found in PATH. Install Azure CLI and run `az login` first.'
    exit 1
}

$ExpandedSourcePath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($SourcePath)
if (-not (Test-Path $ExpandedSourcePath -PathType Leaf)) {
    Write-Error "Source file not found: $SourcePath"
    exit 1
}
$ResolvedSourcePath = (Resolve-Path $ExpandedSourcePath).ProviderPath

try {
    $null = Invoke-AzJson -Arguments @('account', 'show')
}
catch {
    Write-Error "Azure CLI is not logged in. Run `az login` first. $($_.Exception.Message)"
    exit 1
}

$ContainerExists = Invoke-AzTsv -Arguments @(
    'storage', 'container', 'exists',
    '--auth-mode', 'login',
    '--account-name', $AccountName,
    '--name', $ContainerName,
    '--query', 'exists'
)
if ($ContainerExists -ne 'true') {
    Write-Error "Container '$ContainerName' does not exist in storage account '$AccountName'."
    exit 1
}

$CommitSha = Get-GitValue -Arguments @('rev-parse', 'HEAD')
$ShortSha = Get-GitValue -Arguments @('rev-parse', '--short=12', 'HEAD')
$BranchName = Get-GitValue -Arguments @('symbolic-ref', '--quiet', '--short', 'HEAD') -AllowFailure
if (-not $BranchName) {
    $BranchName = "detached-$ShortSha"
}

$Timestamp = (Get-Date).ToUniversalTime().ToString('yyyy-MM-dd_HHmmssZ')
$MetadataTimestamp = (Get-Date).ToUniversalTime().ToString('yyyyMMddTHHmmssZ')
$BranchSegment = Convert-ToBlobSegment -Value $BranchName -AllowSlash
$SnapshotKind = if ($MilestoneName) { 'milestone' } else { 'auto' }
$LabelSuffix = if ($MilestoneName) { '-' + (Convert-ToBlobSegment -Value $MilestoneName) } else { '' }
$ArchiveName = "cataclysm-i64-$Timestamp-$ShortSha$LabelSuffix.zip"
$PrefixRoot = (Convert-ToBlobSegment -Value $Prefix -AllowSlash).TrimEnd('/')
$BlobName = if ($MilestoneName) {
    $MilestoneSegment = Convert-ToBlobSegment -Value $MilestoneName
    "$PrefixRoot/$BranchSegment/milestones/$MilestoneSegment/$ArchiveName"
}
else {
    "$PrefixRoot/$BranchSegment/auto/$ArchiveName"
}

$ArchivePath = Join-Path $TempDir $ArchiveName

Write-Host "Source:     $ResolvedSourcePath"
Write-Host "Account:    $AccountName"
Write-Host "Container:  $ContainerName"
Write-Host "Blob:       $BlobName"
Write-Host "Snapshot:   $SnapshotKind"
Write-Host "Branch:     $BranchName"
Write-Host "Commit:     $ShortSha"

$DeletedBlobs = @()

try {
    if (-not $PSCmdlet.ShouldProcess($ArchivePath, 'Create local zip archive')) {
        return
    }

    [System.IO.Directory]::CreateDirectory($TempDir) | Out-Null

    Write-Host 'Creating zip archive...' -ForegroundColor Cyan
    New-ZipArchiveFromFile -InputPath $ResolvedSourcePath -ArchivePath $ArchivePath

    $ArchiveItem = Get-Item $ArchivePath
    $SourceItem = Get-Item $ResolvedSourcePath
    $ArchiveHash = (Get-FileHash -Path $ArchivePath -Algorithm SHA256).Hash.ToLowerInvariant()

    Write-Host ("Archive:    {0} ({1:N2} MiB)" -f $ArchivePath, ($ArchiveItem.Length / 1MB))

    if ($PSCmdlet.ShouldProcess($BlobName, 'Upload IDA snapshot to Azure Blob Storage')) {
        Invoke-AzNone -Arguments @(
            'storage', 'blob', 'upload',
            '--auth-mode', 'login',
            '--account-name', $AccountName,
            '--container-name', $ContainerName,
            '--name', $BlobName,
            '--file', $ArchivePath,
            '--overwrite', 'false',
            '--metadata',
            "git_commit=$CommitSha",
            "snapshot_kind=$SnapshotKind",
            "created_utc=$MetadataTimestamp",
            "archive_sha256=$ArchiveHash",
            "source_name=$($SourceItem.Name)"
        )
    }

    if (-not $MilestoneName) {
        $AutoPrefix = "$PrefixRoot/$BranchSegment/auto/"
        $DeletedBlobs = Remove-OldAutomaticSnapshots -AutoPrefix $AutoPrefix
    }

    Write-Host 'Upload complete.' -ForegroundColor Green
    Write-Host ("SourceSize: {0:N2} MiB" -f ($SourceItem.Length / 1MB))
    Write-Host ("ZipSize:    {0:N2} MiB" -f ($ArchiveItem.Length / 1MB))
    Write-Host "SHA256:     $ArchiveHash"
    Write-Host "URL:        https://$AccountName.blob.core.windows.net/$ContainerName/$BlobName"
    if ($DeletedBlobs.Count -gt 0) {
        Write-Host "Pruned:     $($DeletedBlobs.Count) old automatic snapshot(s)"
        foreach ($Deleted in $DeletedBlobs) {
            Write-Host "  $Deleted" -ForegroundColor DarkGray
        }
    }
}
finally {
    if (-not $KeepArchive -and (Test-Path $ArchivePath)) {
        Remove-Item $ArchivePath -Force
    }
}

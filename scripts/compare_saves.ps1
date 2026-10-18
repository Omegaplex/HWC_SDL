<#
.SYNOPSIS
    Compares two Cataclysm save files by chunk stream.

.DESCRIPTION
    Parses both save files, reports overall length delta, exact type/size count
    differences, normalized base-type/size differences, and normalized ship
    chunk differences keyed by size/type/race.

.PARAMETER OldSave
    Path to the reference save file.

.PARAMETER NewSave
    Path to the comparison save file.

.EXAMPLE
    .\scripts\compare_saves.ps1 `
        -OldSave "$env:CATACLYSM_PATH\SavedGames\SinglePlayer\02 - Outskirts of Hiigaran System" `
        -NewSave "$env:CATACLYSM_PATH\SavedGames\SinglePlayer\02 - Outskirts new"
#>
[CmdletBinding(PositionalBinding = $false)]
param(
    [Parameter(Mandatory = $true)]
    [string]$OldSave,

    [Parameter(Mandatory = $true)]
    [string]$NewSave
)

$ErrorActionPreference = 'Stop'

function Parse-Save {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    $FullPath = (Resolve-Path $Path).Path
    $FileStream = [System.IO.File]::OpenRead($FullPath)
    try {
        $Reader = New-Object System.IO.BinaryReader($FileStream)
        [void]$Reader.ReadInt32()

        $Chunks = @()
        $Ordinal = 0
        while ($FileStream.Position -lt $FileStream.Length) {
            if (($FileStream.Length - $FileStream.Position) -lt 8) {
                break
            }

            $Offset = $FileStream.Position
            $Type = $Reader.ReadInt32()
            $Size = $Reader.ReadInt32()
            if ($Size -lt 0 -or ($FileStream.Position + $Size) -gt $FileStream.Length) {
                break
            }

            $Payload = $Reader.ReadBytes($Size)
            $ShipType = $null
            $ShipRace = $null
            if (([uint32]([int64]$Type -band 0xFFFFFFFFL)) -eq 0x40000010 -and $Size -ge 24) {
                $ShipType = [BitConverter]::ToUInt16($Payload, 20)
                $ShipRace = $Payload[22]
            }

            $Chunks += [pscustomobject]@{
                Ordinal  = $Ordinal
                Offset   = $Offset
                Type     = ('0x{0:X8}' -f ([uint32]([int64]$Type -band 0xFFFFFFFFL)))
                BaseType = ('0x{0:X8}' -f ([uint32]([int64]$Type -band 0x0FFFFFFFL)))
                Size     = $Size
                ShipType = $ShipType
                ShipRace = $ShipRace
            }
            $Ordinal++
        }

        return $Chunks
    }
    finally {
        $FileStream.Dispose()
    }
}

function Get-CountDiffRows {
    param(
        [Parameter(Mandatory = $true)]
        [object[]]$OldItems,
        [Parameter(Mandatory = $true)]
        [object[]]$NewItems,
        [Parameter(Mandatory = $true)]
        [scriptblock]$KeySelector,
        [Parameter(Mandatory = $true)]
        [scriptblock]$RowFactory
    )

    $Counts = @{}
    foreach ($Item in $OldItems) {
        $Key = & $KeySelector $Item
        if (-not $Counts.ContainsKey($Key)) {
            $Counts[$Key] = @{ Old = 0; New = 0; Item = $Item }
        }
        $Counts[$Key].Old++
    }

    foreach ($Item in $NewItems) {
        $Key = & $KeySelector $Item
        if (-not $Counts.ContainsKey($Key)) {
            $Counts[$Key] = @{ Old = 0; New = 0; Item = $Item }
        }
        $Counts[$Key].New++
    }

    return $Counts.GetEnumerator() | ForEach-Object {
        $Value = $_.Value
        if ($Value.Old -ne $Value.New) {
            & $RowFactory $Value.Item $Value.Old $Value.New
        }
    }
}

$OldPath = (Resolve-Path $OldSave).Path
$NewPath = (Resolve-Path $NewSave).Path

$OldChunks = Parse-Save -Path $OldPath
$NewChunks = Parse-Save -Path $NewPath

$OldLength = (Get-Item $OldPath).Length
$NewLength = (Get-Item $NewPath).Length

Write-Output ("OldSave={0}" -f $OldPath)
Write-Output ("NewSave={0}" -f $NewPath)
Write-Output ("OldLength={0}" -f $OldLength)
Write-Output ("NewLength={0}" -f $NewLength)
Write-Output ("LengthDelta={0}" -f ($NewLength - $OldLength))
Write-Output ''

Write-Output 'Exact type/size count diffs:'
$ExactRows = Get-CountDiffRows `
    -OldItems $OldChunks `
    -NewItems $NewChunks `
    -KeySelector { param($Item) "$($Item.Type):$($Item.Size)" } `
    -RowFactory {
        param($Item, $OldCount, $NewCount)
        [pscustomobject]@{
            Type  = $Item.Type
            Size  = $Item.Size
            Old   = $OldCount
            New   = $NewCount
            Delta = $NewCount - $OldCount
        }
    }
$ExactRows | Sort-Object Type, Size | Format-Table -AutoSize | Out-String | Write-Output

Write-Output 'Normalized base-type/size diffs:'
$NormalizedRows = Get-CountDiffRows `
    -OldItems $OldChunks `
    -NewItems $NewChunks `
    -KeySelector { param($Item) "$($Item.BaseType):$($Item.Size)" } `
    -RowFactory {
        param($Item, $OldCount, $NewCount)
        [pscustomobject]@{
            BaseType   = $Item.BaseType
            Size       = $Item.Size
            Old        = $OldCount
            New        = $NewCount
            Delta      = $NewCount - $OldCount
            DeltaBytes = ($NewCount - $OldCount) * ($Item.Size + 8)
        }
    }
$NormalizedRows | Sort-Object { [math]::Abs($_.DeltaBytes) } -Descending | Format-Table -AutoSize | Out-String | Write-Output

Write-Output 'Normalized ship chunk diffs by size/type/race:'
$OldShipChunks = $OldChunks | Where-Object { $_.BaseType -eq '0x00000010' -and $null -ne $_.ShipType }
$NewShipChunks = $NewChunks | Where-Object { $_.BaseType -eq '0x00000010' -and $null -ne $_.ShipType }
$ShipRows = Get-CountDiffRows `
    -OldItems $OldShipChunks `
    -NewItems $NewShipChunks `
    -KeySelector { param($Item) "$($Item.Size):$($Item.ShipType):$($Item.ShipRace)" } `
    -RowFactory {
        param($Item, $OldCount, $NewCount)
        [pscustomobject]@{
            Size     = $Item.Size
            ShipType = $Item.ShipType
            ShipRace = $Item.ShipRace
            Old      = $OldCount
            New      = $NewCount
            Delta    = $NewCount - $OldCount
        }
    }
$ShipRows | Sort-Object Size, ShipRace, ShipType | Format-Table -AutoSize | Out-String | Write-Output

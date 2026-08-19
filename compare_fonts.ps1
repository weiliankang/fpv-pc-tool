# Compare font_bf_24.png vs font_inav_24.png char blocks using LockBits
Add-Type -AssemblyName System.Drawing

function Get-CharBlocks($path, $fontW, $fontH) {
    $bmp = [System.Drawing.Bitmap]::FromFile((Resolve-Path $path))
    $W = $bmp.Width; $H = $bmp.Height
    $rect = New-Object System.Drawing.Rectangle(0, 0, $W, $H)
    $data = $bmp.LockBits($rect, [System.Drawing.Imaging.ImageLockMode]::ReadOnly, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $stride = $data.Stride
    $bytes = New-Object byte[] ($stride * $H)
    [System.Runtime.InteropServices.Marshal]::Copy($data.Scan0, $bytes, 0, $bytes.Length)
    $bmp.UnlockBits($data)

    $xNum = [int]($W / $fontW); $yNum = [int]($H / $fontH)
    $blocks = @{}
    for ($x = 0; $x -lt $xNum; $x++) {
        for ($y = 0; $y -lt $yNum; $y++) {
            $idx = $x * $yNum + $y
            if ($idx -ge 1024) { continue }
            $row = New-Object 'System.Collections.Generic.List[int32]'
            for ($cy = 0; $cy -lt $fontH; $cy++) {
                $base = ($y*$fontH + $cy) * $stride + ($x*$fontW) * 4
                for ($cx = 0; $cx -lt $fontW; $cx++) {
                    $p = $base + $cx * 4
                    $b = $bytes[$p]; $g = $bytes[$p+1]; $r = $bytes[$p+2]; $a = $bytes[$p+3]
                    if ($a -gt 128 -and ($r+$g+$b) -lt 600) {
                        $row.Add([int32]1) | Out-Null
                    } else {
                        $row.Add([int32]0) | Out-Null
                    }
                }
            }
            $blocks[$idx] = $row
        }
    }
    $bmp.Dispose()
    return @{ blocks=$blocks; xNum=$xNum; yNum=$yNum }
}

$fontW = 24; $fontH = 36
Write-Host "Reading font_bf_24.png ..." -NoNewline
$bf   = Get-CharBlocks "X:\open_project\lkwei_project\fpv-pc-tool\fonts\font_bf_24.png"  $fontW $fontH
Write-Host " done. grid $($bf.xNum)x$($bf.yNum)"
Write-Host "Reading font_inav_24.png ..." -NoNewline
$inav = Get-CharBlocks "X:\open_project\lkwei_project\fpv-pc-tool\fonts\font_inav_24.png" $fontW $fontH
Write-Host " done. grid $($inav.xNum)x$($inav.yNum)"

$diffList = @()
for ($i = 0; $i -lt 1024; $i++) {
    $b = $bf.blocks[$i];   $n = $inav.blocks[$i]
    if ($null -eq $b -or $null -eq $n) { continue }
    $diff = 0
    $cnt = [Math]::Min($b.Count, $n.Count)
    for ($k = 0; $k -lt $cnt; $k++) {
        if ($b[$k] -ne $n[$k]) { $diff++ }
    }
    if ($diff -gt 0) {
        $diffList += [pscustomobject]@{ Index=$i; DiffPixels=$diff }
    }
}

Write-Host ""
Write-Host "Chars with differences: $($diffList.Count)"
Write-Host ""
Write-Host "=== Top 50 by diff pixels (good visual check points) ==="
$diffList | Sort-Object DiffPixels -Descending | Select-Object -First 50 | ForEach-Object {
    $hex = ('{0:X2}' -f $_.Index)
    $ascii = if ($_.Index -ge 0x20 -and $_.Index -le 0x7E) { [char]$_.Index } else { '.' }
    Write-Host ("idx {0,4} (0x{1}, '{2}')  diffpx: {3}" -f $_.Index, $hex, $ascii, $_.DiffPixels)
}
Write-Host ""
Write-Host "=== Common ASCII visible range (0x20-0x7E) differences ==="
$commonDiff = $diffList | Where-Object { $_.Index -ge 0x20 -and $_.Index -le 0x7E }
Write-Host "ASCII visible chars with diff: $($commonDiff.Count)"
$commonDiff | Sort-Object DiffPixels -Descending | Select-Object -First 25 | ForEach-Object {
    $hex = ('{0:X2}' -f $_.Index)
    Write-Host ("  '{0}' (0x{1})  diff: {2} px" -f [char]$_.Index, $hex, $_.DiffPixels)
}

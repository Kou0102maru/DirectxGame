# Check battle.cpp, player.cpp, player.h, battle.h for encoding issues
$files = @('battle.cpp', 'player.cpp', 'player.h', 'battle.h', 'game.cpp')

foreach ($fname in $files) {
    $path = "C:\DirectxGame\$fname"
    $bytes = [System.IO.File]::ReadAllBytes($path)

    # Read line 3 bytes (find 2nd newline, then read until 3rd)
    $lineStarts = @(0)
    for ($i = 0; $i -lt $bytes.Length; $i++) {
        if ($bytes[$i] -eq 10) { $lineStarts += ($i + 1) }
        if ($lineStarts.Count -gt 4) { break }
    }

    Write-Host "=== $fname ==="
    # Show bytes of line 3
    if ($lineStarts.Count -gt 3) {
        $start = $lineStarts[2]
        $end = $lineStarts[3] - 1
        $hex = ""
        $ascii = ""
        for ($j = $start; $j -lt $end -and $j -lt $start + 60; $j++) {
            $hex += $bytes[$j].ToString('X2') + " "
            if ($bytes[$j] -ge 0x20 -and $bytes[$j] -le 0x7E) {
                $ascii += [char]$bytes[$j]
            } else {
                $ascii += "."
            }
        }
        Write-Host "  Line 3 hex: $hex"
        Write-Host "  Line 3 asc: $ascii"
    }

    # Count question marks (0x3F) on line 3
    $qmarks = 0
    if ($lineStarts.Count -gt 3) {
        for ($j = $lineStarts[2]; $j -lt $lineStarts[3]; $j++) {
            if ($bytes[$j] -eq 0x3F) { $qmarks++ }
        }
    }

    # Count non-ASCII
    $nonAscii = 0
    foreach ($b in $bytes) { if ($b -gt 0x7F) { $nonAscii++ } }

    Write-Host "  Question marks on line 3: $qmarks"
    Write-Host "  Total non-ASCII bytes: $nonAscii"
    Write-Host ""
}

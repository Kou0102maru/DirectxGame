Add-Type -AssemblyName System.Drawing

$dir = "C:\DirectxGame\resource\texture"

function Make-Btn($text, $file, $w, $h, $fs) {
    $bmp = New-Object System.Drawing.Bitmap($w, $h)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAliasGridFit
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
    $g.Clear([System.Drawing.Color]::Transparent)
    $font = New-Object System.Drawing.Font("Yu Gothic UI", $fs, [System.Drawing.FontStyle]::Bold)
    $sf = New-Object System.Drawing.StringFormat
    $sf.Alignment = [System.Drawing.StringAlignment]::Center
    $sf.LineAlignment = [System.Drawing.StringAlignment]::Center
    $rect = New-Object System.Drawing.RectangleF(0, 0, $w, $h)
    $g.DrawString($text, $font, [System.Drawing.Brushes]::White, $rect, $sf)
    $sf.Dispose(); $font.Dispose(); $g.Dispose()
    $bmp.Save($file, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    Write-Host "Created: $file"
}

Make-Btn "スタート"   "$dir\btn_start.png"     256 64 28
Make-Btn "操作説明"   "$dir\btn_controls.png"   256 64 28
Make-Btn "リトライ"   "$dir\btn_retry.png"      256 64 28
Make-Btn "タイトルへ" "$dir\btn_title.png"      256 64 28

# 操作説明オーバーレイ（WASD + パッド対応）
$bmp = New-Object System.Drawing.Bitmap(550, 480)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAliasGridFit
$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
$g.Clear([System.Drawing.Color]::Transparent)

$fontH = New-Object System.Drawing.Font("Yu Gothic UI", 22, [System.Drawing.FontStyle]::Bold)
$fontS = New-Object System.Drawing.Font("Yu Gothic UI", 17, [System.Drawing.FontStyle]::Bold)
$fontB = New-Object System.Drawing.Font("Yu Gothic UI", 14)
$fontF = New-Object System.Drawing.Font("Yu Gothic UI", 13)
$wb = [System.Drawing.Brushes]::White
$yb = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(255, 255, 220, 100))

$y = [float]8
$g.DrawString("[ 操作説明 ]", $fontH, $wb, [float]160, $y)

$y += 50
$g.DrawString("-- キーボード --", $fontS, $yb, [float]30, $y)
$y += 32
$g.DrawString("W/A/S/D       :  移動", $fontB, $wb, [float]30, $y)
$y += 28
$g.DrawString("Tab                :  仲間切り替え", $fontB, $wb, [float]30, $y)
$y += 28
$g.DrawString("Space            :  弾を発射（バトル中）", $fontB, $wb, [float]30, $y)
$y += 28
$g.DrawString("Enter              :  決定", $fontB, $wb, [float]30, $y)
$y += 28
$g.DrawString("M                   :  ステータス表示", $fontB, $wb, [float]30, $y)
$y += 28
$g.DrawString("Esc                 :  戻る", $fontB, $wb, [float]30, $y)

$y += 38
$g.DrawString("-- コントローラー --", $fontS, $yb, [float]30, $y)
$y += 32
$g.DrawString("左スティック / 十字キー :  移動 / 選択", $fontB, $wb, [float]30, $y)
$y += 28
$g.DrawString("Aボタン                         :  決定", $fontB, $wb, [float]30, $y)
$y += 28
$g.DrawString("Bボタン                         :  戻る", $fontB, $wb, [float]30, $y)
$y += 28
$g.DrawString("RB                                 :  仲間切り替え", $fontB, $wb, [float]30, $y)
$y += 28
$g.DrawString("RT                                 :  弾を発射", $fontB, $wb, [float]30, $y)
$y += 28
$g.DrawString("Start                              :  ステータス", $fontB, $wb, [float]30, $y)

$y += 36
$g.DrawString("[Enter / Esc / A / B] 戻る", $fontF, $wb, [float]130, $y)

$yb.Dispose(); $fontH.Dispose(); $fontS.Dispose(); $fontB.Dispose(); $fontF.Dispose(); $g.Dispose()
$bmp.Save("$dir\controls_info.png", [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
Write-Host "Created: controls_info.png"
Write-Host "All textures generated!"

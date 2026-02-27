# DirectxGame プロジェクト ルール

## 文字コード規則（必須）

本プロジェクトのソースコード（`.cpp` / `.h`）は **Shift_JIS** 形式で管理されている。

### ソースファイル編集後の必須手順

ソースファイルへのコメント追加・コード変更を行った場合、編集後に必ず以下の PowerShell コマンドで Shift_JIS へ変換すること。

```powershell
powershell -Command "
$filePath = 'C:\DirectxGame\<対象ファイル>';
$content = [System.IO.File]::ReadAllText($filePath, [System.Text.Encoding]::UTF8);
[System.IO.File]::WriteAllText($filePath, $content, [System.Text.Encoding]::GetEncoding('shift_jis'));
Write-Host 'Converted to Shift_JIS: ' + $filePath
"
```

複数ファイルを一括変換する場合:

```powershell
powershell -Command "
$files = @('file1.cpp', 'file2.h');
foreach ($f in $files) {
  $p = 'C:\DirectxGame\' + $f;
  $c = [System.IO.File]::ReadAllText($p, [System.Text.Encoding]::UTF8);
  [System.IO.File]::WriteAllText($p, $c, [System.Text.Encoding]::GetEncoding('shift_jis'));
  Write-Host 'Converted: ' + $p
}
"
```

### スラッシュコマンド

- `/sjis-convert <ファイル名>` — 指定ファイルを Shift_JIS へ変換する

## プロジェクト概要

- DirectX 11 を使用した 3D ゲームサンプル
- Visual Studio プロジェクト: `GameSample01.sln`
- 主要ソースファイル: `main.cpp`, `game.cpp`, `player.cpp` など

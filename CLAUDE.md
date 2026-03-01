# DirectxGame プロジェクト ルール

## 文字コード規則（必須）

本プロジェクトのソースコード（`.cpp` / `.h`）は **Shift_JIS** 形式で管理されている。

### ソースファイル編集時の必須手順（3ステップ）

Claude のエディタは UTF-8 で動作するため、Shift_JIS ファイルをそのまま編集すると
日本語コメントが**文字化け**する。以下の手順を**必ず**守ること。

#### ステップ 1: 編集前 — Shift_JIS → UTF-8 変換

```powershell
powershell -Command "[System.IO.File]::WriteAllText('C:\DirectxGame\<対象ファイル>', [System.IO.File]::ReadAllText('C:\DirectxGame\<対象ファイル>', [System.Text.Encoding]::GetEncoding('shift_jis')), [System.Text.Encoding]::UTF8); Write-Host 'To UTF-8: <対象ファイル>'"
```

#### ステップ 2: Edit ツールでファイルを編集

（この時点でファイルは UTF-8 なので、日本語コメントも正しく表示される）

#### ステップ 3: 編集後 — UTF-8 → Shift_JIS 変換

```powershell
powershell -Command "[System.IO.File]::WriteAllText('C:\DirectxGame\<対象ファイル>', [System.IO.File]::ReadAllText('C:\DirectxGame\<対象ファイル>', [System.Text.Encoding]::UTF8), [System.Text.Encoding]::GetEncoding('shift_jis')); Write-Host 'To SJIS: <対象ファイル>'"
```

**重要**: Bash 経由で PowerShell を実行する場合、`$` 変数は bash に消されるため使用不可。
上記の **1行形式** をファイルごとに実行すること。
複数ファイルの場合は、ファイルごとに上記コマンドを **並列実行** する。

### スラッシュコマンド

- `/sjis-to-utf8 <ファイル名>` — 編集前に Shift_JIS → UTF-8 変換
- `/sjis-convert <ファイル名>` — 編集後に UTF-8 → Shift_JIS 変換
- `/build [debug|release]` — MSBuild でプロジェクトをビルド

## プロジェクト概要

- DirectX 11 を使用した 3D ゲームサンプル
- Visual Studio プロジェクト: `GameSample01.sln`
- 主要ソースファイル: `main.cpp`, `game.cpp`, `player.cpp` など

## ビルド

MSBuild は **必ず PowerShell 経由** で実行すること（bash 直接実行は `/p:` スイッチがパス変換されて壊れる）。

```powershell
powershell -Command "& 'C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe' 'C:\DirectxGame\GameSample01.sln' /p:Configuration=Debug /p:Platform=x64 /m /nologo /v:minimal 2>&1"
```

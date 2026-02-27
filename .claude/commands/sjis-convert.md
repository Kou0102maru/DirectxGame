Convert the specified source file(s) to Shift_JIS encoding using PowerShell.

Arguments: $ARGUMENTS

## Instructions

The argument(s) are file path(s) relative to the project root (C:\DirectxGame), or absolute paths.
Multiple files can be specified, separated by spaces.

For each specified file, run the following PowerShell command via the Bash tool to convert it to Shift_JIS:

```powershell
powershell -Command "
$filePath = 'C:\DirectxGame\<FILENAME>';
$content = [System.IO.File]::ReadAllText($filePath, [System.Text.Encoding]::UTF8);
[System.IO.File]::WriteAllText($filePath, $content, [System.Text.Encoding]::GetEncoding('shift_jis'));
Write-Host 'Converted: ' + $filePath
"
```

Replace `<FILENAME>` with the actual file name or path from the arguments.

If no arguments are given, ask the user which file(s) to convert.

After conversion, report which files were successfully converted to Shift_JIS.

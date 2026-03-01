Convert the specified Shift_JIS source file(s) to UTF-8 encoding for editing.

**This must be run BEFORE using the Edit tool on any Shift_JIS source file.**

Arguments: $ARGUMENTS

## Instructions

The argument(s) are file path(s) relative to the project root (`C:\DirectxGame`), or absolute paths.
Multiple files can be specified, separated by spaces.

For **each** specified file, run a separate PowerShell command via the Bash tool.
**IMPORTANT**: Do NOT use `$` variables in the command string — bash will strip them.
Use the following single-line format per file:

```
powershell -Command "[System.IO.File]::WriteAllText('C:\DirectxGame\<FILENAME>', [System.IO.File]::ReadAllText('C:\DirectxGame\<FILENAME>', [System.Text.Encoding]::GetEncoding('shift_jis')), [System.Text.Encoding]::UTF8); Write-Host 'Converted <FILENAME> to UTF-8'"
```

Replace `<FILENAME>` with the actual file name or path from the arguments.

If multiple files are given, run the Bash commands **in parallel** (one Bash tool call per file).

If no arguments are given, ask the user which file(s) to convert.

After conversion, report which files were successfully converted to UTF-8.
Remind the user to run `/sjis-convert` after editing is complete.

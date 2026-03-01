Build the Visual Studio project `GameSample01.sln` using MSBuild.

Arguments: $ARGUMENTS

## Instructions

Build the DirectX Game project using MSBuild. The argument (optional) specifies the build configuration:
- `debug` or no argument → Debug build (default)
- `release` → Release build

### Build Command

**IMPORTANT**: Must use `powershell -Command` wrapper. Do NOT run MSBuild directly from bash (path mangling breaks `/p:` switches).

Run the following via the Bash tool:

```
powershell -Command "& 'C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe' 'C:\DirectxGame\GameSample01.sln' /p:Configuration=<Debug or Release> /p:Platform=x64 /m /nologo /v:minimal 2>&1"
```

### After Build

1. Report whether the build succeeded or failed.
2. If the build failed, show the **error messages** and suggest fixes.
3. If there are warnings, briefly summarize them.

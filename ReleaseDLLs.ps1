param (
    [string]$ReleaseDir,
    [string]$AppName
)

$ExecutableDirectory = Resolve-Path $ReleaseDir
$CompilerDirectory = Split-Path (Get-Command g++).Path

do {
    $CopiedSomething = $false
    $files = @(Get-ChildItem $ExecutableDirectory -Filter *.dll) +
             @(Get-Item (Join-Path $ExecutableDirectory "$AppName.exe"))
    foreach ($f in $files) {
        $RequiredDLLs = objdump -p $f.FullName |
                        Select-String 'DLL Name: (.*)' |
                        ForEach-Object { $_.Matches.Groups[1].Value.Trim() }
        foreach ($DLL in $RequiredDLLs) {
            $DestinationPath = Join-Path $ExecutableDirectory $DLL
            if (Test-Path $DestinationPath) {
                continue
            }
            $SourcePath = Join-Path $CompilerDirectory $DLL
            if (Test-Path $SourcePath) {
                Write-Host ("[COPY] $DLL $SourcePath -> $DestinationPath")
                Copy-Item $SourcePath $DestinationPath -Force
                $CopiedSomething = $true
            }
        }
    }
} while ($CopiedSomething)

param([string]$Gcc='C:\msys64\ucrt64\bin\gcc.exe')
$ErrorActionPreference='Stop'
Push-Location (Split-Path -Parent $PSScriptRoot)
try {
    New-Item -ItemType Directory -Path 'Tests/build' -Force | Out-Null
    & $Gcc -std=c99 -Wall -Wextra -Werror -IConfig -IControl Tests/test_robot.c Control/tracker.c Control/drive.c -o Tests/build/test_robot.exe
    if($LASTEXITCODE){throw 'Host compile failed'}
    & .\Tests\build\test_robot.exe
    if($LASTEXITCODE){throw 'Host test failed'}
    [xml]$project=Get-Content -LiteralPath 'MDK-ARM/RobotLine.uvprojx' -Raw
    foreach($f in $project.SelectNodes('//FilePath')){
        if(!(Test-Path -LiteralPath (Join-Path 'MDK-ARM' $f.InnerText))){throw "Missing project file: $($f.InnerText)"}
    }
    Write-Host 'PASS: all Keil file paths resolve'
}finally{Pop-Location}

@echo off
for %%i in ("%~dp0..\VS2013\") do set vs2013_dir=%%~si
@REM clean VS2017 dir
>nul 2>&1 dir "%~dp0..\VS2017\" &&(
    cd /d "%~dp0..\VS2017\"
    for /r %%j in (*.vcxproj,*.filters,*.sln) do (
        del "%%~fj" /F /Q >nul 2>&1
    )
)
@REM genarate VS2017 dir
cd /d "%vs2013_dir%"
call "%~dp0sed.bat"
setlocal EnableDelayedExpansion
for /r %%j in (*.vcxproj,*.filters,*.sln) do (
    set file_path=%%~fj
    set target_file_path=!file_path:\VS2013\=\VS2017\!
    @REM copy it!
    echo F|xcopy "!file_path!" "!target_file_path!" /Y /C /R /Q >nul
    @REM if *.sln, modify it.
    if \".sln\"==\"%%~xj\" (
        echo Update %%~nxj
        for %%h in ("!target_file_path!") do rd /s /q "%%~dph.vs" >nul 2>&1
        sed -i 's:# Visual Studio 2013:# Visual Studio 15:g' "!target_file_path!"
        sed -i 's:^VisualStudioVersion = .*$:VisualStudioVersion = 15.0.28307.421:g' "!target_file_path!"
    )
    
 
)
endlocal

exit /b
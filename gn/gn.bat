@echo off

call "%~dp0..\vcproj\batch-scripts\ExpandGn-build.bat"

call "%~dp0..\vcproj\batch-scripts\gn.bat"

call "%~dp0..\vcproj\batch-scripts\ninja.bat"

@REM Check the necessary tool chain.
>nul 2>&1 gn --version ||(
    echo [WARN] gn not exists.
)

>nul 2>&1 ninja --version ||(
    echo [WARN] ninja not exists.
)

>nul 2>&1 python --version ||(
    echo [WARN] python not exists.
)

>nul 2>&1 git --version ||(
    echo [WARN] git not exists.
)

>nul 2>&1 7z --help ||(
    echo [WARN] 7z not exists.
    echo [info] Run `choco install 7zip.portable` in cmd with administrator priviledge.
)

>nul 2>&1 aria2c --version ||(
    echo [WARN] aria2c not exists.
    echo [info] Run `choco install aria2` in cmd with administrator priviledge.
)

@REM 
if not \"\"==\"%1\" call "%~dp0..\vcproj\batch-scripts\gn.bat" %*

:final
exit /b

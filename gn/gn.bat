@echo off

call "%~dp0..\vcproj\batch-scripts\ExpandGn-build.bat"

call "%~dp0..\vcproj\batch-scripts\gn.bat"

call "%~dp0..\vcproj\batch-scripts\ninja.bat"

@REM Check the necessary tool chain.
>nul 2>&1 gn --version ||(
    goto [WARN] gn not exists.
)

>nul 2>&1 ninja --version ||(
    goto [WARN] ninja not exists.
)

>nul 2>&1 python --version ||(
    goto [WARN] python not exists.
)

>nul 2>&1 git --version ||(
    goto [WARN] git not exists.
)

@REM 
if not \"\"==\"%1\" call "%~dp0..\vcproj\batch-scripts\gn.bat" %*

:final
exit /b

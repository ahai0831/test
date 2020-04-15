@echo off 
:spin
>nul 2>&1 sed.exe --version &&(
    goto work
)
for %%i in ("%~dp0..\..\tools\") do set tools_dir=%%~si
set tools_sed_dir=%tools_dir%sed\
>nul 2>&1 %tools_sed_dir%sed.exe --version &&(
    set PATH=%tools_sed_dir%;%PATH%
    goto work
)

>nul 2>&1 dir "%tools_sed_dir%" ||(
    >nul 2>&1 mkdir "%tools_sed_dir%"
)
setlocal EnableDelayedExpansion
>nul 2>&1 call "%~dp0decompression.bat" "%tools_dir%sed.7z" "%tools_sed_dir%"
endlocal
set PATH=%tools_sed_dir%;%PATH%
goto spin

:work
if not \"\"==\"%1\" sed.exe %*

:final
exit /b

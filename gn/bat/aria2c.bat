@echo off 
:spin
>nul 2>&1 aria2c.exe --version &&(
    goto work
)

for %%i in ("%~dp0..\..\tools\") do set tools_dir=%%~si
set tools_aria2c_dir=%tools_dir%aria2c\
>nul 2>&1 %tools_aria2c_dir%aria2c.exe --version &&(
    set PATH=%tools_aria2c_dir%;%PATH%
    goto work
)

>nul 2>&1 dir "%tools_aria2c_dir%" ||(
    >nul 2>&1 mkdir "%tools_aria2c_dir%"
)
setlocal EnableDelayedExpansion
>nul 2>&1 call "%~dp0decompression.bat" "%tools_dir%aria2c.7z" "%tools_aria2c_dir%"
endlocal
set PATH=%tools_aria2c_dir%;%PATH%
goto spin

:work
if not \"\"==\"%1\" aria2c.exe %*

:final
exit /b

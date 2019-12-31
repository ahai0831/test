@echo off 
:spin
>nul 2>&1 ninja --version &&(
    goto work
)
for %%i in ("%~dp0..\..\gn\") do set gn_tools_dir=%%~si
set gn_tools_ninja_dir=%gn_tools_dir%ninja\

setlocal EnableDelayedExpansion
>nul 2>&1 %gn_tools_ninja_dir%ninja --version &&(
    endlocal
    call :setpath
    goto work
) ||(
    endlocal
)

>nul 2>&1 dir "%gn_tools_ninja_dir%" ||(
    >nul 2>&1 mkdir "%gn_tools_ninja_dir%"
)
setlocal EnableDelayedExpansion
call "%~dp0decompression.bat" "%gn_tools_dir%ninja-win.zip" "%gn_tools_ninja_dir%"
endlocal
call :setpath
goto spin

:work
if not \"\"==\"%1\" ninja %*

:final
exit /b

:setpath
set PATH=%gn_tools_ninja_dir%;%PATH%
exit /b

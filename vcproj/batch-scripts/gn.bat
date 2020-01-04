@echo off 
:spin
>nul 2>&1 gn --version &&(
    goto work
)
for %%i in ("%~dp0..\..\gn\") do set gn_tools_dir=%%~si
set gn_tools_gn_dir=%gn_tools_dir%gn\

setlocal EnableDelayedExpansion
>nul 2>&1 %gn_tools_gn_dir%gn --version &&(
    endlocal
    call :setpath
    goto work
) ||(
    endlocal
)

>nul 2>&1 dir "%gn_tools_gn_dir%" ||(
    >nul 2>&1 mkdir "%gn_tools_gn_dir%"
)
setlocal EnableDelayedExpansion
call "%~dp0decompression.bat" "%gn_tools_dir%gn-windows-amd64.zip" "%gn_tools_gn_dir%"
endlocal
call :setpath
goto spin

:work
if not \"\"==\"%1\" gn %*

:final
exit /b

:setpath
set PATH=%gn_tools_gn_dir%;%PATH%
exit /b

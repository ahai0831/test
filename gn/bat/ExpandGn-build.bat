@REM  2019.12.24
@REM  Joe
@REM  Expand gn-build-master.zip automatically
@echo off
setlocal ENABLEDELAYEDEXPANSION

for %%i in ("%~dp0..\..\gn\") do set gn_tools_dir=%%~si
for %%i in ("%~dp0..\..\build\") do set GnbuildDir=%%~si
for %%i in ("%~dp0..\..\") do set RootDir=%%~si

set GnbuildZip=%gn_tools_dir%gn-build-master.zip
set GnbuildTmpDir=%gn_tools_dir%gn-build-master\
set TargetDir=%gn_tools_dir%
set Rar=%~dp0decompression.bat
set Sed=%~dp0sed.bat
set num=0
set ValidNum=63

@REM Check file number in folder "%GnbuildDir%"
for /f "delims=" %%a in ('dir "%GnbuildDir%" /s /b /a-d-s-h 2^>nul ^| find /v /c ""') do (
  @REM File number in folder "%GnbuildDir%": %%a
  set num=%%a
)

if %num% GEQ %ValidNum% (
    @REM OK
) else (
    @REM Should expand "%GnbuildZip%"
    @REM Check if "%GnbuildZip%" exists.
    if exist "%GnbuildZip%" (
        @REM Start expand.
        rd "%GnbuildDir%" /s /q >nul 2>&1
        rd "%GnbuildTmpDir%" /s /q >nul 2>&1
        "%Rar%" "%GnbuildZip%" "%TargetDir%" >nul 2>&1
        @REM move "%gn_tools_dir%gn-build-master" "%RootDir%" >nul 2>&1
        @REM rename "%RootDir%gn-build-master" build >nul 2>&1
        rename "%gn_tools_dir%gn-build-master" build >nul 2>&1
        move "%gn_tools_dir%build" "%RootDir%" >nul 2>&1
        @REM delete ' /showIncludes' in "%GnbuildDir%toolchain\win\BUILD.gn"
        %Sed% -i 's: /showIncludes::g' "%GnbuildDir%toolchain\win\BUILD.gn"
    ) else (
        echo "%GnbuildZip%" not exists.
        goto final_error
    )
)

for /f "delims=" %%a in ('dir "%GnbuildDir%" /s /b /a-d-s-h 2^>nul ^| find /v /c ""') do (
    set num=%%a
)
if not %num% GEQ %ValidNum% (
    echo Expand "%GnbuildZip%" failed.
    goto final_error
) else (
    @REM "%GnbuildZip%" expand to "%TargetDir%" successfully.
)


:final_success
endlocal
exit /b

:final_error
endlocal
exit /b -1

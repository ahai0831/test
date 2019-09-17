@REM this script works with a *.csv file, then download it from source and check it.
@REM work dir is the dir of *.csv file.
@echo off
if \"\"==\"%1\" goto error

setlocal EnableDelayedExpansion
cd /d "%~dp1"
for %%a in ("%~1") do set target_csv_file=%%~fa
echo [info]Solve:%target_csv_file%
@REM analysis *.csv
for /f "usebackq skip=1 tokens=1,2,3 delims=," %%i in ("%target_csv_file%") do (
    @REM download file
    >nul 2>&1 dir "%%i" ||(
        echo [info]Download %%i ...************************************************
        echo [info]Try %%j
        call "%~dp0aria2c.bat" -o%%i -s3 -x3 -k3M -c %%j
        echo [info]************************************************
    )
    @REM download file if resume from break point
    >nul 2>&1 dir "%%i.aria2" &&(
        echo [info]Download %%i ...************************************************
        echo [info]Try %%j
        call "%~dp0aria2c.bat" -o%%i -s3 -x3 -k3M -c %%j
        echo [info]************************************************
    )
    @REM check file
    set n=1
    for /f "usebackq skip=1" %%m in (`certutil -hashfile "%%i" SHA1`) do (
        if 1==!n! (
            echo [info]File SHA1:%%m ? SHA1 in csv:%%k
            if %%m==%%k (
                echo [Valid_file]%%~fi
                goto final
            )
        )
        set /a n+=1
    )
)
:error
endlocal
exit /b 65535

:final
endlocal
exit /b

@echo off
>nul 2>&1 7z --help &&(
    @REM 本机7z可用，则直接用
    goto work
)
for %%i in ("%~dp0..\..\tools\") do set tools_dir=%%~si
set tools_7z_dir=%tools_dir%7z\

@REM 如果已有解压的7z工具且可用，则直接调用
>nul 2>&1 "%tools_7z_dir%7z" --help &&(
    >nul 2>&1 del /f /q "%tools_dir%7z.cab.lock"
    goto prework
)

@REM 如果7Z工具cab包被占用，反复自旋
:spin
>nul 2>&1 dir "%tools_dir%7z.cab.lock" &&(
    >nul 2>&1 ping 127.1 -n 2 -l 0 -i 1
    >nul 2>&1 "%tools_7z_dir%7z" --help &&(
        >nul 2>&1 del /f /q "%tools_dir%7z.cab.lock"
        goto prework
    )
    goto spin
)||(
    echo lock>"%tools_dir%7z.cab.lock"
)

@REM 检测7Z工具是否被解压，未解压，则处理
>nul 2>&1 dir "%tools_7z_dir%" ||(
    >nul 2>&1 mkdir "%tools_7z_dir%"
    >nul expand "%tools_dir%7z.cab" -f:* %tools_7z_dir% &&(
        >nul 2>&1 del /f /q "%tools_dir%7z.cab.lock"
    )||(
        @REM 延时3s即可
        >nul 2>&1 ping 127.1 -n 4 -l 0 -i 1
        @REM 无论如何3s都是已经足够，不应再持有锁
        >nul 2>&1 del /f /q "%tools_dir%7z.cab.lock"
    )
)

:prework
@REM 将7Z工具所在目录添加到环境变量中
set PATH=%tools_7z_dir%;%PATH%
@REM 检测7Z工具是否可用
>nul 2>&1 7z --help ||(
    echo 7z Command Line Tool not exists.
    goto spin
)

:work
@REM 将指定的压缩包%1，解压到指定的目录去%2
if \"\"==\"%~2\" exit /b
for %%i in ("%~1") do set zip_input_path=%%~fi
for %%i in ("%~2") do set zip_output_path=%%~fi
echo Extract %zip_input_path% to %zip_output_path%
@REM 处理覆盖选项
set overwrite_option=aos
if \"force\"==\"%3\" set overwrite_option=aoa
@REM 处理*.tar.gz
if \".tar.gz\"==\"%zip_input_path:~-7%\" (
    7z x "%zip_input_path%" -so | 7z x -si -ttar -%overwrite_option% -o"%zip_output_path%" >nul
    goto final
)
@REM 处理*.tar.xz
if \".tar.xz\"==\"%zip_input_path:~-7%\" (
    7z x "%zip_input_path%" -so | 7z x -si -ttar -%overwrite_option% -o"%zip_output_path%" >nul
    goto final
)
@REM 处理其他一般格式的压缩包
7z x "%zip_input_path%" -%overwrite_option% -o"%zip_output_path%" >nul
:final
exit /b
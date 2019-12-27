@echo off
>nul 2>&1 7z --help &&(
    @REM ����7z���ã���ֱ����
    goto work
)
for %%i in ("%~dp0..\..\tools\") do set tools_dir=%%~si
set tools_7z_dir=%tools_dir%7z\

@REM ������н�ѹ��7z�����ҿ��ã���ֱ�ӵ���
>nul 2>&1 "%tools_7z_dir%7z" --help &&(
    >nul 2>&1 del /f /q "%tools_dir%7z.cab.lock"
    goto prework
)

@REM ���7Z����cab����ռ�ã���������
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

@REM ���7Z�����Ƿ񱻽�ѹ��δ��ѹ������
>nul 2>&1 dir "%tools_7z_dir%" ||(
    >nul 2>&1 mkdir "%tools_7z_dir%"
    >nul expand "%tools_dir%7z.cab" -f:* %tools_7z_dir% &&(
        >nul 2>&1 del /f /q "%tools_dir%7z.cab.lock"
    )||(
        @REM ��ʱ3s����
        >nul 2>&1 ping 127.1 -n 4 -l 0 -i 1
        @REM �������3s�����Ѿ��㹻����Ӧ�ٳ�����
        >nul 2>&1 del /f /q "%tools_dir%7z.cab.lock"
    )
)

:prework
@REM ��7Z��������Ŀ¼��ӵ�����������
set PATH=%tools_7z_dir%;%PATH%
@REM ���7Z�����Ƿ����
>nul 2>&1 7z --help ||(
    echo 7z Command Line Tool not exists.
    goto spin
)

:work
@REM ��ָ����ѹ����%1����ѹ��ָ����Ŀ¼ȥ%2
if \"\"==\"%~2\" exit /b
for %%i in ("%~1") do set zip_input_path=%%~fi
for %%i in ("%~2") do set zip_output_path=%%~fi
echo Extract %zip_input_path% to %zip_output_path%
@REM ������ѡ��
set overwrite_option=aos
if \"force\"==\"%3\" set overwrite_option=aoa
@REM ����*.tar.gz
if \".tar.gz\"==\"%zip_input_path:~-7%\" (
    7z x "%zip_input_path%" -so | 7z x -si -ttar -%overwrite_option% -o"%zip_output_path%" >nul
    goto final
)
@REM ����*.tar.xz
if \".tar.xz\"==\"%zip_input_path:~-7%\" (
    7z x "%zip_input_path%" -so | 7z x -si -ttar -%overwrite_option% -o"%zip_output_path%" >nul
    goto final
)
@REM ��������һ���ʽ��ѹ����
7z x "%zip_input_path%" -%overwrite_option% -o"%zip_output_path%" >nul
:final
exit /b
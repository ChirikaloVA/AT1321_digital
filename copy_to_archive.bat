chcp 1251 >NUL
REM copying hex to proper folders that depends on version number. 3 releases of firmware.
@ECHO off

setlocal enabledelayedexpansion

Set paths=d:\IARARMProjects\SPRD2_5
Set /p version="Enter version number:"
Set pathd=d:\� �����\AT1321
ECHO Please check the path before copying
echo %pathd%

echo copying...

Set name="SPIRID1_SNM"
Set mainver="A"
call :archive

Set name="SPIRID1"
Set mainver="B"
call :archive

Set name="SPIRID1_BNC"
Set mainver="C"
call :archive

Set name="SPiRiD1IAEASNM"
Set mainver="I"
call :archive

Set name="SPIRID1_RF"
Set mainver="RF"
call :archive


@ECHO Please check the copy result
Set /p any="Click any key "

exit /b 0






:merge_files

set FILE1=%1
set FILE2=%2
set DESTFILE=%3

rem �������� ������������� ������
if not exist %FILE1% (
    echo READFAIL: ���� %FILE1% �����������
    exit /b 1
)
if not exist %FILE2% (
    echo READFAIL: ���� %FILE2% �����������
    exit /b 1
)

copy /b %FILE1% + %FILE2% %DESTFILE%


exit /b 0


:archive

Set pathd2=%pathd%\%name%\%version%%mainver%
Set file1=%paths%\�������� ��������� %name%.txt
Set file2=%paths%\�������� ��������� SPRD.txt
Set destfile=%pathd2%\�������� ��������� %name%.txt

echo %pathd2%
rem �����������
md "%pathd2%"
XCOPY "%paths%\output\%name%.hex" "%pathd2%\" /y /e
rem �������� ��������
call :merge_files "%file1%" "%file2%" "%destfile%"|| exit /b 1

exit /b 0
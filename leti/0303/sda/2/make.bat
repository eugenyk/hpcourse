@echo off

set "app=mingw32-g++"
set "f_app=AStyle\bin\AStyle.exe"


setlocal enabledelayedexpansion
set args="%*"

if not x%args:formatter=%==x%args% GOTO FORMATTER
goto CHECK_CLEAN
:FORMATTER
call %f_app% "%cd%/*.cpp" "%cd%/*.h"
goto CHECK_CLEAN

:CHECK_CLEAN
if [%1]==[] GOTO CLEAN
if not x%args:rebuild=%==x%args% GOTO CLEAN
if not x%args:clean=%==x%args% GOTO CLEAN
GOTO CHECK_COMPILE

:CLEAN
IF exist bin rmdir /s /q "bin"
IF exist obj rmdir /s /q "obj"
goto CHECK_COMPILE

:CHECK_COMPILE
if [%1]==[] GOTO COMPILE
if not x%args:compile=%==x%args% GOTO COMPILE
if not x%args:build=%==x%args% GOTO COMPILE
goto CHECK_LINK

:COMPILE
IF not exist obj mkdir "obj"
set "objs= "
rem add after for/R for recursive
for %%I in (*.cpp) do (
 echo COMPILE... %%~nxI
 call %app%  -Wall -std=c++11 -fexceptions -O2 -c "%%I" -o  "obj\%%~nI.o"
 set "objs=!objs!obj\%%~nI.o "
)
echo successful compilation!


:CHECK_LINK
if [%1]==[] GOTO LINK
if not x%args:link=%==x%args% GOTO LINK
if not x%args:build=%==x%args% GOTO LINK
exit /b0

:LINK
if not defined objs (
   setlocal enabledelayedexpansion
   set "objs= "
   for %%I in (*.cpp) do (
	set "objs=!objs!obj\%%~nI.o "
   )
)

IF not exist bin mkdir "bin"
echo MAKE... bin\sda_2_cpp.exe
call %app% -o bin\sda_2_cpp.exe %objs% -s

endlocal
echo successful build!

pause


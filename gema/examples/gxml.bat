@echo off

if [%1]==[/install] goto install
if not "%GXMLPATH%"=="" goto start

if not exist %windir%\gxmlpath.bat goto noinst

call %windir%\gxmlpath.bat

:start
echo GXML v0.9 (%GXMLPATH%)

if "%1"=="/l" goto list

set prog=gel

if not "%1"=="/x" goto start2

shift
set prog=%1
shift

:start2
if "%2"=="" goto usage

%prog% -p @set{s;win} -f %GXMLPATH%\xml.gel -l %GXMLPATH%\%1handler.lua %2

goto theend

::::

:install

if not exist gxml.lib\xml.gel goto noinst

%COMSPEC%/E:2000/CFOR %%x in (1 2) do PROMPT set GXMLPATH=$P\gxml.lib$_ | find /V "$" >%windir%\gxmlpath.bat

call %windir%\gxmlpath.bat
echo GXML installed in "%GXMLPATH%"

:usage
echo Usage:
echo   gxml /install         Install in the current directory
echo   gxml /l               List available xml handlers
echo   gxml /x gel.exe       Specify the full name of gel.exe
echo   gxml handler file     Parse file using handler
echo.
echo Now installed into: "%GXMLPATH%"

goto theend

:list
type %GXMLPATH%\handlers
goto theend

:noinst
echo To install gxml execute "gxml /I" from the directory where gxml.bat is.

:theend

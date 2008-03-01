:: .Title Gema build - Windows batch
:: .Author Remo Dentato

:: .: Abstract
::   A batch file to automatically build '|gema| (and friends)
:: on a Win32 system. Read '<"Building gema" buildnotes> for
:: general instruction on how to build.

::   This batch file uses two techniques largely documented on
:: the Internet: the "'|find|" trick for generating a batch file
:: and the "'|%COMSPEC|" usage for getting the directory name 
:: in a script variable.

::   This batch is not particulary environment friendly, having
:: 4096K of environment memory is advisable.

::   I do not claim to be an expert in all the compiler supported
:: by this script. If you have any better option for the switches
:: please let me know.
:: ..

@echo off

echo Gema building script
echo.

:: .% Config
:: Change following variables as appropriate

set LUAHOME=..\..\Lua-5.0.2
set LUAINCPATH=%LUAHOME%\include
set LUALIBPATH=%LUAHOME%\lib

:: Define "'|NDEBUG|" to exclude debugging code
:: Define "'|TRACE|" to enable the tracing code ("'|-trace|" option)

set CCF=-DNDEBUG -DTRACE -DMSDOS

:: Be sure this into your path
set INSTDIR=%windir%

goto service

:start

:: .% Check compiler

set A2=

:: .%% Check for lcc-win32

call k.bat "lcc-win" lcc -v
if errorlevel 1 goto nolcc

set SYS=lcc
set CC=lcc -O -c 
set LN=lcclnk -s -subsystem console -o 
set AR=lcclib /OUT:

goto savecfg

:nolcc

:: .%% Check for Digital Mars C/C++
call k.bat "--" dmc --
if errorlevel 1 goto nodmc

set SYS=dmc
set CC=dmc -Jm -w -v0 -o -w- -c -mn
set LN=dmc -Jm -w -v0 -o -w- -mn -o
set AR=lib -c 

goto savecfg

:nodmc

:: .%% Check for Microsoft Visual C++
call k.bat "cl" cl /nologo
if errorlevel 1 goto nomsvc

set SYS=msvc
set CC=cl /nologo /O2 /W3 /c
set LN=link /nologo /subsystem:console /incremental:no /out:
set AR=link -lib /nologo /out:

goto savecfg

:nomsvc

:: .%% Check for Borland C++
call k.bat "--" bcc32 --
if errorlevel 1 goto nobcc

set SYS=bcc
set CC=bcc32 -q -v- -O2 -c
set LN=bcc32 -q -v- -O2 -WC -e
set AR=tlib 
set A2=/a

goto savecfg

:nobcc

:: .%% Check for Open Watcom C
::   Watcom linker has a rather complex syntax. To ease
:: our task, an helper will be built ('|l.bat|)
:: to be used instead of the linker itself.

call k.bat "Watcom" wcc386 -?
if errorlevel 1 goto nowcc

set SYS=wcc
set CC=wcc386 -i=%INCLUDE% -w1 -e25 -zq -ox -5r -bt=nt -mf 
set LN=call %CWD%\l.bat 
set AR=wlib -q -b -n 

:: .%%% Linker helper 

@set X="%%_1%%"
find %X% <%0.bat >l.bat

goto savecfg

%_1% @set X=%1
%_1% @set F=%2
     :loop %_1%
%_1% @shift
%_1% @if "%2"=="" goto eloop
%_1% @set F=%F%,%2
%_1% @goto loop
     :eloop %_1%
%_1% @echo FIL %F%>x.lk1
%_1% @wlink name _X_ SYS nt op m op maxe=25 op q op symf @x.lk1
%_1% @if exist %X% del %X%
%_1% @copy _X_.exe %X% >nul
%_1% @del _X_.*


:nowcc

:: .%% No compiler!

echo echo No Compiler found>conf.bat
set SYS=nocc
set CC=echo 
set LN=echo 
set AR=echo 
set CCF=

:savecfg 

echo set SYS=%SYS%> conf.bat
echo set CC=%CC%>> conf.bat
echo set LN=%LN%>> conf.bat
echo set AR=%AR%>> conf.bat
echo set A2=%A2%>> conf.bat
echo set CCF=%CCF%>> conf.bat
echo set LUAHOME=%LUAHOME%>> conf.bat
echo set LUAINCPATH=%LUAINCPATH%>> conf.bat
echo set LUALIBPATH=%LUALIBPATH%>> conf.bat
echo set CWD=%CWD%>>conf.bat
echo set PLAT=%PLAT%>>conf.bat

if "%SYS%"=="nocc" goto noccx

echo Gema build configured for %SYS%

:: .{ Goto the end if only configuration was required
if "%1"=="config" goto theend
:: .}

:: .% Compile Lua
:: If (and only if) requested explicitly, call the script
:: to build lua

if NOT "%1"=="lua" goto compile
call lbuild.bat

:: Back to the gema src directory
cd %CWD%

:: .% Compile gema
:compile

echo on

%CC% %CCF% version.c
%CC% %CCF% var.c
%CC% %CCF% util.c
%CC% %CCF% reg-expr.c
%CC% %CCF% read.c
%CC% %CCF% match.c
%CC% %CCF% gema_glob.c
%CC% %CCF% gema.c
%CC% %CCF% cstream.c
%CC% %CCF% action.c

%LN%gema.exe gema.obj version.obj var.obj util.obj reg-expr.obj read.obj match.obj gema_glob.obj cstream.obj action.obj

@echo off

:: If explicitly requested to only compile gema, go to the end!
if "%1"=="gema" goto clean

:: .% Compile Gel
:: Check for lua libraries
if not exist %LUALIBPATH%\liblua.lib goto clean

set CCF=%CCF% -DLUA -I%LUAINCPATH%

set LUALIBS=%LUALIBPATH%\liblua.lib %LUALIBPATH%\liblualib.lib

if exist %LUALIBPATH%\luadll.lib set LUALIBS=%LUALIBPATH%\luadll.lib

@echo on

%CC% %CCF% version.c
%CC% %CCF% var.c
%CC% %CCF% util.c
%CC% %CCF% reg-expr.c
%CC% %CCF% read.c
%CC% %CCF% match.c
%CC% %CCF% gema_glob.c
%CC% %CCF% gel_bind.c
%CC% %CCF% cstream.c
%CC% %CCF% action.c

:: .%% Gel library
@echo off
if "%PLAT%"=="9" ctty nul
copy reg-expr.obj reg_expr.obj >nul
del libgel.lib %EF%>nul
if "%PLAT%"=="9" ctty con
echo on

%AR%libgel.lib %A2% version.obj var.obj util.obj reg_expr.obj read.obj match.obj gel_bind.obj gema_glob.obj cstream.obj action.obj 

:: .%% Gua
%CC% %CCF% gua.c
%LN%gua.exe gua.obj libgel.lib %LUALIBS%

:: .%% Gel
%CC% %CCF% gema.c
%LN%gel.exe gema.obj libgel.lib %LUALIBS%

@echo off
goto clean

:: .% Cleanup
:: Entry point for the full cleanup!

:klean
if "%PLAT%"=="9" ctty nul
cd %CWD%
del ..\test\tf???.* %EF%>nul
del ..\test\test*.out %EF%>nul
del conf.bat %EF%>nul
del libgel.lib %EF%>nul
del gema.exe %EF%>nul
del gel.exe %EF%>nul
del gua.exe %EF%>nul

:: After compilation cleanup

:clean
if "%PLAT%"=="9" ctty nul
cd %CWD%
del *.obj %EF%>nul
del *.x  %EF%>nul
del *.tds %EF%>nul
del *.map %EF%>nul
del *.err %EF%>nul
del ?.* %EF%>nul
if "%PLAT%"=="9" ctty con

echo Cleaned up

:: .% Testing
:test 
if "%1" == "-test" goto theend

if not exist .\gema.exe goto theend
echo Gema successfully compiled. Testing...

cd ..\test
call .\gematest.bat ..\src\gema.exe
cd %CWD%

if not exist .\gel.exe goto theend
echo Gel successfully compiled. Testing...

cd ..\test
set X=%GXMLPATH%
set GXMLPATH=..\examples\gxml.lib
call ..\examples\gxml /x ..\src\gel tsuite gel_testsuite.xml
if exist .\tfgel.bat call tfgel.bat

cd %CWD%

set GXMLPATH=%X%
goto theend

:: .% Service routines

:service

:: .%% No extension allowed
if not "%0"=="gbuild" goto usage

:: .%% Install

if not "%1"=="install" goto noinst

echo on
@if exist gema.exe copy gema.exe %INSTDIR%\gema.exe >nul
@if exist gel.exe copy gel.exe %INSTDIR%\gel.exe >nul
@if exist gua.exe copy gua.exe %INSTDIR%\gua.exe >nul
@echo off

set X=Gema installed in %INSTDIR%
if not exist gema.exe set X=No exe found. Do a gbuild first!
echo %X%
goto theend

:noinst

:: .%% Uninstall

if not "%1"=="uninstall" goto nouninst

@if exist %INSTDIR%\gema.exe del %INSTDIR%\gema.exe >nul
@if exist %INSTDIR%\gel.exe del %INSTDIR%\gel.exe >nul
@if exist %INSTDIR%\gua.exe del %INSTDIR%\gua.exe >nul

echo Gema uninstalled
goto theend

:nouninst

:: .%% Get variables
:: Store current directory in "|CWD|"

%COMSPEC%/E:2000/CFOR %%x in (1 2) do PROMPT set CWD=$P$_ | find /V "$" >x.bat
call x.bat

:: .%% Test only
if "%1" == "test" goto test

:: .%% Usage

echo [][all][config][gema][clean][lua][-test] >cc.x
find "[%1]" <cc.x >nul
if not errorlevel 1 goto serv1

:usage
echo Usage:
echo   gbuild [all]       Build gema, gel and gua. Perform tests at the end.
echo   gbuild gema        Build only gema.
echo   gbuild lua         Recompile also lua libraries (if source found).
echo   gbuild config      Only do the configuration step.
echo   gbuild clean       Delete all the temporary files.
echo   gbuild test        Only perform testsuites
echo   gbuild -test       Do not perform testsuites
echo   gbuild install     Copy gema, gel and gua into "%INSTDIR%".
echo   gbuild uninstall   Delete gema, gel and gua from "%INSTDIR%".
echo.

goto theend

:serv1

:: .%% Check Win version
:: Assume 9x
set PLAT=9
set EF=

if "%OS%"=="" goto verok
set PLAT=N
set EF=2

:verok

:: .%% Do a cleanup!
if "%1"=="clean" goto klean

:: .%% A script to check if compiler exists
::   The actual script lines are below, "find" will extract the right
:: ones thanks to the string before each one of them.

find "_%PLAT%_" <%0.bat >k.bat

:: .{ Services done
goto start
:: .}

:: .%%% Source code
::   This script will be stored in the "|k.bat|" batch file.
::   It executes its arguments redirecting error message to nul and
:: standard output to cc.x

%_9_%    ctty nul
%_9_N_%  echo. >cc.x
%_N_%    %2 %3 %4 %5 >cc.x 2>nul
%_9_%    %2 %3 %4 %5 >cc.x
%_9_%    ctty con
%_9_N_%  find %1 cc.x >nul

:: .}

:: .%% No compiler!
:noccx
echo No compiler found

:: .%% That's all folks

:theend

:: .{ Clean variables
set SYS=
set CC=
set LN=
set AR=
set A2=
set CCF=
set LUAHOME=
set LUAINCPATH=
set LUALIBPATH=
set CWD=
set PLAT=
set X=
set EF=
:: .}
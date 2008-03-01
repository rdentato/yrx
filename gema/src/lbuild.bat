:: .T Lua build
:: .I lbuild
:: .a
::   This batch file follows the same pattern as
:: the build shell script that is in the standard
:: Lua 5.0 distribution.
:: ..


@echo off

:: .% Configure for your compiler
if exist conf.bat goto conf

call gbuild config

:conf

call conf.bat

if "%SYS%"=="nocc" goto done

cd %LUAHOME%

if errorlevel 1 goto done
if not exist include\lua.h goto done

if not exist lib mkdir lib
if not exist bin mkdir bin

:: .% Build core library

echo Building Lua core library... 
cd src

:: .{ Compile source
echo on
for %%f in (*.c) do %CC% -I..\include %%f
@echo off
:: .}

:: .{ Generate list of obj files
echo set X=> x.bat
for %%f in (*.obj) do echo set X=%%X%% %%f>>x.bat
call x.bat
:: .}

:: .{ Create library
if exist ..\lib\liblua.lib del ..\lib\liblua.lib
echo on
%AR%..\lib\liblua.lib %A2% %X%
@echo off
:: .}

:: .{ Cleanup
del *.obj
del x.bat
:: .}

:: .% Build standard library

echo Building standard library...
cd lib

:: .{ Compile source
echo on
@for %%f in (*.c) do %CC% -I..\..\include %%f
@echo off
:: .}

:: .{ Generate list of obj files
echo set X=> x.bat
for %%f in (*.obj) do echo set X=%%X%% %%f>>x.bat
call x.bat
:: .}

:: .{ Create library
if exist ..\..\lib\liblualib.lib del ..\..\lib\liblualib.lib
echo on
%AR%..\..\lib\liblualib.lib %A2% %X%
@echo off
:: .}

:: .{ Cleanup
del *.obj
del x.bat
:: .}

:: .% Build Lua interpreter
echo Building Lua...
cd ..\lua

:: .{ Generate executable
echo on
@for %%f in (*.c) do %CC% -I..\..\include %%f
@echo off

:: .{ Generate list of obj files
echo set X=> x.bat
for %%f in (*.obj) do echo set X=%%X%% %%f>>x.bat
call x.bat
:: .}

:: .{ Link
echo on
%LN%..\..\bin\lua.exe %X% ..\..\lib\liblua.lib ..\..\lib\liblualib.lib
@echo off
:: .}
:: .}

:: .{ Cleanup
del *.obj
:: .}

:: .% Build Lua bytecode compiler
echo Building luac... '
cd ..\luac

:: .{ Generate executable
echo on
@for %%f in (*.c) do %CC% -I.. -I..\..\include %%f
%CC% -DLUA_OPNAMES -I.. -I..\..\include ..\lopcodes.c
@echo off

:: .{ Generate list of obj files
echo set X=> x.bat
for %%f in (*.obj) do echo set X=%%X%% %%f>>x.bat
call x.bat
:: .}

:: .{ Link
echo on
%LN%..\..\bin\luac.exe %X% ..\..\lib\liblua.lib ..\..\lib\liblualib.lib
@echo off
:: .}

echo done
:: .}

cd ..\..
bin\lua test\hello.lua

:done

@echo off 

REM  "gema" test script for MS-DOS or Windows
REM   $Id$

set program=..\src\gema.exe

if NOT "%1"=="" set program=%1
del test*.out %EF%>nul

%program% -version -f testpat.dat testin.dat test.out
fc testout.dat test.out | find "FC:"
%program% -f testpat.dat -i -idchars "-_" -out test2.out test2.dat
fc test2out.dat test2.out | find "FC:"
%program% -f testpat.dat -filechars ".,:/\\-_" -out test3.out -in - < test3.dat
fc test3out.dat test3.out | find "FC:"
%program% -t -f testtok.dat testin.dat test4.out
fc testout.dat test4.out | find "FC:"
%program% -T -W -MATCH -F TESTTW.DAT -ODIR %TEMP% -OTYP .OUT TEST5IN.DAT
fc test5out.dat %TEMP%\test5in.out | find "FC:"
rem  test 6 is for new features and bug fixes since version 1.3
%program% -ml -f test6pat.dat  test6in.dat > test6.out
fc test6out.dat  test6.out | find "FC:"

echo The test passes if no errors or file comparison differences are shown above.

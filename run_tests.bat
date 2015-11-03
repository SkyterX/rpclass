@echo off
for /f %%f in ('dir /b build.win\bin\Release\*-test.exe') do (
	echo running %%f
	call .\build.win\bin\Release\%%f .\test\data\
)


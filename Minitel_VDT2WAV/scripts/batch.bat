rem @echo off
REM Batch convert vdt files in a folder to wav.

del  OUT.WAV

echo Folder : %1

for /f "delims=|" /f %%f in ('dir /b "%1\*.vdt"') do ..\build\vdt2wav -vdt:"%1\%%f" -wave:OUT.WAV

@call clean
@sdcc -c demo_minitel.c --xram-loc 0x0000 --xram-size 0x10000 --opt-code-speed
@sdcc -c troisd_func.c --xram-loc 0x0000 --xram-size 0x10000 --opt-code-speed
@sdcc -c deuxd_func.c --xram-loc 0x0000 --xram-size 0x10000 --opt-code-speed
@sdcc -c minitel_hw.c --xram-loc 0x0000 --xram-size 0x10000 --opt-code-speed
@sdcc demo_minitel.rel troisd_func.rel deuxd_func.rel minitel_hw.rel --xram-loc 0x0000 --xram-size 0x10000 --opt-code-speed
rem hex2bin demo_minitel.ihx
rem copy demo_minitel.bin  F:\msys64\home\User\mame\roms\minitel2\demo_minitel.bin
@IF NOT "%ERRORLEVEL%" == "0" goto errorend
rem copy demo_minitel.ihx Y:\projets\projets\Minitel2\test_demo
loadice
goto noerror
:errorend
@pause
:noerror

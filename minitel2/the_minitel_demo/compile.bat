@call clean

@echo Select the target Minitel model:
@echo 1. NFZ 330 (RTIC Minitel 1B)
@echo 2. NFZ 400 (Philips Minitel 2)
@set SEL=
@set MODEL=
@set /P SEL=Target model (1 or 2)?
@IF "%SEL%" == "1" set MODEL=NFZ330
@IF "%SEL%" == "2" set MODEL=NFZ400
@IF NOT "%MODEL%" == "" goto compile
@echo Invalid selection
goto errorend

:compile
@sdcc -c demo_minitel.c --xram-loc 0x0000 --xram-size 0x10000 --opt-code-speed -DMINITEL_%MODEL%
@sdcc -c troisd_func.c --xram-loc 0x0000 --xram-size 0x10000 --opt-code-speed -DMINITEL_%MODEL%
@sdcc -c deuxd_func.c --xram-loc 0x0000 --xram-size 0x10000 --opt-code-speed -DMINITEL_%MODEL%
@sdcc -c minitel_hw.c --xram-loc 0x0000 --xram-size 0x10000 --opt-code-speed -DMINITEL_%MODEL%
@sdcc demo_minitel.rel troisd_func.rel deuxd_func.rel minitel_hw.rel --xram-loc 0x0000 --xram-size 0x10000 --opt-code-speed -DMINITEL_%MODEL%
rem hex2bin demo_minitel.ihx
rem copy demo_minitel.bin  F:\msys64\home\User\mame\roms\minitel2\demo_minitel.bin
@IF NOT "%ERRORLEVEL%" == "0" goto errorend
rem copy demo_minitel.ihx Y:\projets\projets\Minitel2\test_demo
loadice
goto noerror
:errorend
@pause
:noerror

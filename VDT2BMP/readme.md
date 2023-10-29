VDT2BMP
(C) 2022-2023 Jean-François DEL NERO

- Videotex file (*.vdt) to bmp file converter.
- Videotex file (*.vdt) to wav file converter. (V.23 modulation)
- Videotex file (*.vdt) to movie converter. (with ffmpeg)
- Minitel server through a sound card (Software V.23 modulation/demodulation)
- Minitel emulation through a sound card (Software V.23 modulation/demodulation) (work in progress)

Usage examples :

vdt to bmp conversion :

vdt2bmp -bmp:OUT.BMP a_vdt_file.vdt

Minitel server :

vdt2bmp -server:vdt/script.txt -outfile:inscriptions.csv

Available command line options :
```c
	-bmp[:out_file.bmp]       : Generate bmp file(s)
	-ani                      : Generate animation (Simulate Minitel page loading.)
	-server:[script path]     : Server mode
	-disable_window           : Disable videotex window
	-mic                      : Use the Microphone/Input line instead of files
	-audio_list               : List the available audio input(s)/output(s)
	-audio_in/audio_out:[id]  : Select audio input/output to use
	-stdout                   : stdout mode (for piped ffmpeg compression)
```

[![Watch the video](http://hxc2001.free.fr/minitel/pages_minitel/minitel_etam.png)](http://hxc2001.free.fr/minitel/pages_minitel/minitel_etam.mp4)


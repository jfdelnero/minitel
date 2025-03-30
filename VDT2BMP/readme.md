# VDT2BMP
(C) 2022-2025 Jean-François DEL NERO

* Videotex file (*.vdt) to bmp file converter.
* Videotex file (*.vdt) to wav file converter. (V.23 modulation)
* Videotex file (*.vdt) to movie converter. (with ffmpeg)
* Minitel server through a sound card (Software V.23 modulation/demodulation)
* Minitel emulation through a sound card (Software V.23 modulation/demodulation) (work in progress)
* Websocket to Minitel bridge (to connect the Minitel to MiniPavi)

## Build

Linux, no SDL support (No display/sound capability)

```c
   make clean
   make
```

Linux, with SDL support (Full capability)

```c
   make clean
   make SDL_SUPPORT=1
```

Linux, target Windows (with mingw32), with SDL support (Full capability)

```c
   make clean
   make TARGET=mingw32 SDL_SUPPORT=1
```

## Available command line options

```c
	-bmp:[out_file.bmp]           : Generate bmp file(s)
	-ani                          : Generate animation (Simulate Minitel page loading)
	-server:[script path]         : Server mode
	-ws:[websocket server url]    : Websocket client/bridge mode (default url : MiniPavi)
	-disable_window               : Disable videotex window
	-mic                          : Use the Microphone/Input line instead of files
	-audio_list                   : List the available audio input(s)/output(s)
	-audio_in/audio_out:[id]      : Select audio input/output to use
	-show:[file.vdt]              : Display a vdt file
	-greyscale                    : Greyscale display mode
	-zoom:[1-8]                   : Set display/window zoom
	-stdout                       : stdout mode
	-help                         : This help
```

## Usage examples

	Animation                     : vdt2bmp -ani -fps:30 -stdout /path/*.vdt | ffmpeg -y -f rawvideo -pix_fmt argb -s 320x250 -r 30 -i - -an out_video.mkv
	Video + audio merging         : ffmpeg -i out_video.mkv -i out_audio.wav -c copy output.mkv

	VDT to BMP conversion         : vdt2bmp -bmp /path/*.vdt
	VDT to BMP conversion         : vdt2bmp -bmp:out.bmp /path/videotex.vdt

	Minitel server                : vdt2bmp -server:vdt/script.txt -audio_out:1 -audio_in:1 -outfile:inscriptions.csv

	Bridge to MiniPavi            : vdt2bmp -ws -zoom:2
	Bridge to another server      : vdt2bmp -ws:ws://3615co.de/ws -zoom:2


[![Watch the video](http://hxc2001.free.fr/minitel/pages_minitel/minitel_etam.png)](http://hxc2001.free.fr/minitel/pages_minitel/minitel_etam.mp4)


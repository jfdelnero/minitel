VidéoTex file (*.vdt) to wav file converter
(C) 2022 Jean-François DEL NERO

Usage Example : vdt2wav -vdt:a_vdt_file.vdt -wave:OUT.WAV

The default configuration target the V23 1200 Bauds French Minitel Terminal

Available command line options :
    -vdt:file                 : vdt file
    -wave:file                : wave file

    -samplerate:[RATE]        : wave sample rate (default 11025Hz)
    -volume:[volume]          : wave volume (0-100 default 80)

    -bauds:[bauds]            : Baud rate (default 1200 Bauds)
    -zero_freq:[Hz]           : '0' bits frequency (default 2100 Hz)
    -one_freq:[Hz]            : '1' bits frequency (default 1300 Hz)
    -idle_freq:[Hz]           : Idle/carrier frequency (default 1300 Hz)

    -ser_msbfirst:[0/1]       : MSB first (default 0)
    -ser_nbstart:[nb bits]    : Start bit duration (bits) (default 1)
    -ser_nbits:[nb bits]      : Number of bits (default 7)
    -ser_parity:[0/1/2]       : Parity (0:None, 1: Even, 2: Odd) (default : 1 - Even)
    -ser_nbstop:[nb bits]     : Stop bit duration (bits) (default 1)
    -ser_preidle:[nb bits]    : Start pause (bits) (default 0)
    -ser_postidle:[nb bits]   : Stop pause (bits) (default 0)

    -initial_start_delay:[ms] : Initial start pause (ms) (default 4000)
    -page_start_delay:[ms]    : Page start pause (ms) (default 1000)
    -page_stop_delay:[ms]     : Page stop pause (ms) (default 1000)

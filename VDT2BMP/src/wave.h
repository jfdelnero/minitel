/*
//
// Copyright (C) 2022-2023 Jean-François DEL NERO
//
// This file is part of vdt2bmp.
//
// vdt2bmp may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// vdt2bmp is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// vdt2bmp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with vdt2bmp; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/

#pragma pack(1)

typedef struct wav_hdr_ //
{
	char                     RIFF[4];        // RIFF Header
	int                      ChunkSize;      // RIFF Chunk Size
	char                     WAVE[4];        // WAVE Header
	char                     fmt[4];         // FMT header
	int                      Subchunk1Size;  // Size of the fmt chunk
	short int                AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
	short int                NumOfChan;      // Number of channels 1=Mono 2=Stereo
	int                      SamplesPerSec;  // Sampling Frequency in Hz
	int                      bytesPerSec;    // bytes per second */
	short int                blockAlign;     // 2=16-bit mono, 4=16-bit stereo
	short int                bitsPerSample;  // Number of bits per sample
	char                     Subchunk2ID[4]; // "data"  string
	int                      Subchunk2Size;  // Sampled data length
}wav_hdr;

#pragma pack()


#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include <memory.h>

//#include "fs_codes.h"


short bitrates[] = { 32,  40,  48,  56,  64,  80,  96, 112, 128, 160,
192, 224, 256, 320, 384, 448, 512, 576, 640 };

short fsize44[] = {  70,   88,  105,  122,  140,  175,  209,  244,  279,  349,
418,  488,  558,  697,  836,  976, 1115, 1254, 1394 };

unsigned long DecFS(char sratecode, unsigned long fsizecode)
{
	switch (sratecode)
	{
		case 0:
			return (bitrates[fsizecode / 2] * 2);
		case 1:
			return ((fsizecode % 2) ? fsize44[fsizecode / 2] : fsize44[fsizecode / 2] - 1);
		case 2:
			return (bitrates[fsizecode / 2] * 3);
		default:
			return 0;
	}

	return 0;
}




struct WAVEHEADER {
    unsigned long  ChunkID;
    unsigned long  ChunkSize;
    unsigned long  Format;
    unsigned long  SubChunk1ID;
    unsigned long  SubChunk1Size;
    unsigned short AudioFormat;
    unsigned short NumChannels;
    unsigned long  SampleRate;
    unsigned long  ByteRate;
    unsigned short BlockAlign;
    unsigned short BitsPerSample;
    unsigned long  SubChunk2ID;
    unsigned long  SubChunk2Size;
};


struct AC3PREAMBLE {
	unsigned short sync1;
	unsigned short sync2;
	unsigned char burst_infoLSB;
	unsigned char burst_infoMSB;
	unsigned short lengthcode;
};

char burst[6144];

FILE *infile, *outfile;

unsigned long bytesread;
unsigned long payloadbytes;
char sampleratecode;
unsigned long framesizecode;

struct AC3PREAMBLE ac3p = { 0xF872, 0x4E1F, 1, 0, 0x3800 };
struct WAVEHEADER wavhdr = { 0x46464952,
							0,
							0x45564157,
							0x20746D66,
							16,
							1,
							2,
							0,
							0,
							4,
							16,
							0x61746164,
							0 };

unsigned long i;
char temp;

int main( int argc, char *argv[ ], char *envp[ ] )
{
	if (argc < 3)
	{
		printf("Wrong syntax. AC3PACK <In.ac3> <Out.wav>.\n");
		return 0;
	}

	infile = fopen(argv[1], "rb");
	outfile = fopen(argv[2], "wb");
	//fseek(outfile, SEEK_SET, 44);
	fwrite (&wavhdr, sizeof(struct WAVEHEADER), 1, outfile);



	for(;;)
	{
		memset (burst, 0, 6144);
		bytesread = fread(&burst[8], 1, 6, infile);
		if (bytesread < 6)
		{
			printf ("EOF reached (Frame Header reading)!\nCurrent position in INFILE: %i\n", ftell(infile));
			break;
		}
		if ((burst[8] != 0x0B) || (burst[9] != 0x77))
		{
			printf("ERROR: INVALID SYNCWORD !\nCurrent position in INFILE: %i\n", ftell(infile));
			break;
		}
		framesizecode = (burst[12] & 63);
		sampleratecode = ((burst[12] & 192) / 64);

		if (wavhdr.SampleRate == 0)
		{
				printf ("First Sampleratecode: %i\n", sampleratecode);

				switch (sampleratecode)
				{
				case 0:
					wavhdr.SampleRate = 48000;
					wavhdr.ByteRate = 192000;
					break;
				case 1:
					wavhdr.SampleRate = 44100;
					wavhdr.ByteRate = 176000;
					break;
				case 2:
					wavhdr.SampleRate = 32000;
					wavhdr.ByteRate = 128000;
					break;
				default:
					wavhdr.SampleRate = 48000;
					wavhdr.ByteRate = 192000;
				}
		}

		payloadbytes = DecFS (sampleratecode, framesizecode);
		payloadbytes *= 2;

		bytesread = fread (&burst[14], 1, payloadbytes - 6, infile);
		if ((bytesread + 6) < payloadbytes)
		{
			printf ("EOF reached (Burst Reading)!\nCurrent position in INFILE: %i\n", ftell(infile));
			printf ("Frame size: %i  .. Bytes read: %i\n", payloadbytes, bytesread);
			break;
		}
		ac3p.burst_infoMSB = (burst[13] & 7);
		ac3p.lengthcode = (short)(payloadbytes * 8);

		for (i = 8; i < (payloadbytes + 8); i += 2)
		{
			temp = burst[i];
			burst[i] = burst[i + 1];
			burst[i + 1] = temp;
		}

		memcpy (burst, &ac3p, sizeof(struct AC3PREAMBLE));

		fwrite (burst, 1, 6144, outfile);

	}

	printf ("Last Sampleratecode: %i\n", sampleratecode);

	wavhdr.SubChunk2Size = (ftell(outfile) - 44);
	wavhdr.ChunkSize = wavhdr.SubChunk2Size + 36;

	fseek (outfile, SEEK_SET, 0);
	fwrite (&wavhdr, sizeof(struct WAVEHEADER), 1, outfile);

	fclose (infile);
	fclose (outfile);

	return 0;
}

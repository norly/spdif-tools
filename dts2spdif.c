
#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include <memory.h>

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


unsigned char burst[6144];

FILE *infile, *outfile;

unsigned long bytesread;
unsigned long pcmbytes;
unsigned long pcmbytesmultiplier;
char sampleratecode;
unsigned long framesize;

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
		printf("Wrong syntax. dts2spdif <In.dts> <Out.wav>.\n");
		return 0;
	}

	infile = fopen(argv[1], "rb");
	outfile = fopen(argv[2], "wb");
	//fseek(outfile, SEEK_SET, 44);
	fwrite (&wavhdr, sizeof(struct WAVEHEADER), 1, outfile);


	for(;;)
	{
		memset (burst, 0, 6144);
		bytesread = fread(burst, 1, 9, infile);
		if (bytesread < 9)
		{
			printf ("EOF reached (Frame Header reading)!\nCurrent position in INFILE: %i\n", ftell(infile));
			break;
		}
		if (((unsigned long*)burst)[0] != 0x0180fe7f)
		{
			printf("ERROR: INVALID SYNCWORD !\nCurrent position in INFILE: %i\n", ftell(infile));
			break;
		}
		framesize = (burst[5] & 3);
		framesize = framesize * 4096;
		framesize = framesize + (burst[6] * 16);
		framesize = framesize + ((burst[7] & 240) / 16);
		framesize = framesize + 1;

		sampleratecode = ((burst[8] & 60) / 4);

		if (wavhdr.SampleRate == 0)
		{
				printf ("First Sampleratecode: %i\n", sampleratecode);

				switch (sampleratecode)
				{
				case 1:
					pcmbytesmultiplier = 4;
				case 2:
					pcmbytesmultiplier = 2;
				case 3:
					pcmbytesmultiplier = 1;
					wavhdr.SampleRate = 32000;
					wavhdr.ByteRate = 128000;
					break;
				case 6:
					pcmbytesmultiplier = 4;
				case 7:
					pcmbytesmultiplier = 2;
				case 8:
					pcmbytesmultiplier = 1;
					wavhdr.SampleRate = 44100;
					wavhdr.ByteRate = 176000;
					break;
				case 11:
					pcmbytesmultiplier = 4;
				case 12:
					pcmbytesmultiplier = 2;
				case 13:
					pcmbytesmultiplier = 1;
					wavhdr.SampleRate = 48000;
					wavhdr.ByteRate = 192000;
					break;
				default:
					wavhdr.SampleRate = 0;
					wavhdr.ByteRate = 0;
					printf ("Invalid Sampleratecode ! Aborting process...\n");
					break;
				}
		}

		pcmbytes = (burst[4] & 1);
		pcmbytes = pcmbytes * 64;
		pcmbytes = pcmbytes + ((burst[5] & 252) / 4);
		pcmbytes = pcmbytes + 1;
		pcmbytes = pcmbytes * 128;
		pcmbytes = pcmbytes * pcmbytesmultiplier;


		bytesread = fread (&burst[9], 1, framesize - 9, infile);
		if ((bytesread + 9) < framesize)
		{
			printf ("EOF reached (Burst Reading)!\nCurrent position in INFILE: %i\n", ftell(infile));
			printf ("Frame size: %i  .. Bytes read: %i\n", framesize, bytesread);
			break;
		}

		for (i = 0; i < framesize; i += 2)
		{
			temp = burst[i];
			burst[i] = burst[i + 1];
			burst[i + 1] = temp;
		}


		fwrite (burst, 1, framesize, outfile);
		if (pcmbytes > framesize)
		{
			fwrite ("", 1, pcmbytes - framesize, outfile);
		}
		else
		{
			printf("Warning: Frame Size > LPCM Frame Size! Buffer underrun may occur/File damaged?\n");
			printf("Frame start at INFILE Offset: %i\n", ftell(infile) - framesize);
		}
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define N 1024
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888

typedef struct
{
	double real;
	double imag;
} Complex;

typedef struct
{
	char riff[4];
	int overall_size;
	char wave[4];
	char fmt_chunk_marker[4];
	int length_of_fmt;
	short format_type;
	short channels;
	int sample_rate;
	int byterate;
	short block_align;
	short bits_per_sample;
	char data_chunk_header[4];
	int data_size;
} WavHeader;

void chiffrer_frequences(Complex *X, unsigned int size)
{
	// Pour l'instant, cette fonction ne fait rien.
}

// Bit-Reversal & FFT Cooley-Tukey Itérative
unsigned int reverseBits(unsigned int num, unsigned int log2n)
{
	unsigned int reverseNum = 0;
	for (unsigned int i = 0; i < log2n; i++)
	{
		if ((num & (1 << i)))
			reverseNum |= (1 << ((log2n - 1) - i));
	}
	return reverseNum;
}

void fft_cooley_tukey(Complex *X, unsigned int size)
{
	unsigned int log2n = (unsigned int)log2(size);
	for (unsigned int i = 0; i < size; i++)
	{
		unsigned int j = reverseBits(i, log2n);
		if (i < j)
		{
			Complex temp = X[i];
			X[i] = X[j];
			X[j] = temp;
		}
	}
	for (unsigned int len = 2; len <= size; len <<= 1)
	{
		double angle = -2.0 * M_PI / len;
		Complex wlen = {cos(angle), sin(angle)};
		for (unsigned int i = 0; i < size; i += len)
		{
			Complex w = {1.0, 0.0};
			for (unsigned int j = 0; j < len / 2; j++)
			{
				Complex u = X[i + j];
				Complex v = {
					X[i + j + len / 2].real * w.real - X[i + j + len / 2].imag * w.imag,
					X[i + j + len / 2].real * w.imag + X[i + j + len / 2].imag * w.real};
				X[i + j].real = u.real + v.real;
				X[i + j].imag = u.imag + v.imag;
				X[i + j + len / 2].real = u.real - v.real;
				X[i + j + len / 2].imag = u.imag - v.imag;
				Complex next_w = {w.real * wlen.real - w.imag * wlen.imag, w.real * wlen.imag + w.imag * wlen.real};
				w = next_w;
			}
		}
	}
}

int main()
{
	FILE *infile = fopen("input.wav", "rb");
	if (!infile)
	{
		perror("Erreur : Impossible d'ouvrir input.wav. Place-le dans le même répertoire");
		return 1;
	}

	WavHeader header;
	fread(&header, sizeof(WavHeader), 1, infile);

	if (header.channels != 1 || header.bits_per_sample != 16)
	{
		printf("Erreur format : Le fichier 'input.wav' doit obligatoirement être MONO et 16-BITS PCM.\n");
		fclose(infile);
		return 1;
	}

	int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	short int audioBuffer[N];
	Complex fftBuffer[N];

	printf("[CLIENT] Analyse fréquentielle et streaming réseau démarrés...\n");

	while (fread(audioBuffer, sizeof(short int), N, infile) == N)
	{
		for (int i = 0; i < N; i++)
		{
			fftBuffer[i].real = (double)audioBuffer[i];
			fftBuffer[i].imag = 0.0;
		}

		// 1. Passage dans le domaine fréquentiel
		fft_cooley_tukey(fftBuffer, N);

		// 2. Application de la brique de chiffrement
		chiffrer_frequences(fftBuffer, N);

		// 3. Envoi du bloc de coefficients complexes modifiés
		sendto(client_socket, (char *)fftBuffer, sizeof(fftBuffer), 0,
			   (struct sockaddr *)&server_addr, sizeof(server_addr));

		// Temporisation (10ms) pour rythmer le flux UDP sans saturer le buffer
		usleep(10000);
	}

	// Signal de fin de transmission (1 octet)
	char fin = 0;
	sendto(client_socket, &fin, 1, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

	printf("[CLIENT] Fichier entièrement transmis au serveur.\n");
	fclose(infile);
	close(client_socket);
	return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <mmsystem.h>
#include <winsock2.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 44100
#define N 1024
#define SERVER_IP "127.0.0.1" // IP du serveur (ici en local)
#define SERVER_PORT 8888

typedef struct
{
	double real;
	double imag;
} Complex;

// Bit-Reversal & FFT (Cooley-Tukey)
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
	// ---- 1. INITIALISATION RÉSEAU (WINSOCK) ----
	WSADATA wsa;
	SOCKET client_socket;
	struct sockaddr_in server_addr;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Erreur d'initialisation Winsock.\n");
		return 1;
	}

	if ((client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		printf("Erreur de creation du socket : %d\n", WSAGetLastError());
		return 1;
	}

	memset((char *)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);

	// ---- 2. INITIALISATION AUDIO (MICRO) ----
	HWAVEIN hWaveIn;
	WAVEFORMATEX wfx;
	WAVEHDR waveHdr;
	short int audioBuffer[N];
	Complex fftBuffer[N];

	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 1;
	wfx.nSamplesPerSec = SAMPLE_RATE;
	wfx.wBitsPerSample = 16;
	wfx.cbSize = 0;
	wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	if (waveInOpen(&hWaveIn, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
	{
		fprintf(stderr, "Erreur micro.\n");
		return 1;
	}

	waveHdr.lpData = (LPSTR)audioBuffer;
	waveHdr.dwBufferLength = N * sizeof(short int);
	waveHdr.dwFlags = 0;

	waveInPrepareHeader(hWaveIn, &waveHdr, sizeof(WAVEHDR));

	printf("[CLIENT] Emission des coefficients FFT activee...\n");

	while (1)
	{
		waveInAddBuffer(hWaveIn, &waveHdr, sizeof(WAVEHDR));
		waveInStart(hWaveIn);

		while (!(waveHdr.dwFlags & WHDR_DONE))
			Sleep(2);

		waveInStop(hWaveIn);
		waveHdr.dwFlags &= ~WHDR_DONE;

		// Préparation et calcul FFT
		for (int i = 0; i < N; i++)
		{
			fftBuffer[i].real = (double)audioBuffer[i];
			fftBuffer[i].imag = 0.0;
		}
		fft_cooley_tukey(fftBuffer, N);

		// Envoi du tableau complet de structures complexes via UDP
		// Taille totale : 1024 blocs * 16 octets (2x double) = 16 Ko (Taille standard acceptée en UDP)
		int reponse = sendto(client_socket, (char *)fftBuffer, sizeof(fftBuffer), 0,
							 (struct sockaddr *)&server_addr, sizeof(server_addr));

		if (reponse == SOCKET_ERROR)
		{
			printf("Erreur d'envoi UDP : %d\n", WSAGetLastError());
		}
	}

	closesocket(client_socket);
	WSACleanup();
	return 0;
}
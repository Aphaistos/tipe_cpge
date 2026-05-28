#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <winsock2.h>
#include <windows.h>
#include <mmsystem.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 44100
#define N 1024
#define PORT 8888

typedef struct
{
	double real;
	double imag;
} Complex;

// Bit-Reversal
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

// FFT / IFFT de Cooley-Tukey Itératif
void cooley_tukey(Complex *X, unsigned int size, int inverse)
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
		double angle = (inverse ? 2.0 : -2.0) * M_PI / len;
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

	if (inverse)
	{
		for (unsigned int i = 0; i < size; i++)
		{
			X[i].real /= size;
			X[i].imag /= size;
		}
	}
}

// Optionnel : Fonction de déchiffrement si votre client chiffre
void dechiffrer_bloc(Complex *X, unsigned int size, int cle)
{
	srand(cle);
	unsigned int halfN = size / 2;
	unsigned int *targets = (unsigned int *)malloc(halfN * sizeof(unsigned int));

	for (unsigned int i = 1; i < halfN; i++)
	{
		targets[i] = 1 + (rand() % (halfN - 1));
	}
	for (int i = (int)halfN - 1; i >= 1; i--)
	{
		unsigned int target = targets[i];
		Complex temp = X[i];
		X[i] = X[target];
		X[target] = temp;
		X[size - i].real = X[i].real;
		X[size - i].imag = -X[i].imag;
		X[size - target].real = X[target].real;
		X[size - target].imag = -X[target].imag;
	}
	free(targets);
}

int main()
{
	// ---- 1. INITIALISATION DE LA SORTIE AUDIO (Haut-parleurs) ----
	HWAVEOUT hWaveOut;
	WAVEFORMATEX wfx;
	// On crée 2 tampons pour le Double Buffering (évite les coupures)
	WAVEHDR waveHdr1, waveHdr2;
	short int audioBuffer1[N] = {0};
	short int audioBuffer2[N] = {0};

	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 1;
	wfx.nSamplesPerSec = SAMPLE_RATE;
	wfx.wBitsPerSample = 16;
	wfx.cbSize = 0;
	wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
	{
		printf("Erreur : Impossible d'ouvrir le peripherique audio de sortie.\n");
		return 1;
	}

	// Configuration des en-têtes audio
	waveHdr1.lpData = (LPSTR)audioBuffer1;
	waveHdr1.dwBufferLength = N * sizeof(short int);
	waveHdr1.dwFlags = 0;
	waveHdr2.lpData = (LPSTR)audioBuffer2;
	waveHdr2.dwBufferLength = N * sizeof(short int);
	waveHdr2.dwFlags = 0;
	waveOutPrepareHeader(hWaveOut, &waveHdr1, sizeof(WAVEHDR));
	waveOutPrepareHeader(hWaveOut, &waveHdr2, sizeof(WAVEHDR));

	// ---- 2. INITIALISATION RÉSEAU (Winsock UDP) ----
	WSADATA wsa;
	SOCKET server_socket;
	struct sockaddr_in server_addr, client_addr;
	int client_addr_len = sizeof(client_addr);
	Complex recvBuffer[N];

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
		return 1;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
		return 1;

	printf("[SERVEUR LECTEUR] Pret ! En ecoute sur le port %d...\n", PORT);

	int toggleBuffer = 0; // Permet d'alterner entre le tampon 1 et 2

	while (1)
	{
		// Attente du bloc de coefficients FFT
		int bytes_received = recvfrom(server_socket, (char *)recvBuffer, sizeof(recvBuffer), 0,
									  (struct sockaddr *)&client_addr, &client_addr_len);

		if (bytes_received == sizeof(recvBuffer))
		{

			// Étape A : Déchiffrement (Décommentez si vous utilisez la clé secrète du client)
			// dechiffrer_bloc(recvBuffer, N, 4213);

			// Étape B : IFFT (Retour dans le domaine temporel)
			cooley_tukey(recvBuffer, N, 1); // 1 = Inverse FFT

			// Étape C : Sélection du tampon de lecture disponible
			short int *currentAudioBuffer = (toggleBuffer == 0) ? audioBuffer1 : audioBuffer2;
			WAVEHDR *currentWaveHdr = (toggleBuffer == 0) ? &waveHdr1 : &waveHdr2;

			// Attendre brièvement que la carte son ait fini de jouer ce tampon spécifique
			while (currentWaveHdr->dwFlags & WHDR_INQUEUE)
			{
				Sleep(1);
			}

			// Étape D : Conversion des doubles de la IFFT en entiers 16-bits PCM
			for (int i = 0; i < N; i++)
			{
				currentAudioBuffer[i] = (short int)(recvBuffer[i].real + 0.5);
			}

			// Étape E : Envoi du tampon temporel à la carte son
			waveOutWrite(hWaveOut, currentWaveHdr, sizeof(WAVEHDR));

			// Alternance du buffer pour le prochain bloc réseau
			toggleBuffer = (toggleBuffer == 0) ? 1 : 0;
		}
	}

	// Nettoyage
	waveOutUnprepareHeader(hWaveOut, &waveHdr1, sizeof(WAVEHDR));
	waveOutUnprepareHeader(hWaveOut, &waveHdr2, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
	closesocket(server_socket);
	WSACleanup();
	return 0;
}
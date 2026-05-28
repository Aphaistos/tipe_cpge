#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <alsa/asoundlib.h>

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

int main()
{
	// ---- 1. CONFIGURATION AUDIO ALSA (Lecture / Haut-parleurs) ----
	snd_pcm_t *pcm_handle;
	short int audioBuffer[N];
	Complex recvBuffer[N];

	// "default" ouvre la sortie audio principale sous XUbuntu
	if (snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
	{
		fprintf(stderr, "Erreur d'ouverture du périphérique de lecture ALSA.\n");
		return 1;
	}

	snd_pcm_set_params(pcm_handle,
					   SND_PCM_FORMAT_S16_LE,
					   SND_PCM_ACCESS_RW_INTERLEAVED,
					   1, // Mono
					   SAMPLE_RATE,
					   1,	   // Autoriser la resynchronisation
					   50000); // Latence

	// ---- 2. CONFIGURATION RÉSEAU (Sockets POSIX Linux) ----
	int server_socket;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Erreur socket");
		return 1;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("Erreur Bind");
		return 1;
	}

	printf("[SERVEUR LINUX] Écoute et lecture réseau activées sur le port %d...\n", PORT);

	while (1)
	{
		// Réception du paquet UDP contenant les complexes
		int bytes_received = recvfrom(server_socket, (char *)recvBuffer, sizeof(recvBuffer), 0,
									  (struct sockaddr *)&client_addr, &client_addr_len);

		if (bytes_received == sizeof(recvBuffer))
		{
			// IFFT : Retour dans le domaine temporel
			cooley_tukey(recvBuffer, N, 1);

			// Conversion des doubles en short int 16-bit
			for (int i = 0; i < N; i++)
			{
				audioBuffer[i] = (short int)(recvBuffer[i].real + 0.5);
			}

			// Écriture directe sur la carte son (Haut-parleurs)
			int pcm_rc = snd_pcm_writei(pcm_handle, audioBuffer, N);
			if (pcm_rc == -EPIPE)
			{
				snd_pcm_prepare(pcm_handle); // Récupération en cas d'underrun (si le réseau a du retard)
			}
			else if (pcm_rc < 0)
			{
				fprintf(stderr, "Erreur d'écriture ALSA : %s\n", snd_pcm_strerror(pcm_rc));
			}
		}
	}

	snd_pcm_close(pcm_handle);
	close(server_socket);
	return 0;
}
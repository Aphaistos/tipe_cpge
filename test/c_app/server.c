#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define N 1024
#define PORT 8888

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

void dechiffrer_frequences(Complex *X, unsigned int size)
{
	// Doit réaliser l'exact inverse mathématique de chiffrer_frequences
}

// Cooley-Tukey Itératif (FFT et IFFT selon le paramètre 'inverse')
void cooley_tukey(Complex *X, unsigned int size, int inverse)
{
	unsigned int log2n = (unsigned int)log2(size);
	for (unsigned int i = 0; i < size; i++)
	{
		unsigned int j = 0;
		for (unsigned int k = 0; k < log2n; k++)
		{
			if ((i & (1 << k)))
				j |= (1 << ((log2n - 1) - k));
		}
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
	FILE *outfile = fopen("output.wav", "wb");
	if (!outfile)
	{
		perror("Erreur de création de output.wav");
		return 1;
	}

	// Écriture d'un header temporaire (Mono, 44100Hz, 16 bits PCM)
	WavHeader header = {
		{'R', 'I', 'F', 'F'}, 0, {'W', 'A', 'V', 'E'}, {'f', 'm', 't', ' '}, 16, 1, 1, 44100, 44100 * 2, 2, 16, {'d', 'a', 't', 'a'}, 0};
	fwrite(&header, sizeof(WavHeader), 1, outfile);

	int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("Erreur Bind");
		return 1;
	}

	Complex recvBuffer[N];
	short int audioBuffer[N];
	int total_samples_written = 0;

	printf("[SERVEUR] En attente du flux de données chiffrées...\n");

	while (1)
	{
		int bytes_received = recvfrom(server_socket, (char *)recvBuffer, sizeof(recvBuffer), 0,
									  (struct sockaddr *)&client_addr, &client_addr_len);

		// Interception du signal de fin de transmission
		if (bytes_received <= 10)
		{
			printf("[SERVEUR] Signal de fin détecté. Clôture de l'enregistrement.\n");
			break;
		}

		if (bytes_received == sizeof(recvBuffer))
		{
			// 1. Opération de déchiffrement fréquentiel
			dechiffrer_frequences(recvBuffer, N);

			// 2. IFFT (Retour au domaine temporel)
			cooley_tukey(recvBuffer, N, 1);

			for (int i = 0; i < N; i++)
			{
				audioBuffer[i] = (short int)(recvBuffer[i].real + 0.5);
			}

			fwrite(audioBuffer, sizeof(short int), N, outfile);
			total_samples_written += N;
		}
	}

	// Calcul et mise à jour finale des tailles dans l'en-tête du fichier WAV
	int data_bytes = total_samples_written * sizeof(short int);
	header.data_size = data_bytes;
	header.overall_size = data_bytes + sizeof(WavHeader) - 8;

	fseek(outfile, 0, SEEK_SET);
	fwrite(&header, sizeof(WavHeader), 1, outfile);

	printf("[SERVEUR] Fichier 'output.wav' écrit avec succès (%d échantillons).\n", total_samples_written);
	fclose(outfile);
	close(server_socket);
	return 0;
}
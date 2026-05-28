#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct
{
	double real;
	double imag;
} Complex;

// 1. Bit-Reversal
unsigned int reverseBits(unsigned int num, unsigned int log2n)
{
	unsigned int reverseNum = 0;
	for (unsigned int i = 0; i < log2n; i++)
	{
		if ((num & (1 << i)))
		{
			reverseNum |= (1 << ((log2n - 1) - i));
		}
	}
	return reverseNum;
}

// 2. FFT / IFFT de Cooley-Tukey Itératif
void cooley_tukey(Complex *X, unsigned int N, int inverse)
{
	unsigned int log2n = (unsigned int)log2(N);

	for (unsigned int i = 0; i < N; i++)
	{
		unsigned int j = reverseBits(i, log2n);
		if (i < j)
		{
			Complex temp = X[i];
			X[i] = X[j];
			X[j] = temp;
		}
	}

	for (unsigned int len = 2; len <= N; len <<= 1)
	{
		double angle = (inverse ? 2.0 : -2.0) * M_PI / len;
		Complex wlen = {cos(angle), sin(angle)};

		for (unsigned int i = 0; i < N; i += len)
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

				Complex next_w = {
					w.real * wlen.real - w.imag * wlen.imag,
					w.real * wlen.imag + w.imag * wlen.real};
				w = next_w;
			}
		}
	}

	if (inverse)
	{
		for (unsigned int i = 0; i < N; i++)
		{
			X[i].real /= N;
			X[i].imag /= N;
		}
	}
}

// 3. Algorithme de Déchiffrement (Permutation Inverse)
void dechiffrer_bloc(Complex *X, unsigned int N, int cle)
{
	srand(cle);

	unsigned int halfN = N / 2;
	// Allocation d'un tableau pour mémoriser l'historique des permutations générées
	unsigned int *targets = (unsigned int *)malloc(halfN * sizeof(unsigned int));

	// Étape 1 : On génère exactement la même suite de permutations que le chiffrement
	for (unsigned int i = 1; i < halfN; i++)
	{
		targets[i] = 1 + (rand() % (halfN - 1));
	}

	// Étape 2 : On applique les permutations à l'envers (du dernier i vers le premier)
	for (int i = (int)halfN - 1; i >= 1; i--)
	{
		unsigned int target = targets[i];

		// Permutation inverse des coefficients
		Complex temp = X[i];
		X[i] = X[target];
		X[target] = temp;

		// Maintien de la symétrie hermitienne
		X[N - i].real = X[i].real;
		X[N - i].imag = -X[i].imag;
		X[N - target].real = X[target].real;
		X[N - target].imag = -X[target].imag;
	}

	free(targets);
}

int main()
{
	const unsigned int N = 1024;
	const int CLE_SECRET = 4213; // La même clé que pour le chiffrement

	FILE *f_in = fopen("output_chiffre.wav", "rb");
	FILE *f_out = fopen("output_dechiffre.wav", "wb");

	if (!f_in || !f_out)
	{
		perror("Erreur d'ouverture des fichiers. Assurez-vous que 'output_chiffre.wav' existe");
		return 1;
	}

	// Copie de l'en-tête WAV (44 octets)
	char header[44];
	fread(header, sizeof(char), 44, f_in);
	fwrite(header, sizeof(char), 44, f_out);

	Complex *buffer = (Complex *)malloc(N * sizeof(Complex));
	short int *samples = (short int *)malloc(N * sizeof(short int));

	printf("Déchiffrement du flux audio en cours...\n");

	while (fread(samples, sizeof(short int), N, f_in) == N)
	{

		// Domaine temporel
		for (unsigned int i = 0; i < N; i++)
		{
			buffer[i].real = (double)samples[i];
			buffer[i].imag = 0.0;
		}

		// Passage en fréquence
		cooley_tukey(buffer, N, 0);

		// Application du déchiffrement (permutation inverse)
		dechiffrer_bloc(buffer, N, CLE_SECRET);

		// Retour en temporel
		cooley_tukey(buffer, N, 1);

		// Re-conversion 16 bits
		for (unsigned int i = 0; i < N; i++)
		{
			samples[i] = (short int)(buffer[i].real + 0.5);
		}

		fwrite(samples, sizeof(short int), N, f_out);
	}

	printf("Déchiffrement terminé. Fichier clair sauvegardé sous 'output_dechiffre.wav'.\n");

	free(buffer);
	free(samples);
	fclose(f_in);
	fclose(fclose(f_out));

	return 0;
}
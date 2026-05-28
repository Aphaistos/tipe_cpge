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
// Si inverse = 0 -> FFT, si inverse = 1 -> IFFT
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
		// Changement de signe de l'angle pour l'IFFT
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

	// Normalisation indispensable pour l'IFFT
	if (inverse)
	{
		for (unsigned int i = 0; i < N; i++)
		{
			X[i].real /= N;
			X[i].imag /= N;
		}
	}
}

// 3. Algorithme de Chiffrement par permutation fréquentielle
void chiffrer_bloc(Complex *X, unsigned int N, int cle)
{
	srand(cle); // Clé secrète du chiffrement

	// On permute la première moitié du spectre (hors composante continue i=0)
	for (unsigned int i = 1; i < N / 2; i++)
	{
		unsigned int target = 1 + (rand() % (N / 2 - 1));

		// Permutation
		Complex temp = X[i];
		X[i] = X[target];
		X[target] = temp;

		// Maintien de la symétrie hermitienne pour garantir un signal réel en sortie
		X[N - i].real = X[i].real;
		X[N - i].imag = -X[i].imag;
		X[N - target].real = X[target].real;
		X[N - target].imag = -X[target].imag;
	}
}

int main()
{
	const unsigned int N = 1024; // Taille de bloc (puissance de 2)
	const int CLE_SECRET = 4213; // Exemple de clé de chiffrement

	FILE *f_in = fopen("input.wav", "rb");
	FILE *f_out = fopen("output_chiffre.wav", "wb");

	if (!f_in || !f_out)
	{
		perror("Erreur d'ouverture des fichiers");
		return 1;
	}

	// Copie brute de l'en-tête WAV (44 octets) pour conserver la lisibilité du fichier de sortie
	char header[44];
	fread(header, sizeof(char), 44, f_in);
	fwrite(header, sizeof(char), 44, f_out);

	Complex *buffer = (Complex *)malloc(N * sizeof(Complex));
	short int *samples = (short int *)malloc(N * sizeof(short int));

	printf("Traitement du flux audio par blocs de %u...\n", N);

	// Boucle principale : lecture du flux par blocs de taille N
	while (fread(samples, sizeof(short int), N, f_in) == N)
	{

		// Étape A : Remplissage du buffer complexe (domaine temporel)
		for (unsigned int i = 0; i < N; i++)
		{
			buffer[i].real = (double)samples[i];
			buffer[i].imag = 0.0;
		}

		// Étape B : Passage dans le domaine fréquentiel (FFT)
		cooley_tukey(buffer, N, 0);

		// Étape C : Chiffrement des coefficients
		chiffrer_bloc(buffer, N, CLE_SECRET);

		// Étape D : Retour dans le domaine temporel (IFFT)
		cooley_tukey(buffer, N, 1);

		// Étape E : Re-conversion en entiers 16 bits pour le flux de sortie
		for (unsigned int i = 0; i < N; i++)
		{
			// Arrondi et cast sécurisé
			samples[i] = (short int)(buffer[i].real + 0.5);
		}

		// Écriture du bloc chiffré dans le flux de sortie
		fwrite(samples, sizeof(short int), N, f_out);
	}

	printf("Chiffrement du flux terminé. Fichier 'output_chiffre.wav' généré.\n");

	free(buffer);
	free(samples);
	fclose(f_in);
	fclose(f_out);

	return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <mmsystem.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 44100 // Fréquence d'échantillonnage standard (44.1 kHz)
#define N 1024			  // Taille du bloc FFT (Puissance de 2)

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

// 2. FFT Cooley-Tukey Itératif
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

				Complex next_w = {
					w.real * wlen.real - w.imag * wlen.imag,
					w.real * wlen.imag + w.imag * wlen.real};
				w = next_w;
			}
		}
	}
}

int main()
{
	HWAVEIN hWaveIn;
	WAVEFORMATEX wfx;
	WAVEHDR waveHdr;
	short int audioBuffer[N]; // Tampon pour stocker les échantillons 16-bit du micro
	Complex fftBuffer[N];

	// Configuration du format audio : 44100Hz, 16 bits, Mono
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 1;
	wfx.nSamplesPerSec = SAMPLE_RATE;
	wfx.wBitsPerSample = 16;
	wfx.cbSize = 0;
	wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	// Ouverture du périphérique de capture audio (Microphone par défaut)
	if (waveInOpen(&hWaveIn, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
	{
		fprintf(stderr, "Erreur : Impossible d'ouvrir le microphone.\n");
		return 1;
	}

	// Préparation de l'en-tête du tampon
	waveHdr.lpData = (LPSTR)audioBuffer;
	waveHdr.dwBufferLength = N * sizeof(short int);
	waveHdr.dwBytesRecorded = 0;
	waveHdr.dwUser = 0;
	waveHdr.dwFlags = 0;
	waveHdr.dwLoops = 0;

	if (waveInPrepareHeader(hWaveIn, &waveHdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		fprintf(stderr, "Erreur : Impossible de préparer le tampon audio.\n");
		waveInClose(hWaveIn);
		return 1;
	}

	printf(">>> Analyseur Spectral en Temps Reel Lance <<<\n");
	printf("Appuyez sur Ctrl+C pour arreter.\n\n");

	// Boucle infinie d'écoute du flux
	while (1)
	{
		// Lancer la capture sur le tampon
		waveInAddBuffer(hWaveIn, &waveHdr, sizeof(WAVEHDR));
		waveInStart(hWaveIn);

		// Attendre que le tampon de taille N soit complètement rempli par le micro
		while (!(waveHdr.dwFlags & WHDR_DONE))
		{
			Sleep(5); // Petite pause pour ne pas surcharger le CPU
		}
		waveInStop(hWaveIn);
		waveHdr.dwFlags &= ~WHDR_DONE; // Réinitialiser le flag pour le prochain tour

		// Étape A : Copie des données du micro dans le buffer complexe
		for (int i = 0; i < N; i++)
		{
			fftBuffer[i].real = (double)audioBuffer[i];
			fftBuffer[i].imag = 0.0;
		}

		// Étape B : Calcul de la FFT
		fft_cooley_tukey(fftBuffer, N);

		// Étape C : Recherche de la fréquence dominante principale (hors composante continue continue i=0)
		double maxMagnitude = 0.0;
		int maxIndex = 0;

		// On inspecte la moitié utile du spectre (0 à N/2)
		for (int i = 1; i < N / 2; i++)
		{
			double magnitude = sqrt(fftBuffer[i].real * fftBuffer[i].real + fftBuffer[i].imag * fftBuffer[i].imag);
			if (magnitude > maxMagnitude)
			{
				maxMagnitude = magnitude;
				maxIndex = i;
			}
		}

		// Étape D : Conversion de l'indice de la FFT en fréquence réelle (Hz)
		double dominanteFreq = (double)maxIndex * SAMPLE_RATE / N;

		// Affichage dynamique (on efface la ligne précédente avec \r)
		if (maxMagnitude > 50000.0)
		{ // Seuil de bruit pour éviter d'afficher le silence
			printf("\rFrequence dominante detectee : %7.1f Hz (Index FFT: %3d) [Mag: %.0f]          ",
				   dominanteFreq, maxIndex, maxMagnitude);
		}
		else
		{
			printf("\rSilence... (En attente d'un signal audio clair)                                ");
		}
		fflush(stdout);
	}

	// Nettoyage (théoriquement inaccessible à cause de la boucle infinie)
	waveInUnprepareHeader(hWaveIn, &waveHdr, sizeof(WAVEHDR));
	waveInClose(hWaveIn);
	return 0;
}
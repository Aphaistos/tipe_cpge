#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <alsa/asoundlib.h>
#include <pthread.h> // <-- Ajout pour les threads

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

typedef struct
{
	struct timespec timestamp;
	Complex fftData[N];
} Packet;

// ---- VARIABLES GLOBALES POUR LES THREADS ----
pthread_mutex_t mutex_latency = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_latency = PTHREAD_COND_INITIALIZER;
double shared_latency_ms = -1.0;
double shared_timestamp_sec = 0.0;
int keep_running = 1;

// Fonction de Bit-Reversal
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

// FFT / IFFT Itératif
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

// ---- CODE DU THREAD SECONDAIRE (ÉCRITURE CSV) ----
void *csv_logging_thread(void *arg)
{
	FILE *csv_file = fopen("latence.csv", "w");
	if (!csv_file)
	{
		perror("Erreur lors de la creation du fichier CSV");
		return NULL;
	}

	// Écriture de l'en-tête du fichier CSV
	fprintf(csv_file, "Temps_Ecoule_s,Latence_ms\n");
	fflush(csv_file);

	struct timespec start_time;
	clock_gettime(CLOCK_REALTIME, &start_time);

	while (keep_running)
	{
		pthread_mutex_lock(&mutex_latency);

		// Attendre que le thread principal signale l'arrivée d'une nouvelle mesure
		while (shared_latency_ms < 0 && keep_running)
		{
			pthread_cond_wait(&cond_latency, &mutex_latency);
		}

		if (!keep_running)
		{
			pthread_mutex_unlock(&mutex_latency);
			break;
		}

		// Calcul du temps relatif par rapport au lancement de l'enregistrement
		double current_time_s = shared_timestamp_sec - (double)start_time.tv_sec - ((double)start_time.tv_nsec / 1000000000.0);
		double latency_to_write = shared_latency_ms;

		// Consommation de la donnée
		shared_latency_ms = -1.0;

		pthread_mutex_unlock(&mutex_latency);

		// Écriture non-bloquante pour le flux audio principal
		fprintf(csv_file, "%.4f,%.3f\n", current_time_s, latency_to_write);
		fflush(csv_file); // Force l'écriture sur le disque
	}

	fclose(csv_file);
	printf("\n[THREAD CSV] Fichier 'latence.csv' sauvegarde avec succes.\n");
	return NULL;
}

int main()
{
	snd_pcm_t *pcm_handle;
	short int audioBuffer[N];
	Packet recvPacket;
	struct timespec arrivalTime;
	pthread_t log_thread_id;

	if (snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
	{
		fprintf(stderr, "Erreur d'ouverture du périphérique de lecture ALSA.\n");
		return 1;
	}
	snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, SAMPLE_RATE, 1, 50000);

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

	// ---- LANCEMENT DU THREAD ÉCRIVAIN ----
	if (pthread_create(&log_thread_id, NULL, csv_logging_thread, NULL) != 0)
	{
		fprintf(stderr, "Erreur de creation du thread CSV.\n");
		return 1;
	}

	printf("[SERVEUR LINUX] Ecoute active, audio operationnel et enregistrement CSV lance...\n");

	while (1)
	{
		int bytes_received = recvfrom(server_socket, (char *)&recvPacket, sizeof(Packet), 0,
									  (struct sockaddr *)&client_addr, &client_addr_len);

		clock_gettime(CLOCK_REALTIME, &arrivalTime);

		if (bytes_received == sizeof(Packet))
		{

			// Calcul de la latence
			double diff_sec = (double)(arrivalTime.tv_sec - recvPacket.timestamp.tv_sec);
			double diff_nsec = (double)(arrivalTime.tv_nsec - recvPacket.timestamp.tv_nsec);
			double latency_ms = (diff_sec * 1000.0) + (diff_nsec / 1000000.0);

			printf("\r[Flux Audio] Latence : %6.3f ms | Fichier CSV en cours de remplissage...", latency_ms);
			fflush(stdout);

			// ---- TRANSMISSION SÉCURISÉE AU THREAD CSV ----
			pthread_mutex_lock(&mutex_latency);
			shared_latency_ms = latency_ms;
			shared_timestamp_sec = (double)arrivalTime.tv_sec + ((double)arrivalTime.tv_nsec / 1000000000.0);
			pthread_cond_signal(&cond_latency); // On réveille le thread écrivain
			pthread_mutex_unlock(&mutex_latency);

			// Traitement FFT Inverse immédiat sans interruption
			cooley_tukey(recvPacket.fftData, N, 1);

			for (int i = 0; i < N; i++)
			{
				audioBuffer[i] = (short int)(recvPacket.fftData[i].real + 0.5);
			}

			int pcm_rc = snd_pcm_writei(pcm_handle, audioBuffer, N);
			if (pcm_rc == -EPIPE)
			{
				snd_pcm_prepare(pcm_handle);
			}
			else if (pcm_rc < 0)
			{
				fprintf(stderr, "\nErreur d'ecriture ALSA : %s\n", snd_strerror(pcm_rc));
			}
		}
	}

	// Code de nettoyage (Théorique)
	keep_running = 0;
	pthread_cond_signal(&cond_latency);
	pthread_join(log_thread_id, NULL);
	snd_pcm_close(pcm_handle);
	close(server_socket);
	return 0;
}
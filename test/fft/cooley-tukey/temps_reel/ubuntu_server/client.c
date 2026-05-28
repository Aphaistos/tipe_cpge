#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <alsa/asoundlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 44100
#define N 1024
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888

typedef struct {
    double real;
    double imag;
} Complex;

// Bit-Reversal & FFT
unsigned int reverseBits(unsigned int num, unsigned int log2n) {
    unsigned int reverseNum = 0;
    for (unsigned int i = 0; i < log2n; i++) {
        if ((num & (1 << i))) reverseNum |= (1 << ((log2n - 1) - i));
    }
    return reverseNum;
}

void fft_cooley_tukey(Complex *X, unsigned int size) {
    unsigned int log2n = (unsigned int)log2(size);
    for (unsigned int i = 0; i < size; i++) {
        unsigned int j = reverseBits(i, log2n);
        if (i < j) { Complex temp = X[i]; X[i] = X[j]; X[j] = temp; }
    }
    for (unsigned int len = 2; len <= size; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        Complex wlen = {cos(angle), sin(angle)};
        for (unsigned int i = 0; i < size; i += len) {
            Complex w = {1.0, 0.0};
            for (unsigned int j = 0; j < len / 2; j++) {
                Complex u = X[i + j];
                Complex v = {
                    X[i + j + len / 2].real * w.real - X[i + j + len / 2].imag * w.imag,
                    X[i + j + len / 2].real * w.imag + X[i + j + len / 2].imag * w.real
                };
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

int main() {
    // ---- 1. CONFIGURATION RÉSEAU (Sockets POSIX Linux) ----
    int client_socket;
    struct sockaddr_in server_addr;

    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Erreur de création du socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // ---- 2. CONFIGURATION AUDIO ALSA (Capture / Microphone) ----
    snd_pcm_t *pcm_handle;
    short int audioBuffer[N];
    Complex fftBuffer[N];

    // "default" ouvre le périphérique de capture par défaut de XUbuntu
    if (snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_CAPTURE, 0) < 0) {
        fprintf(stderr, "Erreur d'ouverture du périphérique de capture ALSA.\n");
        return 1;
    }

    // Configuration simplifiée du matériel (16 bits, Mono, 44100Hz)
    snd_pcm_set_params(pcm_handle,
                       SND_PCM_FORMAT_S16_LE,     // Interleaved 16-bit Little Endian
                       SND_PCM_ACCESS_RW_INTERLEAVED,
                       1,                         // Mono
                       SAMPLE_RATE,
                       1,                         // Autoriser la resynchronisation logicielle
                       50000);                    // Latence de 50ms

    printf("[CLIENT LINUX] Émission des coefficients FFT en cours...\n");

    while (1) {
        // Lecture de N échantillons depuis le micro (Bloquant jusqu'à ce que le buffer soit plein)
        int pcm_rc = snd_pcm_readi(pcm_handle, audioBuffer, N);
        if (pcm_rc == -EPIPE) {
            snd_pcm_prepare(pcm_handle); // Récupération en cas d'overrun
            continue;
        } else if (pcm_rc < 0) {
            fprintf(stderr, "Erreur de lecture ALSA : %s\n", snd_pcm_strerror(pcm_rc));
            break;
        }

        // Passage en complexe et calcul de la FFT
        for (int i = 0; i < N; i++) {
            fftBuffer[i].real = (double)audioBuffer[i];
            fftBuffer[i].imag = 0.0;
        }
        fft_cooley_tukey(fftBuffer, N);

        // Envoi des coefficients par UDP
        sendto(client_socket, (char*)fftBuffer, sizeof(fftBuffer), 0, 
               (struct sockaddr *)&server_addr, sizeof(server_addr));
    }

    snd_pcm_close(pcm_handle);
    close(client_socket);
    return 0;
}
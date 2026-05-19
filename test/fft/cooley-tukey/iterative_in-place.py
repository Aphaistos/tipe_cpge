import numpy as np
import scipy.io.wavfile as wav
import matplotlib.pyplot as plt
import time
from scipy.stats import entropy

def calculate_entropy(signal):
    """Calcule l'entropie de Shannon sur la distribution des amplitudes."""
    # On discrétise le signal en 256 niveaux (8 bits) pour avoir une base de comparaison
    hist, _ = np.histogram(signal, bins=256, range=(-1, 1), density=True)
    return entropy(hist + 1e-12, base=2) # Ajout d'un epsilon pour éviter log(0)

def bit_reverse_traversal(x):
    """Réordonne le tableau pour l'algorithme itératif (Bit-reversal)."""
    n = len(x)
    j = 0
    for i in range(1, n):
        bit = n >> 1
        while j & bit:
            j ^= bit
            bit >>= 1
        j ^= bit
        if i < j:
            x[i], x[j] = x[j], x[i]
    return x

def optimized_fft(x):
    """FFT itérative de Cooley-Tukey (In-place)."""
    n = len(x)
    # 1. Réorganisation des données
    x = bit_reverse_traversal(x)
    
    # 2. Calcul des étages du papillon
    length = 2
    while length <= n:
        ang = -2j * np.pi / length
        wlen = np.exp(ang)
        for i in range(0, n, length):
            w = 1 + 0j
            for j in range(length // 2):
                u = x[i + j]
                v = x[i + j + length // 2] * w
                x[i + j] = u + v
                x[i + j + length // 2] = u - v
                w *= wlen
        length <<= 1
    return x

def optimized_ifft(X):
    """
    IFFT itérative de Cooley-Tukey.
    On utilise la propriété : IFFT(X) = 1/N * conj(FFT(conj(X)))
    """
    n = len(X)
    # 1. Conjugué complexe des entrées
    X_conj = np.conj(X)
    
    # 2. On applique la FFT directe sur les conjugués
    # (On réutilise ici la fonction optimized_fft définie précédemment)
    result = optimized_fft(X_conj)
    
    # 3. Conjugué du résultat et normalisation par N
    return np.conj(result) / n


class SecureAudioProcessor:
    def __init__(self, frame_size=512):
        self.frame_size = frame_size
        self.window = np.hanning(frame_size) # Fenêtrage pour éviter les fuites
        
    def process_and_reconstruct(self, audio_buffer):
        """
        Chaîne complète : 
        Signal -> FFT -> [Compression/Chiffrement] -> [Décompression/Déchiffrement] -> IFFT
        """
        # --- CÔTÉ ÉMETTEUR ---
        # Fenêtrage et Passage en fréquentiel
        freq_domain = optimized_fft((audio_buffer * self.window).astype(complex))
        
        # Réduction d'entropie (Quantification pour la sécurité)
        # On simule ici la perte d'information volontaire
        quantized = np.round(freq_domain / 0.8) * 0.8
        
        # --- CÔTÉ RÉCEPTEUR ---
        # Retour au domaine temporel
        reconstructed_signal = optimized_ifft(quantized)
        
        # Le signal reconstruit est complexe, on ne garde que la partie réelle
        return np.real(reconstructed_signal)

if __name__ == "__main__":
    # Paramètres conformes à la téléphonie standard (N=512 à 16kHz ~= 32ms de latence)
    input_file, output_file, frame_size = "../../huffman_compression/records/low_entropy.wav", "output/low_entropy.wav", 512
    

    # 1. Chargement et préparation
    sample_rate, data = wav.read(input_file)
    if data.dtype == np.int16:
        data = data.astype(np.float32) / 32768.0
    if len(data.shape) > 1: data = data[:, 0]

    hop_size = frame_size // 2
    BLOCK_MS = (hop_size / sample_rate) * 1000
    window = np.hanning(frame_size)

    # Listes pour la visualisation
    processing_times = []
    entropies = []
    time_axis = []

    print(f"Analyse en cours (Blocs de {BLOCK_MS:.2f} ms)...")

    output_audio = np.zeros(len(data) + frame_size)

    # 2. Boucle de traitement avec monitoring
    for i in range(0, len(data) - frame_size, hop_size):
        start_time = time.perf_counter()

        # --- PHASE ÉMETTEUR ---
        frame = data[i:i+frame_size]
        # FFT de Cooley-Tukey (Utilise ta fonction optimized_fft)
        freq_domain = optimized_fft((frame * window).astype(complex))

        # Quantification (Compression pour réduire l'entropie)
        pas = 0.5
        quantized = np.round(freq_domain / pas) * pas

        # --- PHASE RÉCEPTEUR ---
        reconstructed_frame = np.real(optimized_ifft(quantized))

        # --- MONITORING ---
        end_time = time.perf_counter()

        output_audio[i:i+frame_size] += reconstructed_frame * window
        processing_times.append((end_time - start_time) * 1000) # en ms
        entropies.append(calculate_entropy(reconstructed_frame))
        time_axis.append(i / sample_rate * 1000)

    # Variables pour le bloc de visualisation fourni
    num_blocks = len(processing_times)
    time_axis = np.array(time_axis)
    processing_times = np.array(processing_times)
    entropies = np.array(entropies)

    # --- 3. VISUALISATION DES RÉSULTATS (Ton bloc de code) ---
    fig = plt.figure(figsize=(14, 10))
    grid = plt.GridSpec(2, 2, hspace=0.3, wspace=0.3)

    # A. Signal Temporel (Waveform)
    ax1 = fig.add_subplot(grid[0, 0])
    ax1.plot(np.linspace(0, len(data)/sample_rate, len(data)), data, color='teal', linewidth=0.5)
    ax1.set_title("Signal Temporel Brut")
    ax1.set_xlabel("Temps (s)")
    ax1.set_ylabel("Amplitude")
    ax1.grid(alpha=0.3)

    # B. Histogramme Global
    ax2 = fig.add_subplot(grid[0, 1])
    ax2.hist(data, bins=256, color='orangered', alpha=0.7, edgecolor='black')
    ax2.set_title(f"Distribution Globale\nEntropie moyenne : {calculate_entropy(data):.2f} bits/sample")
    ax2.set_xlabel("Valeur de l'échantillon")
    ax2.grid(alpha=0.3)

    # C. Analyse Latence et Entropie
    ax3 = fig.add_subplot(grid[1, :])
    ax3.plot(time_axis, entropies, color='blue', label="Entropie $H(X)$ (bits/sample)")
    ax3.set_xlabel("Temps écoulé (ms)")
    ax3.set_ylabel("Entropie", color='blue')
    ax3.set_title("Évolution de la charge d'information et du temps de traitement")
    ax3.grid(True, linestyle='--', alpha=0.5)

    ax4 = ax3.twinx()
    ax4.plot(time_axis, processing_times, color='red', alpha=0.6, label="Temps de traitement $T_{traitement}$ (ms)")
    ax4.set_ylabel("Temps CPU (ms)", color='red')

    # Seuils et Statistiques
    median_time = np.median(processing_times)
    p99_time = np.percentile(processing_times, 99)

    ax4.axhline(y=BLOCK_MS, color='black', linestyle=':', linewidth=2, label=f"Limite temps réel ({BLOCK_MS:.1f}ms)")
    ax4.axhline(y=median_time, color='green', linestyle='--', label=f"Médiane : {median_time:.3f} ms")
    ax4.axhline(y=p99_time, color="orange", linestyle=':', label=f"P99 (Stabilité): {p99_time:.4f} ms")

    lines, labels = ax3.get_legend_handles_labels()
    lines2, labels2 = ax4.get_legend_handles_labels()
    ax3.legend(lines + lines2, labels + labels2, loc='upper right', fontsize='small')

    plt.show()

    # Rapport final
    print(f"\n--- RAPPORT DE LATENCE (T_traitement) ---")
    print(f"Moyenne      : {np.mean(processing_times):.4f} ms")
    print(f"99ème centile: {p99_time:.4f} ms  <-- Garantie de fluidité")
    print(f"Budget total : {BLOCK_MS:.4f} ms")
    
    output_audio = np.clip(output_audio, -1.0, 1.0)
    output_audio = (output_audio * 32767).astype(np.int16)

    wav.write(output_file, sample_rate, output_audio[:len(data)])
    print(f"Fichier sécurisé généré : {output_file}")

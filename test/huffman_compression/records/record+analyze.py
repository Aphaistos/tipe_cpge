import sounddevice as sd
import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
import collections
import math
import time

# --- CONFIGURATION ---
FS = 44100          # Fréquence d'échantillonnage (Hz)
DURATION = 5        # Durée de l'enregistrement (s)
BLOCK_MS = 20       # Taille du bloc pour l'analyse temps réel (ms)
FILENAME = "test_huffman.wav"

def calculate_entropy(signal):
    """Calcule l'entropie de Shannon H(X) en bits/échantillon."""
    if len(signal) == 0: return 0
    counts = collections.Counter(signal)
    total = len(signal)
    return -sum((c/total) * math.log2(c/total) for c in counts.values())

# --- 1. PHASE D'ENREGISTREMENT ---
print(f"🔴 ENREGISTREMENT : {DURATION}s à {FS}Hz...")
recording = sd.rec(int(DURATION * FS), samplerate=FS, channels=1, dtype='float32')
sd.wait()
print("✅ Enregistrement terminé.")

# Conversion PCM 16-bit
data_16bit = (recording.flatten() * 32767).astype(np.int16)
wavfile.write(FILENAME, FS, data_16bit)

# --- 2. PHASE D'ÉTUDE AUTOMATIQUE (FENÊTRAGE) ---
print("📊 Analyse des performances en cours...")

block_size = int(FS * (BLOCK_MS / 1000))
num_blocks = len(data_16bit) // block_size

# Listes pour stocker les métriques par bloc
entropies = []
processing_times = []
time_axis = []

for i in range(num_blocks):
    block = data_16bit[i * block_size : (i + 1) * block_size]
    
    # Mesure du temps de traitement (T_traitement)
    start_time = time.perf_counter()
    
    h = calculate_entropy(block)
    
    end_time = time.perf_counter()
    
    # Stockage
    entropies.append(h)
    processing_times.append((end_time - start_time) * 1000) # en ms
    time_axis.append(i * BLOCK_MS)

# --- 3. VISUALISATION DES RÉSULTATS ---
fig = plt.figure(figsize=(14, 10))
grid = plt.GridSpec(2, 2, hspace=0.3, wspace=0.3)

# A. Signal Temporel (Waveform)
ax1 = fig.add_subplot(grid[0, 0])
ax1.plot(np.linspace(0, DURATION, len(data_16bit)), data_16bit, color='teal', linewidth=0.5)
ax1.set_title("Signal Temporel Brut")
ax1.set_xlabel("Temps (s)")
ax1.set_ylabel("Amplitude (PCM 16-bit)")
ax1.grid(alpha=0.3)

# B. Histogramme Global de Distribution
ax2 = fig.add_subplot(grid[0, 1])
ax2.hist(data_16bit, bins=256, color='orangered', alpha=0.7, edgecolor='black')
ax2.set_title(f"Distribution Globale\nEntropie moyenne : {calculate_entropy(data_16bit):.2f} bits/sample")
ax2.set_xlabel("Valeur de l'échantillon")
ax2.set_ylabel("Fréquence d'apparition")
ax2.grid(alpha=0.3)

# C. Analyse de la Latence et de l'Entropie par Bloc (La preuve TIPE)
ax3 = fig.add_subplot(grid[1, :]) # Prend toute la largeur en bas
ax3.plot(time_axis, entropies, color='blue', label="Entropie $H(X)$ (bits/sample)")
ax3.set_xlabel("Temps écoulé (ms)")
ax3.set_ylabel("Entropie (bits/sample)", color='blue')
ax3.set_title("Évolution de la charge d'information et du temps de traitement")
ax3.grid(True, linestyle='--', alpha=0.5)

# Axe secondaire pour le temps de calcul
ax4 = ax3.twinx()
ax4.plot(time_axis, processing_times, color='red', alpha=0.6, label="Temps de traitement $T_{traitement}$ (ms)")
ax4.set_ylabel("Temps CPU (ms)", color='red')

# Seuil critique G.114 (20ms car c'est la durée du bloc)
ax4.axhline(y=BLOCK_MS, color='black', linestyle=':', linewidth=2, label="Limite temps réel (20ms)")

# Regroupement des légendes
lines, labels = ax3.get_legend_handles_labels()
lines2, labels2 = ax4.get_legend_handles_labels()
ax3.legend(lines + lines2, labels + labels2, loc='upper right')

plt.show()

print(f"Analyse terminée. Gain théorique moyen : {(1 - calculate_entropy(data_16bit)/16)*100:.1f}%")
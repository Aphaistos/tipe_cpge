import sounddevice as sd
import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
import collections
import math

# --- CONFIGURATION ---
FS = 44100  # Fréquence d'échantillonnage (Standard CD/Yeti)
DURATION = 20  # Durée en secondes
FILENAME = "test_huffman.wav"

def get_entropy(signal):
    counts = collections.Counter(signal)
    total = len(signal)
    return -sum((c/total) * math.log2(c/total) for c in counts.values())

# ENREGISTREMENT
print(f"Enregistrement de {DURATION} secondes lancé...")
# On enregistre en float32 par défaut, puis on convertit
recording = sd.rec(int(DURATION * FS), samplerate=FS, channels=1, dtype='float32')
sd.wait()  # Attend la fin de l'enregistrement
print("Enregistrement terminé.")

# 2. CONVERSION EN PCM 16-BIT (Signé)
# Le Blue Yeti produit des valeurs entre -1.0 et 1.0 en float32.
# On les mappe sur l'intervalle [-32768, 32767]
data_16bit = (recording.flatten() * 32767).astype(np.int16)

# 3. SAUVEGARDE
wavfile.write(FILENAME, FS, data_16bit)

# 4. CALCUL ET VISUALISATION
entropy_val = get_entropy(data_16bit)

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6))

# Signal Temporel
ax1.plot(data_16bit, color='teal', linewidth=0.5)
ax1.set_title(f"Signal Audio : {FILENAME}")
ax1.set_xlabel("Samples")
ax1.set_ylabel("Amplitude (Int16)")
ax1.grid(alpha=0.3)

# Histogramme pour l'entropie
ax2.hist(data_16bit, bins=256, color='orangered', alpha=0.7, edgecolor='black')
ax2.set_title(f"Analyse Statistique\nEntropie = {entropy_val:.2f} bits/sample")
ax2.set_xlabel("Valeurs (PCM 16-bit)")
ax2.set_ylabel("Fréquence")
ax2.grid(alpha=0.3)

plt.tight_layout()
plt.show()
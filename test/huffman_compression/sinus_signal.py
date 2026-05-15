import numpy as np
import pyaudio
import matplotlib.pyplot as plt

# Configuration des paramètres
sampling_rate = 44100  # Fréquence d'échantillonnage
duration = 2.0         # Durée en secondes
frequency = 392.00     # Fréquence du Sol en Hertzs
amplitude = 0.5        # Volume (entre 0 et 1)

# 1. Génération du signal avec NumPy
t = np.linspace(0, duration, int(sampling_rate * duration), endpoint=False)
signal = amplitude * np.sin(2 * np.pi * frequency * t)

# Conversion en format supporté par la carte son (float32 ou int16)
samples = signal.astype(np.float32).tobytes()

# 2. Lecture du son avec PyAudio
p = pyaudio.PyAudio()
stream = p.open(format=pyaudio.paFloat32,
                channels=1,
                rate=sampling_rate,
                output=True)

print(f"Lecture d'un Sol ({frequency} Hz)...")
stream.write(samples)
stream.stop_stream()
stream.close()
p.terminate()

# 3. Affichage avec Matplotlib
plt.figure(figsize=(10, 4))
# On n'affiche que les 1000 premiers points pour voir la sinusoïde clairement
plt.plot(t[:1000], signal[:1000])
plt.title(f"Signal Sinusoïdal - Note Sol ({frequency} Hz)")
plt.xlabel("Temps (s)")
plt.ylabel("Amplitude")
plt.grid(True)
plt.show()
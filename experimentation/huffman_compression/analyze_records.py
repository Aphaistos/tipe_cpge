import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
import collections
import math
import time
import heapq

class HuffmanNode:
    def __init__(self, char, freq):
        self.char = char
        self.freq = freq
        self.left = None
        self.right = None

    def __lt__(self, other):
        return self.freq < other.freq

def build_huffman_tree(data):
    # Compter les occurrences de chaque octet
    frequency = collections.Counter(data)
    heap = [HuffmanNode(char, freq) for char, freq in frequency.items()]
    heapq.heapify(heap)

    while len(heap) > 1:
        node1 = heapq.heappop(heap)
        node2 = heapq.heappop(heap)
        merged = HuffmanNode(None, node1.freq + node2.freq)
        merged.left = node1
        merged.right = node2
        heapq.heappush(heap, merged)

    return heap[0]

def generate_codes(node, current_code="", codes={}):
    if node is None:
        return
    if node.char is not None:
        codes[node.char] = current_code
    generate_codes(node.left, current_code + "0", codes)
    generate_codes(node.right, current_code + "1", codes)
    return codes

def save_compressed_file(filename, encoded_string, codes):
    # On ajoute des zéros à la fin pour avoir des octets complets (padding)
    padding = 8 - (len(encoded_string) % 8)
    encoded_string += "0" * padding
    
    # Conversion de la chaîne "0101" en véritables octets
    byte_array = bytearray()
    for i in range(0, len(encoded_string), 8):
        byte = encoded_string[i:i+8]
        byte_array.append(int(byte, 2))

    with open(filename, 'wb') as f:
        # On peut stocker le padding et le dictionnaire pour la décompression plus tard
        # Ici on simplifie en ne stockant que les données pour la mesure
        f.write(byte_array)

    return len(byte_array)

filename = 'records/la_0_1_amp.wav'

# --- CONFIGURATION ---
FS = 44100
BLOCK_MS = 20  
BLOCK_SIZE = int(FS * (BLOCK_MS / 1000))

def calculate_entropy(signal):
    """Calcule l'entropie de Shannon H(X) en bits/échantillon."""
    if len(signal) == 0: return 0
    counts = collections.Counter(signal)
    total = len(signal)
    return -sum((c/total) * math.log2(c/total) for c in counts.values())

samplerate, data = wavfile.read(filename)
if len(data.shape) > 1: data = data[:, 0]
    
num_blocks = len(data) // BLOCK_SIZE
print(num_blocks)
    
# Listes pour stocker les métriques par bloc
entropies = []
processing_times = []
time_axis = []

for i in range(num_blocks):
    block = data[i * BLOCK_SIZE : (i + 1) * BLOCK_SIZE]
    
    # Mesure du temps de traitement (T_traitement)
    start_time = time.perf_counter()
    
    h = calculate_entropy(block)
    
	# Compression
    tree = build_huffman_tree(block)
    codes = generate_codes(tree)
    encoded_data = "".join(codes[byte] for byte in block)
    
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
ax1.plot(np.linspace(0, num_blocks*BLOCK_MS, len(data)), data, color='teal', linewidth=0.5)
ax1.set_title("Signal Temporel Brut")
ax1.set_xlabel("Temps (s)")
ax1.set_ylabel("Amplitude (PCM 16-bit)")
ax1.grid(alpha=0.3)

# B. Histogramme Global de Distribution
ax2 = fig.add_subplot(grid[0, 1])
ax2.hist(data, bins=256, color='orangered', alpha=0.7, edgecolor='black')
ax2.set_title(f"Distribution Globale\nEntropie moyenne : {calculate_entropy(data):.2f} bits/sample")
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


#for i in range(num_blocks):
#   ax4.axvline(x=i*BLOCK_MS, color='green', linestyle=':')
mean_time = np.mean(processing_times)
median_time = np.median(processing_times)
max_time = np.max(processing_times)
p99_time = np.percentile(processing_times, 99)

ax4.plot(time_axis, processing_times, color='red', alpha=0.6, label="Temps de traitement $T_{traitement}$ (ms)")
ax4.set_ylabel("Temps CPU (ms)", color='red')

# Seuil critique G.114 (20ms car c'est la durée du bloc)
ax4.axhline(y=BLOCK_MS, color='black', linestyle=':', linewidth=2, label="Limite temps réel (20ms)")
ax4.axhline(y=median_time, color='green', linestyle='--', linewidth=1.5, 
            label=f"Médiane : {median_time:.3f} ms")
ax4.axhline(y=p99_time, color='orange', linestyle=':', linewidth=1.5, 
            label=f"P99 (Stabilité) : {p99_time:.3f} ms")

# Regroupement des légendes
lines, labels = ax3.get_legend_handles_labels()
lines2, labels2 = ax4.get_legend_handles_labels()
ax3.legend(lines + lines2, labels + labels2, loc='upper right', fontsize='small')
plt.show()
plt.close()

print(f"\n--- RAPPORT DE LATENCE (T_traitement) ---")
print(f"Moyenne      : {mean_time:.4f} ms")
print(f"Médiane      : {median_time:.4f} ms  <-- Performance réelle")
print(f"Maximum      : {max_time:.4f} ms  <-- Pic observé")
print(f"99ème centile: {p99_time:.4f} ms  <-- Garantie de fluidité")
print(f"Budget total : {BLOCK_MS}.0000 ms")
print(f"Marge de sécu: {BLOCK_MS - p99_time:.4f} ms")
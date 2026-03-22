import heapq
import collections
import wave
import struct

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

# Lecture d'un fichier WAV
with wave.open('records/high_entropy.wav', 'rb') as f:
    raw_data = f.readframes(f.getnframes())

# Compression
tree = build_huffman_tree(raw_data)
codes = generate_codes(tree)
encoded_data = "".join(codes[byte] for byte in raw_data)

print(f"Taille originale : {len(raw_data)} octets")
print(f"Taille compressée : {save_compressed_file('minimal_entropy.huff', encoded_data, codes)} octets")
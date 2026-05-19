import collections
import math

def calculate_entropy(data):
    """
    Calcule l'entropie de Shannon en bits par symbole.
    Un signal avec une entropie élevée est plus difficile à prédire/compresser.
    """
    if len(data) == 0:
        return 0
    
    # On arrondit légèrement pour grouper les valeurs proches 
    # et créer une distribution de probabilité cohérente
    data_rounded = np.round(data, 2)
    
    # Calcul des fréquences d'apparition de chaque symbole
    counter = collections.Counter(data_rounded)
    total_count = len(data_rounded)
    
    entropy = 0
    for count in counter.values():
        p_x = count / total_count
        entropy += - p_x * math.log2(p_x)
        
    return entropy

# --- Intégration dans ton processus ---

# 1. Entropie du signal brut (fréquentiel)
entropy_initial = calculate_entropy(np.abs(freq_domain))

# 2. Entropie après quantification (réduction d'entropie)
entropy_quantized = calculate_entropy(np.abs(quantized))

print(f"Entropie initiale : {entropy_initial:.2f} bits/symbole")
print(f"Entropie après compression : {entropy_quantized:.2f} bits/symbole")
print(f"Gain de 'distance d'unicité' théorique : {((1 - entropy_quantized/entropy_initial)*100):.1f}%")

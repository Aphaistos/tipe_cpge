import numpy as np

def cooley_tukey_fft(x):
    """
    Une implémentation récursive de la FFT 1D.
    L'entrée x doit avoir une taille puissance de 2.
    """
    N = len(x)
    
    # Cas de base : la FFT d'un seul point est le point lui-même
    if N <= 1:
        return x
    
    # Division : on sépare les échantillons pairs et impairs
    even = cooley_tukey_fft(x[0::2])
    odd =  cooley_tukey_fft(x[1::2])
    
    # Combinaison (le "Papillon")
    # Calcul des facteurs de rotation (Twiddle Factors) : e^(-2j * pi * k / N)
    T = [np.exp(-2j * np.pi * k / N) * odd[k] for k in range(N // 2)]
    
    # On concatène les résultats
    return np.array([even[k] + T[k] for k in range(N // 2)] + \
                    [even[k] - T[k] for k in range(N // 2)])

# Exemple d'utilisation
if __name__ == "__main__":
    # Génération d'un signal de test (ex: sinus à 50Hz)
    fs = 8000  # Fréquence d'échantillonnage type téléphonie
    t = np.linspace(0, 0.032, 256) # Fenêtre de 32ms (256 points)
    signal = np.sin(2 * np.pi * 50 * t)
    
    fft_result = cooley_tukey_fft(signal)
    print(f"Calcul terminé pour {len(fft_result)} points.")

import pandas as pd
import matplotlib.pyplot as plt

# Charger les données générées par le code C
data = pd.read_csv("latence.csv")

# Graphique 1 : Évolution temporelle (Met en évidence le Jitter)
plt.figure(figsize=(10, 4))
plt.plot(data['Temps_Ecoule_s'], data['Latence_ms'], color='blue', alpha=0.6)
plt.title("Mesure temporelle du Jitter (Gigue numérique) en boucle locale")
plt.xlabel("Temps écoulé (secondes)")
plt.ylabel("Latence (ms)")
plt.grid(True)
plt.savefig("jitter_temporel.png")

# Graphique 2 : Histogramme de distribution (Analyse statistique)
plt.figure(figsize=(10, 4))
plt.hist(data['Latence_ms'], bins=50, color='crimson', edgecolor='black', alpha=0.7)
plt.title("Distribution statistique de la latence logicielle")
plt.xlabel("Latence (ms)")
plt.ylabel("Nombre de paquets")
plt.grid(True)
plt.savefig("histogramme_latence.png")

plt.show()
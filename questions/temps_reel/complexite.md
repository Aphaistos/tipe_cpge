[Revenir sur la page principale](https://github.com/Aphaistos/tipe_cpge/)
# L'impact de la complexité algorithmique sur la latence ($T_{traitement}$)

Un algorithme de chiffrement est dit **"lourd"** s'il nécessite un grand nombre d'opérations élémentaires (additions, multiplications, itérations) pour transformer un bloc de données. En informatique théorique, cela se mesure par sa **complexité temporelle**.

### Le lien entre Complexité et Latence
Si l'on note $N$ la taille des données à chiffrer (un échantillon audio) et $f(N)$ la complexité de l'algorithme, le temps de traitement est proportionnel au nombre d'opérations :
$$T_{traitement} \approx \frac{k \cdot f(N)}{P_{cpu}}$$
Où $P_{cpu}$ est la puissance de calcul (opérations par seconde) et $k$ une constante liée à l'implémentation.

- Si l'algorithme a une complexité élevée (ex: $O(N^3)$ ou exponentielle), le temps de calcul peut dépasser la durée de l'échantillon audio lui-même.
- **Le lag perceptible :** Si $T_{traitement} > \text{durée de l'échantillon}$, le processeur ne finit pas de chiffrer le premier paquet avant que le suivant n'arrive. Les données s'accumulent dans une file d'attente (buffer), créant un retard qui croît indéfiniment.

**Conclusion :** Un algorithme trop lourd introduit un lag dès que sa complexité sature les ressources du processeur. L'enjeu du TIPE est de prouver qu'une méthode offre un haut niveau de sécurité tout en restant dans une classe de complexité $O(N)$ permettant le temps réel.

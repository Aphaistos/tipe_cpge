[Revenir sur la page principale](https://github.com/Aphaistos/tipe_cpge/) \
[Revenir sur la page des questions](../questions.md)


# Quelles sont les limites mathématiques de la compression ?

La limite absolue de la compression est dictée par la **[théorie de l'information de Claude Shannon](../../documents/bibnum-1190.pdf)**. Elle se divise en deux aspects selon que l'on accepte ou non une dégradation du signal :

### La limite sans perte (Entropie)

Mathématiquement, un message ne peut pas être compressé en dessous de son entropie $H(X)$, définie par la formule :

$$H(X) = - \sum_{i} p_i \log_2(p_i)$$

où $p_i$ est la probabilité d'apparition d'un symbole (ou d'un échantillon audio). Si l'on descend sous cette borne, il est mathématiquement impossible de retrouver le message original (perte d'information irréversible).

### La limite avec perte (Théorie du débit-distorsion)

Pour la voix, on utilise la compression "avec perte". La limite n'est plus seulement l'entropie, mais le compromis entre le débit binaire ($R$) et la distorsion acceptable ($D$). Shannon a démontré qu'il existe une fonction $R(D)$ qui donne le débit minimal pour une erreur de reconstruction donnée.

# Jusqu'où peut-on réduire la taille avant que le message ne devienne inaudible ?

Dans le cadre d'une communication vocale, la limitation maximale de la taille est atteinte en exploitant deux types de limites []

### La limite fréquentielle (Nyquist-Shannon)

Pour que la voix reste intelligible, il faut conserver une bande passante minimale. La voix humaine utile en téléphonie se situe entre 300 Hz et 3400 Hz. Selon le théorème d'échantillonnage, il faut échantillonner à au moins $2 \times f_{max}$ (soit environ 8 kHz). En dessous, le phénomène de repliement spectral (aliasing) rend le son métallique et incompréhensible.

### La limite psychoacoustique (Seuil d'audibilité)

Le message devient "inaudible" (ou plutôt inintelligible) quand le **bruit de quantification** (l'erreur introduite par la réduction de précision) dépasse l'énergie des formants de la voix. Les codecs modernes comme Code-Excited Linear Prediction (CELP), utilisés dans nos téléphones, permettent de descendre jusqu'à 4,8 kbit/s voire 2,4 kbit/s en ne transmettant que les paramètres du conduit vocal plutôt que l'onde elle-même.


<ins>**Pour conclure**</ins>: La limite de sobriété pour la voix est d'environ 1,2 à 2,4 kbit/s. En dessous de ce seuil, les algorithmes ne parviennent plus à modéliser les caractéristiques phonétiques humaines, et le signal devient un bruit dénué de sens sémantique.
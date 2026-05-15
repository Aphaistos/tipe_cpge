[Revenir sur la page principale](https://github.com/Aphaistos/tipe_cpge/) \
[Revenir sur la page des questions](../questions.md)

# Quel algorithme choisir pour compresser le flux audio?
Pour compresser un flux audio dans un système de communication en temps réel, le choix de l'algorithme doit arbitrer le conflit entre taux de compression (réduction de l'entropie) et latence algorithmique. 

# Solutions
Si le codage de Huffman ou le Lempel-Ziv offrent une compression sans perte, leur efficacité sur un signal source brut est trop faible pour de la téléphonie. 
À l'inverse, des codecs comme le MP3 (utilisant la MDCT) offrent des taux élevés mais introduisent une latence de fenêtrage prohibitive pour le bidirectionnel.
Le choix optimal se porte alors sur des transformées de la famille de la FFT de Cooley-Tukey ou, plus spécifiquement pour l'audio, la Transformée en Cosinus Discrète (DCT) intégrée dans des codecs à faible latence comme Opus (CELT/SILK) ou G.722.

L'intérêt de privilégier une approche basée sur Cooley-Tukey réside dans sa capacité à basculer le signal dans le domaine fréquentiel avec une complexité quasi-linéaire en $O(N \log N)$ ([Voir preuve](./preuve_complexite_cooley-turkey.md)). Cela permet d'appliquer instantanément un modèle psychoacoustique pour supprimer les composantes spectrales inaudibles (masquage). On choisit cette solution car elle seule permet de réduire drastiquement l'entropie de la source — facilitant ainsi un chiffrement robuste ultérieur — tout en maintenant le délai de traitement $T_{traitement}$ largement en dessous du seuil critique des 150 ms, garantissant ainsi la fluidité indispensable à un échange sécurisé.

## Synthèse des solutions de compression

| Algorithme / Codec | Type | Latence | Usage idéal |
| ---- | ---- | ---- | ---- |
| Huffman | Sans perte | Très faible | Réduction finale après quantification. |
| MP3 (MDCT) | Avec perte | Élevée (>100ms) | Stockage et streaming unidirectionnel. |
| Opus (FFT-based) | Hybride | Très faible (5-20ms) | Téléphonie IP et VoIP sécurisée. |
| G.711 (Log-PCM) | Temporel | Nulle | Téléphonie classique (mais faible compression). |

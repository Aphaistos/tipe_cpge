[Revenir sur la page principale](https://github.com/Aphaistos/tipe_cpge/) \
[Revenir sur la page des questions](./questions.md)

# 2. Protocole de comparaison : FFT (Cooley-Tukey) + SRTP vs ChaCha20-Poly1305

Pour la démarche expérimentale, on peut confronter les chaînes de traitement sur trois critères mesurables :

## A. Le budget de latence temps réel (Contrainte des 150 ms)

En téléphonie (recommandation UIT-T G.114), la latence totale de bout en bout ne doit pas dépasser 150 ms. Pour une fenêtre audio de N = 1024 échantillons à 48 kHz, la durée d'acquisition est de ≈ 21,3 ms. Le traitement (FFT + Crypto + Réseau + IFFT) doit tenir dans une fraction de ce temps.
- ChaCha20-Poly1305 seul : ChaCha20 repose sur des opérations ARX (Add-Rotate-XOR) sur des mots de 32 bits, parfaitement alignées avec les registres des processeurs ARM des smartphones. Le temps de chiffrement est quasi nul devant le temps de calcul de la FFT en place.
- SRTP (ex. avec AES-GCM ou ChaCha20-Poly1305) : SRTP ajoute une étape de dérivation de clé et de construction d'en-tête. Sur un PC (architecture x86 avec instructions matérielles AES-NI), SRTP + AES est ultra-rapide. Sur un smartphone sans accélération matérielle AES, SRTP configuré avec ChaCha20-Poly1305 conserve l'avantage de rapidité.

## B. La robustesse aux erreurs de transmission (Sensibilité des coefficients FFT)

Une trame compressée après Cooley-Tukey contient des coefficients spectraux quantifiés (les fréquences dominantes).
- Avec ChaCha20-Poly1305 nu : Si vous chiffrez le flux en continu sans mécanisme de resynchronisation réseau, la perte d'un seul paquet UDP décale le compteur de blocs (le nonce). À la réception, le déchiffrement échoue et l'algorithme de Cooley-Tukey inverse (IFFT) transforme ce bruit aléatoire en un signal audio saturé et inaudible sur toute la fin de l'appel.
- Avec SRTP : SRTP utilise le numéro de séquence du paquet RTP pour isoler chaque trame FFT. Si un paquet est perdu, Poly1305 (ou le tag MAC de SRTP) rejette le paquet corrompu, mais la trame FFT suivante est déchiffrée normalement. L'impact se limite à une micro-coupure de 21,3 ms.

## C. La vulnérabilité à l'analyse de trafic (Effet de la compression spectrale)

Après la FFT de Cooley-Tukey, la compression consiste généralement à annuler les coefficients sous un certain seuil (seuillage spectral) et à encoder les valeurs restantes.
- Le problème : Pendant un silence, presque tous les coefficients sont nuls ; le paquet compressé est minuscule. Pendant une parole riche en harmoniques, le paquet est volumineux.
- Comparaison : ChaCha20-Poly1305 chiffre le contenu mais conserve exactement la longueur du message d'entrée : un espion peut reconstituer l'enveloppe de la parole en mesurant la taille des paquets. SRTP permet d'activer le remplissage (padding) au niveau protocole pour masquer ces variations liées à la FFT.

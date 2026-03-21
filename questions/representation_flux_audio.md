[Revenir sur la page principale](https://github.com/Aphaistos/tipe_cpge/) \
[Revenir sur la page des questions](./questions.md)

# Comment représenter numériquement un flux audio?

## Définition
  Le son est une vibration, une variation de la pression de l'air par rapport à la pression atmosphérique. Pour traiter cette information en informatique, on transforme ce signal analogique continu en une suite finie de **valeurs discrètes**. Dans ce TIPE, on va s'intéresse plus particulièrement à un signal sonore provenant d'une voix humaine.

## Processus de numérisation
  Ce processus de numérisation repose sur **l'échantillonnage (Sampling)** et **la quantification (Quantization)**.

### L'échantillonnage (Sampling)
  On prélève la valeur du signal à intervalles de temps réguliers $T_s$. 
<ins>Selon le théorème de Shannon-Nyquist</ins>, pour reconstruire fidèlement un signal de fréquence maximale $f_{max}$, la fréquence d'échantillonnage $f_s = \frac{1}{T_s}$ doit vérifier $f_s > 2 f_{max}$. Pour la voix humaine, une fréquence de $8\,000$ Hz à $16\,000$ Hz est généralement suffisante.

### La quantification (Quantization) : 
  On approche la valeur de l'amplitude échantillonnée par l'élément le plus proche d'un ensemble fini de valeurs (codage sur $n$ bits).

## Contraintes de données
  La représentation brute d'un flux audio (format PCM - Pulse Code Modulation) génère une quantité massive de données. Le débit binaire ($D$) exprime la quantité d'information transmise par unité de temps :
  $$D = f_s \times n \times c$$
- $f_s$ : Fréquence d'échantillonnage (ex: 8 000 Hz pour la voix, 44 100 Hz pour la musique).
- $n$ : Résolution (ex: 16 bits).
- $c$ : Nombre de canaux (1 pour mono, 2 pour stéréo).
La compléxité spatiale d'une telle représentation est en $O(T \cdot f_s)$ pour une durée $T$. Sans compression, une minute d'audio haute fidélité occupe environ 10 Mo, ce qui est inefficace pour une transmission temps réel sur des réseaux contraints.

## Enjeux de la compression et de la complexité
  Pour respecter la contrainte de latence de 150 ms ([voir contrainte de communication](./temps_reel/latence.md)), la compression est indispensable. Elle réduit $D$ pour diminuer le temps de transmission réseau ($T_{réseau}$), mais elle ajoute une charge de calcul ($T_{traitement}$). Ainsi, l'algorithme de compression doit s'exécuter en un temps $t < T_{budget}$ pour éviter de hacher le flux.

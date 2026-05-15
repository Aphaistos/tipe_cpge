[Revenir sur la page principale](https://github.com/Aphaistos/tipe_cpge/) \
[Revenir sur la page des questions](../questions.md)

# Preuve de la complexité de l'algorithme de Cooley-Tukey (Radix-2)

Soit $N = 2^n$ la taille du signal d'entrée. On note $C(N)$ le nombre d'opérations élémentaires (multiplications et additions complexes) nécessaires au calcul de la DFT.

## 1. Version Récursive (Approche Divide and Conquer)

**Principe :** L'algorithme repose sur la décomposition d'une DFT de taille $N$ en deux DFT de taille $N/2$ (indices pairs et impairs).

**Établissement de la relation de récurrence :**

1. **Division :** Le calcul des deux sous-DFT de taille $N/2$ requiert $2 \times C(N/2)$ opérations.
2. **Synthèse (Papillons) :** Pour reconstruire le vecteur final, on applique pour chaque $k \in \{0, \dots, N/2-1\}$ l'opération de papillon :
* $X_k = E_k + \omega^k O_k$
* $X_{k+N/2} = E_k - \omega^k O_k$
Chaque papillon nécessite 1 multiplication complexe (par $\omega^k$) et 2 additions/soustractions complexes. Pour les $N/2$ papillons, on a donc un coût linéaire en $O(N)$.



On obtient la relation :


$$C(N) = 2C\left(\frac{N}{2}\right) + \beta N \quad (\text{où } \beta \text{ est une constante})$$

**Résolution :**
Par itération de la récurrence pour $N = 2^n$ :


$$C(2^n) = 2C(2^{n-1}) + \beta 2^n$$


En divisant par $2^n$ :


$$\frac{C(2^n)}{2^n} = \frac{C(2^{n-1})}{2^{n-1}} + \beta$$


La suite $u_n = \frac{C(2^n)}{2^n}$ est une suite arithmétique de raison $\beta$. On a donc :


$$u_n = u_0 + n\beta \implies \frac{C(N)}{N} = \frac{C(1)}{1} + \beta \log_2(N)$$


D'où :


$$C(N) = O(N \log_2 N)$$

---

## 2. Version Itérative (Approche Bottom-Up)

**Principe :** L'algorithme itératif traite le signal en place après une permutation par inversion de bits (*bit-reversal*). Le calcul s'effectue par "étages".

**Dénombrement des opérations :**
L'algorithme se structure en trois niveaux d'itérations :

1. **Nombre d'étages :** Pour atteindre la taille $N = 2^n$, on doit fusionner les blocs de taille $2^1, 2^2, \dots, 2^n$. Il y a donc exactement $n = \log_2 N$ étages de calcul.
2. **Traitement par étage :** À chaque étage $s \in \{1, \dots, n\}$, on traite l'intégralité des $N$ données du tableau. Les données sont groupées par paires pour former des papillons.
3. **Nombre de papillons :** Puisque chaque papillon prend 2 entrées et produit 2 sorties, il y a exactement $N/2$ opérations de papillon par étage.

**Calcul final :**
Le coût total est le produit du nombre d'étages par le coût d'un étage :


$$C(N) = (\text{Nombre d'étages}) \times (\text{Nombre de papillons par étage}) \times (\text{Coût d'un papillon})$$

$$C(N) = \log_2 N \times \frac{N}{2} \times K$$


Où $K$ est le nombre d'opérations dans un papillon (constant).

On en conclut la complexité temporelle :


$$C(N) = O(N \log_2 N)$$
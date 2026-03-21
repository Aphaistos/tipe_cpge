[Revenir sur la page principale](https://github.com/Aphaistos/tipe_cpge/)

# Questions du sujet

## Sur le "temps réel"
- [Quelle est la limite de temps acceptable pour qu'une conversation reste fluide ?](./temps_reel/latence.md)
- [Quel est l'impact de la complexité temporelle de l'algorithme sur la latence ?](./temps_reel/complexite.md)
- En streaming vocal, si un "paquet" de données est perdu ou arrive en retard, comment l'algorithme doit-il réagir pour ne pas casser la communication ?

## Sur la compression
- [Quelles sont les limites mathématiques de la compression ? Jusqu'où peut-on réduire la taille des données avant que le message ne devienne inaudible ?](./compression/limites.md)
- [Comment représenter numériquement un flux audio?](./representation_flux_audio.md)
- Comment mesurer la quantité d'information réelle dans un signal vocal pour supprimer les redondances ?

## Sur l'architecture de la sécurité
- On attend d'avoir un "paquet" de données complet pour le chiffrer. Est-ce compatible avec le temps réel ?
- On chiffre les bits un par un au fur et à mesure qu'ils arrivent. Pourquoi est-ce théoriquement plus adapté à la voix ?

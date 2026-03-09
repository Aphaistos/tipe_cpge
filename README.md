# TIPE 2027 - (Efficacité, Sobriété et Optimisation)
## *Comment sécurisé une communication vocal en temps réels ? (Compressions audios, Chiffrements de flux)*
- **Matières**: Mathématique, et Informatique

[Découvrir la cryptographie](https://www.bibmath.net/crypto/)

## Introduction
## Questions à
### Sur la condition "temps réel"
- [Quelle est la limite de temps acceptable pour qu'une conversation reste fluide ?](./questions/temps_reel/latence.md)
- [Quel est l'impact de la complexité temporelle de l'algorithme sur la latence ?](./questions/temps_reel/complexite.md)
- En streaming vocal, si un "paquet" de données est perdu ou arrive en retard, comment l'algorithme doit-il réagir pour ne pas casser la communication ?
### Sur la compression
- Quelles sont les limites mathématiques de la compression ? Jusqu'où peut-on réduire la taille des données (sobriété) avant que le message ne devienne inaudible ?
- Comment mesurer la quantité d'information réelle dans un signal vocal pour supprimer les redondances ?
### Architecture de la Sécurité
- On attend d'avoir un "paquet" de données complet pour le chiffrer. Est-ce compatible avec le temps réel ?
- On chiffre les bits un par un au fur et à mesure qu'ils arrivent. Pourquoi est-ce théoriquement plus adapté à la voix ?

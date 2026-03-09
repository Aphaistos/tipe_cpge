[Revenir sur la page principale](https://github.com/Aphaistos/tipe_cpge/)
# Quelle est la limite de temps acceptable pour qu'une conversation reste fluide ?

La recommandation [**ITU-T G.114**](../../documents/T-REC-G.114-200305-I!!PDF-E.pdf) est la norme de référence mondiale pour la qualité de service (QoS) de la voix sur IP (VoIP). Elle définit trois paliers de latence (délai aller simple) :

- <u>0 à 150 ms : **Qualité excellente**.</u> Le délai est imperceptible pour l'utilisateur moyen. C'est l'objectif de l'optimisation pour un système de communication moderne.
- <u>150 à 400 ms : **Qualité acceptable**.</u> La conversation reste possible, mais les utilisateurs commencent à s'interrompre mutuellement (phénomène de collision de parole).
- <u>Au-dessus de 400 ms : **Inacceptable**<u>. La communication devient pénible.

On cherche à avoir une communication claire, on va donc fixer la limite maximale de latence à **150 ms**.

La latence de notre système de communication se décompose en la somme de plusieurs facteurs d'optimisation:

$$T_{total} = T_{captation} + T_{traitement} + T_{réseau} + T_{restitution}$$

Ce TIPE s'intéresse à la réduction du **délaie de traitement ($T_{traitement}$)** et du **délai de réstitution ($T_{restitution}$)**.

Le **délai de traitement** est le temps qu'il faut, pour la machine qui envoie le signal audio, à compresser puis chiffrer le flux audio.

Le **délai de réstitution** est le temps qu'il faut, pour la machine qui reçoit le signal audio, à déchiffrer puis décompresser le flux audio.

---
Au-delà de la latence pure, la **fluidité** dépend de la régularité. Si le délai de traitement varie d'un paquet à l'autre, le son devient haché. __**L'optimisation consiste donc à minimiser la latence tout en garantissant une stabilité temporelle.**__

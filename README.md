# TIPE 2027 - (Efficacité, Sobriété et Optimisation)
## *Comment sécuriser une communication vocale en temps réels ? (Compressions audios, Chiffrements de flux)*
- **Matières**: Mathématiques et Informatique
- **MCOT**: [Voir document](./MCOT-MAGIMEL-AURELIEN.pdf)
- **Questions**: [Voir la page](./questions/questions.md)
- **Document de départ**: [Comprendre la compression du son](./documents/Potiron.pdf)

## Présentation
Ce projet propose une implémentation de la chaîne de sécurisation de la parole en temps réel, articulée autour de la réduction de l'entropie par transformée de Fourier (FFT) et de l'analyse des contraintes de latence déterministe sur systèmes embarqués.

## Démarche Scientifique

Le point de départ de ce projet repose sur les travaux de Claude Shannon concernant la théorie de l'information. Initialement, l'objectif était de minimiser la redondance d'un flux audio pour atteindre un débit proche de l'entropie de la source, garantissant ainsi une efficacité de transmission maximale. Ma première approche s'est naturellement tournée vers le codage statistique de Huffman. En traitant le signal dans le domaine temporel, j'ai pu observer une réduction de l'entropie, mais celle-ci restait limitée par la structure intrinsèque de la voix humaine : les échantillons successifs demeurent fortement corrélés, laissant subsister une redondance structurelle que le codage de Huffman seul ne peut briser sans perte d'information majeure.

Cette constatation a conduit à un pivotement vers le domaine fréquentiel. En introduisant l'algorithme de Cooley-Tukey (FFT), l'idée était de décomposer le signal en ses composantes spectrales pour appliquer une quantification basée sur des critères psychoacoustiques. Cette transition change radicalement la perspective : au lieu de coder des amplitudes temporelles redondantes, on traite des coefficients fréquentiels décorrélés. Cette démarche a permis non seulement de diviser l'entropie par trois, mais aussi d'augmenter significativement la distance d'unicité du système. Ce passage de la "compression statistique" à la "transformation fréquentielle" constitue le cœur de ma recherche, permettant de transformer un signal audio complexe en un flux de données dense et imprévisible, idéal pour un chiffrement robuste et fluide.

## Remerciments

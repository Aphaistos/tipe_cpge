# Enjeux
### === Confidentialité ===
* Les communications ne doivent jamais être lisibles par un tiers, même le serveur de signalisation.
* Protection contre :
	- Sniffing réseau (Wi-Fi, Internet, 4G)
	- Compromission du serveur
	- Attaques actives (MITM, injection de paquets)
* <u>Objectif</u> : chiffrement bout-à-bout (E2EE), même si le transport est compromis

### === Intégrité ===
* Les données audio ne doivent pas pouvoir être modifiées sans détection.
* Nécessite :
- Authentification des messages (HMAC, signatures numériques)
- Vérification de chaque paquet audio ou signalisation
* <u>Objectif</u> : détecter et rejeter les altérations.

### === Authenticité ===
* Chaque terminal doit vérifier l’identité de son interlocuteur avant de commencer la communication.
* Solution connue: identité cryptographique longue durée (Ed25519) + empreinte humaine pour éviter la substitution.

### === Anonymat / confidentialité des métadonnées ===
* Même si le contenu est chiffré, le serveur ou un observateur réseau pourrait déduire qui parle à qui, quand, combien de temps.
* Solutions connues :
	- Identifiants opaques ou éphémères
	- Serveur aveugle (store-and-forward)
	- Minimisation des métadonnées (horodatage, fréquence)
* <u>Objectif</u> : protéger la relation sociale et opérationnelle, pas seulement le contenu.

### === Disponibilité / résilience ===
* Le système doit fonctionner même si :
	- Un serveur est compromis ou indisponible
	- Les terminaux sont derrière NAT ou firewall
* Solutions connues:
	- WebRTC pour NAT traversal
	- Mode peer-to-peer privilégié
	- Gestion de reconnexion et retry
* <u>Objectif</u> : aucune défaillance réseau n’empêche l’échange sécurisé.

### === Forward secrecy et post-compromise security ===
* **Forward secrecy :** compromise d’une clé aujourd’hui ne doit pas permettre de lire des communications passées.
* **Post-compromise security :** compromise temporaire d’un terminal ne doit pas compromettre indéfiniment la communication.
* Solutions connues:
	- Ratcheting (Signal Protocol)
	- Rotation périodique des clés
*  <u>Objectif</u> : limiter l’impact d’une compromission.

### === /!\ Contraintes techniques mobiles /!\ ===
* Limitations CPU / batterie → pas de chiffrement inutilement lourd
* Latence minimale pour un appel fluide
* Stockage sécurisé → Android Keystore / hardware-backed

# Restrictions

- **Identité:** *Clé publique unique par terminal, générée localement*
- **Signalisation:** *Serveur aveugle, aucune clé persistante ni mapping identité → utilisateur*
- **Transport:** *WebRTC ou autre canal NAT-friendly, mais considéré comme non fiable pour la crypto*
- **Méta-données: ** *Minimiser / anonymiser, ne jamais stocker numéro ou ID permanent*
- **Audio:** *Chiffrement applicatif, DSP intact (AEC/AGC), pas de chiffrement PCM brut*
- **Keys:** *Rotation régulière, destruction mémoire immédiate*
- **Scalabilité:** *Limiter l’accès aux contacts autorisés, pas d’appel “ouvert” vers tout le monde*
- **Sécurité opérationnelle:** *Aucun appel automatique sans authentification / vérification d’identité*
- **Testabilité:** *Possibilité de valider le système et de simuler des attaques*

# Cahier des charges:
1. Échange sécurisé bout-à-bout
2. Authentification et ratcheting
3. Indépendance du serveur
4. Anonymat et minimisation des métadonnées
5. Résilience réseau et performance mobile
6. Forward secrecy + post-compromise security
7. Contrôle strict de la liste de contacts

En résumé :
	Un système où le serveur est simple transporteur aveugle, chaque terminal est responsable de sa crypto, et l’identité est strictement cryptographique et locale.
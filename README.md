# Laboratoire 2 — Multithreading et synchronisation

**Cours :** 8INF342 — Systèmes d'exploitation  
**Université :** Université du Québec à Chicoutimi (UQAC)  
**Session :** Automne 2026  
**Professeure :** Sara Séguin  
**Étudiant :** Naoufel Amiar

---

## 1. Présentation du projet

Ce dépôt contient le travail réalisé dans le cadre du laboratoire 2 du cours **8INF342 — Systèmes d'exploitation**.

L'objectif est de développer un système de fabrication d'objets en C++ utilisant plusieurs threads et des mécanismes de synchronisation.

Le programme simule la création d'objets pour un jeu vidéo. Chaque objet nécessite deux matériaux, dont un seul exemplaire est disponible à la fois.

Le laboratoire consiste principalement à gérer les accès concurrents aux ressources partagées, tout en évitant les interblocages (*deadlocks*) et les situations de famine (*starvation*).

Le programme initial et son interface graphique X11 sont fournis par la professeure. Le développement porte essentiellement sur la fonction `create(int i, craft_t *c)`.

## 2. Objectifs techniques

- Utiliser les threads C++ avec `std::thread`.
- Mettre en œuvre plusieurs mutex.
- Synchroniser les accès aux matériaux partagés.
- Réserver simultanément les deux matériaux nécessaires à chaque objet.
- Gérer l'attente des threads avec `std::condition_variable`.
- Mettre en place une politique de priorité pour éviter la famine.
- Simuler un temps de fabrication de 200 millisecondes.
- Produire au minimum dix objets de chaque type.
- Enregistrer les événements dans un fichier texte.
- Mesurer les durées individuelles et celles des cycles de fabrication.

## 3. Objets et matériaux

Le programme contient sept types d'objets, chacun associé à un thread.

| Thread | Objet | Matériau 1 | Matériau 2 |
|---|---|---|---|
| 0 | Coupe | Argent | Or |
| 1 | Porte | Or | Bois |
| 2 | Table | Bois | Métal |
| 3 | Bague | Métal | Diamant |
| 4 | Chandelier | Diamant | Fer |
| 5 | Épée | Fer | Jade |
| 6 | Bracelet | Jade | Argent |

Les matériaux sont stockés dans une liste partagée au sein de la structure `craft_t`.

Chaque thread doit obtenir ses deux matériaux avant de commencer sa fabrication. Après 200 ms, il restitue les matériaux pour permettre aux autres threads de progresser.

## 4. Environnement de développement

Le projet est développé et testé dans une machine virtuelle Ubuntu fournie pour le cours.

**Environnement utilisé :**

- Ubuntu 22.04 LTS
- Visual Studio Code
- G++ 11.4
- GDB 12.1
- Git et GitHub
- Bibliothèque graphique X11
- Bibliothèque standard C++ pour les threads et mutex
- Valgrind et Helgrind pour les diagnostics

### Installation des dépendances

```bash
sudo apt update
sudo apt install build-essential gdb git libx11-dev valgrind
```

### Récupération du dépôt

```bash
git clone https://github.com/Naoufel-Amiar/TD_Labo2_SE.git

cd TD_Labo2_SE
```

## 5. Compilation et exécution

### Compilation manuelle

```bash
g++ -g -O0 labo2_etudiants.cpp -o labo2_etudiants -pthread -lX11
```

### Lancement du programme

```bash
./labo2_etudiants
```

Une fenêtre graphique intitulée `Lab 2 - 8INF342` apparaît.

### Commandes disponibles

| Touche | Action |
|---|---|
| `s` | Exécuter un cycle de fabrication |
| `r` | Rafraîchir l'affichage |
| `t` | Réinitialiser l'affichage des ressources |
| `q` | Quitter le programme |

Il faut lancer dix cycles avec `s` pour obtenir les 70 objets dans la version développée.

### Utilisation de Visual Studio Code

Le dépôt contient les fichiers de configuration dans `.vscode/`.

- **Ctrl + Maj + B** : compilation automatique.
- **F5** : compilation et lancement du débogueur GDB.
- **Points d'arrêt** : inspection des variables et du fonctionnement des threads.

## 6. Architecture de synchronisation

La synchronisation repose sur trois mutex et une variable conditionnelle.

### `mutexMateriaux`

Protège les accès aux matériaux partagés, aux compteurs de fabrication et aux informations de priorité des threads.

La vérification de disponibilité et la réservation des deux matériaux sont exécutées dans une même section critique.

Un thread ne conserve jamais un seul matériau pendant qu'il attend le second.

### `mutexFichier`

Protège les écritures dans le terminal et dans `sortieCreation.txt`.

Cela évite que plusieurs threads écrivent simultanément leurs messages et mélangent les informations.

### `mutexChrono`

Protège les variables utilisées pour mesurer la durée des cycles.

Le premier thread à entrer dans `create()` démarre la mesure. Le dernier thread à terminer permet de calculer la durée du cycle.

### Variable conditionnelle

`std::condition_variable` permet de suspendre un thread lorsque ses matériaux ne peuvent pas être réservés.

Les threads sont notifiés lors des changements de disponibilité et réévaluent leur condition de fabrication sous mutex.

### Gestion des priorités

Chaque demande reçoit un ticket croissant.

Les demandes plus anciennes sont prioritaires lorsqu'elles entrent en conflit avec une demande plus récente.

Cette politique vise à limiter les risques de famine, tout en permettant à des fabrications utilisant des matériaux indépendants de progresser simultanément.

## 7. Journalisation et chronométrage

Le fichier `sortieCreation.txt` enregistre les événements de fabrication.

Exemples de messages :

```text
Coupe en creation.
Porte en attente de Or.
Coupe cree : 1
Coupe - duree de fabrication : 200 ms
Cycle 1 - duree : 609 ms
Temps total cumule : 609 ms
```

Les mesures sont effectuées avec `std::chrono::steady_clock`.

Le temps cumulé correspond à la somme des durées mesurées pendant les cycles. Il n'inclut pas les pauses entre les interventions de l'utilisateur.

Le fichier est ouvert en mode ajout : les exécutions successives peuvent donc être conservées dans le même journal.

## 8. Résultats expérimentaux

Trois exécutions complètes ont été réalisées sur la machine virtuelle Ubuntu.

Chaque essai a permis de fabriquer 70 objets, soit dix exemplaires de chacun des sept types.

| Essai | Objets fabriqués | Cycles | Temps cumulé |
|---|---:|---:|---:|
| Test 1 | 70 | 10 | 8 277 ms |
| Test 2 | 70 | 10 | 9 092 ms |
| Test 3 | 70 | 10 | 7 467 ms |
| **Moyenne** | **70** | **10** | **8 279 ms** |

Les différences de durée proviennent notamment de l'ordre d'exécution des threads et du degré de parallélisme obtenu selon les cycles.

Les fabrications individuelles ont présenté des durées proches de 200 ms, conformément à la temporisation programmée.

Les trois exécutions se sont terminées sans interblocage observé.

## 9. Vérifications avec Valgrind — Helgrind

Le programme a également été analysé avec **Helgrind**, l'outil de Valgrind spécialisé dans la détection de problèmes de synchronisation.

### Compilation de diagnostic

```bash
g++ -g -O0 labo2_etudiants.cpp -pthread -lX11 -o labo2_helgrind
```

### Exécution

```bash
valgrind --tool=helgrind \
    --history-level=full \
    --log-file=helgrind.log \
    ./labo2_helgrind
```

### Résultats

Un premier diagnostic a révélé 139 avertissements, principalement associés aux notifications des variables conditionnelles.

Après adaptation de l'ordre des opérations de notification et de déverrouillage, un nouvel essai a donné :

```text
ERROR SUMMARY: 0 errors from 0 contexts
```

Ce résultat correspond à zéro erreur non supprimée détectée durant cette exécution. Il ne constitue pas une preuve formelle de l'absence de toute erreur de concurrence ou de famine.

## 10. Organisation du dépôt

```text
TD_Labo2_SE/
│
├── .vscode/
│   ├── launch.json
│   └── tasks.json
│
├── labo2_etudiants.cpp
├── sortieCreation.txt
├── test1.txt
├── test2.txt
├── test3.txt
├── sortieCreation_reference.txt
├── .gitignore
└── README.md
```

**Description des fichiers :**

| Fichier | Fonction |
|---|---|
| `labo2_etudiants.cpp` | Code source du laboratoire |
| `sortieCreation.txt` | Journal de fabrication |
| `test1.txt` | Première exécution de validation |
| `test2.txt` | Deuxième exécution de validation |
| `test3.txt` | Historique du troisième essai, incluant plusieurs exécutions |
| `sortieCreation_reference.txt` | Fichier de référence actuellement vide |
| `.vscode/tasks.json` | Configuration de compilation |
| `.vscode/launch.json` | Configuration de débogage |
| `.gitignore` | Exclusion des exécutables et fichiers temporaires |

Les exécutables compilés ne sont pas nécessaires dans le dépôt, puisqu'ils peuvent être générés depuis les sources.

## 11. Limites et observations

Le fonctionnement multithread dépend de l'ordonnancement du système d'exploitation.

Les temps d'exécution peuvent donc varier, même lorsque le nombre d'objets produits reste identique.

Le système de tickets et la réservation simultanée permettent de structurer l'attente et d'éviter les conflits classiques liés aux ressources partagées.

Les résultats expérimentaux, ainsi que le diagnostic Helgrind, complètent l'analyse logique de la synchronisation sans remplacer une démonstration formelle de ses propriétés.

## 12. Référence

Sara Séguin, *8INF342 — Systèmes d'exploitation, Laboratoire 2 : Multithreading et synchronisation*, Université du Québec à Chicoutimi, automne 2026.

---

**Projet réalisé dans le cadre du cours 8INF342 — Systèmes d'exploitation, UQAC.**
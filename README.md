# Simulateur de Gestion de Fichier

## Description
Ce simulateur est un outil permettant de gérer des fichiers dans une mémoire secondaire simulée
 sous forme de blocs sur un disque. Chaque fichier est une collection de blocs, et les fichiers
  peuvent être manipulés selon différentes organisations.

### Fonctionnalités principales :
- Création de fichiers.
- Insertion de nouveaux éléments dans un fichier.
- Suppression logique ou physique d'un élément.
- Renommage d'un fichier.
- Suppression complète d'un fichier.
- initialization de la memoire secondaire.
-vider la memoire secondire.
-afficher l'etat des blocs.
- Gestion de fichiers selon 4 types de présentation :
  1. **Contigu ordonné** : Les enregistrements sont stockés dans des blocs contigus, triés.
  2. **Contigu non ordonné** : Les enregistrements sont stockés dans des blocs contigus, sans tri.
  3. **Chaîné ordonné** : Les blocs d'un fichier sont liés entre eux, et les enregistrements sont triés.
  4. **Chaîné non ordonné** : Les blocs d'un fichier sont liés entre eux, sans tri des enregistrements.

---

## Prérequis
- Un compilateur C (comme GCC).
- Un éditeur de texte pour modifier les fichiers si nécessaire.
- Système d'exploitation prenant en charge les fichiers binaires.

---

## Installation
1. Téléchargez les fichiers du projet.
2. Compilez le fichier principal avec la commande suivante :
   ```bash
   gcc simulateur.c -o simulateur
   ```
3. Assurez-vous que l'exécutable **simulateur** est créé dans le répertoire courant.

---

## Utilisation
Exécutez le programme avec la commande suivante :
```bash
./simulateur
```

Un menu interactif apparaîtra, offrant les options suivantes :

### 1. **Création d'un fichier**
- Choisissez le mode d'organisation :
  - **1.1** : Contigu non ordonné
  - **1.0** : Contigu ordonné
  - **0.1** : Chaîné non ordonné
  - **0.0** : Chaîné ordonné
- Entrez le nom du fichier et le nombre d'enregistrements.

### 2. **Insertion d'un nouvel élément**
- Sélectionnez un fichier existant.
- Entrez les informations du nouvel élément à insérer.
- Si de l'espace est disponible, l'élément est ajouté.

### 3. **Suppression d'un élément**
- **Suppression logique :** L'élément est marqué comme supprimé, mais reste physiquement présent.
- **Suppression physique :** L'élément est retiré du fichier, et l'espace est libéré.

### 4. **Renommage d'un fichier**
- Entrez l'ancien nom du fichier.
- Fournissez le nouveau nom souhaité.

### 5. **Suppression d'un fichier**
- Sélectionnez un fichier à supprimer.
- Tous ses blocs sont libérés, et ses métadonnées sont retirées.

---

## Organisation des fichiers
- Les fichiers sont représentés sous forme de blocs dans un disque simulé.
- Chaque type d'organisation a ses propres caractéristiques :
  - **Contigu** : Tous les blocs d'un fichier sont adjacents.
  - **Chaîné** : Les blocs d'un fichier sont liés entre eux, mais ne sont pas nécessairement adjacents.
  - **Ordonné** : Les enregistrements dans les blocs sont triés.
  - **Non ordonné** : Les enregistrements sont ajoutés sans tri.

---

## Exemple d'utilisation
1. Lancez le programme :
   ```bash
   ./simulateur
   ```
2. Choisissez une option dans le menu (par exemple, création d'un fichier).
3. Suivez les instructions affichées pour entrer les données nécessaires.

---

## Auteurs
- Tebani Hiba 
- Nadir Meroua
- Menasra Nour El Imene 
- Hariz Soumia
- Keteb Yousra
- Nadir Manel

## Licence
MIT License

Copyright (c) [2024] [SFSD TEAM]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


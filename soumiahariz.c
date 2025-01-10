#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Définir la structure fichiermetadonnes
typedef struct {
    char nom[50];
    int taille;
    int blocDebut;
} fichiermetadonnes;

// Définir les structures
typedef struct {
    int etat;
    int adrdebloc;
} TableLocation;

typedef struct {
    int typedebloc;
    union {
        struct {
            int nbrbloc;
            int nbrblocutil;
            TableLocation tablelocation[20];
        } allocation;
        struct {
            int nbrMetadonnees;
            int next;
            fichiermetadonnes T[20];
        } metadataTable;
    } content;
} Bloc;

// Implémentation fictive pour la vérification
bool verifierEspaceSuffisant(FILE* disque, int taille) {
    return true;
}

// Fonction pour mettre à jour la table d'allocation
void metajourtableallocation(FILE* disque, int blocIndex, int etat) {
    printf("Mise à jour de l'entrée de la table d'allocation pour le blocIndex %d avec l'état %d.\n", blocIndex, etat);
    Bloc buffer;
    buffer.typedebloc = 3;

    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    buffer.content.allocation.tablelocation[blocIndex].etat = etat;

    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    if (fwrite(&buffer, sizeof(Bloc), 1, disque) == 1) {
        printf("Table d'allocation mise à jour avec succès pour le blocIndex %d.\n", blocIndex);
    } else {
        printf("Erreur : Échec d'écriture dans le bloc 0.\n");
    }
}

// Fonction pour créer la table d'allocation
void CreeTableAllocation(FILE* disque) {
    printf("Création de la table d'allocation.\n");
    Bloc buffer;
    rewind(disque);

    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    buffer.typedebloc = 3;
    buffer.content.allocation.nbrbloc = 20;
    buffer.content.allocation.nbrblocutil = 3;

    for (int i = 0; i < 20; i++) {
        buffer.content.allocation.tablelocation[i].adrdebloc = i;
        buffer.content.allocation.tablelocation[i].etat = (i < 3) ? 1 : 0;
        printf("Entrée de la table d'allocation %d initialisée avec l'état %d.\n", i, buffer.content.allocation.tablelocation[i].etat);
    }

    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    if (fwrite(&buffer, sizeof(Bloc), 1, disque) == 1) {
        printf("Table d'allocation créée avec succès.\n");
    } else {
        printf("Erreur : Échec d'écriture dans le bloc 0.\n");
    }
}

// Fonction pour vider l'espace mémoire
void ViderMs(FILE* disque) {
    printf("Vidage de l'espace mémoire.\n");
    Bloc buffer = {0}; // Initialiser un bloc vide (toutes valeurs à 0/null)
    int nombreBlocs = 20; // Supposons 20 blocs pour simplifier

    for (int i = 0; i < nombreBlocs; i++) {
        fseek(disque, i * sizeof(Bloc), SEEK_SET);
        if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Erreur : Impossible de réinitialiser le bloc %d.\n", i);
            return;
        }
        printf("Bloc %d réinitialisé à NULL.\n", i);
    }

    printf("Tous les blocs ont été réinitialisés à NULL.\n");
}

// Fonction pour initialiser l'espace mémoire
void InitMs(FILE* disque, int nombreBlocs) {
    printf("Initialisation de l'espace mémoire avec %d blocs.\n", nombreBlocs);
    Bloc buffer = {0};

    // Initialiser le bloc 0 (table d'allocation)
    buffer.typedebloc = 3;
    CreeTableAllocation(disque);

    // Initialiser les blocs 1 et 2 (métadonnées)
    buffer.typedebloc = 1;
    buffer.content.metadataTable.nbrMetadonnees = 0;
    buffer.content.metadataTable.next = 2;
    fseek(disque, 1 * sizeof(Bloc), SEEK_SET);
    if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
        printf("Erreur : Échec d'écriture dans le bloc %d.\n", 1);
        return;
    } else {
        printf("Bloc de métadonnées %d initialisé avec succès.\n", 1);
    }

    buffer.content.metadataTable.next = -1;
    fseek(disque, 2 * sizeof(Bloc), SEEK_SET);
    if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
        printf("Erreur : Échec d'écriture dans le bloc %d.\n", 2);
        return;
    } else {
        printf("Bloc de métadonnées %d initialisé avec succès.\n", 2);
    }
}

int main() {
    printf("Ouverture du fichier disque.\n");
    FILE *disque = fopen("disque.dat", "rb+");
    if (disque == NULL) {
        disque = fopen("disque.dat", "wb+"); // Créer le fichier s'il n'existe pas
        if (disque == NULL) {
            printf("Erreur : Impossible de créer le fichier disque.\n");
            return 1;
        }
        printf("Fichier disque créé avec succès.\n");
    } else {
        printf("Fichier disque ouvert avec succès.\n");
    }

    // Initialiser l'espace mémoire
    InitMs(disque, 20);

    // Exemple de mise à jour de l'entrée de la table d'allocation
    metajourtableallocation(disque, 4, 1);

    // Exemple de vidage de l'espace mémoire
    ViderMs(disque);

    fclose(disque);
    printf("Fichier disque fermé.\n");
    return 0;
}

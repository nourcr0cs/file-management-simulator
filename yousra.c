#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure représentant un produit
typedef struct {
    int id; // Identifiant unique du produit
    char eleName[30]; // Nom du produit
    float price; // Prix du produit
    int nbr; // Quantité disponible
    char etat[30]; // État du produit ("disponible", "supprimé", etc.)
} produit;

// Structure contenant les métadonnées d'un fichier
typedef struct {
    char FName[20]; // Nom du fichier
    int nbrBloc; // Nombre de blocs dans le fichier
    int nbrRecord; // Nombre total d'enregistrements
    produit *adrele1; // Pointeur vers les enregistrements
    char OrgGlobale[10]; // Mode d'organisation globale
    char OrgInterne[12]; // Mode d'organisation interne
} MetaTOF;

// Structure pour représenter la position d'un enregistrement
typedef struct {
    int blocNbr; // Numéro du bloc
    int deplacement; // Position dans le bloc
} pos;

// Fonction pour créer un fichier
void creerFichier(MetaTOF *meta, const char *nom, int nbrRecords, const char *orgGlobale, const char *orgInterne) {
    // Copier le nom du fichier
    strcpy(meta->FName, nom);
    // Calculer le nombre de blocs nécessaires (6 enregistrements par bloc)
    meta->nbrBloc = (nbrRecords + 5) / 6;
    // Stocker le nombre total d'enregistrements
    meta->nbrRecord = nbrRecords;
    // Copier les modes d'organisation
    strcpy(meta->OrgGlobale, orgGlobale);
    strcpy(meta->OrgInterne, orgInterne);
    // Allouer la mémoire pour les enregistrements
    meta->adrele1 = (produit *)malloc(nbrRecords * sizeof(produit));
    printf("Fichier %s créé avec %d enregistrements.\n", meta->FName, meta->nbrRecord);
}

// Fonction pour charger un fichier
void chargerFichier(MetaTOF *meta) {
    // Allouer de la mémoire pour les enregistrements
    meta->adrele1 = (produit *)malloc(meta->nbrRecord * sizeof(produit));
    if (meta->adrele1) {
        printf("Blocs nécessaires alloués pour le fichier %s.\n", meta->FName);
    } else {
        printf("Erreur d'allocation pour le fichier %s.\n", meta->FName);
    }
}

// Fonction pour insérer un nouvel enregistrement
void insererEnregistrement(MetaTOF *meta, produit newProduit) {
	int i;
    for ( i = 0; i < meta->nbrRecord; i++) {
        // Trouver une position vide pour insérer le produit
        if (strcmp(meta->adrele1[i].etat, "vide") == 0) {
            meta->adrele1[i] = newProduit;
            printf("Enregistrement inséré à la position %d.\n", i);
            return;
        }
    }
    printf("Pas d'espace disponible pour insérer un nouvel enregistrement.\n");
}

// Fonction pour rechercher un enregistrement
pos rechercherEnregistrement(MetaTOF *meta, int id) {
    pos position = {-1, -1}; // Initialisation de la position
    int i ;
    for ( i = 0; i < meta->nbrRecord; i++) {
        if (meta->adrele1[i].id == id) {
            // Calculer le bloc et le déplacement
            position.blocNbr = i / 6;
            position.deplacement = i % 6;
            printf("Enregistrement trouvé au bloc %d, déplacement %d.\n", position.blocNbr, position.deplacement);
            return position;
        }
    }
    printf("Enregistrement avec ID %d non trouvé.\n", id);
    return position;
}

// Fonction pour supprimer un enregistrement (logique)
void supprimerLogique(MetaTOF *meta, int id) {
	int i;
    for ( i = 0; i < meta->nbrRecord; i++) {
        if (meta->adrele1[i].id == id) {
            // Marquer l'enregistrement comme supprimé
            strcpy(meta->adrele1[i].etat, "supprime");
            printf("Enregistrement ID %d marqué comme supprimé.\n", id);
            return;
        }
    }
    printf("Enregistrement avec ID %d non trouvé.\n", id);
}

// Fonction pour supprimer un enregistrement (physique)
void supprimerPhysique(MetaTOF *meta) {
    int index = 0; // Position pour réorganiser les enregistrements
    int i;
    for ( i = 0; i < meta->nbrRecord; i++) {
        if (strcmp(meta->adrele1[i].etat, "supprime") != 0) {
            meta->adrele1[index++] = meta->adrele1[i];
        }
    }
    // Mettre à jour le nombre d'enregistrements
    meta->nbrRecord = index;
    printf("Suppression physique terminée.\n");
}

// Fonction pour défragmenter un fichier
void defragmenter(MetaTOF *meta) {
    supprimerPhysique(meta);
    // Recalculer le nombre de blocs après la défragmentation
    meta->nbrBloc = (meta->nbrRecord + 5) / 6;
    printf("Défragmentation terminée. Nombre de blocs : %d.\n", meta->nbrBloc);
}

// Fonction pour renommer un fichier
void renommerFichier(MetaTOF *meta, const char *nouveauNom) {
    strcpy(meta->FName, nouveauNom);
    printf("Fichier renommé en %s.\n", meta->FName);
}

// Fonction pour supprimer un fichier
void supprimerFichier(MetaTOF *meta) {
    free(meta->adrele1); // Libérer la mémoire allouée
    printf("Fichier %s supprimé.\n", meta->FName);
}

// Fonction pour remplir automatiquement des enregistrements
void remplirAutomatiquement(MetaTOF *meta) {
	int i;
    for ( i = 0; i < meta->nbrRecord; i++) {
        meta->adrele1[i].id = i + 1;
        sprintf(meta->adrele1[i].eleName, "Produit %d", i + 1);
        meta->adrele1[i].price = (float)(i + 1) * 10.0;
        meta->adrele1[i].nbr = i + 2;
        strcpy(meta->adrele1[i].etat, "disponible");
    }
    printf("Les enregistrements ont été remplis automatiquement.\n");
}
// Fonction principale pour tester toutes les fonctionnalités
int main() {
    MetaTOF meta;
    int choix;
    int id;
    produit newProduit;
    pos position;
    char nouveauNom[20];

    do {
        printf("\nMenu de gestion des fichiers TOF:\n");
        printf("1. Créer un fichier\n");
        printf("2. Charger un fichier\n");
        printf("3. Remplir automatiquement les enregistrements\n");
        printf("4. Insérer un nouvel enregistrement\n");
        printf("5. Rechercher un enregistrement\n");
        printf("6. Supprimer un enregistrement (logique)\n");
        printf("7. Défragmenter le fichier\n");
        printf("8. Renommer le fichier\n");
        printf("9. Supprimer le fichier\n");
        printf("0. Quitter\n");
        printf("Votre choix : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                printf("Nom du fichier : ");
                scanf("%s", meta.FName);
                printf("Nombre d'enregistrements : ");
                scanf("%d", &meta.nbrRecord);
                printf("Organisation globale : ");
                scanf("%s", meta.OrgGlobale);
                printf("Organisation interne : ");
                scanf("%s", meta.OrgInterne);
                creerFichier(&meta, meta.FName, meta.nbrRecord, meta.OrgGlobale, meta.OrgInterne);
                break;

            case 2:
                chargerFichier(&meta);
                break;

            case 3:
                remplirAutomatiquement(&meta);
                break;

            case 4:
                printf("ID : ");
                scanf("%d", &newProduit.id);
                printf("Nom : ");
                scanf("%s", newProduit.eleName);
                printf("Prix : ");
                scanf("%f", &newProduit.price);
                printf("Quantité : ");
                scanf("%d", &newProduit.nbr);
                strcpy(newProduit.etat, "disponible");
                insererEnregistrement(&meta, newProduit);
                break;

            case 5:
                printf("ID à rechercher : ");
                scanf("%d", &id);
                position = rechercherEnregistrement(&meta, id);
                if (position.blocNbr != -1) {
                    printf("Enregistrement trouvé au bloc %d, déplacement %d.\n", position.blocNbr, position.deplacement);
                }
                break;

            case 6:
                printf("ID à supprimer : ");
                scanf("%d", &id);
                supprimerLogique(&meta, id);
                break;

            case 7:
                defragmenter(&meta);
                break;

            case 8:
                printf("Nouveau nom du fichier : ");
                scanf("%s", nouveauNom);
                renommerFichier(&meta, nouveauNom);
                break;

            case 9:
                supprimerFichier(&meta);
                break;

            case 0:
                printf("Au revoir !\n");
                break;

            default:
                printf("Choix invalide. Veuillez réessayer.\n");
        }
    } while (choix != 0);

    return 0;
}

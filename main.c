#include <string.h>
#include <stdio.h>
typedef struct {
    char Nomdufichier[20];
    int Taillefichierblocs;
    int Taillefichierenregistrements;
    int Adrpremierbloc;  // adresse du 1 bloc 
    int Modeorganisationglobale; // si est =0 alors chaine
    int Modeorganisationinterne;  // si est =0 alors ordonne
} fichiermetadonnes;

typedef struct {
    int id;               
    char name[15];  
    int age;
    char sexe[10];
    char adresse[30];
    int nmbrdevisite;
    int suprimelogiqument; // 1 si est suprimé logiquement
} maladie;

typedef struct {
    int adrdebloc; // adresse de bloc
    int etat;      // si vide = 0 pleine = 1
} Tableallocation;

typedef struct {
    int nbrblocutil; // nombre de bloc utilisé
    int nbrbloc;
    int FB;
} MS;

typedef struct {
    Tableallocation tablelocation[20];
    MS ms;
} BlocAllocation;

typedef struct {
    fichiermetadonnes T[20]; // Tableau de métadonnées
    int nbrMetadonnees;       // Nombre actuel de métadonnées dans ce bloc
    int next;
} BlocMetadonnees;

typedef struct {
    maladie T[20];
    int nbrmaladie;
    int next;
} BlocData;

typedef struct {
    union {
        BlocMetadonnees metadataTable;
        BlocData fileData;
        BlocAllocation allocation;
    } content;
    int typedebloc; // 1 = metadata, 2 = file data, 3 = allocation
} Bloc;
// Définition de la fonction afficherEtatMemoire
void afficherEtatMemoire(BlocAllocation *blocAlloc) {
    printf("Etat de la mémoire secondaire (avec des boîtes):\n\n");

    // Afficher les informations globales
    printf("Nombre total de blocs: %d\n", blocAlloc->ms.nbrbloc);
    printf("Nombre de blocs utilisés: %d\n", blocAlloc->ms.nbrblocutil);
    printf("FB (autre info): %d\n\n", blocAlloc->ms.FB);

    // Affichage des blocs avec des boîtes autour de chaque bloc
    for (int i = 0; i < 20; i++) {
        const char* etat = (blocAlloc->tablelocation[i].etat == 0) ? "Vide" : "Plein";
        
        int largeur = 20 + 2 + 2 * strlen(etat); // pour l'adresse + état, ajustable selon la taille du texte
        printf("Bloc %d:\n", i + 1);
        printf("╔%.*s╗\n", largeur, "====================================="); // ligne du dessus (boîte)
        printf("║ Adresse: %d %*s║\n", blocAlloc->tablelocation[i].adrdebloc, largeur - 15, " "); // Adresse
        printf("║ Etat   : %s %*s║\n", etat, largeur - strlen(etat) - 12, " "); // Etat
        printf("╚%.*s╝\n", largeur, "====================================="); // ligne du dessous (boîte)
        printf("\n"); // Saut de ligne entre chaque bloc
    }
}




// Fonction de compactage de la mémoire secondaire
void compactMS(BlocAllocation *blocAlloc) {
    if (blocAlloc == NULL) {
        printf("Erreur: Bloc d'allocation NULL.\n");
        return;
    }

    // Initialisation de l'indice de bloc vide (nextFreeBlock)
    int nextFreeBlock = 0;  // Indice du prochain bloc vide
    for (int i = 0; i < 20; i++) {
        // Si le bloc est vide, déplacer à la fin des blocs vides
        if (blocAlloc->tablelocation[i].etat == 0) {
            // Ignorer les blocs vides et passer au suivant
            continue;
        }

        // Si le bloc est plein, déplacer vers la position du prochain bloc vide
        if (i != nextFreeBlock) {
            // Déplacer le bloc plein à l'emplacement du bloc vide
            blocAlloc->tablelocation[nextFreeBlock] = blocAlloc->tablelocation[i];
            blocAlloc->tablelocation[i].etat = 0;  // Marquer le bloc original comme vide
        }

        // Mettre à jour l'indice pour le prochain bloc vide
        nextFreeBlock++;
    }

    // Mettre à jour le nombre de blocs utilisés
    blocAlloc->ms.nbrblocutil = nextFreeBlock;
    printf("Compactage terminé. %d blocs utilisés après compactage.\n", blocAlloc->ms.nbrblocutil);

    // Afficher l'état de la mémoire après le compactage
    afficherEtatMemoire(blocAlloc);
}



// Fonction qui vérifie s'il y a de l'espace contigu dans la mémoire centrale
void gestionEspace(Bloc *bloc, int nbrBlocsRequis) {
    int espaceTrouve = 0;
    int debutEspace = -1;
    
    // Vérifie si la table d'allocation est accessible
    if (bloc->typedebloc != 3) {
        printf("Type de bloc invalide pour l'allocation.\n");
        return;
    }

    // Recherche de l'espace contigu disponible dans la table d'allocation
    for (int i = 0; i < bloc->content.allocation.ms.nbrbloc - nbrBlocsRequis + 1; i++) {
        int espaceLibre = 0;

        // Vérifie si les `nbrBlocsRequis` blocs sont vides et contigus
        for (int j = i; j < i + nbrBlocsRequis; j++) {
            if (bloc->content.allocation.tablelocation[j].etat == 0) {
                espaceLibre++;
            } else {
                break;  // Si un bloc est plein, on arrête la vérification
            }
        }

        // Si on trouve suffisamment de blocs vides et contigus
        if (espaceLibre == nbrBlocsRequis) {
            debutEspace = i;
            espaceTrouve = 1;
            break;
        }
    }

    if (espaceTrouve) {
        // Afficher l'espace trouvé
        printf("Espace trouvé : %d blocs contigus à partir de l'adresse %d\n", nbrBlocsRequis, bloc->content.allocation.tablelocation[debutEspace].adrdebloc);
    } else {
        // Si l'espace n'est pas contigu, appelle la fonction de compactage
        printf("Aucun espace contigu trouvé. Appel de la fonction de compactage...\n");
        compactage(&bloc->content.allocation);
    }
}

// Déclaration de la fonction afficherEtatMemoire avant main
void afficherEtatMemoire(BlocAllocation *blocAlloc);

int main() {
    int choix;
    int modeG;
    int modeI;
    do {
        printf("\n--- Gestion de la Mémoire Secondaire ---\n");
        printf("1. Initialiser la mémoire secondaire\n");
        printf("2. Créer un fichier\n");
        printf("3. Afficher l'état de la mémoire secondaire\n");
        printf("4. Afficher les métadonnées des fichiers\n");
        printf("5. Rechercher un enregistrement\n");
        printf("6. Insérer un nouvel enregistrement\n");
        printf("7. Supprimer un enregistrement\n");
        printf("8. Défragmenter un fichier\n");
        printf("9. Supprimer un fichier\n");
        printf("10. Renommer un fichier\n");
        printf("11. Compacter la mémoire secondaire\n");
        printf("12. Vider la mémoire secondaire\n");
        printf("0. Quitter\n");
        printf("Votre choix : ");
        scanf("%d", &choix);

        switch(choix) {
            case 1:
                printf("Initialisation de la mémoire secondaire\n");
                // Appeler la fonction d'initialisation ici
                break;
            case 2:
                printf("Création d'un fichier\n");
                printf("Votre choix d'organisation globale: ");
                scanf("%d", &modeG);
                printf("Votre choix d'organisation interne: ");
                scanf("%d", &modeI);
                // Créer le fichier en fonction des choix d'organisation
                break;
            case 3:
                // Afficher l'état de la mémoire secondaire
                afficherEtatMemoire(NULL); // Exemple d'appel
                break;
            case 4:
                printf("Affichage des métadonnées du fichier :\n");
                // Afficher les métadonnées
                break;
            case 5:
                printf("Recherche d'enregistrement\n");
                break;
            case 6:
                printf("Insertion d'enregistrement\n");
                break;
            case 7:
                printf("Suppression d'enregistrement\n");
                break;
            case 8:
                printf("Défragmentation effectuée\n");
                break;
            case 9:
                printf("Suppression de fichier\n");
                break;
            case 10:
                printf("Renommage de fichier\n");
                break;
            case 11:
                printf("Compactage de la mémoire secondaire\n");
                // Affichage avant compactage
                 printf("Avant compactage :\n");
                 afficherEtatMemoire(/*latable dallocation*/);

                  // Appeler la fonction de compactage
                  compactMS(/*latableallocationn*/);
                break;
            case 12:
                printf("Mémoire secondaire vidée\n");
                break;
            case 0:
                printf("Programme terminé !\n");
                break;
            default:
                printf("Choix invalide. Veuillez réessayer.\n");
        }
    } while (choix != 0);

    return 0;
}



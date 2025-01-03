#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
typedef struct 
{
  char Nomdufichier[20];
  int Taillefichierblocs;
  int Taillefichierenregistrements;
  int Adrpremierbloc;  //adresse  du 1 bloc 
  int Modeorganisationglobale; // si est =0 alors chaine
  int Modeorganisationinterne;  // si est=0 alors ordone
}fichiermetadonnes;


typedef struct 
{
  int id;               
  char name[15];  
  int age;
  char sexe[10];
  char adresse[30];
  int nmbrdevisite;
  int suprimelogiqument;// 1 si est suprimé logiquement

}maladie;
typedef struct {
    int adrdebloc; // adresse de bloc
    int etat;      // si vide = 0 pleine = 1
} Tableallocation;

typedef struct {
    Tableallocation tablelocation[20];
} BlocAllocation;

typedef struct {
    fichiermetadonnes T[20]; // Tableau de métadonnées
    int nbrMetadonnees;       // Nombre actuel de métadonnées dans ce bloc
    int next;
} BlocMetadonnees;
typedef struct 
{
    maladie T[20];
    int nbrmaladie;
    int next;
   
}BlocData;
typedef struct {
    int nbrblocutil; // nombre de bloc utilise
    int nbrbloc;
    int FB;
} MS;

typedef struct {
    union {
        BlocMetadonnees metadataTable;
        BlocData fileData;
        BlocAllocation allocation;
    } content;
    int typedebloc; // 1 = metadata, 2 = file data, 3 = allocation
} Bloc;

void metajourtableallocation(FILE* disque, int blocIndex, int etat) {
    Bloc buffer;
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    if (buffer.typedebloc != 3) {
        printf("Erreur : Le premier bloc n'est pas un bloc d'allocation.\n");
        return;
    }

    // Update the allocation table entry for the specified blocIndex
    buffer.content.allocation.tablelocation[blocIndex].etat = etat;

    // Write the updated allocation table back to the first block
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
}

void CreeTableAllocation(MS *ms, FILE* disque) {
    Bloc buffer;
    buffer.typedebloc = 3; // Set the block type to 3 for table allocation
    int i;
    for ( i = 0; i < ms->nbrbloc; i++) {
        buffer.content.allocation.tablelocation[i].adrdebloc = i;
        buffer.content.allocation.tablelocation[i].etat = 0;
    }

    // Write the initial allocation table to the first block
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    // Mark the first block as allocated
    metajourtableallocation(disque, 0, 1);
}

void ViderMs(FILE* disque, MS *ms) {
    ms->nbrblocutil = 1; // nombre de bloc utilise
    CreeTableAllocation(ms, disque); // revenir à la 1er état 
}

void InitMs(MS *ms, FILE* disque) {
    ms->nbrbloc = 20;
    ms->FB = 20; // Nombre maximum d'enregistrements dans un bloc c'est le facteur du blocage
    ms->nbrblocutil = 1;
    CreeTableAllocation(ms, disque);
}


//cree fichier...................................................................

void creerFichier(FILE *disque) {
    fichiermetadonnes fichier;
    printf("Entrez le nom du fichier : ");
    scanf("%s", fichier.Nomdufichier);
    
    printf("Entrez la taille en blocs du fichier : ");
    scanf("%d", &fichier.Taillefichierblocs);
    
    printf("Entrez le nombre d'enregistrements dans le fichier : ");
    scanf("%d", &fichier.Taillefichierenregistrements);

    printf("Entrez le mode d'organisation globale (1 pour globale, 0 pour chaîne) : ");
    scanf("%d", &fichier.Modeorganisationglobale);
    
    printf("Entrez le mode d'organisation interne (1 pour ordonnée, 0 pour non ordonnée) : ");
    scanf("%d", &fichier.Modeorganisationinterne);

    // Écrire les métadonnées dans le fichier
    fseek(disque, 0, SEEK_END);
    fwrite(&fichier, sizeof(fichiermetadonnes), 1, disque);
    printf("Fichier créé avec succès.\n");
}


//charge fichie


void chargerFichier(FILE *disque, MS *ms) {
    // Allouer les blocs nécessaires à partir de la table d'allocation
    fseek(disque, 0, SEEK_SET);
    Bloc buffer;
    
    // Lire les métadonnées
    fread(&buffer, sizeof(Bloc), 1, disque);
    
    // Initialiser les blocs et marquer les blocs comme alloués
    int i;
    for ( i = 0; i < ms->nbrbloc; i++) {
        if (buffer.content.allocation.tablelocation[i].etat == 0) {
            // Trouver un bloc vide et le marquer comme plein
            buffer.content.allocation.tablelocation[i].etat = 1;
            break;
        }
    }
    // Écrire les données mises à jour
    fseek(disque, 0, SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
}




//insertion ...................................................................


void insererEnregistrement(FILE *disque, maladie enregistrement, MS *ms) {
    fseek(disque, 0, SEEK_SET);
    Bloc buffer;
    fread(&buffer, sizeof(Bloc), 1, disque);
    
    // Chercher un bloc disponible
    int i;
    for ( i = 0; i < ms->nbrbloc; i++) {
        if (buffer.content.allocation.tablelocation[i].etat == 0) {
            // Insérer l'enregistrement dans le bloc disponible
            buffer.content.fileData.T[i] = enregistrement;
            buffer.content.fileData.nbrmaladie++;
            buffer.content.allocation.tablelocation[i].etat = 1; // Marquer le bloc comme plein
            break;
        }
    }
    
    // Mettre à jour le disque
    fseek(disque, 0, SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
    printf("Enregistrement inséré avec succès.\n");
}



//recherche 

void rechercheParID(FILE *disque, int idRecherche, MS *ms) {
    fseek(disque, 0, SEEK_SET);
    Bloc buffer;
    fread(&buffer, sizeof(Bloc), 1, disque);
    
    // Recherche binaire dans un bloc ordonné
    int gauche = 0, droite = ms->FB - 1;
    while (gauche <= droite) {
        int milieu = (gauche + droite) / 2;
        if (buffer.content.fileData.T[milieu].id == idRecherche) {
            printf("Enregistrement trouvé dans le bloc %d, à la position %d.\n", buffer.content.fileData.T[milieu].id, milieu);
            return;
        }
        if (buffer.content.fileData.T[milieu].id < idRecherche) {
            gauche = milieu + 1;
        } else {
            droite = milieu - 1;
        }
    }
    
    printf("Enregistrement avec ID %d non trouvé.\n", idRecherche);
}



// supprecion logique 

void suppressionLogique(FILE *disque, int id, MS *ms) {
    fseek(disque, 0, SEEK_SET);
    Bloc buffer;
    fread(&buffer, sizeof(Bloc), 1, disque);
    
    // Chercher l'enregistrement par ID
    int i;
    for ( i = 0; i < ms->FB; i++) {
        if (buffer.content.fileData.T[i].id == id) {
            // Marquer l'enregistrement comme supprimé logiquement
            buffer.content.fileData.T[i].suprimelogiqument = 1;
            printf("Enregistrement avec ID %d supprimé logiquement.\n", id);
            break;
        }
    }
    
    // Mettre à jour le disque
    fseek(disque, 0, SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
}



//suppresion phisique

void suppressionPhysique(FILE *disque, int id, MS *ms) {
    fseek(disque, 0, SEEK_SET);
    Bloc buffer;
    fread(&buffer, sizeof(Bloc), 1, disque);
    
    // Chercher l'enregistrement par ID
    int i;
    for ( i = 0; i < ms->FB; i++) {
        if (buffer.content.fileData.T[i].id == id) {
            // Réorganiser physiquement les blocs en déplaçant l'enregistrement à la fin
            buffer.content.fileData.T[i] = buffer.content.fileData.T[ms->FB - 1];
            buffer.content.fileData.nbrmaladie--;  // Réduire le nombre d'enregistrements
            printf("Enregistrement avec ID %d supprimé physiquement.\n", id);
            break;
        }
    }
    
    // Mettre à jour le disque
    fseek(disque, 0, SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
}


// defragmantation 


void defragmentation(FILE *disque, MS *ms) {
    fseek(disque, 0, SEEK_SET);
    Bloc buffer;
    fread(&buffer, sizeof(Bloc), 1, disque);

    int index = 0;
    int i;
    for ( i = 0; i < ms->nbrbloc; i++) {
        if (buffer.content.fileData.T[i].suprimelogiqument == 0) {
            buffer.content.fileData.T[index] = buffer.content.fileData.T[i];
            index++;
        }
    }

    // Mettre à jour les blocs après la défragmentation
    fseek(disque, 0, SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
    printf("Défragmentation terminée.\n");
}
   
   
   //compactage 
   
   
   void compactage(FILE *disque, MS *ms) {
    // Parcours des blocs utilisés pour récupérer l'espace inutilisé
    int i;
    for ( i = 0; i < ms->nbrblocutil; i++) {
        // Lecture du bloc de données
        fseek(disque, i * sizeof(Bloc), SEEK_SET);
        Bloc buffer;
        fread(&buffer, sizeof(Bloc), 1, disque);

        // Vérifier si c'est un bloc de données
        if (buffer.typedebloc == 2) {
            int shiftIndex = 0;  // Indice pour déplacer les enregistrements valides

            // Parcours des enregistrements dans le bloc
            int j;
            for ( j = 0; j < ms->FB; j++) {
                // Vérifier si l'enregistrement est marqué pour suppression logique
                if (buffer.content.fileData.T[j].suprimelogiqument == 0) {
                    // Déplacer l'enregistrement valide
                    if (j != shiftIndex) {
                        buffer.content.fileData.T[shiftIndex] = buffer.content.fileData.T[j];
                        buffer.content.fileData.T[j] = (maladie){0}; // Réinitialiser l'ancien emplacement
                    }
                    shiftIndex++;  // Augmenter l'indice de l'enregistrement valide
                }
            }

            // Mettre à jour le nombre d'enregistrements valides
            buffer.content.fileData.nbrmaladie = shiftIndex;

            // Réécrire le bloc modifié dans le disque
            fseek(disque, i * sizeof(Bloc), SEEK_SET);
            fwrite(&buffer, sizeof(Bloc), 1, disque);
        }
    }

    printf("Compactage terminé. L'espace inutilisé a été récupéré.\n");
}



//  rennomet fichie

void renommerFichier(FILE *disque, const char *ancienNom, const char *nouveauNom) {
    fseek(disque, 0, SEEK_SET);
    Bloc buffer;
    fread(&buffer, sizeof(Bloc), 1, disque);

    // Chercher le fichier et le renommer
    int i;
    for ( i = 0; i < 20; i++) {
        if (strcmp(buffer.content.metadataTable.T[i].Nomdufichier, ancienNom) == 0) {
            strcpy(buffer.content.metadataTable.T[i].Nomdufichier, nouveauNom);
            break;
        }
    }

    // Mettre à jour le disque
    fseek(disque, 0, SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
    printf("Fichier renommé en %s.\n", nouveauNom);
}


// supprimet fichie 

void supprimerFichier(FILE *disque, const char *nomFichier) {
    fseek(disque, 0, SEEK_SET);
    Bloc buffer;
    fread(&buffer, sizeof(Bloc), 1, disque);

    // Chercher et supprimer le fichier
    int i;
    for ( i = 0; i < 20; i++) {
        if (strcmp(buffer.content.metadataTable.T[i].Nomdufichier, nomFichier) == 0) {
            buffer.content.metadataTable.T[i].Nomdufichier[0] = '\0'; // Marquer comme supprimé
            break;
        }
    }

    // Mettre à jour le disque
    fseek(disque, 0, SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
    printf("Fichier %s supprimé.\n", nomFichier);
}



//test les fonction 



int main() {
    FILE *disque = fopen("disque.bin", "rb+");  // Fichier disque où les données seront stockées
    if (!disque) {
        printf("Erreur d'ouverture du fichier disque.\n");
        return 1;
    }

    int id;
    const char *ancienNom;
    const char *nouveauNom;
    const char *nomFichier;

    maladie enregistrement;  // Déclarez un enregistrement pour l'insertion
    int idRecherche;  // Déclarez une variable pour l'ID de recherche

    MS ms;  // Structure pour la gestion de la mémoire secondaire
    InitMs(&ms, disque);  // Initialisation de la mémoire secondaire
    int choix;

    do {
        // Menu pour l'utilisateur
        printf("\nMenu:\n");
        printf("1. Creer un fichier\n");
        printf("2. Charger un fichier\n");
        printf("3. Inserer un enregistrement\n");
        printf("4. Rechercher un enregistrement\n");
        printf("5. Suppression logique d'un enregistrement\n");
        printf("6. Suppression physique d'un enregistrement\n");
        printf("7. Compactage\n");
        printf("8. Renommer un fichier\n");
        printf("9. Supprimer un fichier\n");
        printf("0. Quitter\n");
        printf("Choisissez une option: ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                creerFichier(disque);
                break;
            case 2:
                chargerFichier(disque, &ms);
                break;
            case 3:
                printf("Entrez les détails de l'enregistrement :\n");
                printf("ID: ");
                scanf("%d", &enregistrement.id);
                printf("Nom: ");
                scanf("%s", enregistrement.name);  // Utilisez %s pour lire une chaîne
                printf("Âge: ");
                scanf("%d", &enregistrement.age);
                printf("Sexe: ");
                scanf("%s", enregistrement.sexe);  // Sexe
                printf("Adresse: ");
                scanf("%s", enregistrement.adresse);  // Adresse
                printf("Nombre de visites: ");
                scanf("%d", &enregistrement.nmbrdevisite);  // Nombre de visites
                enregistrement.suprimelogiqument = 0;  // Par défaut, pas supprimé logiquement
                insererEnregistrement(disque, enregistrement, &ms);
                break;
            case 4:
                printf("Entrez l'ID à rechercher : ");
                scanf("%d", &idRecherche);
                rechercheParID(disque, idRecherche, &ms);
                break;
            case 5:
                printf("Entrez l'ID à supprimer logiquement : ");
                scanf("%d", &id);
                suppressionLogique(disque, id, &ms);
                break;
            case 6:
                printf("Entrez l'ID à supprimer physiquement : ");
                scanf("%d", &id);
                suppressionPhysique(disque, id, &ms);
                break;
            case 7:
                compactage(disque, &ms);
                break;
            case 8:
                printf("Entrez l'ancien nom de fichier : ");
                scanf("%s", (char *)ancienNom);
                printf("Entrez le nouveau nom de fichier : ");
                scanf("%s", (char *)nouveauNom);
                renommerFichier(disque, ancienNom, nouveauNom);
                break;
            case 9:
                printf("Entrez le nom du fichier à supprimer : ");
                scanf("%s", (char *)nomFichier);
                supprimerFichier(disque, nomFichier);
                break;
            case 0:
                printf("Au revoir!\n");
                break;
            default:
                printf("Option invalide, veuillez réessayer.\n");
        }
    } while (choix != 0);

    fclose(disque);  // Fermer le fichier
    return 0;
}

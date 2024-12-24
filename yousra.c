#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define FB 10  // Nombre maximum d'enregistrements dans un bloc (facteur de bloc)

typedef struct 
{
  char Nomdufichier[20];  // Nom du fichier
  int Taillefichierblocs;  // Nombre de blocs du fichier
  int Taillefichierenregistrements;  // Nombre d'enregistrements dans le fichier
  int Adrpremierbloc;  // Adresse du premier bloc
  int Modeorganisationglobale;  // Mode d'organisation global (0 : chaîne, 1 : autre)
  int Modeorganisationinterne;  // Mode d'organisation interne (0 : ordonné, 1 : autre)
} fichiermetadonnes;

typedef struct 
{
  int id;  // Identifiant unique pour chaque enregistrement
  char name[15];  // Nom du patient
  int age;  // Âge du patient
  char sexe[10];  // Sexe du patient
  char adresse[30];  // Adresse du patient
  int nmbrdevisite;  // Nombre de visites du patient
  int suprimelogiqument;  // 1 si l'enregistrement est supprimé logiquement, sinon 0
} maladie;

typedef struct 
{
  maladie T[FB];  // Tableau contenant les enregistrements d'un bloc
  int nbrmaladie;  // Nombre d'enregistrements dans ce bloc
  int next;  // Adresse du bloc suivant (pour la chaîne)
} Bloc;

typedef struct
{
  int adrdebloc;  // Adresse du bloc
  int etat;  // État du bloc (0 : vide, 1 : plein)
} Tableallocation;

typedef struct 
{
  int nbrblocutil;  // Nombre de blocs utilisés
  int nbrbloc;  // Nombre total de blocs disponibles
  Tableallocation tablelocation[30];  // Tableau de table d'allocation pour chaque bloc
} MS;

// Fonction pour créer la table d'allocation
Tableallocation CreerTableAllocation(MS *ms)
{
    Tableallocation TA;  // Déclare une table d'allocation
    int i;
    for ( i = 0; i < ms->nbrbloc; i++)  // Parcours de chaque bloc
    {
        TA.adrdebloc = i;  // L'adresse du bloc est l'index du tableau
        TA.etat = 0;  // Initialement, tous les blocs sont vides
    }
    return TA;  // Retourne la table d'allocation initialisée
}

// Fonction d'initialisation de la mémoire de stockage (MS)
void InitialiserMS(MS *ms)
{
    ms->nbrbloc = 20;  // Initialisation du nombre de blocs à 20
    ms->nbrblocutil = 0;  // Aucun bloc utilisé au départ
    ms->tablelocation[0] = CreerTableAllocation(ms);  // Initialise la première table d'allocation
    printf("MS Initialisée avec %d blocs.\n", ms->nbrbloc);  // Affiche un message de confirmation
}

// Fonction pour créer un fichier avec les paramètres donnés par l'utilisateur
void CreerFichier(fichiermetadonnes *file)
{
    printf("Entrez le nom du fichier : ");
    scanf("%s", file->Nomdufichier);  // Demande le nom du fichier à l'utilisateur

    printf("Entrez le nombre d'enregistrements : ");
    scanf("%d", &file->Taillefichierenregistrements);  // Demande le nombre d'enregistrements

    printf("Entrez le nombre de blocs : ");
    scanf("%d", &file->Taillefichierblocs);  // Demande le nombre de blocs

    printf("Entrez le mode d'organisation globale (0 : chaîne, 1 : autre) : ");
    scanf("%d", &file->Modeorganisationglobale);  // Demande le mode d'organisation globale

    printf("Entrez le mode d'organisation interne (0 : ordonné, 1 : autre) : ");
    scanf("%d", &file->Modeorganisationinterne);  // Demande le mode d'organisation interne

    file->Adrpremierbloc = 0;  // L'adresse du premier bloc est 0 par défaut
    printf("Fichier '%s' créé avec %d enregistrements et %d blocs.\n", 
            file->Nomdufichier, file->Taillefichierenregistrements, file->Taillefichierblocs);  // Affiche les informations du fichier
}

// Fonction pour insérer un enregistrement dans le fichier
void InsererEnregistrement(MS *ms, fichiermetadonnes *file, maladie nouvelEnregistrement) {
    int blocTrouve = 0;
    Bloc *bloc = malloc(sizeof(Bloc));
    int i, j;

    // Cas 1: Organisation en blocs contigus et ordonnés par ID
    if (file->Modeorganisationglobale == 0) {
        for (i = 0; i < ms->nbrbloc; i++) {
            if (ms->tablelocation[i].etat == 1) {  // Si le bloc est plein
                for (j = 0; j < FB; j++) {
                    if (bloc->T[j].id == 0 || bloc->T[j].id > nouvelEnregistrement.id) {
                        // Décale les enregistrements pour faire de la place à l'insertion
                        int k;
                        for ( k = bloc->nbrmaladie - 1; k >= j; k--) {
                            bloc->T[k + 1] = bloc->T[k];
                        }
                        bloc->T[j] = nouvelEnregistrement;  // Insère l'enregistrement à la bonne position
                        bloc->nbrmaladie++;  // Incrémente le nombre d'enregistrements dans ce bloc
                        blocTrouve = 1;
                        break;
                    }
                }
            }
        }

        if (!blocTrouve) {
            // Si aucun bloc plein n'a été trouvé, essayer d'ajouter dans un bloc vide
            for (i = 0; i < ms->nbrbloc; i++) {
                if (ms->tablelocation[i].etat == 0) {  // Si le bloc est vide
                    bloc->T[bloc->nbrmaladie] = nouvelEnregistrement;
                    bloc->nbrmaladie++;
                    ms->tablelocation[i].etat = 1;  // Marque le bloc comme plein
                    ms->nbrblocutil++;  // Incrémente le nombre de blocs utilisés
                    blocTrouve = 1;
                    break;
                }
            }
        }

        if (!blocTrouve) {
            // Si aucun bloc n'est disponible, demander à l'utilisateur s'il veut ajouter un nouveau bloc
            printf("Tous les blocs sont pleins. Voulez-vous ajouter un nouveau bloc ? (1 : Oui, 0 : Non): ");
            int choix;
            scanf("%d", &choix);
            if (choix == 1) {
                ms->nbrbloc++;  // Augmente le nombre total de blocs
                ms->tablelocation[ms->nbrbloc - 1].etat = 1;  // Marque le nouveau bloc comme plein
                bloc->T[bloc->nbrmaladie] = nouvelEnregistrement;  // Ajoute l'enregistrement dans ce nouveau bloc
                bloc->nbrmaladie++;
                ms->nbrblocutil++;
                printf("Enregistrement ajouté dans un nouveau bloc.\n");
            } else {
                printf("Aucun bloc libre disponible pour l'insertion.\n");
            }
        }
    }
}

// Fonction pour rechercher un enregistrement par son ID
void RechercherEnregistrement(MS *ms, fichiermetadonnes *file, int idRecherche)
{
    // Recherche dans tous les blocs pour l'enregistrement avec l'ID donné
    int i;
    for ( i = 0; i < ms->nbrbloc; i++)
    {
        if (ms->tablelocation[i].etat == 1)  // Si le bloc n'est pas vide
        {
        	int j;
            for ( j = 0; j < FB; j++)
            {
                if (ms->tablelocation[i].adrdebloc == idRecherche)  // Si l'ID correspond
                {
                    printf("Enregistrement trouvé dans le bloc %d.\n", i);  // Affiche l'emplacement du bloc
                    return;  // Retourne dès que l'enregistrement est trouvé
                }
            }
        }
    }
    printf("Enregistrement non trouvé.\n");  // Si l'enregistrement n'est pas trouvé
}

// Fonction pour supprimer un enregistrement logiquement (marquer comme supprimé)
void SupprimerLogiquement(MS *ms, fichiermetadonnes *file, int id) {
    int i, j;
    int trouve = 0;

    // Recherche de l'enregistrement à supprimer logiquement
    for (i = 0; i < ms->nbrbloc; i++) {
        Bloc *bloc = malloc(sizeof(Bloc));
        for (j = 0; j < bloc->nbrmaladie; j++) {
            if (bloc->T[j].id == id) {  // Si l'ID correspond à celui recherché
                // Marquer l'enregistrement comme supprimé logiquement
                bloc->T[j].suprimelogiqument = 1;  // L'enregistrement est supprimé logiquement
                printf("Enregistrement avec ID %d supprimé logiquement.\n", id);
                trouve = 1;
                break;
            }
        }
        if (trouve) {
            break;
        }
    }

    if (!trouve) {
        printf("Enregistrement avec ID %d non trouvé.\n", id);
    }
}

// Fonction pour supprimer un enregistrement physiquement
void SupprimerPhysiquement(MS *ms, fichiermetadonnes *file, int id) {
    int i, j, k;
    int trouve = 0;

    // Recherche de l'enregistrement à supprimer
    for (i = 0; i < ms->nbrbloc; i++) {
        Bloc *bloc = malloc(sizeof(Bloc));
        for (j = 0; j < bloc->nbrmaladie; j++) {
            if (bloc->T[j].id == id) {  // Si l'ID correspond à celui recherché
                // Déplacer les enregistrements suivants pour combler l'espace
                for (k = j; k < bloc->nbrmaladie - 1; k++) {
                    bloc->T[k] = bloc->T[k + 1];  // Décaler les enregistrements vers la gauche
                }
                bloc->nbrmaladie--;  // Réduire le nombre d'enregistrements dans le bloc
                printf("Enregistrement avec ID %d supprimé physiquement.\n", id);
                ms->tablelocation[i].etat = 0;  // Marquer le bloc comme vide
                trouve = 1;
                break;
            }
        }
        if (trouve) {
            break;
        }
    }

    if (!trouve) {
        printf("Enregistrement avec ID %d non trouvé.\n", id);
    }
}
// Fonction pour effectuer la défragmentation (réorganiser les blocs)
void Defragmenter(MS *ms, fichiermetadonnes *file) {
    int i, j, k;
    int blocLibere = 0;

    // Recherche des enregistrements supprimés logiquement
    for (i = 0; i < ms->nbrbloc; i++) {
        Bloc *bloc = malloc(sizeof(Bloc));
        for (j = 0; j < bloc->nbrmaladie; j++) {
            if (bloc->T[j].suprimelogiqument == 1) {  // Si l'enregistrement est supprimé logiquement
                // Trouver un enregistrement à déplacer
                for (k = j + 1; k < bloc->nbrmaladie; k++) {
                    bloc->T[k - 1] = bloc->T[k];  // Déplacer l'enregistrement vers la gauche
                }
                bloc->nbrmaladie--;  // Réduire le nombre d'enregistrements dans le bloc
                ms->tablelocation[i].etat = 0;  // Marquer le bloc comme vide
                blocLibere = 1;  // Signaler qu'un bloc a été libéré
            }
        }
    }

    // Re-organisation des enregistrements pour combler l'espace libre
    if (blocLibere) {
        for (i = 0; i < ms->nbrbloc; i++) {
            Bloc *bloc = malloc(sizeof(Bloc));
            if (ms->tablelocation[i].etat == 0) {  // Si le bloc est vide
                for (j = i + 1; j < ms->nbrbloc; j++) {
                    if (ms->tablelocation[j].etat == 1) {  // Si un bloc plein suit
                        for (k = 0; k < bloc->nbrmaladie; k++) {
                            bloc->T[k] = bloc->T[k + 1];  // Déplacer les enregistrements pour remplir le bloc
                        }
                        bloc->nbrmaladie--;
                        ms->tablelocation[i].etat = 1;  // Marquer le bloc comme plein
                    }
                }
            }
        }
    }

    printf("Défragmentation terminée.\n");
}


// Fonction pour renommer un fichier
void RenommerFichier(fichiermetadonnes *file, char *nouveauNom)
{
    strcpy(file->Nomdufichier, nouveauNom);  // Copie le nouveau nom dans la structure du fichier
    printf("Fichier renommé en %s.\n", file->Nomdufichier);  // Affiche un message de confirmation
}


   
int main() {
    MS ms;
    fichiermetadonnes file;
    maladie nouvelEnregistrement;
    int choix, idRecherche;

    // Initialisation de la mémoire de stockage
    InitialiserMS(&ms);

    do {
        printf("\n=== MENU ===\n");
        printf("1. Créer un fichier\n");
        printf("2. Insérer un enregistrement\n");
        printf("3. Rechercher un enregistrement\n");
        printf("4. Supprimer un enregistrement logiquement\n");
        printf("5. Supprimer un enregistrement physiquement\n");
        printf("6. Défragmenter la mémoire\n");
        printf("7. Renommer un fichier\n");
        printf("0. Quitter\n");
        printf("Choisissez une option : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                CreerFichier(&file);
                break;

            case 2:
                printf("Entrez les informations de l'enregistrement :\n");
                printf("ID : ");
                scanf("%d", &nouvelEnregistrement.id);
                printf("Nom : ");
                scanf("%s", nouvelEnregistrement.name);
                printf("Âge : ");
                scanf("%d", &nouvelEnregistrement.age);
                printf("Sexe : ");
                scanf("%s", nouvelEnregistrement.sexe);
                printf("Adresse : ");
                scanf("%s", nouvelEnregistrement.adresse);
                printf("Nombre de visites : ");
                scanf("%d", &nouvelEnregistrement.nmbrdevisite);
                nouvelEnregistrement.suprimelogiqument = 0; // Non supprimé logiquement
                InsererEnregistrement(&ms, &file, nouvelEnregistrement);
                break;

            case 3:
                printf("Entrez l'ID de l'enregistrement à rechercher : ");
                scanf("%d", &idRecherche);
                RechercherEnregistrement(&ms, &file, idRecherche);
                break;

            case 4:
                printf("Entrez l'ID de l'enregistrement à supprimer logiquement : ");
                scanf("%d", &idRecherche);
                SupprimerLogiquement(&ms, &file, idRecherche);
                break;

            case 5:
                printf("Entrez l'ID de l'enregistrement à supprimer physiquement : ");
                scanf("%d", &idRecherche);
                SupprimerPhysiquement(&ms, &file, idRecherche);
                break;

            case 6:
                Defragmenter(&ms, &file);
                break;

            case 7: {
                char nouveauNom[20];
                printf("Entrez le nouveau nom du fichier : ");
                scanf("%s", nouveauNom);
                RenommerFichier(&file, nouveauNom);
                break;
            }

            case 0:
                printf("Au revoir !\n");
                break;

            default:
                printf("Option invalide. Veuillez réessayer.\n");
        }
    } while (choix != 0);

    return 0;
}

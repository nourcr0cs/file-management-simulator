#include <string.h>
#include <stdio.h>
#include <disque.bin>
#include <function.c>
#include <hiba.c>
#include <maroua.c>
#include <nour.c>
#include <nour.h>
#include <soumia.c>
#include <soumiahariz.exe>
#include  <yousra.c>
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
void afficherdetaillebloc(BlocAllocation *blocAlloc) {
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

int main(){


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
        printf("13. suppression logique de fichier\n");
        printf("0. Quitter\n");
        printf("Votre choix : ");
        scanf("%d", &choix);

 int modeI,modeG;
 Ms disque;
 int choix ;
scanf("%d",&choix);
 switch(choix) {
            case 1:
                printf("Initialisation de la mémoire secondaire\n");
                InitMs(&disque, 20);
                break;
            case 2:
                printf("Création d'un fichier\n");
                printf("Votre choix d'organisation globale: ");
                scanf("%d", &modeG);
                printf("Votre choix d'organisation interne: ");
                scanf("%d", &modeI);
                // Créer le fichier en fonction des choix d'organisation
                
              if(modeG=0){
                 if(modeI=0){
                    creerfichierCO(FILE* disque); 
                 }
                 else{
                    creationLL_OF(disque,ms,nbrbloc);
                 }
              }else 
              {
                 if(modeI=0){
                      creerFichier(disque);
                  }else {
                      chargerFileTNOF(disque);
                  }  
              }
                break;
            case 3:
                // Afficher l'état de la mémoire secondaire
                afficherEtatMemoire(NULL); // Exemple d'appel
                break;
            case 4:
                printf("Afficher les détails de la mémoire secondaire :\n");
    afficherMS(disque) ;      
    Bloc buffer;
    rewind(disque);

    printf("========== État de la Mémoire Secondaire ==========\n");

    // Parcourir tous les blocs
    for (int i = 0; i < 20; i++) {
        fseek(disque, i * sizeof(Bloc), SEEK_SET);

        if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Bloc %d : Erreur : Impossible de lire le bloc. Bloc vide ou corrompu.\n", i);
            continue;
        }

        printf("Bloc %d : ", i);

        switch (buffer.typedebloc) {
            case 1: // Bloc de métadonnées
                printf("Bloc de métadonnées\n");
                printf("  Nombre de fichiers : %d\n", buffer.content.metadataTable.nbrMetadonnees);
                printf("  Next : %d\n", buffer.content.metadataTable.next);
                for (int j = 0; j < buffer.content.metadataTable.nbrMetadonnees; j++) {
                    printf("    Fichier %d : Adresse premier bloc = %d\n",
                           j, buffer.content.metadataTable.T[j].Adrpremierbloc);
                }
                break;

            case 2: // Bloc de données de fichier
                printf("Bloc de données de fichier\n");
                printf("  Nombre d'enregistrements : %d\n", buffer.content.fileData.nbrmaladie);
                printf("  Next : %d\n", buffer.content.fileData.next);
                break;

            case 3: // Bloc de table d'allocation
                printf("Bloc de table d'allocation\n");
                printf("  Nombre total de blocs : %d\n", buffer.content.allocation.nbrbloc);
                printf("  Nombre de blocs utilisés : %d\n", buffer.content.allocation.nbrblocutil);
                for (int j = 0; j < buffer.content.allocation.nbrbloc; j++) {
                    printf("    Bloc %d : %s\n", j,
                           buffer.content.allocation.tablelocation[j].etat == 1 ? "Alloué" : "Libre");
                }
                break;

            default: // Bloc vide ou type inconnu
                printf("Erreur : Bloc ne contient aucun type valide.\n");
                break;
        }
    }

    printf("===================================================\n");
}

                
                break;
            case 5:
                printf("Recherche d'enregistrement\n");
                printf("Votre choix d'organisation globale: ");
                scanf("%d", &modeG);
                printf("Votre choix d'organisation interne: ");
                scanf("%d", &modeI);
                
              if(modeG=0){
                 if(modeI=0){
                int ID;
                scanf("%d",ID)
                  rechercheebregistrement(disque,nomfichier,ID); 
                 }
                 else{
                    researchDis(disque,ms,fichernom) ;
                 }
              }else 
              {
                 if(modeI=0){
                     int id;
                     sacanf("%d",&id)
                      rechercherparid(disque,id);
                  }else {
                      int id;
                      scanf("%d",&id);
                      RechercheFILETOF(disque,id);
                  }  
              }
                break;
            case 6:
                printf("Insertion d'enregistrement\n");
               
                printf("Votre choix d'organisation globale: ");
                scanf("%d", &modeG);
                printf("Votre choix d'organisation interne: ");
                scanf("%d", &modeI);
                 printf("Donnez le nom de fichier  ");
                 string  nomfichier ;
                 scanf("%s",&nomfichier);
                scanf("%d", &modeI);
                //  le fichier en fonction des choix d'organisation
              if(modeG=0){
                 if(modeI=0){
                     insertionenregistrement(disque,nomfichier);
                 }
                 else{
                    insertDis(disque,searchld,nomfichier) ;
                 }
              }else 
              {
                 if(modeI=0){
                     IsererEnregistrement(disque,nomfichier,ms);
                  }else {
                     IsertiofileTNOF(disque,nomfichier);
                  }  
              }
                break;
            case 7:
                printf("Suppression d'enregistrement\n");
                
                printf("Votre choix d'organisation globale: ");
                scanf("%d", &modeG);
                printf("Votre choix d'organisation interne: ");
                scanf("%d", &modeI);
                 printf("Votre nom de fichier: ");
                scanf("%d", &filename);
                // le fichier en fonction des choix d'organisation
                //supretio physique
              if(modeG=0){
                 if(modeI=0){
                     suprimereregistrement(disque,fileame,ID);
                 }
                 else{
                     suppPhysique(disque,ms,fileame);
                 }
              }else 
              {
                 if(modeI=0){
                     int id  ;
                     scanf("%d",&id);
                      supretionPhysique(disque,id,ms);
                  }else {
                      supprPhysiqueDefichierTNOF(disque,filename);
                  }  
              }
                break;
            case 8:
                printf("Défragmentation effectuée\n");
                 
                printf("Votre choix d'organisation globale: ");
                scanf("%d", &modeG);
                printf("Votre choix d'organisation interne: ");
                scanf("%d", &modeI);
                string nomfichier;
                // le fichier en fonction des choix d'organisation
              if(modeG=0){
                 if(modeI=0){
                     
                     scanf("%s",&nomfichier);
                     defragmentation(disque,nomfichier);
                 }
                 else{
                      printf("donner nom fichier: ");
                      scanf("%s", &nomfichier);
                     defragmentation(disque,ms,nomfichier);
                 }
              }else 
              {
                 if(modeI=0){
                      defragmentation(disque,ms);
                  }else {
                      defragmetationFileTNOF(disque);
                  }  
              }
                break;
            case 9:
                printf("Suppression de fichier\n");
                 printf("Création d'un fichier\n");
                printf("Votre choix d'organisation globale: ");
                scanf("%d", &modeG);
                printf("Votre choix d'organisation interne: ");
                scanf("%d", &modeI);
                printf("donner  le nom du fichier");
                scan("%s",fichiern);
                // le fichier en fonction des choix d'organisation
              if(modeG=0){
                 if(modeI=0){
                     suprimerFCO(disque,fichiern);
                 }
                 else{
                     deleteL_OF(disque,fichiern);
                 }
              }else 
              {
                 if(modeI=0){
                      Supprimerfichier(disque,fichiern);
                      
                  }else {
                      supprPhysiqueTNOF(disque,id);
                  }  
              }
                break;
            case 10:
                printf("Renommage de fichier\n");
                string  newname,lastname;
                printf("donner  le nom du fichier que  vous voulez change son nom");
                scanf("%s",&lastname);
                printf("donner le nouveau nom");
                scanf("%s",&newname);
                changeFileName( disque ,newname ,lastname );
                
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
               viderMS(disque, 20);
                break;
            case  13:
                printf("suppretion logique d'enregistrement");
                string nomfichier;
                sacnf("%d",&nomfichier);
                 printf("Création d'un fichier\n");
                printf("Votre choix d'organisation globale: ");
                scanf("%d", &modeG);
                printf("Votre choix d'organisation interne: ");
                scanf("%d", &modeI);
                string nomfichier;
                printf("VDoer le   nom de  fichier: ");
                scanf("%s", &nomfichier);
                int id  ;
                printf("Donner  le id de votre enregistrement : ");
                scanf("%d", &id);
              if(modeG=0){
                 if(modeI=0){
                    suppretionLogique(disque , id,nomfichier);
                 }
                 else{
                     string nomfichier;
                     scanf("%s",&nomfichier);
                    suppLogique(disque , id,nomfichier);
                 }
              }else 
              {
                 if(modeI=0){
                      SuppressionLogique(disque,id,ms);
                  }else {
                      supprLogiqueFileTNOF(disque,id);
                  }  
              }
            case 0:
                printf("Programme terminé !\n");
                break;
            default:
                printf("Choix invalide. Veuillez réessayer.\n");
        }
    } while (choix != 0);
return 0;
}

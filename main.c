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
//----------------------------------------------------- Définition de la fonction afficherEtatMemoire--------------------------------------------------


// Fonction pour afficher un bloc avec un cadre
void afficherAvecCadre(const char* titre, const char* contenu) {
    int largeur = 50;  // Largeur du cadre (en caractères)
    printf("+");
    for (int i = 0; i < largeur - 2; i++) {
        printf("-");  // Afficher les bords du cadre
    }
    printf("+\n");

    // Afficher le titre du bloc centré
    printf("| %-*s |\n", largeur - 4, titre);

    // Afficher le contenu du bloc
    printf("| %-*s |\n", largeur - 4, contenu);

    // Afficher les bords du cadre
    printf("+");
    for (int i = 0; i < largeur - 2; i++) {
        printf("-");  // Afficher les bords du cadre
    }
    printf("+\n");
}

// Fonction pour afficher l'état du fichier .bin avec des cadres
void afficherEtatFichier(Bloc* blocs, int nbrBlocs, BlocAllocation* allocation) {
    printf("=== Etat du fichier .bin ===\n");
    
    // Affichage de l'état des blocs dans la mémoire secondaire (MS)
    for (int i = 0; i < nbrBlocs; i++) {
        // Créer un titre pour chaque bloc
        char titre[50];
        sprintf(titre, "Bloc %d", i);

        // Créer un contenu pour chaque bloc en fonction de son type
        char contenu[500];
        contenu[0] = '\0';  // Initialiser le contenu

        switch (blocs[i].typedebloc) {
            case 1:
                // Bloc de métadonnées
                strcat(contenu, "Type: Métadonnées\n");
                BlocMetadonnees* blocMeta = &blocs[i].content.metadataTable;
                char temp[100];
                sprintf(temp, "Nombre de métadonnées: %d", blocMeta->nbrMetadonnees);
                strcat(contenu, temp);
                strcat(contenu, "\n");
                for (int j = 0; j < blocMeta->nbrMetadonnees; j++) {
                    sprintf(temp, "Fichier %d - Nom: %s, Taille: %d, Premier bloc: %d\n",
                            j, blocMeta->T[j].Nomdufichier, blocMeta->T[j].Taillefichierblocs, blocMeta->T[j].Adrpremierbloc);
                    strcat(contenu, temp);
                }
                break;
                
            case 2:
                // Bloc de données
                strcat(contenu, "Type: Données\n");
                BlocData* blocData = &blocs[i].content.fileData;
                sprintf(temp, "Nombre de maladies: %d", blocData->nbrmaladie);
                strcat(contenu, temp);
                strcat(contenu, "\n");
                for (int j = 0; j < blocData->nbrmaladie; j++) {
                    sprintf(temp, "Maladie %d - ID: %d, Nom: %s, Age: %d\n", j, blocData->T[j].id, blocData->T[j].name, blocData->T[j].age);
                    strcat(contenu, temp);
                }
                break;
                
            case 3:
                // Bloc d'allocation
                strcat(contenu, "Type: Allocation\n");
                Tableallocation* tableAlloc = &blocs[i].content.allocation;
                for (int j = 0; j < 20; j++) {
                    sprintf(temp, "Bloc d'allocation %d - Etat: %s\n", j,
                            tableAlloc->tablelocation[j].etat == 0 ? "Vide" : "Plein");
                    strcat(contenu, temp);
                }
                break;
                
            default:
                strcat(contenu, "Type: Inconnu\n");
                break;
        }
        
        // Afficher chaque bloc dans un cadre
        afficherAvecCadre(titre, contenu);

        // Affichage de l'état du bloc dans la table d'allocation (si applicable)
        if (i < allocation->nbrbloc) {
            char allocationInfo[100];
            sprintf(allocationInfo, "Adresse: %d, Etat: %s",
                    allocation->tablelocation[i].adrdebloc,
                    allocation->tablelocation[i].etat == 0 ? "Vide" : "Plein");
            afficherAvecCadre("Etat du bloc dans la table d'allocation", allocationInfo);
        }
    }

    // Affichage des blocs utilisés dans la table d'allocation
    printf("=== Etat de la table d'allocation ===\n");
    for (int i = 0; i < allocation->nbrbloc; i++) {
        char allocationInfo[100];
        sprintf(allocationInfo, "Bloc d'allocation %d - Etat: %s\n", i,
                allocation->tablelocation[i].etat == 0 ? "Vide" : "Plein");
        afficherAvecCadre("Etat des blocs d'allocation", allocationInfo);
    }

    char summary[100];
    sprintf(summary, "Nombre total de blocs utilisés : %d / %d", allocation->nbrblocutil, allocation->nbrbloc);
    afficherAvecCadre("Résumé de l'allocation", summary);

    printf("=== Fin de l'état du fichier .bin ===\n");
}




//------------------------------------------- Fonction de compactage du fichier en réorganisant les blocs.-------------------------------------------------------------------

void compactage(Bloc* blocs, int nbrBlocs, BlocAllocation* allocation) {
    // Parcours des blocs pour déplacer les données
    int i, j;
    int blocLibreIndex = -1;

    // Première étape : Réorganiser les blocs de données en éliminant les blocs vides (non utilisés).
    for (i = 0; i < nbrBlocs; i++) {
        if (blocs[i].typedebloc == 2 && blocs[i].content.fileData.nbrmaladie > 0) {
            // Si le bloc est utilisé et contient des données, il doit être compacté
            if (blocLibreIndex != -1) {
                // Déplacer les données du bloc actuel vers le bloc libre
                blocs[blocLibreIndex] = blocs[i];
                blocs[i].typedebloc = 0;  // Marquer comme vide après déplacement
                allocation->tablelocation[blocLibreIndex].etat = 1;  // Bloc maintenant plein
                allocation->tablelocation[i].etat = 0;  // Bloc maintenant vide
            }
        } else if (blocs[i].typedebloc == 0) {
            // Si le bloc est vide, nous le marquons comme disponible
            blocLibreIndex = i;
        }
    }

    // Mise à jour de la table d'allocation : compactage des blocs et gestion des adresses
    for (i = 0; i < allocation->nbrblocutil; i++) {
        if (allocation->tablelocation[i].etat == 1) {
            // Bloc utilisé, vérifier s'il doit être déplacé
            for (j = 0; j < allocation->nbrbloc; j++) {
                if (allocation->tablelocation[j].etat == 0) {
                    // Déplacer le contenu dans un bloc vide si nécessaire
                    allocation->tablelocation[j] = allocation->tablelocation[i];
                    allocation->tablelocation[i].etat = 0;
                    break;
                }
            }
        }
    }

    // Mise à jour des métadonnées après compactage
    BlocMetadonnees* blocMeta = &blocs[0].content.metadataTable; // Par exemple, on commence avec le premier bloc
    for (i = 0; i < blocMeta->nbrMetadonnees; i++) {
        // Parcourir les métadonnées et ajuster les adresses des blocs
        fichiermetadonnes* fichier = &blocMeta->T[i];
        if (fichier->Modeorganisationglobale == 0) {
            // Si c'est une chaîne, mettre à jour les adresses si nécessaire
            // Exemple de mise à jour des adresses des blocs de données
            fichier->Adrpremierbloc = allocation->tablelocation[fichier->Adrpremierbloc].adrdebloc;
        }
    }
    
    // Mise à jour du nombre de blocs utilisés après compactage
    allocation->nbrblocutil = 0;
    for (i = 0; i < allocation->nbrbloc; i++) {
        if (allocation->tablelocation[i].etat == 1) {
            allocation->nbrblocutil++;
        }
    }
    
    printf("Compactage terminé. Nombre de blocs utilisés après compactage : %d\n", allocation->nbrblocutil);
}

  



//------------------------------------------- Fonction qui vérifie s'il y a de l'espace contigu dans la mémoire centrale--------------------------------------


// Fonction pour vérifier la disponibilité de blocs libres
bool verifierEspaceLibre(BlocAllocation* allocation, int nombreBlocsRequis) {
    int blocsLibres = 0;
    // Compter le nombre de blocs libres
    for (int i = 0; i < allocation->nbrbloc; i++) {
        if (allocation->tablelocation[i].etat == 0) {
            blocsLibres++;
        }
    }
    return blocsLibres >= nombreBlocsRequis; // Retourne vrai si assez de blocs libres
}

// Fonction de compactage proposée si l'espace est insuffisant
void proposerCompactage(Bloc* blocs, int nbrBlocs, BlocAllocation* allocation) {
    printf("Espace insuffisant.  compactage en courants ...\n");

    // Appel de la fonction de compactage pour réorganiser les blocs et récupérer de l'espace libre
    compactage(blocs, nbrBlocs, allocation);
}

// Fonction principale de gestion de l'espace avant une opération (création ou insertion)
void gererEspace(Bloc* blocs, int nbrBlocs, BlocAllocation* allocation, int nombreBlocsRequis) {
    // Vérifier si l'espace libre est suffisant
    if (verifierEspaceLibre(allocation, nombreBlocsRequis)) {
        printf("Espace suffisant pour effectuer l'opération.\n");
        return;  // L'espace est suffisant, on peut continuer l'opération
    }

    // Si l'espace est insuffisant, proposer un compactage
    proposerCompactage(blocs, nbrBlocs, allocation);

    // Re-vérifier l'espace après compactage
    if (verifierEspaceLibre(allocation, nombreBlocsRequis)) {
        printf("Compactage effectué. L'espace est maintenant suffisant pour effectuer l'opération.\n");
        return;  // Après compactage, l'espace est suffisant
    }

    // Si l'espace reste insuffisant après compactage
    printf("Erreur : Espace insuffisant même après compactage. La mémoire secondaire est pleine.\n");
}



//--------le mennu interactif(le programme principale ----------------------------------------------------------------------------------------------------------------------------
int  main (){
    int modeoI,modeoG ;
    MS ms ;
    int nbrBloc,nbrBlocs ;
    bloc blocs;
    BlocAllocation* allocation;
    printf ("give the global organisation of ur file");//demander le mode org globale
    scanf("%d",&modeoG);
     printf ("give the internel organisation of ur file");//demander le mode d organisation interne
    scanf ("%d",&modeoI);
    if( modeoG == 0 ){
        if (modeoI==1)
        {
            modeoG = 1 ;//metre le modeG a 1 pour distinguer les cas 
        }
    }
      if( modeoG == 1 ){
        if (modeoI==1)
        {
            modeoG = 3 ;//metre le modeG a 3 pour distinguer les cas 
        }else {
            modeoG = 2 ;
        }
    }
    /*sortir de ces deux if avec les cas suivant si mmodeoG = 0 et modeoI = 0 alors modeoG = 0
    si mmodeoG = 0 et modeoI = 1 alors modeoG = 1 . si mmodeoG = 1 et modeoI = 0 alors modeoG  . si mmodeoG = 0 et modeoI = 0 alors modeoG = 3*/
   

   //un switch pour le choix de l organisation 
  switch (modeoG)
     {
    case 0 :
        print("choisisser l'opeartion ");
        scan("%d",&choix);
        print("\n--- Gestion de la Mémoire Secondaire ---\n");
        print("1. Initialiser la mémoire secondaire\n");
        print("2. Créer un fichier\n");
        print("3. Afficher l'état de la mémoire secondaire\n");
        print("4. Afficher les métadonnées des fichiers\n");
        print("5. Rechercher un enregistrement\n");
        print("6. Insérer un nouvel enregistrement\n");
        print("7. Supprimer un enregistrement\n");
        print("8. Défragmenter un fichier\n");
        print("9. Supprimer un fichier\n");
        print("10. Renommer un fichier\n");
        print("11. Compacter la mémoire secondaire\n");
        print("12. Vider la mémoire secondaire\n");
        print("13. suppression logique de fichier\n");
        print("0. Quitter\n");
        print("Votre choix : ");
        scan("%d", &choix);

 
scanf("%d",&choix);
 switch(choix) {
            case 1:
                print("Initialisation de la mémoire secondaire\n");
                InitMs(&disque, 20);
                break;
            case 2:
                print("Création d'un fichier\n");
                // Créer le fichier en fonction des choix d'organisation
            
                creerfichierCO(FILE* disque); 
                
                break;
            case 3:
                // Afficher l'état de la mémoire secondaire
                afficherEtatMemoire(NULL); // Exemple d'appel
                break;
            case 4:
                print("Afficher les détails de la mémoire secondaire :\n");
                afficherEtatFichier( blocs, nbrBlocs, allocation);
                break;
            case 5:
                print("Recherche d'enregistrement\n");
                print("Donnez le nom de fichier  ");
                 string  nomfichier ;
                 scan("%s",&nomfichier);
                int ID;
                scan("%d",ID)
                  rechercheebregistrement(disque,nomfichier,ID); 
            
               
                break;
            case 6:
                print("Insertion d'enregistrement\n");
                 print("Donnez le nom de fichier  ");
                 string  nomfichier ;
                 scan("%s",&nomfichier);
              
                //  le fichier en fonction des choix d'organisation
             
                    insertionenregistrement(disque,nomfichier);
              
                break;
            case 7:
                print("Suppression d'enregistrement\n");
                print("Votre nom de fichier: ");
                scan("%d", &filename);
                // le fichier en fonction des choix d'organisation
                //supretio physique
                  suprimereregistrement(disque,fileame,ID);
                
                break;
            case 8:
                print("Défragmentation effectuée\n");
                string nomfichier;
                // le fichier en fonction des choix d'organisation
                scan("%s",&nomfichier);
                defragmentation(disque,nomfichier);
                
                break;
            case 9:
                print("Suppression de fichier\n");
                print("donner  le nom du fichier");
                scan("%s",fichiern);
                // le fichier en fonction des choix d'organisation
                     suprimerFCO(disque,fichiern);
                 
                break;
            case 10:
                print("Renommage de fichier\n");
                string  newname,lastname;
                print("donner  le nom du fichier que  vous voulez change son nom");
                scan("%s",&lastname);
                print("donner le nouveau nom");
                scan("%s",&newname);
                changeFileName( disque ,newname ,lastname );
                
                break;
            case 11:
                print("Compactage de la mémoire secondaire\n");
                // Appeler la fonction de compactage
                compactage( blocs, nbrBlocs, allocation);
                break;
            case 12:
                print("Mémoire secondaire vidée\n");
                viderMS(disque, 20);
                break;
            case  13:
                print("suppretion logique d'enregistrement");
                string nomfichier;
                sacn("%d",&nomfichier);
                int id  ;
                print("Donner  le id de votre enregistrement : ");
                scan("%d", &id);
                suppretionLogique(disque , id,nomfichier);
                 
               
            case 0:
                print("Programme terminé !\n");
                break;
            default:
                print("Choix invalide. Veuillez réessayer.\n");
        }
    } while (choix != 0);
    break;
    case 1 : 
     print("choisisser l'opeartion ");
        scan("%d",&choix);
        print("\n--- Gestion de la Mémoire Secondaire ---\n");
        print("1. Initialiser la mémoire secondaire\n");
        print("2. Créer un fichier\n");
        print("3. Afficher l'état de la mémoire secondaire\n");
        print("4. Afficher les métadonnées des fichiers\n");
        print("5. Rechercher un enregistrement\n");
        print("6. Insérer un nouvel enregistrement\n");
        print("7. Supprimer un enregistrement\n");
        print("8. Défragmenter un fichier\n");
        print("9. Supprimer un fichier\n");
        print("10. Renommer un fichier\n");
        print("11. Compacter la mémoire secondaire\n");
        print("12. Vider la mémoire secondaire\n");
        print("13. suppression logique de fichier\n");
        print("0. Quitter\n");
        print("Votre choix : ");
        scan("%d", &choix);

 
scanf("%d",&choix);
 switch(choix) {
            case 1:
                print("Initialisation de la mémoire secondaire\n");
                InitMs(&disque, 20);
                break;
            case 2:
                print("Création d'un fichier\n");
                // Créer le fichier en fonction des choix d'organisation
            
                creationLL_OF(disque , ms , nbrBloc); 
                
                break;
            case 3:
                // Afficher l'état de la mémoire secondaire
                afficherEtatMemoire(NULL); // Exemple d'appel
                break;
            case 4:
                print("Afficher les détails de la mémoire secondaire :\n");
                afficherEtatFichier( blocs, nbrBlocs, allocation);
                break;
            case 5:
                print("Recherche d'enregistrement\n");
                print("Donnez le nom de fichier  ");
                 string  nomfichier ;
                 scan("%s",&nomfichier);
                int ID;
                scan("%d",ID)
                  researchDis(disque, ms ,nomfichier); 
            
               
                break;
            case 6:
                print("Insertion d'enregistrement\n");
                 print("Donnez le nom de fichier  ");
                 string  nomfichier ;
                 scan("%s",&nomfichier);
              
                //  le fichier en fonction des choix d'organisation
                insertDis(disque,nomchifier);
                break;
            case 7:
                print("Suppression d'enregistrement\n");
                print("Votre nom de fichier: ");
                scan("%d", &filename);
                // le fichier en fonction des choix d'organisation
                //supretio physique
                  suppPhysique(disque,ms,fileame);
                
                break;
            case 8:
                print("Défragmentation effectuée\n");
                string nomfichier;
                // le fichier en fonction des choix d'organisation
                scan("%s",&nomfichier);
                defragmentation(disque,ms,nomfichier);
                
                break;
            case 9:
                print("Suppression de fichier\n");
                print("donner  le nom du fichier");
                scan("%s",fichiern);
                // le fichier en fonction des choix d'organisation
                     deleteL_OF(disque,fichiern);
                break;
            case 10:
                print("Renommage de fichier\n");
                string  newname,lastname;
                print("donner  le nom du fichier que  vous voulez change son nom");
                scan("%s",&lastname);
                print("donner le nouveau nom");
                scan("%s",&newname);
                changeFileName( disque ,newname ,lastname );
                
                break;
            case 11:
                print("Compactage de la mémoire secondaire\n");
                // Appeler la fonction de compactage
                compactage( blocs, nbrBlocs, allocation);
                break;
            case 12:
                print("Mémoire secondaire vidée\n");
                viderMS(disque, 20);
                break;
            case  13:
                print("suppretion logique d'enregistrement");
                string nomfichier;
                sacn("%d",&nomfichier);
                int id  ;
                print("Donner  le id de votre enregistrement : ");
                scan("%d", &id);
                suppLogique(disque , id,nomfichier);
                 
               
            case 0:
                print("Programme terminé !\n");
                break;
            default:
                print("Choix invalide. Veuillez réessayer.\n");
        }
    } while (choix != 0);
    break;
    case 2 :
     print("choisisser l'opeartion ");
        scan("%d",&choix);
        print("\n--- Gestion de la Mémoire Secondaire ---\n");
        print("1. Initialiser la mémoire secondaire\n");
        print("2. Créer un fichier\n");
        print("3. Afficher l'état de la mémoire secondaire\n");
        print("4. Afficher les métadonnées des fichiers\n");
        print("5. Rechercher un enregistrement\n");
        print("6. Insérer un nouvel enregistrement\n");
        print("7. Supprimer un enregistrement\n");
        print("8. Défragmenter un fichier\n");
        print("9. Supprimer un fichier\n");
        print("10. Renommer un fichier\n");
        print("11. Compacter la mémoire secondaire\n");
        print("12. Vider la mémoire secondaire\n");
        print("13. suppression logique de fichier\n");
        print("0. Quitter\n");
        print("Votre choix : ");
        scan("%d", &choix);

 
scanf("%d",&choix);
 switch(choix) {
            case 1:
                print("Initialisation de la mémoire secondaire\n");
                InitMs(&disque, 20);
                break;
            case 2:
                print("Création d'un fichier\n");
                // Créer le fichier en fonction des choix d'organisation
            
                creerFichier(disque ); 
                
                break;
            case 3:
                // Afficher l'état de la mémoire secondaire
                afficherEtatMemoire(NULL); // Exemple d'appel
                break;
            case 4:
                print("Afficher les détails de la mémoire secondaire :\n");
                afficherEtatFichier( blocs, nbrBlocs, allocation);
                break;
            case 5:
                print("Recherche d'enregistrement\n");
                print("Donnez le nom de fichier  ");
                 string  nomfichier ;
                 scan("%s",&nomfichier);
                int ID;
                scan("%d",ID)
                  rechercheparid(disque, idRecherche ); 
            
               
                break;
            case 6:
                print("Insertion d'enregistrement\n");
                 print("Donnez le nom de fichier  ");
                 string  nomfichier ;
                 scan("%s",&nomfichier);
              
                //  le fichier en fonction des choix d'organisation
                IsererEnregistrement(disque,id);
                break;
            case 7:
                print("Suppression d'enregistrement\n");
                print("Votre nom de fichier: ");
                scan("%d", &filename);
                // le fichier en fonction des choix d'organisation
                //supretio physique
                int id ;
                print("donner le id");
                scan("%d",&id);
                  supretionPhsique(disque,id ,ms)
                
                break;
            case 8:
                print("Défragmentation effectuée\n");
                string nomfichier;
                // le fichier en fonction des choix d'organisation
                scan("%s",&nomfichier);
                defragmentation(disque;
                
                break;
            case 9:
                print("Suppression de fichier\n");
                print("donner  le nom du fichier");
                scan("%s",fichiern);
                // le fichier en fonction des choix d'organisation
                    Supprimerfichier(disque,nomFichier);
                break;
            case 10:
                print("Renommage de fichier\n");
                string  newname,lastname;
                print("donner  le nom du fichier que  vous voulez change son nom");
                scan("%s",&lastname);
                print("donner le nouveau nom");
                scan("%s",&newname);
                changeFileName( disque ,newname ,lastname );
                
                break;
            case 11:
                print("Compactage de la mémoire secondaire\n");
                // Appeler la fonction de compactage
                compactage( blocs, nbrBlocs, allocation);
                break;
            case 12:
                print("Mémoire secondaire vidée\n");
                viderMS(disque, 20);
                break;
            case  13:
                print("suppretion logique d'enregistrement");
                string nomfichier;
                sacn("%d",&nomfichier);
                int id  ;
                print("Donner  le id de votre enregistrement : ");
                scan("%d", &id);
                SuppressionLogique(disque , id);
                 
               
            case 0:
                print("Programme terminé !\n");
                break;
            default:
                print("Choix invalide. Veuillez réessayer.\n");
        }
    } while (choix != 0);
    break;
    case 3 : 
     print("choisisser l'opeartion ");
        scan("%d",&choix);
        print("\n--- Gestion de la Mémoire Secondaire ---\n");
        print("1. Initialiser la mémoire secondaire\n");
        print("2. Créer un fichier\n");
        print("3. Afficher l'état de la mémoire secondaire\n");
        print("4. Afficher les métadonnées des fichiers\n");
        print("5. Rechercher un enregistrement\n");
        print("6. Insérer un nouvel enregistrement\n");
        print("7. Supprimer un enregistrement\n");
        print("8. Défragmenter un fichier\n");
        print("9. Supprimer un fichier\n");
        print("10. Renommer un fichier\n");
        print("11. Compacter la mémoire secondaire\n");
        print("12. Vider la mémoire secondaire\n");
        print("13. suppression logique de fichier\n");
        print("0. Quitter\n");
        print("Votre choix : ");
        scan("%d", &choix);

 
scanf("%d",&choix);
 switch(choix) {
            case 1:
                print("Initialisation de la mémoire secondaire\n");
                InitMs(&disque, 20);
                break;
            case 2:
                print("Création d'un fichier\n");
                // Créer le fichier en fonction des choix d'organisation
            
                chargerFileTNOF(disque , ms , nbrBloc); 
                
                break;
            case 3:
                // Afficher l'état de la mémoire secondaire
                afficherEtatMemoire(NULL); // Exemple d'appel
                break;
            case 4:
                print("Afficher les détails de la mémoire secondaire :\n");
                afficherEtatFichier( blocs, nbrBlocs, allocation);
                break;
            case 5:
                print("Recherche d'enregistrement\n");
                print("Donnez le nom de fichier  ");
                 string  nomfichier ;
                 scan("%s",&nomfichier);
                int ID;
                scan("%d",ID)
                  RechercheFILETOF(disque, ID); ////yousraaaa
            
               
                break;
            case 6:
                print("Insertion d'enregistrement\n");
                 print("Donnez le nom de fichier  ");
                 string  nomfichier ;
                 scan("%s",&nomfichier);
              
                //  le fichier en fonction des choix d'organisation
                insertioFileTNOF(disque,nomchifier);
                break;
            case 7:
                print("Suppression d'enregistrement\n");
                print("Votre nom de fichier: ");
                scan("%d", &filename);
                // le fichier en fonction des choix d'organisation
                //supretio physique
                  supprPhysiqueDefichierTNOF(disque,fileame);
                
                break;
            case 8:
                print("Défragmentation effectuée\n");
                string nomfichier;
                // le fichier en fonction des choix d'organisation
                scan("%s",&nomfichier);
                defragmentationFileTNOF(disque,ms,nomfichier);
                
                break;
            case 9:
                print("Suppression de fichier\n");
                print("donner  le nom du fichier");
                scan("%s",fichiern);
                // le fichier en fonction des choix d'organisation
                    supprPhysiqueTNOF(disque,fichiern);//supp du phichier yousraaaaa
                break;
            case 10:
                print("Renommage de fichier\n");
                string  newname,lastname;
                print("donner  le nom du fichier que  vous voulez change son nom");
                scan("%s",&lastname);
                print("donner le nouveau nom");
                scan("%s",&newname);
                changeFileName( disque ,newname ,lastname );
                
                break;
            case 11:
                print("Compactage de la mémoire secondaire\n");
                // Appeler la fonction de compactage
                compactage( blocs, nbrBlocs, allocation);
                break;
            case 12:
                print("Mémoire secondaire vidée\n");
                viderMS(disque, 20);
                break;
            case  13:
                print("suppretion logique d'enregistrement");
                string nomfichier;
                sacn("%d",&nomfichier);
                int id  ;
                print("Donner  le id de votre enregistrement : ");
                scan("%d", &id);
                supprLogiqueFileTOF(disque , id,nomfichier);
                 
               
            case 0:
                print("Programme terminé !\n");
                break;
            default:
                print("Choix invalide. Veuillez réessayer.\n");
        }
    } while (choix != 0);
    break;
  }
retur 0;
}

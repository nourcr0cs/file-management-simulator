#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>



    typedef struct {
        char Nomdufichier[20];
        int Taillefichierblocs;
        int Taillefichierenregistrements;
        int Adrpremierbloc; // Adresse du premier bloc
        int Modeorganisationglobale; // Si = 0 alors cha�ne
        int Modeorganisationinterne; // Si = 0 alors ordonn�
    } fichiermetadonnes;

    typedef struct {
        int id;
        char name[15];
        int age;
        char sexe[10];
        char adresse[30];
        int nmbrdevisite;
        int suprimelogiquement; // 1 si supprim� logiquement
    } maladie;

    typedef struct {
        maladie T[20]; // Facteur de blocage = 20
        int nbrmaladie;
        int next; // Pour cha�ner les blocs
    } BlocData;

    typedef struct {
        int adrdebloc; // Adresse de bloc
        int etat; // Si vide = 0, pleine = 1
    } Tableallocation;

    typedef struct {
        fichiermetadonnes T[20]; // Tableau de m�tadonn�es
        int nbrMetadonnees; // Nombre actuel de m�tadonn�es dans ce bloc
        int next; // Pour cha�ner les blocs de m�tadonn�es
    } BlocMetadonnees;

    typedef struct {
        Tableallocation tablelocation[20];
        int nbrblocutil; // Nombre de blocs utilis�s
        int nbrbloc; // Nombre total de blocs
    } BlocAllocation;

    typedef struct
    {

    int numerodebloc;
    int index;

    }adressemetadonnes;

    typedef struct {

        union {
            BlocMetadonnees metadataTable;
            BlocData fileData;
            BlocAllocation allocation;
        } content;

        int typedebloc;
        // 1 = m�tadonn�es, 2 = donn�es de fichier, 3 = allocation
    } Bloc;





    // Function to check if there is enough space for the requested number of blocks
    bool verifierEspaceSuffisant(FILE* disque, int nbrBlocsVoulu) {
        Bloc buffer;
        rewind(disque);
        buffer.typedebloc=3;
        fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        if (buffer.typedebloc != 3) {
            printf("Erreur : Le bloc 0 n'est pas un bloc d'allocation.\n");
            return false;
        }

        int blocsLibres = buffer.content.allocation.nbrbloc - buffer.content.allocation.nbrblocutil;

       if (nbrBlocsVoulu > blocsLibres) {
           printf("Erreur : Espace insuffisant. %d blocs n�cessaires, %d disponibles.\n", nbrBlocsVoulu,blocsLibres);
           return false;
       }

       printf("Succ�s : Il y a suffisamment d'espace. %d blocs disponibles.\n", blocsLibres);
       return true;
   }

   // Function to obtain the number of blocks based on the option provided
   int obtenirNombreBlocs(FILE* disque, int option) {
       Bloc buffer;
       buffer.typedebloc=3;
       fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
       fread(&buffer, sizeof(Bloc), 1, disque);

       if (buffer.typedebloc != 3) {
           printf("Erreur : Le bloc 0 n'est pas un bloc d'allocation.\n");
           return -1; // Error indicator
       }

       BlocAllocation* allocation = &buffer.content.allocation;

       switch (option) {
           case 1: // Nombre de blocs utilis�s
               return allocation->nbrblocutil;
           case 2: // Nombre total de blocs
               return allocation->nbrbloc;
           default:
               printf("Erreur : Option invalide. Utilisez 1 pour blocs utilis�s ou 2 pour blocs totaux.\n");
               return -1; // Error indicator
       }
   }

   // Function to update the number of blocks used or total blocks
   void mettreAJourNombreBlocs(FILE* disque, int option, int nouvelleValeur) {
       Bloc buffer;


       buffer.typedebloc=3;


       fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
       fread(&buffer, sizeof(Bloc), 1, disque);



       BlocAllocation* allocation = &buffer.content.allocation;

       switch (option) {
           case 1: // Update number of blocks used
               allocation->nbrblocutil = nouvelleValeur;
               break;
           case 2: // Update total number of blocks
               if (nouvelleValeur < allocation->nbrblocutil) {
                   return; // Cannot reduce total blocks below used blocks
               }
               allocation->nbrbloc = nouvelleValeur;
               break;
           default:
               return;
       }

      fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
      fwrite(&buffer, sizeof(Bloc), 1, disque);
   }

   void metajourtableallocation(FILE* disque, int blocIndex, int etat) {
       Bloc buffer;
       buffer.typedebloc=3;
       fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
       fread(&buffer, sizeof(Bloc), 1, disque);



       // Update the allocation table entry for the specified blocIndex
       buffer.content.allocation.tablelocation[blocIndex].etat = etat;

       // Write the updated allocation table back to the first block
       fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
       if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
           printf("Erreur : �chec d'�criture dans le bloc 0.\n");
           return;
       }
   }

   // Function to create the allocation table in the first block
   void CreeTableAllocation( FILE* disque) {
       Bloc buffer;
       rewind(disque);
        // Set the block type to 3 for table allocation
       fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
       fread(&buffer, sizeof(Bloc), 1, disque);
      buffer.typedebloc = 3;
      buffer.content.allocation.tablelocation[0].etat=1;
      buffer.content.allocation.tablelocation[1].etat=1;
      buffer.content.allocation.tablelocation[2].etat=1;
      buffer.content.allocation.nbrbloc = 20;
      buffer.content.allocation.nbrblocutil = 3;

       for (int i = 3; i < 20; i++) {
           buffer.content.allocation.tablelocation[i].adrdebloc = i;
           buffer.content.allocation.tablelocation[i].etat = 0;
       }

       // Write the initial allocation table to the first block
       fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
       if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
           printf("Erreur : �chec d'�criture dans le bloc 0.\n");
           return;
       }


       // Mark the first block as allocated

        printf("succes");
   }

   // Function to clear memory space by resetting block usage counts and allocation table
   void ViderMs(FILE* disque) {
    Bloc buffer = {0}; // Initialiser un bloc vide (toutes valeurs � 0/null)
    int nombreBlocs = obtenirNombreBlocs(disque, 2); // Obtenir le nombre total de blocs

    // Parcourir tous les blocs et les r�initialiser
    for (int i = 0; i < nombreBlocs; i++) {
        fseek(disque, i * sizeof(Bloc), SEEK_SET);
        if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Erreur : Impossible de r�initialiser le bloc %d.\n", i);
            return;
        }
    }

    printf("Tous les blocs ont �t� r�initialis�s � NULL.\n");
}


   // Function to initialize memory space with a specified number of blocks


   void InitMs(FILE* disque, int nombreBlocs) {
       Bloc buffer = {0};

       // Initialisation du bloc 0 (table d'allocation)
       buffer.typedebloc = 3; // Bloc d'allocation

       CreeTableAllocation(  disque);

       buffer.typedebloc=1;

       // Initialisation des blocs 1 et 2 (m�tadonn�es)
       buffer.typedebloc = 1; // Bloc de m�tadonn�es
       buffer.content.metadataTable.nbrMetadonnees = 0;
       buffer.content.metadataTable.next = 2;


       fseek(disque, 1 * sizeof(Bloc), SEEK_SET);
           if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : �chec d'�criture dans le bloc %d.\n", 1);
               return;
           }

         buffer.content.metadataTable.nbrMetadonnees = 0;
          buffer.content.metadataTable.next = -1;

        fseek(disque, 2 * sizeof(Bloc), SEEK_SET);
           if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : �chec d'�criture dans le bloc %d.\n", 2);
               return;
           }

   }


   // Function to add metadata for a file into the system's memory structure
   bool ajoutermetadonnes(FILE* disque,fichiermetadonnes metadonnes,int taille){
      Bloc buffer;
      rewind(disque);

      if(!verifierEspaceSuffisant(disque, taille)) {
               return false;
      }



      int blocactuelle = 1;

      while(blocactuelle != -1) {
          fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
          fread(&buffer,sizeof(Bloc),1,disque);

          if(buffer.content.metadataTable.nbrMetadonnees < 20) {
              buffer.content.metadataTable.T[buffer.content.metadataTable.nbrMetadonnees] = metadonnes;
              buffer.content.metadataTable.nbrMetadonnees++;

              fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
              fwrite(&buffer,sizeof(Bloc),1,disque);

              printf("M�tadonn�es ajout�es avec succ�s au bloc %d.\n", blocactuelle);
              return true;
          } else {
              blocactuelle = buffer.content.metadataTable.next;
          }
      }
      return false;


   }

   // Function to search for metadata by file name and return its address information.
   adressemetadonnes recherchemetadonnes(FILE* disque, const char* nomfichier) {
       Bloc buffer;
       adressemetadonnes resultat = {-1, -1}; // Initialisation : non trouv�
       int blocActuel = 1; // Commence par le premier bloc de m�tadonn�es

       // Parcours des blocs de m�tadonn�es en cha�ne
       while (blocActuel != -1) {
           // Lecture du bloc actuel
           fseek(disque, blocActuel * sizeof(Bloc), SEEK_SET);
           if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : Impossible de lire le bloc %d.\n", blocActuel);
               break;
           }

           // V�rifier si le bloc est un bloc de m�tadonn�es
           if (buffer.typedebloc != 1) {
               printf("Erreur : Le bloc %d n'est pas un bloc de m�tadonn�es.\n", blocActuel);
               break;
           }

           // Parcourir les m�tadonn�es dans le bloc
           for (int j = 0; j < buffer.content.metadataTable.nbrMetadonnees; j++) {
               if (buffer.content.metadataTable.T[j].Nomdufichier[0] != '\0' &&
                   strcmp(buffer.content.metadataTable.T[j].Nomdufichier, nomfichier) == 0) {
                   // M�tadonn�es trouv�es
                   resultat.numerodebloc = blocActuel;
                   resultat.index = j;
                   return resultat;
               }
           }

           // Passer au bloc suivant
           blocActuel = buffer.content.metadataTable.next;
       }

       printf("M�tadonn�es non trouv�es pour le fichier : %s\n", nomfichier);
       return resultat; // Retourne {-1, -1} si non trouv�
   }


   // Function to read specific metadata characteristics based on given parameter.
   int liremetadonnes(FILE* disque,const char* nomFichier,int caracteristique ){
        Bloc buffer ;
        adressemetadonnes adresse=recherchemetadonnes(disque ,nomFichier );
        rewind(disque);

        if(adresse.numerodebloc==-1) { // V�rifier si le fichier existe
            printf("Fichier introuvable\n");
            return -1; // Indicate error since file not found.
        }

        fseek(disque ,adresse.numerodebloc*sizeof(Bloc ),SEEK_SET );

        fread(&buffer,sizeof(Bloc ),1 ,disque );

        switch(caracteristique) {
            case 1: // taille en blocs
                return buffer.content.metadataTable.T[adresse.index].Taillefichierblocs ;
            case 2: // taille en enregistrements
                return buffer.content.metadataTable.T[adresse.index].Taillefichierenregistrements ;
            case 3: // adresse du premier bloc
                return buffer.content.metadataTable.T[adresse.index].Adrpremierbloc ;
            case 4: // mode d'organisation globale
                return buffer.content.metadataTable.T[adresse.index].Modeorganisationglobale ;
            case 5: // mode d'organisation interne
                return buffer.content.metadataTable.T[adresse.index].Modeorganisationinterne ;

            default:
                printf("Caract�ristique non trouv�e\n");
                return -1;
        }
   }

   // Function to update metadata after insertion or deletion.
   void miseAJourMetadonnees(FILE* disque,const char* nomFichier,int champ,int nouvelleValeur){
        adressemetadonnes adresse=recherchemetadonnes(disque ,nomFichier );
        rewind(disque);
        if(adresse.numerodebloc==-1){
            printf("Fichier introuvable pour mise � jour des m�tadonn�es.\n");
            return ;
        }

        Bloc buffer ;
        fseek(disque ,adresse.numerodebloc*sizeof(Bloc ),SEEK_SET );

        fread(&buffer,sizeof(Bloc ),1 ,disque );

        if(buffer.typedebloc!=1){
            printf("Erreur : Le bloc trouv� ne contient pas de m�tadonn�es.\n");
            return ;
        }

        switch(champ){
            case 1:
                buffer.content.metadataTable.T[adresse.index].Taillefichierblocs=nouvelleValeur ;
                break ;
            case 2:
                buffer.content.metadataTable.T[adresse.index].Taillefichierenregistrements=nouvelleValeur ;
                break ;
            case 3:
                buffer.content.metadataTable.T[adresse.index].Adrpremierbloc=nouvelleValeur ;
                break ;
            case 4:
                buffer.content.metadataTable.T[adresse.index].Modeorganisationglobale=nouvelleValeur ;
                break ;
            case 5:
                buffer.content.metadataTable.T[adresse.index].Modeorganisationinterne=nouvelleValeur ;
                break ;

            default:
                printf("Champ non valide.\n");
                return ;
        }

        fseek(disque ,adresse.numerodebloc*sizeof(Bloc ),SEEK_SET );

        fwrite(&buffer,sizeof(Bloc ),1 ,disque );
   }

   // Function to create a new file and its associated metadata in the system.
   char* creerfichierCO(FILE* disque){
        char* nomFichier=(char*)malloc(20*sizeof(char));
        if(!nomFichier){
            printf("Erreur : Allocation de memoire echoue.\n");
            return NULL;
        }
        int Taillefichierenregistrements=0;

        fichiermetadonnes metadonnes;
        rewind(disque);

        printf("Donner le nom du fichier : \n");
        scanf("%19s",metadonnes.Nomdufichier);

        printf("Donner la taille de fichier en enregistrement : \n");
        scanf("%d",&Taillefichierenregistrements);

        printf("Donner le mode d'organisation globale : \n");
        scanf("%d",&metadonnes.Modeorganisationglobale);

        printf("Donner le mode d'organisation interne : \n");
        scanf("%d",&metadonnes.Modeorganisationinterne);

     int taille=Taillefichierenregistrements/20+(Taillefichierenregistrements%20!=0);

        metadonnes.Taillefichierblocs=taille;
        metadonnes.Taillefichierenregistrements=0;

        if(!verifierEspaceSuffisant(disque ,taille)){
            free(nomFichier);
            return NULL;
        }

        bool succes=ajoutermetadonnes(disque ,metadonnes ,taille);

        if(succes){
            printf("Le fichier '%s' a ete cree avec succes.\n", metadonnes.Nomdufichier);
            strcpy(nomFichier ,metadonnes.Nomdufichier);
            return nomFichier;
        } else {
            printf("Erreur : espace insuffisant pour creer le fichier '%s'.\n", metadonnes.Nomdufichier);
            free(nomFichier);
            return NULL;
        }
   }

   void chargerfichier(FILE* disque) {

       Bloc buffer;
       char* nomFichier;
       rewind(disque);

       // R�cup�rer les m�tadonn�es du fichier
       fichiermetadonnes metadonnes;
       nomFichier=creerfichierCO(disque);
       int blocnecessaire = liremetadonnes(disque,nomFichier,1);
       int bloctrouve = 0; // Pour compter les blocs trouv�s
       int adrPremierBloc = -1; // Variable pour stocker l'adresse du premier bloc
       int nbrbloctotal=obtenirNombreBlocs(disque,2);
       int nbrblocutiliser=obtenirNombreBlocs(disque,1);
       int blocPrecedent=-1;
       int place=-1;




       // Allouer les blocs n�cessaires
       for (int i = 0; i < nbrbloctotal && bloctrouve < blocnecessaire; i++) {

           for (int j = 0; j < nbrbloctotal; j++) {
            buffer.typedebloc=3;
            fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
            fread(&buffer, sizeof(Bloc), 1, disque);
            if (buffer.content.allocation.tablelocation[j].etat == 0) {
                place = j;
                metajourtableallocation(disque,j,1);

                break;
            }
        }
        if (place == -1) {
            printf("Espace insuffisant pour inserer un nouvel enregistrement.\n");
            return;
        }

    // Si c'est le premier bloc allou�
    if (adrPremierBloc == -1) {
        adrPremierBloc = place;
        blocPrecedent=place;
        fseek(disque, adrPremierBloc * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);
        buffer.content.fileData.next=-1;
        buffer.typedebloc=2;
        fseek(disque, adrPremierBloc * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);


    } else {
        // Cha�nage des blocs : mise � jour du champ "next" du dernier bloc allou�

        fseek(disque, blocPrecedent * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);
        buffer.content.fileData.next= place;  // Mise � jour du champ "next"
        buffer.typedebloc=2;
        fseek(disque, blocPrecedent * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);
        fseek(disque, place* sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);
        buffer.typedebloc=2;
        buffer.content.fileData.next=-1;
        fseek(disque, place * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);



    }

    // Enregistrer l'adresse du bloc actuel pour le cha�nage futur
    blocPrecedent=place ;
    bloctrouve++;
}

       // Si les blocs n�cessaires n'ont pas �t� trouv�s

       if (bloctrouve < blocnecessaire) {
           printf("Espace insuffisant pour allouer tous les blocs necessaires.\n");
           return;

           // Lib�ration des blocs d�j� allou�s
           for (int i = 0; i < nbrbloctotal; i++) {
               if (buffer.content.allocation.tablelocation[i].etat == 1) {
                   metajourtableallocation(disque, i, 0);
                   printf("Bloc %d libere.\n", i);
               }
           }
           return;
       }

       // Mise � jour des m�tadonn�es
       miseAJourMetadonnees(disque, nomFichier, 3, adrPremierBloc);
       mettreAJourNombreBlocs(disque,1,nbrblocutiliser+blocnecessaire);

       printf("Fichier charge avec succes.\n");
   }

  void defragmentation(FILE *disque, const char *nomFichier) {
    Bloc buffer;             // Buffer pour charger les blocs
    maladie temp[20];        // Tableau temporaire pour r�organiser les enregistrements
    int blocactuelle, blocsuivant; // Pointeurs pour le bloc courant et suivant
    int indexTemp = 0;       // Indice pour remplir le tableau temporaire
    int totalEnregistrements = 0; // Compteur pour les enregistrements valides
    int taillefichierblocs;  // Nombre de blocs utilis�s apr�s d�fragmentation
    rewind(disque);

    // Obtenir le premier bloc et le nombre total d'enregistrements
    int debut = liremetadonnes(disque, nomFichier, 3);
    if (debut == -1) {
        printf("Erreur : Le fichier %s est introuvable.\n", nomFichier);
        return;
    }

    totalEnregistrements = liremetadonnes(disque, nomFichier, 2); // Nombre total d'enregistrements
    if (totalEnregistrements == 0) {
        printf("Le fichier %s est vide. Aucune defragmentation necessaire.\n", nomFichier);
        return;
    }

    // �tape 1 : Collecter tous les enregistrements valides
    blocactuelle = debut;
    while (blocactuelle != -1) {
        // Charger le bloc courant
        fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
        if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Erreur : Impossible de lire le bloc %d.\n", blocactuelle);
            return;
        }

        // Collecter les enregistrements valides dans le tableau temporaire
        for (int i = 0; i < buffer.content.fileData.nbrmaladie; i++) {
            if (!buffer.content.fileData.T[i].suprimelogiquement) {
                temp[indexTemp++] = buffer.content.fileData.T[i];
            }
        }

        blocactuelle = buffer.content.fileData.next; // Passer au bloc suivant
    }

    // Calculer le nombre de blocs n�cessaires apr�s d�fragmentation
    taillefichierblocs = (indexTemp + 20 - 1) / 20; // Diviser en arrondissant vers le haut

    // �tape 2 : R��crire les enregistrements dans les blocs n�cessaires
    blocactuelle = debut;
    indexTemp = 0;
    for (int i = 0; i < taillefichierblocs; i++) {
        // Charger ou initialiser un bloc existant
        fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
        if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Erreur : Impossible de lire le bloc %d.\n", blocactuelle);
            return;
        }

        // Remplir le bloc avec les enregistrements valides
        int nbrEnregistrements = 0;
        while (indexTemp < totalEnregistrements && nbrEnregistrements < 20) {
            buffer.content.fileData.T[nbrEnregistrements++] = temp[indexTemp++];
        }

        buffer.content.fileData.nbrmaladie = nbrEnregistrements;

        // Gestion du cha�nage
        if (i == taillefichierblocs - 1) {
            buffer.content.fileData.next = -1; // Dernier bloc
        } else {
            blocsuivant = buffer.content.fileData.next; // Conserver le cha�nage
        }

        // �crire le bloc mis � jour
        fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
        if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Erreur : �chec d'�criture dans le bloc %d.\n", blocactuelle);
            return;
        }

        blocactuelle = buffer.content.fileData.next;
    }

    // �tape 3 : Lib�rer les blocs inutilis�s dans la table d'allocation
    while (blocactuelle != -1) {
        fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
        if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Erreur : Impossible de lire le bloc %d.\n", blocactuelle);
            return;
        }

        // Lib�rer le bloc courant
        metajourtableallocation(disque, blocactuelle, 0);

        blocactuelle = buffer.content.fileData.next;
    }

    // �tape 4 : Mise � jour des m�tadonn�es
    miseAJourMetadonnees(disque, nomFichier, 1, taillefichierblocs); // Taille en blocs
    miseAJourMetadonnees(disque, nomFichier, 2, totalEnregistrements); // Nombre d'enregistrements

    printf("Defragmentation du fichier %s terminee avec succes.\n", nomFichier);
    return;
}












void afficherEnregistrements(FILE* disque, const char* nomFichier) {
    Bloc buffer;
    int blocactuelle = liremetadonnes(disque, nomFichier, 3); // Adresse du premier bloc

    if (blocactuelle == -1) {
        printf("Erreur : Le fichier %s est introuvable.\n", nomFichier);
        return;
    }

    printf("=== Contenu du fichier %s ===\n", nomFichier);

    while (blocactuelle != -1) {
        fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
        if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Erreur : Impossible de lire le bloc %d.\n", blocactuelle);
            break;
        }

        if (buffer.typedebloc != 2) {
            printf("Erreur : Le bloc %d n'est pas un bloc de donn�es.\n", blocactuelle);
            break;
        }

        // Affichage des enregistrements dans le bloc actuel
        for (int j = 0; j < buffer.content.fileData.nbrmaladie; j++) {
            if (buffer.content.fileData.T[j].suprimelogiquement == 0) {
                printf("ID: %d, Age: %d, Sexe: %s, Adresse: %s,Etat: %d\n",
                        buffer.content.fileData.T[j].id,
                        buffer.content.fileData.T[j].age,
                        buffer.content.fileData.T[j].sexe,
                        buffer.content.fileData.T[j].adresse,
                        buffer.content.fileData.T[j].suprimelogiquement);
            }
        }

        blocactuelle = buffer.content.fileData.next; // Passer au bloc suivant
    }

    printf("=== Fin du fichier ===\n");
}















   void insertionenregistrement(FILE*disque,const char* nomFichier)  {

   Bloc buffer;
   maladie m; // maladie  qui en veux inserer
   maladie enrdecale;// l'enregistrement qui va decal� vers le bloc suivant est aussi le derniere enregistrement dans le bloc ou on a trouver la position
   maladie enr;// variable qui va engistr� l'enregistrement qui va changer du bloc
   Bloc adressedubloc;//pour lire l'adresse de bloc
   int blocdernier;//adresse de derniere bloc
   int blocactuelle;
   int position=-1;
   bool decalage= false;
   rewind(disque);
   int ID;

    adressemetadonnes adresse = recherchemetadonnes(disque, nomFichier);

   int debut=liremetadonnes(disque,nomFichier,3);// adresse de 1 bloc

   int nbrenregistrement=liremetadonnes(disque,nomFichier,2);// nombre total de enregistrement dans ce fichier

   int nbrbloc=liremetadonnes(disque,nomFichier,1);// nombre total de bloc

   int nbrbloctotal=obtenirNombreBlocs(disque,2);

   int nombrblocutil=obtenirNombreBlocs(disque,1);

   if (adresse.numerodebloc == -1) {
           printf("Fichier introuvable : %s\n", nomFichier);
           return;
       }

   // mtnsaych compactage

   // verifier si il ya un espace pour un enregistrement  sinon on alouer un nouveaux bloc


   bool allouer = false;
   if(nbrbloc*20==nbrenregistrement){
    allouer=true;
   }else{allouer=false;}

   // verifier si il ya un decalge (insertion dans 1 bloc

    blocactuelle= debut;

   // verifier si il ya un decalge (insertion dans 1 bloc)

   // Parcourir les blocs jusqu'au dernier ou atteindre le nombre de blocs utilis�s
   buffer.typedebloc=2;
    for (int i = 0; i <nbrbloc ; i++) {
        fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
        if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Erreur : Impossible de lire le bloc %d.\n", blocactuelle);
            return -1; // Erreur
        }

        if (buffer.content.fileData.next == -1) {
            blocdernier = blocactuelle; // Bloc final atteint
            break;
        }

        blocactuelle = buffer.content.fileData.next;
    }



   // lire le information de la  nouvelle maladie

   printf("ID : \n");
   scanf("%d",&m.id);

   printf("AGE : \n");
   scanf("%d",& m.age);

   printf("SEXE : \n");
   scanf("%s",& m.sexe);

   printf("ADRESSE : \n");
   scanf("%s",& m.adresse);

   m.nmbrdevisite=1;
   m.suprimelogiquement = 0;



   //  cas 1 :si l'enregistrement pour inserer est le  1 enregistrement dans le fichier(fichier vide

    if (nbrenregistrement == 0) {
        printf("Insertion dans un fichier vide.\n");
        fseek(disque, debut * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);
        buffer.typedebloc = 2;
        buffer.content.fileData.T[0] = m;
        buffer.content.fileData.nbrmaladie = 1;
        // ecrire les changement
        fseek(disque, debut * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);
        // mise a jour les metadonnes
        miseAJourMetadonnees(disque, nomFichier, 2,1);
        afficherEnregistrements(disque, nomFichier);
        return;
    }

//cas insertionne dans 1 bloc

  if (nbrenregistrement < 20) {
    printf("Insertion dans le 1er bloc.\n");

    // Charger le bloc
    fseek(disque, debut * sizeof(Bloc), SEEK_SET);
    if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
        printf("Erreur : Impossible de lire le bloc %d.\n", debut);
        return;
    }

    // D�tecter la position exacte pour l'insertion
    int position = buffer.content.fileData.nbrmaladie; // Par d�faut, ins�rer � la fin
    for (int i = 0; i < buffer.content.fileData.nbrmaladie; i++) {
        if (m.id < buffer.content.fileData.T[i].id) { // Crit�re : ID croissant
            position = i;
            break; // Trouv� la position correcte
        }
    }

    printf("Position trouv�e pour l'insertion : %d\n", position);

    // D�caler les enregistrements � partir de la position trouv�e
    for (int j = buffer.content.fileData.nbrmaladie; j > position; j--) {
        buffer.content.fileData.T[j] = buffer.content.fileData.T[j - 1];
    }

    // Ins�rer le nouvel enregistrement � la position correcte
    buffer.content.fileData.T[position] = m;
    buffer.content.fileData.nbrmaladie++;

    // Mettre � jour les m�tadonn�es
    miseAJourMetadonnees(disque, nomFichier, 2, nbrenregistrement + 1);

    // �crire les changements dans le fichier disque
    fseek(disque, debut * sizeof(Bloc), SEEK_SET);
    if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
        printf("Erreur : Impossible d'�crire les modifications dans le bloc %d.\n", debut);
        return;
    }

    printf("Insertion dans le 1er bloc effectu�e avec succ�s.\n");
    afficherEnregistrements(disque, nomFichier); // Afficher les enregistrements pour validation
    return;
}


   // Chercher la position d'insertion

   blocactuelle=debut;// pour passer a bloc suivant

   position=-1;//pour insertion a la fin si en ignore les cas de decalage

   bool positiontrouve=false;

   // parcourir les bloc

       while ((blocactuelle != -1) && (!positiontrouve)) {
        fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);
        for (int i = 0; i < buffer.content.fileData.nbrmaladie; i++) {
            if (m.id < buffer.content.fileData.T[i].id) {
                position = i;
                positiontrouve = true;
                decalage = true;
                break;
            }
        }
        if (!positiontrouve) {
            blocactuelle = buffer.content.fileData.next;
        }
    }

    if (blocdernier == blocactuelle) {
        decalage = false;
    } else {
        decalage = true;
    }

   // cas sans decalge sans allocation
   if (!decalage && !allouer) {

            printf("Insertion dans le dernier bloc existant sans decalage inter bloc est sans  allocation.\n");
            buffer.typedebloc=2;
           fseek(disque, blocdernier * sizeof(Bloc), SEEK_SET);
           fread(&buffer, sizeof(Bloc), 1, disque);

           for (int j = buffer.content.fileData.nbrmaladie; j > position; j--) {
               buffer.content.fileData.T[j] = buffer.content.fileData.T[j - 1]; // D�calage intra bloc
           }
           buffer.content.fileData.T[position] = m;
           buffer.content.fileData.nbrmaladie++;
           // �crire les changements
           fseek(disque, blocdernier * sizeof(Bloc), SEEK_SET);
           fwrite(&buffer, sizeof(Bloc), 1, disque);
           printf("Insertion termine avec succes.\n");
           // Mise � jour des m�tadonn�es
           miseAJourMetadonnees(disque, nomFichier, 2, nbrenregistrement + 1);

           return;
       }

   // cas insertion  (dans le derniere bloc ) est avec allocation

   if(allouer&&!decalage){

       printf("insertion dans le derniere bloc  avec allocation d'un nouveau bloc.\n");

           // Trouver un bloc libre pour l'allocation
           int place = -1;
           buffer.typedebloc=3;
           for (int i = 0; i < nbrbloctotal; i++) {
            fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
            fread(&buffer, sizeof(Bloc), 1, disque);
            if (buffer.content.allocation.tablelocation[i].etat == 0) {
                place = i;
                break;
            }
        }
        if (place == -1) {
            printf("Espace insuffisant pour inserer un nouvel enregistrement.\n");
            return;
        }
           buffer.typedebloc=2;
           // Charger le dernier bloc pour r�cup�rer l'enregistrement � d�caler
           fseek(disque, blocdernier * sizeof(Bloc), SEEK_SET);
           fread(&buffer, sizeof(Bloc), 1, disque);

           if(enrdecale.id>=m.id) {
                enrdecale = buffer.content.fileData.T[buffer.content.fileData.nbrmaladie - 1]; // Dernier enregistrement du bloc
           for (int j = buffer.content.fileData.nbrmaladie - 1; j > position; j--) {
               buffer.content.fileData.T[j] = buffer.content.fileData.T[j - 1];
           }

           buffer.content.fileData.T[position] = m; // Ins�rer le nouvel enregistrement
           buffer.content.fileData.nbrmaladie++;   // Mettre � jour le nombre d'enregistrements
           buffer.content.fileData.next = place; // Mettre � jour le cha�nage

           // �crire les modifications dans le dernier bloc

           fseek(disque, blocdernier * sizeof(Bloc), SEEK_SET);
           fwrite(&buffer, sizeof(Bloc), 1, disque);
           buffer.content.fileData.T[0] = enrdecale;

          } else{
               // Allouer un nouveau bloc pour le d�calage
           fseek(disque, place * sizeof(Bloc), SEEK_SET);
           fread(&buffer, sizeof(Bloc), 1, disque);
   //verifier si le eng que en va inserer et superieur au tous les eng alors il va inserer dans le bloc allouer sinon le derniere eng de dernier bloc il va sauter
            buffer.content.fileData.T[0]=m;}
          // D�placer l'enregistrement d�cal� dans le nouveau bloc


           buffer.content.fileData.nbrmaladie = 1;
           buffer.content.fileData.next = -1;

           // �crire le nouveau bloc
           fseek(disque, place * sizeof(Bloc), SEEK_SET);
           fwrite(&buffer, sizeof(Bloc), 1, disque);

           buffer.typedebloc=3;
           // Mettre � jour la table d'allocation et les m�tadonn�es
           metajourtableallocation(disque, place, 1);
           miseAJourMetadonnees(disque, nomFichier, 3, nbrbloc + 1);
           mettreAJourNombreBlocs(disque, 1, nombrblocutil + 1);

           printf("Decalage avec allocation effectue avec succes.\n");
           return;
       }



   // cas avec decalage l'insertion  n'est pas dans le derniere bloc

   if(decalage){
   printf("Decalage inter-bloc \n");
   buffer.typedebloc=2;
   fseek(disque,blocactuelle* sizeof(Bloc), SEEK_SET);
   fread(&buffer, sizeof(Bloc), 1, disque);

   // buffer est charg� le bloc ou en va inserer

   enrdecale=buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1];// l'enregistrement qui va decal� vers le bloc suivant est aussi le derniere enregistrement dans le bloc

   //decaler les enregistrement dans  bloc ou en a trouver la position

   for(int j=20-1;j>position;j--){ // fb-1 car le 1 index array est
   buffer.content.fileData.T[j] = buffer.content.fileData.T[j - 1]; // pour vider l'espace de position
   }

   buffer.content.fileData.T[position]=m;
   buffer.typedebloc=1;
   miseAJourMetadonnees( disque,nomFichier, 2, nbrenregistrement++);
   buffer.typedebloc=2;
   // ecrire les changement

   fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);

   fwrite(&buffer, sizeof(Bloc), 1, disque);


   // dacaler les enregistrement qui sont dans les bloc suivant (decalage inter est intra bloc)
   //les enregistrement  qui va changer de bloc sont just les derniere enregistrement dans les bloc apres le bloc ou on a inserer
   // en vais faire decalage est on arrete dans le derniere bloc de fichier (next=-1)

   blocactuelle=buffer.content.fileData.next;// sauter vers le bloc suivant


   while(blocactuelle!=-1){

   fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET); // charger le bloc suivant pour faire le decalage
   fread(&buffer, sizeof(Bloc), 1, disque);

   enr=buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1];// avant decalge en engistr� le eng qui va sauter

     for(int i=20-1;i>0;i--) {// decaler pour vider la 1 case
       buffer.content.fileData.T[i]=buffer.content.fileData.T[i-1];
     }

     buffer.content.fileData.T[0]=enrdecale; // met le eng qui a sauter dans la 1 case
     enrdecale=enr;
     // ecrire les modification de bloc actuelle avant de  pass� a le bloc suivant

     fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
     fwrite(&buffer, sizeof(Bloc), 1, disque);

     blocdernier=blocactuelle;//pour engistr� l'adresse de dernier bloc
     blocactuelle=buffer.content.fileData.next;

   }


   // verifier si quand en fait un decalage il faut allouer un nouveux bloc  pour stoker l'enregistrement quia decaler  (enrdecale derniere eng dans fichier)
   if(allouer==true){

   // cas : allouer un nouveaux bloc apres le decalage

   // voir si il ya un  espace pour allouer un noveaux bloc

   if (!verifierEspaceSuffisant(disque,1)) { // Pas de blocs libres

      // verifier si il ya un bloc   vide
      printf(" Espace insuffisant pour insere l'enregistrement  !! \n"); // le cas de pas de espace dans la ms
      return;//quiter la fonction pas d'espace
   }

     // alouer un nouveux bloc
   buffer.typedebloc=3;
   int place=1 ;
   for(int i=0;i<nbrbloctotal;i++)
   {
       fseek(disque,0* sizeof(Bloc), SEEK_SET); // charger le bloc suivant pour faire le decalage
       fread(&buffer, sizeof(Bloc), 1, disque);

      if(buffer.content.allocation.tablelocation[i].etat==0){
          place = i; // la place vide est le bloc  ou en va inserer le noveux enregistrement
          break;
       }
   }

  // met a jour table d'allocation

  metajourtableallocation(disque,place,1);

  // met a jour le chainage pour ajouter le noveaux bloc
   buffer.typedebloc=2;

  fseek(disque, blocactuelle* sizeof(Bloc), SEEK_SET);
   fread(&buffer, sizeof(Bloc), 1, disque);

  buffer.content.fileData.next=place;


  // ecrire les changement

  fseek(disque, blocactuelle* sizeof(Bloc), SEEK_SET);
   fwrite(&buffer, sizeof(Bloc), 1, disque);

  // charger le noveux bloc dans buffer pour stoker le nouveux enregistrement

   fseek(disque, place * sizeof(Bloc), SEEK_SET);
   fread(&buffer, sizeof(Bloc), 1, disque);



  buffer.content.fileData.T[0]=enrdecale;// ecrire le eng qui a sauter dans bloc allouer est qui a ete le derniere eng dans le fichier
   // ecrire les changement

   fseek(disque, place * sizeof(Bloc), SEEK_SET);
   fwrite(&buffer, sizeof(Bloc), 1, disque);

   miseAJourMetadonnees( disque,nomFichier, 3, nbrbloc++);
   mettreAJourNombreBlocs(disque,1,nombrblocutil+1);
   printf("insertion avec decalage est allocation avec succes ");
   return;

  }else{printf("insertion avec decalage est sans allocation avec succes ");
  return;}
   }

  }



adressemetadonnes rechercheenregistrement(FILE* disque, const char* nomFichier, int ID) {
    adressemetadonnes adressetrouve = {-1, -1}; // Initialisation � "non trouv�"
    Bloc buffer;
    int blocactuelle = liremetadonnes(disque, nomFichier, 3); // R�cup�rer l'adresse du premier bloc

    if (blocactuelle == -1) {
        printf("Erreur : Le fichier %s est introuvable.\n", nomFichier);
        return adressetrouve;
    }

    while (blocactuelle != -1) {
        fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
        if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Erreur : Impossible de lire le bloc %d.\n", blocactuelle);
            break;
        }

        // V�rification que c'est un bloc de donn�es
        if (buffer.typedebloc != 2) {
            printf("Erreur : Le bloc %d n'est pas un bloc de donn�es.\n", blocactuelle);
            break;
        }

        // Parcourir les enregistrements du bloc
        for (int j = 0; j < buffer.content.fileData.nbrmaladie; j++) {
            if (buffer.content.fileData.T[j].suprimelogiquement == 0 && buffer.content.fileData.T[j].id == ID) {
                adressetrouve.numerodebloc = blocactuelle;
                adressetrouve.index = j;
                printf("Enregistrement trouv� dans le bloc %d, index %d.\n", blocactuelle, j);
                return adressetrouve;
            }
        }

        blocactuelle = buffer.content.fileData.next; // Passer au bloc suivant
    }

    printf("Enregistrement avec ID %d introuvable dans le fichier %s.\n", ID, nomFichier);
    return adressetrouve; // Retourne {-1, -1} si l'enregistrement n'est pas trouv�
}



   void supprimerEnregistrementLogique(FILE *disque, const char *nomFichier, int ID) {
    // Structure pour stocker les donn�es
    Bloc buffer;

    // Rechercher l'enregistrement par son ID
    adressemetadonnes adresse = rechercheenregistrement(disque, nomFichier, ID);

    // V�rifier si l'enregistrement existe
    if (adresse.numerodebloc == -1) {
        printf("Erreur : Enregistrement avec ID %d introuvable dans le fichier %s.\n", ID, nomFichier);
        return;
    }

    // Charger le bloc contenant l'enregistrement
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
        printf("Erreur : Impossible de lire le bloc %d.\n", adresse.numerodebloc);
        return;
    }

    // Modifier l'�tat de l'enregistrement pour le marquer comme supprim�
    buffer.content.fileData.T[adresse.index].suprimelogiquement = 1;

    // �crire les modifications dans le fichier disque
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
        printf("Erreur : �chec de l'�criture dans le bloc %d.\n", adresse.numerodebloc);
        return;
    }

    // Confirmation de suppression
    printf("L'enregistrement avec ID %d a �t� supprim� logiquement du fichier %s.\n", ID, nomFichier);
}






  void  suprimerenregistrementphisique(FILE*disque,const char*nomFichier,int ID) {

       Bloc buffer;
        rewind(disque);
       int choixSuppression = 0;
       int dernierbloc;
       bool decalage=false;// pour voir si il ya un decalage des bloc
       bool blocvide=false;//si apres decalage le dernier bloc est vide
       int blocactuelle=-1;
       int blocprecedent=-1;//pour la supression avec decalge pour revenir dans le decalage
       maladie m;// pour faire engistr� les enregistrement qui va changer apres decalage

       adressemetadonnes adresse=rechercheenregistrement(disque,nomFichier,ID);

       if (adresse.numerodebloc == -1) {
           printf("Enregistrement introuvable : ID %d dans le fichier %s.\n", ID, nomFichier);
           return;
       }

      adressemetadonnes meta=recherchemetadonnes(disque,nomFichier);//recuperer l'adresse de metadonnes pour met a jour

      if (meta.numerodebloc == -1) {
           printf("M�tadonn�es introuvables pour le fichier : %s\n", nomFichier);
           return;
       }
   int nbrenregistrement=liremetadonnes(disque,nomFichier,2);// nombre total de enregistrement dans ce fichier
       int nbrbloc=liremetadonnes(disque,nomFichier,1);// nombre total de bloc
       int debut=liremetadonnes(disque,nomFichier,3);// adresse de 1 bloc
       int nbrbloctotal=obtenirNombreBlocs(disque,2);
       int nbrblocutil=obtenirNombreBlocs(disque,1);




  // supression phisique

  printf("Vous avez choisi la suppression physique.\n");

  // chercher l'adresse de derniere bloc dans le fichier  pour voir si il ya un decalage est si apres supression le bloc sera vide

  dernierbloc=debut;

  while(dernierbloc!=-1){
      fseek(disque, dernierbloc * sizeof(Bloc), SEEK_SET);// charger le bloc pour point� sur bloc suivant
      fread(&buffer, sizeof(Bloc), 1, disque);
      if (buffer.content.fileData.next == -1) break;
      dernierbloc=buffer.content.fileData.next;

  }

  //verifier si adresse de l'enregistrement est dans le bloc derniere

  if(dernierbloc==adresse.numerodebloc) {
       decalage=false;

  }
   else{decalage=true;}

  // verifier si apres la suppresion le dernier bloc devient vide

  if((buffer.content.fileData.nbrmaladie-1)==0){

  blocvide=true;

  }else{blocvide=false;}

  // cas 1 : supresion sans decalage (l'enregistrement est dans le derniere bloc)

  if(decalage==false)
   {
   fseek(disque, dernierbloc * sizeof(Bloc), SEEK_SET);// charger le derniere bloc
   fread(&buffer, sizeof(Bloc), 1, disque);

  // D�caler les �l�ments vers la gauche � partir de l'index
      for (int i = adresse.index; i < (buffer.content.fileData.nbrmaladie) - 1; i++) {
           buffer.content.fileData.T[i] = buffer.content.fileData.T[i + 1];
       }

      buffer.content.fileData.nbrmaladie--;

      fseek(disque, dernierbloc * sizeof(Bloc), SEEK_SET);// charger le derniere bloc
       fwrite(&buffer, sizeof(Bloc), 1, disque);

      miseAJourMetadonnees(disque, nomFichier, 2,nbrenregistrement--);//mise a jour metadonnes

      if (blocvide) {
               fseek(disque, debut * sizeof(Bloc), SEEK_SET);
               fread(&buffer, sizeof(Bloc), 1, disque);

     while (buffer.content.fileData.next != dernierbloc) {
                   fseek(disque, buffer.content.fileData.next * sizeof(Bloc), SEEK_SET);
                   fread(&buffer, sizeof(Bloc), 1, disque);
               }
               buffer.content.fileData.next = -1;

              fseek(disque, debut * sizeof(Bloc), SEEK_SET);
               fwrite(&buffer, sizeof(Bloc), 1, disque);

              metajourtableallocation(disque, dernierbloc, 0);
               miseAJourMetadonnees(disque, nomFichier, 1, nbrbloc - 1);
               mettreAJourNombreBlocs(disque,1,nbrblocutil-1);
           }

          return;

  return;
   }

  //cas 3 : avec decalage

  if(decalage==true ){
   blocactuelle = adresse.numerodebloc;
   fseek(disque,  blocactuelle* sizeof(Bloc), SEEK_SET);// charger le bloc ou on va suprim�
   fread(&buffer, sizeof(Bloc), 1, disque);

  // faire la supression

  // D�caler les �l�ments vers la gauche � partir de l'index
       for (int i = adresse.index; i < (buffer.content.fileData.nbrmaladie) - 1; i++) {
           buffer.content.fileData.T[i] = buffer.content.fileData.T[i + 1];
       }

      fseek(disque, debut * sizeof(Bloc), SEEK_SET); // engistr� les modification apres changement de chainage
       fwrite(&buffer, sizeof(Bloc), 1, disque);

      miseAJourMetadonnees(disque, nomFichier, 2,nbrenregistrement--);//mise a jour metadonnes



  blocprecedent=adresse.numerodebloc;
   blocactuelle=buffer.content.fileData.next;

  while(buffer.content.fileData.next!=-1) // pour ne entr� pas dans le derniere bloc pour deviser les cas
   {

  fseek(disque,blocactuelle* sizeof(Bloc), SEEK_SET);// charger le bloc suivante
   fread(&buffer, sizeof(Bloc), 1, disque);

  m=buffer.content.fileData.T[0];// le premiere enregistrement il rementera au bloc precedent

  for (int i = 0; i < (buffer.content.fileData.nbrmaladie) - 1; i++) { // fait le decalge parceque le premiere eng est l'element a ete suprim� qui va rementre au bloc precedent
           buffer.content.fileData.T[i] = buffer.content.fileData.T[i + 1];
       }

  fseek(disque,blocactuelle* sizeof(Bloc), SEEK_SET);// charger le bloc suivante
   fwrite(&buffer, sizeof(Bloc), 1, disque);//pour engitr� les changement

  fseek(disque,blocprecedent* sizeof(Bloc), SEEK_SET);// charger le bloc suivante  pour remplir le derniere eng qui est vide apres le decalge
   fread(&buffer, sizeof(Bloc), 1, disque);

  buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1]=m;// le premiere enr de bloc actuelle dans derniere eng de bloc precedent

  fseek(disque, blocprecedent * sizeof(Bloc), SEEK_SET); // engistr� les modification apres decalage vers la gauche
   fwrite(&buffer, sizeof(Bloc), 1, disque);

   blocprecedent=blocactuelle;
   blocactuelle=buffer.content.fileData.next;

  }


  // etrer dans le derniere bloc pour engistr� le 1 eng qui va charg� dans le bloc precedent
   fseek(disque,blocactuelle* sizeof(Bloc), SEEK_SET);// car la boucle a    et� arreter  dans le derniere bloc
   fread(&buffer, sizeof(Bloc), 1, disque);

   m=buffer.content.fileData.T[0];

  fseek(disque,blocprecedent* sizeof(Bloc), SEEK_SET);// pour met le eng dans bloc avant dernier dans derniere place
   fread(&buffer, sizeof(Bloc), 1, disque);

  buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1]=m;

  // ecrire les changement

  fseek(disque, blocprecedent * sizeof(Bloc), SEEK_SET); // engistr� les modification apres decalage
   fwrite(&buffer, sizeof(Bloc), 1, disque);
   }

  // separer les cas si le bloc vide alors en liberer derniere bloc sinon en fais derniere decalage dans le derniere bloc
   // cas 5: avec decalage et bloc vide
   if (blocvide==true && decalage==true) // changer chainage car le bloc est vide
   {



  buffer.content.fileData.next=-1;  // psq je suis dans l'avant derniere

  fseek(disque, blocprecedent * sizeof(Bloc), SEEK_SET); // engistr� les modification de chainage
   fwrite(&buffer, sizeof(Bloc), 1, disque);


  metajourtableallocation(disque,blocactuelle,0);//mise a jour table d'allocation
   miseAJourMetadonnees(disque, nomFichier, 2,nbrbloc--);//mise a jour taille en bloc
   mettreAJourNombreBlocs(disque,1,nbrblocutil-1);

  printf("Suppression physique effectu�e pour l'enregistrement ID %d.\n", ID);

  return;
   }
   else // cas 6 : avec decalage est pas de bloc libre
   {
  fseek(disque,blocactuelle* sizeof(Bloc), SEEK_SET);// pour met le eng dans bloc  dernier dans derniere place
   fread(&buffer, sizeof(Bloc), 1, disque);

  // le derniere eng dans bloc precedent est vide

  m=buffer.content.fileData.T[0];

  for (int i = 0; i < (buffer.content.fileData.nbrmaladie) - 1; i++) { // fait le decalge pparceque le premiere est l'element a ete suprim�
           buffer.content.fileData.T[i] = buffer.content.fileData.T[i + 1];
       }

  buffer.content.fileData.nbrmaladie--;

  fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
// engistr� les modification apres le dernier decalage
   fwrite(&buffer, sizeof(Bloc), 1, disque);

  fseek(disque,blocprecedent* sizeof(Bloc), SEEK_SET);// pour met le 1 eng de bloc  dernier dans l'avant derniere bloc dans derniere place
   fread(&buffer, sizeof(Bloc), 1, disque);

  buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1]=m;

  fseek(disque,blocprecedent* sizeof(Bloc), SEEK_SET);// engistr� les modification
   fwrite(&buffer, sizeof(Bloc), 1, disque);

  printf("Suppression physique effectu�e pour l'enregistrement ID %d.\n", ID);

  return;

  }


  }


  void renommerfichierCO(FILE*disque,const char*nomFichier,const char*nouveaunom)
   {

  adressemetadonnes adress;
   Bloc buffer;


  adress=recherchemetadonnes(disque,nomFichier);

  buffer.typedebloc=1;

     fseek(disque, adress.numerodebloc * sizeof(Bloc), SEEK_SET);
       fread(&buffer, sizeof(Bloc), 1, disque);

  //copier le noveau mot
         strcpy(buffer.content.metadataTable.T[adress.index].Nomdufichier, nouveaunom);



  // engistr� les modification
       fseek(disque, adress.numerodebloc * sizeof(Bloc), SEEK_SET);
       fwrite(&buffer, sizeof(Bloc), 1, disque);

  printf("succes");

  return;
   }



  void suprimerFCO(FILE* disque, const char* nomFichier) {
       Bloc buffer;
       int dernierblocmeta = 1; // Dernier bloc de m�tadonn�es
       int blocprecedent = -1;
       fichiermetadonnes dernierengmeta;
       int nbrbloc = liremetadonnes(disque, nomFichier, 1); // Nombre de blocs utilis�s par le fichier
       int adrpremierbloc = liremetadonnes(disque, nomFichier, 3); // Adresse du premier bloc du fichier
       int nbrblocutil = obtenirNombreBlocs(disque, 1); // Nombre total de blocs utilis�s
       bool vide = false; // Indicateur pour savoir si le dernier bloc sera vide apr�s suppression

      adressemetadonnes adress = recherchemetadonnes(disque, nomFichier);

      if (adress.numerodebloc == -1) {
           printf("Erreur : M�tadonn�es non trouv�es pour le fichier : %s\n", nomFichier);
           return;
       }

      // Chercher l'adresse du dernier bloc qui stocke les m�tadonn�es
       while (1) {
           fseek(disque, dernierblocmeta * sizeof(Bloc), SEEK_SET);
           if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : Impossible de lire le bloc %d.\n", dernierblocmeta);
               return;
           }

          if (buffer.typedebloc != 1) {
               printf("Erreur : Le bloc %d n'est pas un bloc de m�tadonn�es.\n", dernierblocmeta);
               return;
           }

          if (buffer.content.metadataTable.next == -1) break; // Dernier bloc atteint

          blocprecedent = dernierblocmeta;
           dernierengmeta = buffer.content.metadataTable.T[buffer.content.metadataTable.nbrMetadonnees - 1];
           dernierblocmeta = buffer.content.metadataTable.next;
       }

      // V�rifier si le dernier bloc sera vide apr�s suppression
       if (buffer.content.metadataTable.nbrMetadonnees == 1) {
           vide = true;
       }

      // Supprimer la m�tadonn�e et effectuer le d�calage
       fseek(disque, adress.numerodebloc * sizeof(Bloc), SEEK_SET);
       if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
           printf("Erreur : Impossible de lire le bloc %d.\n", adress.numerodebloc);
           return;
       }

      for (int i = adress.index; i < buffer.content.metadataTable.nbrMetadonnees - 1; i++) {
           buffer.content.metadataTable.T[i] = buffer.content.metadataTable.T[i + 1];
       }

      buffer.content.metadataTable.nbrMetadonnees--; // R�duire le nombre de m�tadonn�es

      // Si le dernier bloc devient vide, le lib�rer
       if (vide) {
           if (blocprecedent != -1) {
               fseek(disque, blocprecedent * sizeof(Bloc), SEEK_SET);
               if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
                  printf("Erreur : Impossible de lire le bloc %d.\n", blocprecedent);
                   return;
               }

              buffer.content.metadataTable.next = -1; // Mettre � jour le pointeur du bloc pr�c�dent

              fseek(disque, blocprecedent * sizeof(Bloc), SEEK_SET);
               if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
                   printf("Erreur : �chec de l'�criture dans le bloc %d.\n", blocprecedent);
                   return;
               }
           }

          // Lib�rer le dernier bloc
           memset(&buffer, 0, sizeof(Bloc));
           buffer.typedebloc = 0; // Bloc inutilis�
           fseek(disque, dernierblocmeta * sizeof(Bloc), SEEK_SET);
           if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : �chec de l'�criture dans le bloc %d.\n", dernierblocmeta);
               return;
           }

          metajourtableallocation(disque, dernierblocmeta, 0); // Lib�rer le dernier bloc
           mettreAJourNombreBlocs(disque, 1, nbrblocutil - 1);
       } else {
           // R��crire le bloc avec le d�calage
           fseek(disque, adress.numerodebloc * sizeof(Bloc), SEEK_SET);
           if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : �chec de l'�criture dans le bloc %d.\n", adress.numerodebloc);
               return;
           }

       }

      // Lib�rer les blocs de donn�es du fichier
       int blocActuel = adrpremierbloc;
       while (blocActuel != -1) {
           fseek(disque, blocActuel * sizeof(Bloc), SEEK_SET);
           if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : Impossible de lire le bloc %d.\n", blocActuel);
               return;
           }

          int blocSuivant = buffer.content.fileData.next; // Adresse du bloc suivant
           memset(&buffer, 0, sizeof(Bloc));               // Vider le contenu du bloc
           buffer.typedebloc = 0;                          // Bloc inutilis�
           fseek(disque, blocActuel * sizeof(Bloc), SEEK_SET);
           if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : �chec de l'�criture dans le bloc %d.\n", blocActuel);
               return;
           }

          metajourtableallocation(disque, blocActuel, 0); // Lib�rer le bloc
           blocActuel = blocSuivant;
       }

      // Mettre � jour le nombre de blocs utilis�s
       mettreAJourNombreBlocs(disque, 1, nbrblocutil - nbrbloc);

      printf("Fichier supprim� avec succ�s.\n");
       return;
   }


  void afficherMS(FILE *disque) {
    Bloc buffer;
    rewind(disque);

    printf("========== �tat de la M�moire Secondaire ==========\n");

    // Parcourir tous les blocs
    for (int i = 0; i < 20; i++) {
        fseek(disque, i * sizeof(Bloc), SEEK_SET);

        if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Bloc %d : Erreur : Impossible de lire le bloc. Bloc vide ou corrompu.\n", i);
            continue;
        }

        printf("Bloc %d : ", i);

        switch (buffer.typedebloc) {
            case 1: // Bloc de m�tadonn�es
                printf("Bloc de m�tadonn�es\n");
                printf("  Nombre de fichiers : %d\n", buffer.content.metadataTable.nbrMetadonnees);
                printf("  Next : %d\n", buffer.content.metadataTable.next);
                for (int j = 0; j < buffer.content.metadataTable.nbrMetadonnees; j++) {
                    printf("    Fichier %d : Adresse premier bloc = %d\n",
                           j, buffer.content.metadataTable.T[j].Adrpremierbloc);
                }
                break;

            case 2: // Bloc de donn�es de fichier
                printf("Bloc de donn�es de fichier\n");
                printf("  Nombre d'enregistrements : %d\n", buffer.content.fileData.nbrmaladie);
                printf("  Next : %d\n", buffer.content.fileData.next);
                break;

            case 3: // Bloc de table d'allocation
                printf("Bloc de table d'allocation\n");
                printf("  Nombre total de blocs : %d\n", buffer.content.allocation.nbrbloc);
                printf("  Nombre de blocs utilis�s : %d\n", buffer.content.allocation.nbrblocutil);
                for (int j = 0; j < buffer.content.allocation.nbrbloc; j++) {
                    printf("    Bloc %d : %s\n", j,
                           buffer.content.allocation.tablelocation[j].etat == 1 ? "Allou�" : "Libre");
                }
                break;

            default: // Bloc vide ou type inconnu
                printf("Erreur : Bloc ne contient aucun type valide.\n");
                break;
        }
    }

    printf("===================================================\n");
}



  int main() {
       int choix;
       int modeG, modeI;
       char nomFichier[20], ancienNom[20], nouveaunom[20];
       int ID;

     printf("Program started successfully!\n");

      FILE* disque = fopen("disque.bin", "r+b"); // Ouvrir le fichier pour lecture/�criture
       if (!disque) {
           disque = fopen("disque.bin", "w+b"); // Cr�er le fichier s'il n'existe pas
           if (!disque) {
               printf("Erreur : Impossible de cr�er le fichier disque.bin.\n");
               return 1;
           }
       }

    do {
          printf("\n--- Gestion de la M�moire Secondaire ---\n");
           printf("1. Initialiser la m�moire secondaire\n");
           printf("2. Cr�er un fichier\n");
           printf("3. Afficher l'�tat de la m�moire secondaire\n");
           printf("4. Afficher les m�tadonn�es des fichiers\n");
           printf("5. Rechercher un enregistrement\n");
           printf("6. Ins�rer un nouvel enregistrement\n");
           printf("7. Supprimer un enregistrement\n");
           printf("8. D�fragmenter un fichier\n");
           printf("9. Supprimer un fichier\n");
           printf("10. Renommer un fichier\n");
          printf("11. Compacter la m�moire secondaire\n");
          printf("12. Vider la m�moire secondaire\n");
           printf("0. Quitter\n");
           printf("Votre choix : ");
           scanf("%d", &choix);

          switch (choix) {
               case 1:
                   InitMs(disque, 20);
                   printf("M�moire secondaire initialis�e avec succ�s.\n");
                   break;

              case 2:
                   printf("Cr�ation d'un fichier\n");
                   printf("Votre choix d'organisation globale : ");
                   scanf("%d", &modeG);
                   printf("Votre choix d'organisation interne : ");
                   scanf("%d", &modeI);
                   chargerfichier(disque);
                   break;

              case 3:
                   printf("Affichage de l'�tat de la m�moire secondaire :\n");
                   afficherMS(disque);
                   break;

              case 4:
                  printf("Affichage des m�tadonn�es des fichiers :\n");
                   // Ajouter une fonction pour afficher uniquement les m�tadonn�es
                   break;

              case 5:
                   printf("Recherche d'enregistrement\n");
                   printf("Entrez le nom du fichier : ");
                   scanf("%s", nomFichier);
                   printf("Entrez l'ID de l'enregistrement a rechercher : ");
                   scanf("%d", &ID);
                    adressemetadonnes resultat=rechercheenregistrement(disque,nomFichier,ID);


                   break;

              case 6:
                   printf("Insertion d'enregistrement\n");
                   printf("Entrez le nom du fichier : ");
                   scanf("%s", nomFichier);
                   insertionenregistrement(disque, nomFichier);
                   break;

              case 7:
                   printf("Suppression d'enregistrement\n");
                   printf("Entrez le nom du fichier : ");
                   scanf("%s", nomFichier);
                   printf("Entrez l'ID de l'enregistrement � supprimer : ");
                   scanf("%d", &ID);

                   supprimerEnregistrementLogique(disque,nomFichier,  ID);
                    //suprimerenregistrementphisique(disque,nomFichier, ID);
                   afficherEnregistrements(disque, nomFichier);


                   break;

              case 8:
                   printf("D�fragmentation d'un fichier\n");
                   printf("Entrez le nom du fichier � d�fragmenter : ");
                   scanf("%s", nomFichier);
                   // Appeler la fonction de d�fragmentation ici
                   break;

              case 9:
                   printf("Suppression de fichier\n");
                   printf("Entrez le nom du fichier � supprimer : ");
                   scanf("%s", nomFichier);
                   suprimerFCO(disque, nomFichier);
                   break;

              case 10:
                   printf("Renommage de fichier\n");
                   printf("Entrez le nom actuel du fichier : ");
                   scanf("%s", ancienNom);
                   printf("Entrez le nouveau nom du fichier : ");
                   scanf("%s", nouveaunom);
                   renommerfichierCO(disque, ancienNom, nouveaunom);
                   break;

              case 11:
                   printf("Compactage de la m�moire secondaire\n");
                   // Appeler la fonction de compactage ici
                   break;

              case 12:
                   printf("Vidage de la m�moire secondaire\n");
                   // Appeler la fonction de vidage ici
                   ViderMs(disque);
                   break;

              case 0:
                   printf("Programme termin� !\n");
                  break;

              default:
                   printf("Choix invalide. Veuillez r�essayer.\n");
                  break;
           }
     } while (choix != 0);
     fclose(disque);
      return 0; }

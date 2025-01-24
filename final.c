#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>


#define FB 20

    typedef struct {
        char Nomdufichier[20];
        int Taillefichierblocs;
        int Taillefichierenregistrements;
        int Adrpremierbloc; // Adresse du premier bloc
        int Modeorganisationglobale; // Si = 0 alors chaîne
        int Modeorganisationinterne; // Si = 0 alors ordonné
    } fichiermetadonnes;

    typedef struct {
        int id;
        char name[15];
        int age;
        char sexe[10];
        char adresse[30];
        int nmbrdevisite;
        int suprimelogiquement; // 1 si supprimé logiquement
    } maladie;

    typedef struct {
        maladie T[20]; // Facteur de blocage = 20
        int nbrmaladie;
        int next; // Pour chaîner les blocs
    } BlocData;

    typedef struct {
        int adrdebloc; // Adresse de bloc
        int etat; // Si vide = 0, pleine = 1
    } Tableallocation;

    typedef struct {
        fichiermetadonnes T[20]; // Tableau de métadonnées
        int nbrMetadonnees; // Nombre actuel de métadonnées dans ce bloc
        int next; // Pour chaîner les blocs de métadonnées
    } BlocMetadonnees;

    typedef struct {
        Tableallocation tablelocation[20];
        int nbrblocutil; // Nombre de blocs utilisés
        int nbrbloc; // Nombre total de blocs
    } BlocAllocation;

    typedef struct
    {

    int numerodebloc;
    int index;

    }adressemetadonnes;

    typedef struct {
    int numBloc;
    int deplacement;
} position;

    typedef struct {

        union {
            BlocMetadonnees metadataTable;
            BlocData fileData;
            BlocAllocation allocation;
        } content;

        int typedebloc;
        // 1 = métadonnées, 2 = données de fichier, 3 = allocation
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
           printf("Erreur : Espace insuffisant. %d blocs nécessaires, %d disponibles.\n", nbrBlocsVoulu,blocsLibres);
           return false;
       }

       printf("Succès : Il y a suffisamment d'espace. %d blocs disponibles.\n", blocsLibres);
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
           case 1: // Nombre de blocs utilisés
               return allocation->nbrblocutil;
           case 2: // Nombre total de blocs
               return allocation->nbrbloc;
           default:
               printf("Erreur : Option invalide. Utilisez 1 pour blocs utilisés ou 2 pour blocs totaux.\n");
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
           printf("Erreur : Échec d'écriture dans le bloc 0.\n");
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
           printf("Erreur : Échec d'écriture dans le bloc 0.\n");
           return;
       }


       // Mark the first block as allocated

        printf("succes");
   }

   // Function to clear memory space by resetting block usage counts and allocation table
   void ViderMs(FILE* disque) {
    Bloc buffer = {0}; // Initialiser un bloc vide (toutes valeurs à 0/null)
    int nombreBlocs = obtenirNombreBlocs(disque, 2); // Obtenir le nombre total de blocs

    // Parcourir tous les blocs et les réinitialiser
    for (int i = 0; i < nombreBlocs; i++) {
        fseek(disque, i * sizeof(Bloc), SEEK_SET);
        if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Erreur : Impossible de réinitialiser le bloc %d.\n", i);
            return;
        }
    }

    printf("Tous les blocs ont été réinitialisés à NULL.\n");
}


   // Function to initialize memory space with a specified number of blocks


   void InitMs(FILE* disque, int nombreBlocs) {
       Bloc buffer = {0};

       // Initialisation du bloc 0 (table d'allocation)
       buffer.typedebloc = 3; // Bloc d'allocation

       CreeTableAllocation(  disque);

       buffer.typedebloc=1;

       // Initialisation des blocs 1 et 2 (métadonnées)
       buffer.typedebloc = 1; // Bloc de métadonnées
       buffer.content.metadataTable.nbrMetadonnees = 0;
       buffer.content.metadataTable.next = 2;


       fseek(disque, 1 * sizeof(Bloc), SEEK_SET);
           if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : Échec d'écriture dans le bloc %d.\n", 1);
               return;
           }

         buffer.content.metadataTable.nbrMetadonnees = 0;
          buffer.content.metadataTable.next = -1;

        fseek(disque, 2 * sizeof(Bloc), SEEK_SET);
           if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : Échec d'écriture dans le bloc %d.\n", 2);
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

              printf("Métadonnées ajoutées avec succès au bloc %d.\n", blocactuelle);
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
       adressemetadonnes resultat = {-1, -1}; // Initialisation : non trouvé
       int blocActuel = 1; // Commence par le premier bloc de métadonnées

       // Parcours des blocs de métadonnées en chaîne
       while (blocActuel != -1) {
           // Lecture du bloc actuel
           fseek(disque, blocActuel * sizeof(Bloc), SEEK_SET);
           if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : Impossible de lire le bloc %d.\n", blocActuel);
               break;
           }

           // Vérifier si le bloc est un bloc de métadonnées
           if (buffer.typedebloc != 1) {
               printf("Erreur : Le bloc %d n'est pas un bloc de métadonnées.\n", blocActuel);
               break;
           }

           // Parcourir les métadonnées dans le bloc
           for (int j = 0; j < buffer.content.metadataTable.nbrMetadonnees; j++) {
               if (buffer.content.metadataTable.T[j].Nomdufichier[0] != '\0' &&
                   strcmp(buffer.content.metadataTable.T[j].Nomdufichier, nomfichier) == 0) {
                   // Métadonnées trouvées
                   resultat.numerodebloc = blocActuel;
                   resultat.index = j;
                   return resultat;
               }
           }

           // Passer au bloc suivant
           blocActuel = buffer.content.metadataTable.next;
       }

       printf("Métadonnées non trouvées pour le fichier : %s\n", nomfichier);
       return resultat; // Retourne {-1, -1} si non trouvé
   }


   // Function to read specific metadata characteristics based on given parameter.
   int liremetadonnes(FILE* disque,const char* nomFichier,int caracteristique ){
        Bloc buffer ;
        adressemetadonnes adresse=recherchemetadonnes(disque ,nomFichier );
        rewind(disque);

        if(adresse.numerodebloc==-1) { // Vérifier si le fichier existe
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
                printf("Caractéristique non trouvée\n");
                return -1;
        }
   }

   // Function to update metadata after insertion or deletion.
   void miseAJourMetadonnees(FILE* disque,const char* nomFichier,int champ,int nouvelleValeur){
        adressemetadonnes adresse=recherchemetadonnes(disque ,nomFichier );
        rewind(disque);
        if(adresse.numerodebloc==-1){
            printf("Fichier introuvable pour mise à jour des métadonnées.\n");
            return ;
        }

        Bloc buffer ;
        fseek(disque ,adresse.numerodebloc*sizeof(Bloc ),SEEK_SET );

        fread(&buffer,sizeof(Bloc ),1 ,disque );

        if(buffer.typedebloc!=1){
            printf("Erreur : Le bloc trouvé ne contient pas de métadonnées.\n");
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



/********************************************************************************* */


//-----------------------------------------------------besoin de TNOF------------------------------------------------------
typedef struct position {
    int blocNbr;
    int deplacment;

  }pos;
void copyString(char *destination, const char *source) {
    int i = 0;

    // Copie chaque caractère jusqu'à ce qu'on atteigne le caractère nul '\0'
    while (source[i] != '\0') {
        destination[i] = source[i];
        i++;
    }

    // Ajoute le caractère nul à la fin de la chaîne de destination
    destination[i] = '\0';
}
//-------trouver le bloc libre pour la creation
int trouverPremierBlocLibre(FILE *disque) {
    Bloc buffer;
    int nbrbloc=obtenirNombreBlocs(disque,2);
    // Lire le bloc d'allocation (premier bloc)
    rewind(disque);
    fread(&buffer, sizeof(Bloc), 1, disque);

    if (buffer.typedebloc != 3) {
        printf("Erreur : Le premier bloc n'est pas un bloc d'allocation.\n");
        return -1;
    }

    // Parcourir la table d'allocation
    for (int i = 0; i < nbrbloc; i++) {
        if (buffer.content.allocation.tablelocation[i].etat == 0) {
            // Bloc libre trouvé
            return i;
        }
    }

    // Aucun bloc libre trouvé
    printf("Aucun bloc libre n'a été trouvé.\n");
    return -1;
}

// --------------compactage
void compactageDisque(FILE *disque) {
    Bloc buffer, bufferTemp;
    int blocLibre = 0;
    int blocCourant = 0;

    // Lire le bloc d'allocation
    rewind(disque);
    //fseek(disque, 0, SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    if (buffer.typedebloc != 3) {
        printf("Erreur : Le premier bloc n'est pas un bloc d'allocation.\n");
        return;
    }

    int nbrTotalBlocs = obtenirNombreBlocs(disque,2);

    // Parcourir tous les blocs
    for (blocCourant = 1; blocCourant < nbrTotalBlocs; blocCourant++) {
        fseek(disque, blocCourant * sizeof(Bloc), SEEK_SET);
        fread(&bufferTemp, sizeof(Bloc), 1, disque);

        // Si le bloc est utilisé
        if (buffer.content.allocation.tablelocation[blocCourant].etat == 1) {
            // Si ce n'est pas déjà à la bonne place
            if (blocCourant != blocLibre) {
                // Déplacer le bloc vers l'emplacement libre
                fseek(disque, blocLibre * sizeof(Bloc), SEEK_SET);
                fwrite(&bufferTemp, sizeof(Bloc), 1, disque);

                // Mettre à jour la table d'allocation
                buffer.content.allocation.tablelocation[blocLibre].etat = 1;
                buffer.content.allocation.tablelocation[blocLibre].adrdebloc = blocLibre;
                buffer.content.allocation.tablelocation[blocCourant].etat = 0;
            }
            blocLibre++;
        }
    }

    // Mettre à jour le nombre de blocs utilisés
    mettreAJourNombreBlocs(disque,2,nbrTotalBlocs++);

    // Réécrire le bloc d'allocation mis à jour
    fseek(disque, 0, SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    printf("Compactage terminé. %d blocs utilisés après compactage.\n", blocLibre);
}
//--------------------------------------------------------------------------------------------------------------------------
//_________________________MODE D'ORGANISATION "TABLEAU ,NON ORDONNEE ,TAILLE FIXE"____________________________


//-----------------------CREATION DE FICHIER TNOF--------------------------
fichiermetadonnes creeFileTNOF(){
fichiermetadonnes MT;

printf("entrer le nom de fichier  \n");
scanf("%s",MT.Nomdufichier);
printf("entrer le nombres d'enregistrements  \n");
scanf("%d",&MT.Taillefichierenregistrements);
return MT;
}
//-------------------------CHARGEMENT DE FICHIER TNOF ----------------------------------
// facteur_blocage nbr maximum d'enregistrement dans un bloc
 void chargerFileTNOF(FILE *disque ){

 fichiermetadonnes MT = creeFileTNOF();

  // deffirence with * and without
  int blocVolu  =  MT.Taillefichierenregistrements / 20;
    Bloc buffer;
    maladie malade;
    int cpt=-1;
    int blocNbr;
    int enre;
    int i=0;
 bool compac ;
   int nbrB=0;//nombre de bloc
   int nbrE=0;//nombre de enregistrement  nbr de blocs suffisants =nbrElement\tailleBloc
   compac= verifierEspaceSuffisant(disque,blocVolu);
   if (!compac) {
    compactageDisque(disque);
   }
    rewind(disque);
    int indexBloc = trouverPremierBlocLibre(disque);
    fread(&buffer, sizeof(buffer), indexBloc+1 , disque);
    // pour metre le curseur au bonne position
   int nbrMalade = 0;
   if (enre > 20)
     enre = enre / 20 + enre % 20 ;

    while(nbrMalade <= MT.Taillefichierenregistrements && malade.id !=-1){
    nbrMalade++;
    printf("---------------------------------- \n");
    printf("entrer la reference de malade %d:\n",nbrMalade);
    scanf("%d",&malade.id);
 // si id=-1 alors on arrete de remplir le bloc
    printf("entrer le nom de malade %d:\n",nbrMalade);
    scanf("%s",malade.name);
    printf("entrer l'age de malade %d :\n",nbrMalade);
    scanf("%d",&malade.age);
    printf("entrer le nombre de visite %d:\n",nbrMalade);
    scanf("%d",&malade.nmbrdevisite);
    printf("entrer l'adress de malade %d:\n",nbrMalade);
    scanf("%s",malade.adresse);
    printf("entrer le sexe de malade %d :\n",nbrMalade);
    scanf("%s",malade.sexe);
    cpt ++;

    malade.suprimelogiquement=0;
    buffer.content.fileData.T[i].id =malade.id;//id
    copyString(buffer.content.fileData.T[i].name, malade.name);//name
    copyString(buffer.content.fileData.T[i].sexe,malade.sexe);//sexe
    buffer.content.fileData.T[i].age =malade.age;//age
    buffer.content.fileData.T[i].nmbrdevisite =malade.nmbrdevisite;//nbrvisite
    copyString(buffer.content.fileData.T[i].adresse , malade.adresse);//adress
    buffer.content.fileData.T[i].suprimelogiquement =malade.suprimelogiquement;
    i++;
    if (i==20-1) i=0;

     metajourtableallocation ( disque, indexBloc+nbrB, 1);
       nbrB =nbrB+1;
       buffer.typedebloc=2;
       buffer.content.allocation.nbrbloc++;
      if (cpt==1) blocNbr = buffer.content.allocation.nbrbloc++;
    }
      fwrite(&buffer, sizeof(buffer), indexBloc+nbrB , disque);

    buffer.content.metadataTable.nbrMetadonnees++;
    buffer.typedebloc=1;
   int indexMeta= buffer.content.metadataTable.nbrMetadonnees-1;
   if (indexMeta=20) { // le cas de table d'allocation est plaine
     indexMeta = 0;
     metajourtableallocation (disque, blocNbr+nbrB , 1);
    };
   buffer.content.metadataTable.T[indexMeta].Taillefichierblocs=nbrB;
   buffer.content.metadataTable.T[indexMeta].Taillefichierenregistrements=nbrE;
   copyString(buffer.content.metadataTable.T[indexMeta].Nomdufichier , MT.Nomdufichier);
   buffer.content.metadataTable.T[indexMeta].Modeorganisationglobale= 1;
   buffer.content.metadataTable.T[indexMeta].Modeorganisationinterne=1;
   buffer.content.metadataTable.T[indexMeta].Adrpremierbloc=blocNbr;
   buffer.content.metadataTable.nbrMetadonnees=indexMeta;
   nbrB++;

   fwrite(&buffer, sizeof(buffer), 1, disque);
   rewind (disque);
   fread(&buffer, sizeof(buffer), 1, disque);
   mettreAJourNombreBlocs(disque,2,nbrMalade++);
   printf("le fichier est crée avec succès ");
   }

//-------------------------------INSERTION TNOF---------------------------------
  void InsertionfileTNOF(FILE *disque ){
    Bloc buffer;
    maladie newmalade;
    rewind(disque);
    fread(&buffer, sizeof(buffer), 1, disque);       //   les informations de nouveau malade
    printf("entrer les informations de nouveau malade :\n pour l'arrêt vous pouvez entrer -1 pour la reference \n");
    printf("entrer la reference de malade:\n ");
    scanf("%d",&newmalade.id);
    printf("entrer le nom de malade :\n");
    scanf("%s",newmalade.name);
    printf("entrer l'age de malade:\n");
    scanf("%d",&newmalade.age);
    printf("entrer le nombre de visite :\n");
    scanf("%d",&newmalade.nmbrdevisite);
    printf("entrer l'adress de malade :\n");
    scanf("%s",newmalade.adresse);
    printf("entrer le sexe de malade :\n");
    scanf("%s",newmalade.sexe);

    int indexMeta= buffer.content.metadataTable.nbrMetadonnees-1;
    fread(&buffer, sizeof(buffer), 1, disque);
    int iend =buffer.content.metadataTable.T[indexMeta].Taillefichierenregistrements;
    if (iend/ buffer.content.metadataTable.T[indexMeta].Taillefichierenregistrements!=0){   // il y a espace
    fseek(disque,0,SEEK_END);
    newmalade.suprimelogiquement=0;
    buffer.content.fileData.T[iend+1].id =newmalade.id;//id
    copyString(buffer.content.fileData.T[iend+1].name, newmalade.name);//name
    copyString(buffer.content.fileData.T[iend+1].sexe, newmalade.sexe);//sexe
    buffer.content.fileData.T[iend+1].age =newmalade.age;//age
    buffer.content.fileData.T[iend+1].nmbrdevisite =newmalade.nmbrdevisite;//nbrvisite
    copyString(buffer.content.fileData.T[iend+1].adresse, newmalade.adresse);//adress
    buffer.content.fileData.T[iend+1].suprimelogiquement =newmalade.suprimelogiquement;

    fwrite(&buffer, sizeof(buffer), 1, disque);

    } else {
    int i=0;
    if (!verifierEspaceSuffisant(disque,1)) {
    compactageDisque(disque);
   }
    buffer.content.fileData.T[i].id =newmalade.id;//id
    copyString(buffer.content.fileData.T[i].name,newmalade.name);//name
    copyString(buffer.content.fileData.T[i].sexe,newmalade.sexe);//sexe
    buffer.content.fileData.T[i].age =newmalade.age;//age
    buffer.content.fileData.T[i].nmbrdevisite =newmalade.nmbrdevisite;//nbrvisite
    copyString(buffer.content.fileData.T[i].adresse,newmalade.adresse);//adress
    buffer.content.fileData.T[i].suprimelogiquement =newmalade.suprimelogiquement;
    buffer.typedebloc=2;
     fwrite(&buffer, sizeof(buffer), 1, disque);
      buffer.content.metadataTable.T[indexMeta].Taillefichierblocs++;
      int l = buffer.content.metadataTable.T[indexMeta].Taillefichierblocs + buffer.content.metadataTable.T[indexMeta].Adrpremierbloc;
       metajourtableallocation (disque,l, 1);
}
   int index= buffer.content.metadataTable.nbrMetadonnees;
    buffer.content.metadataTable.T[index].Taillefichierenregistrements++;
    buffer.typedebloc=1;
    fwrite(&buffer, sizeof(buffer), 1, disque);   // to cheak later !!!

  }
   //---------------------------------------LA RECHERCHE dans fichier TNOF-----------------------
  pos rechercheFILETNOF(FILE *disque , int id){
    pos pos;
     Bloc buffer;
   rewind(disque);
   fread(&buffer, sizeof(buffer), 1, disque);
   //char filename[20];

   int nbrB=0;
   //int position =1;
   while (fread(&buffer, sizeof(buffer), 1, disque) && buffer.typedebloc==2) {
     nbrB++;
    for(int i=0;i<20;i++) {
    if(buffer.content.fileData.T[i].id  == id) {
        pos.blocNbr=nbrB;
        pos.deplacment=i;

        return pos;
    }
    }
    }

   printf("l'element n'existe pas dans le fichier");
    pos.blocNbr=-1;
    pos.deplacment=-1;
    return pos;
  }


  // -------------------------------------LA SUPPRESSION D'UN ELEMENT DE FICHIER TNOF-------------------------------
 //-------------------------------------------------------------------------------------------------
// 1 ----------------SUPPRESSION LOGIQUE TNOF------------------------------
void supprLogiqueFileTNOF(FILE*disque , int idSuppr){
Bloc buffer;
int compteur=-1;
pos p = rechercheFILETNOF(disque,idSuppr);
if (p.blocNbr==-1 && p.deplacment==-1){
printf("ERROR!\n < l'element que vous voulez suprimer n'existe pas > \n");
 return;
}

rewind(disque);
while (compteur!=p.blocNbr){
fread(&buffer,sizeof(buffer),1,disque);
compteur=compteur+1;
}
buffer.content.fileData.T[p.deplacment].suprimelogiquement =1; //1 supprimer logiquement
fwrite(&buffer,sizeof(buffer),1,disque);
printf("---------------------le malade est supprimé-------------------");
}


// 2 -----------------------------------SUPPRESSION PHYSIQUE TNOF----------------------------------------------------
void supprPhysiqueFileTNOF(FILE*disque ,int idSuppr ){
Bloc buffer,buffer2;

int compteur=-1;
pos p = rechercheFILETNOF(disque,idSuppr);
if (p.blocNbr==-1 && p.deplacment==-1){
printf("ERROR \n <l'element que vous voulez suprimer n'existe pas> \n ");
}

rewind(disque);
fread(&buffer,sizeof(buffer),1,disque);//lire le bloc de table d'allocation
do{
fread(&buffer,sizeof(buffer),1,disque);
compteur=compteur+1;
}while (compteur!=p.blocNbr);
// decalage intraBloc
int i;
int dep = p.deplacment;
if (dep -20-1) {
for ( i=dep ; i<20-2; i++){ //decalage des element dans le bloc
    buffer.content.fileData.T[i].id =buffer.content.fileData.T[i+1].id ;//id
    copyString(buffer.content.fileData.T[i].name,buffer.content.fileData.T[i+1].name);//name
    copyString(buffer.content.fileData.T[i].sexe,buffer.content.fileData.T[i+1].sexe);//sexe
    buffer.content.fileData.T[i].age = buffer.content.fileData.T[i+1].age ;//age
    buffer.content.fileData.T[i].nmbrdevisite =buffer.content.fileData.T[i+1].nmbrdevisite;//nbrvisite
    copyString(buffer.content.fileData.T[i].adresse,buffer.content.fileData.T[i+1].adresse);//adress
    buffer.content.fileData.T[i].suprimelogiquement =buffer.content.fileData.T[i+1].suprimelogiquement ;
    }
}
// decalage intreBloc
fseek(disque, -1,SEEK_END); // maitre le dernier element dans le bloc ou l'element qu'on a supprime
fread(&buffer2,sizeof(buffer2),1,disque);
int j=0;

while( buffer2.content.fileData.T[j].id !='\0'){//trouver l'indice de dernier element dans le bloc
  j++;
}
// modification dans le fichier
buffer.content.fileData.T[i+1].id =buffer2.content.fileData.T[j-1].id ;//id
copyString(buffer.content.fileData.T[i+1].name , buffer2.content.fileData.T[j-1].name);//name
copyString(buffer.content.fileData.T[i+1].sexe , buffer2.content.fileData.T[j-1].sexe);//sexe
buffer.content.fileData.T[i+1].age =buffer2.content.fileData.T[j-1].age ;//age
buffer.content.fileData.T[i+1].nmbrdevisite =buffer2.content.fileData.T[j-1].nmbrdevisite ;//nbrvisite
copyString(buffer.content.fileData.T[i+1].adresse , buffer2.content.fileData.T[j-1].adresse);//adress
buffer.content.fileData.T[i+1].suprimelogiquement= buffer2.content.fileData.T[j-1].suprimelogiquement ;

int depl=p.deplacment;
if (depl == 20){  //-----le cas suppresion de dernier element
int nombreElements = 20;
    // Réduire le nombre d'éléments
    nombreElements--;
 //demunation de taille logique de T
buffer.content.fileData.T[nombreElements].id =buffer2.content.fileData.T[j-1].id ;//id
copyString(buffer.content.fileData.T[nombreElements].name , buffer2.content.fileData.T[j-1].name);//name
copyString(buffer.content.fileData.T[nombreElements].sexe , buffer2.content.fileData.T[j-1].sexe);//sexe
buffer.content.fileData.T[nombreElements].age =buffer2.content.fileData.T[j-1].age ;//age
buffer.content.fileData.T[nombreElements].nmbrdevisite =buffer2.content.fileData.T[j-1].nmbrdevisite ;//nbrvisite
copyString(buffer.content.fileData.T[nombreElements].adresse , buffer2.content.fileData.T[j-1].adresse);//adress
buffer.content.fileData.T[nombreElements].suprimelogiquement = buffer2.content.fileData.T[j-1].suprimelogiquement ;
}
fwrite(&buffer,sizeof(buffer),1,disque);// bloc ou l'element qu'on veut supprimer
int blocVide=0; // le dernier bloc
//sizeof(buffer2.content.fileData.T)-1;//supprimer le dernier element de dernier bloc
if (sizeof(buffer2.content.fileData.T) == 0) blocVide=1;
fwrite(&buffer2,sizeof(buffer2)-1 , 1,disque);
rewind(disque);
 int nbrB =-1;
int test ;
//modification de meta donnee
while (fread(&buffer,sizeof(buffer),1,disque)) {
    nbrB++;
    if (buffer.typedebloc==1){
      int metaAct = buffer.content.metadataTable.nbrMetadonnees;
      if (buffer.content.metadataTable.T[metaAct].Adrpremierbloc==nbrB)  test = 0;
      if (buffer.content.metadataTable.nbrMetadonnees && test == 0 ){
         // on est dans le fichier metadonnee qui corespond aux ce fichier
         buffer.content.metadataTable.T[metaAct].Taillefichierenregistrements--;
        if (blocVide==1) {
           buffer.content.metadataTable.T[metaAct].Taillefichierblocs--;
           // hadi s9miha mtmchich mli7
           metajourtableallocation (disque, nbrB, 0); // bloc sera vide
}}}};
printf("---------------------le malade %d est supprimé physiquement---------------------\n", buffer.content.fileData.T[i].id);

}// tout les cas sont traiter normalement

// ---------------------------------------Defragmentation TNOF -----------------------
void defragmentationFileTNOF(FILE *disque,const char*nomfichier ){
  Bloc buffer;
rewind(disque);
while (!feof(disque)){
fread(&buffer,sizeof(buffer),1,disque);
for (int i=0;i<20;i++){
  if (buffer.content.fileData.T[i].suprimelogiquement== 1 || buffer.content.fileData.T[i].id ==-2){
     int supp = buffer.content.fileData.T[i].id ;
    supprPhysiqueFileTNOF(disque , supp );
}
}
}}

 // ---------------------------------------Suppression d'un fichier TNOF  -----------------------
 // -------------------------------------suppression physique-----------------------------


 void supprPhysiqueDeFichierTNOF(FILE *disque , const char*filename){
 Bloc buffer;
 int adr,taille;
 int nbrB=-1;
 rewind(disque);
 while (!feof(disque)){
  fread(&buffer, sizeof(buffer),1,disque);
  nbrB++;
  if (buffer.typedebloc==1){ // on a effacer les metadonnee de fichier
    for (int i=0;i<20;i++){
      if (strcmp(buffer.content.metadataTable.T[i].Nomdufichier,filename)==0){
         buffer.content.metadataTable.nbrMetadonnees--;
         adr = buffer.content.metadataTable.T[i].Adrpremierbloc ;
         taille = buffer.content.metadataTable.T[i].Taillefichierblocs;
        for (int j=i;j<20;j++){
          buffer.content.metadataTable.T[j]=buffer.content.metadataTable.T[j+1];
        }}}}
 if( buffer.typedebloc == 2 && nbrB == adr){ //on a trouver le fichier a supprimer
   for (int i=0; i<20; i++){
    buffer.content.fileData.T[i].id = -2;//id
    buffer.content.fileData.T[i].suprimelogiquement=1;
   }
  fwrite(&buffer, sizeof(buffer),taille,disque); //tout les blocs de fichier perdent leur contenue
  defragmentationFileTNOF(disque,filename );
  for(int j= nbrB;j<(nbrB+taille);j++) {
  metajourtableallocation (disque, j, 0); // bloc sera vide
  }
  compactageDisque(disque);
 printf("---------------------le fichier %s est supprimé physiquement---------------------\n", filename);
   return;
 }

 }

 }

 /*-------------------------partie TNo_f----------------------------------------------------*/




void chargerFichier(FILE *disque ) {
    // Allouer les blocs n�cessaires � partir de la table d'allocation
    fseek(disque, 0, SEEK_SET);
    Bloc buffer;
    int nbrbloc=obtenirNombreBlocs(disque,1);

    // Lire les m�tadonn�es
    fread(&buffer, sizeof(Bloc), 1, disque);

    // Initialiser les blocs et marquer les blocs comme allou�s
    int i;
    for ( i = 0; i < nbrbloc; i++) {
        if (buffer.content.allocation.tablelocation[i].etat == 0) {
            // Trouver un bloc vide et le marquer comme plein
            buffer.content.allocation.tablelocation[i].etat = 1;
            break;
        }
    }
    // �crire les donn�es mises � jour
    fseek(disque, 0, SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
}




//insertion ...................................................................

void insererEnregistrement(FILE *disque, const char*filename) {
    int i, j ,MAX_BLOCKS;
    bool inserted = false;
    int Taillefichierenregistrements ;
    Bloc buffer;
    maladie newmalade;

    rewind(disque);
    fread(&buffer, sizeof(Bloc), 1, disque);
    //   les informations de nouveau malade
    printf("entrer les informations de nouveau malade :\n pour l'arr�t vous pouvez entrer -1 pour la reference \n");
    printf("entrer la reference de malade:\n ");
    scanf("%d",&newmalade.id);
    printf("entrer le nom de malade :\n");
    scanf("%s",newmalade.name);
    printf("entrer l'age de malade:\n");
    scanf("%d",&newmalade.age);
    printf("entrer le nombre de visite :\n");
    scanf("%d",&newmalade.nmbrdevisite);
    printf("entrer l'adress de malade :\n");
    scanf("%s",newmalade.adresse);
    printf("entrer le sexe de malade :\n");
    scanf("%s",newmalade.sexe);
    // Chercher un bloc libre ou un bloc o� ins�rer (� partir du bloc 3)
    for (i = 3; i < Taillefichierenregistrements ; i++) {  // � partir du bloc 3 (bloc 1 et 2 sont r�serv�s)
        if (  buffer.content.fileData.nbrmaladie < Taillefichierenregistrements) {
            // Si le bloc est partiellement plein, on ins�re dans l'ordre croissant ou d�croissant
            for (j = 0; j < buffer.content.fileData.nbrmaladie; j++) {
                // V�rification pour ins�rer en ordre croissant ou d�croissant selon le premier ID
                if ((buffer.content.fileData.T[j].id > newmalade.id) || (buffer.content.fileData.T[0].id < newmalade.id)) {
                    // D�calage des enregistrements pour ins�rer
                    int k;
                    for ( k = buffer.content.fileData.nbrmaladie; k > j; k--) {
                        buffer.content.fileData.T[k] = buffer.content.fileData.T[i];
                    }
                    buffer.content.fileData.T[j] = newmalade;
                    buffer.content.fileData.nbrmaladie++;
                    inserted = true;
                    break;
                }
            }
            if (inserted) break; // Insertion r�ussie
        }
    }
    // Si aucune place n'a �t� trouv�e, on v�rifie le compactage et d�fragmentation
    if (!inserted) {
        // Recherche d'un bloc libre ou d'un espace pour d�placer
        for (i = 3; i < Taillefichierenregistrements - 1; i++) {
            if (buffer.content.fileData.nbrmaladie == Taillefichierenregistrements) {
                if (buffer.content.fileData.nbrmaladie == 0) {
                    // D�placement des enregistrements dans le bloc suivant si vide
                    buffer.content.fileData.T[i+1]= buffer.content.fileData.T[i];

                    buffer.content.fileData.nbrmaladie = 0;  // Lib�rer le bloc original

                    break;
                }
            }
        }
        if (!inserted) {
            printf("Aucun espace disponible pour ins�rer le nouvel enregistrement.\n");
        }
    }
    // Mettre � jour le fichier disque avec les changements
    fseek(disque, 0, SEEK_SET); // Revenir au d�but du fichier
    for (i = 0; i < MAX_BLOCKS; i++) {
        // Sauvegarder les informations des blocs dans le fichier
        fwrite(&buffer, sizeof(Bloc), 1, disque);
    }
    // Sauvegarder la table d'allocation des blocs
    fseek(disque, sizeof(Bloc) * MAX_BLOCKS, SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
}




//recherche

void rechercheParID(FILE *disque, int idRecherche) {
    fseek(disque, 0, SEEK_SET);
    Bloc buffer;
    fread(&buffer, sizeof(Bloc), 1, disque);

    // Recherche binaire dans un bloc ordonn�
    int gauche = 0, droite = 20- 1;
    while (gauche <= droite) {
        int milieu = (gauche + droite) / 2;
        if (buffer.content.fileData.T[milieu].id == idRecherche) {
            printf("Enregistrement trouv� dans le bloc %d, � la position %d.\n", buffer.content.fileData.T[milieu].id, milieu);
            return;
        }
        if (buffer.content.fileData.T[milieu].id < idRecherche) {
            gauche = milieu + 1;
        } else {
            droite = milieu - 1;
        }
    }

    printf("Enregistrement avec ID %d non trouv�.\n", idRecherche);
}



// supprecion logique

void suppressionLogique(FILE *disque, int id) {
    fseek(disque, 0, SEEK_SET);
    Bloc buffer;
    fread(&buffer, sizeof(Bloc), 1, disque);

    // Chercher l'enregistrement par ID
    int i;
    for ( i = 0; i < 20; i++) {
        if (buffer.content.fileData.T[i].id == id) {
            // Marquer l'enregistrement comme supprim� logiquement
            buffer.content.fileData.T[i].suprimelogiquement = 1;
            printf("Enregistrement avec ID %d supprim� logiquement.\n", id);
            break;
        }
    }

    // Mettre � jour le disque
    fseek(disque, 0, SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
}



//suppresion phisique

void suppressionPhysique(FILE *disque, int id) {
    fseek(disque, 0, SEEK_SET);
    Bloc buffer;
    fread(&buffer, sizeof(Bloc), 1, disque);

    // Chercher l'enregistrement par ID
    int i;
    for ( i = 0; i < 20; i++) {
        if (buffer.content.fileData.T[i].id == id) {
            // R�organiser physiquement les blocs en d�pla�ant l'enregistrement � la fin
            buffer.content.fileData.T[i] = buffer.content.fileData.T[20 - 1];
            buffer.content.fileData.nbrmaladie--;  // R�duire le nombre d'enregistrements
            printf("Enregistrement avec ID %d supprim� physiquement.\n", id);
            break;
        }
    }

    // Mettre � jour le disque
    fseek(disque, 0, SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
}


// defragmantation


void defragmentationyousra(FILE *disque,const char*nomFichier) {
    fseek(disque, 0, SEEK_SET);
    Bloc buffer;
    fread(&buffer, sizeof(Bloc), 1, disque);
    int nbrbloc=obtenirNombreBlocs(disque,2);
    int nbrutili=obtenirNombreBlocs(disque,1);

    int index = 0;
    int i;
    for ( i = 0; i < nbrbloc; i++) {
        if (buffer.content.fileData.T[i].suprimelogiquement == 0) {
            buffer.content.fileData.T[index] = buffer.content.fileData.T[i];
            index++;
        }
    }

    // Mettre � jour les blocs apr�s la d�fragmentation
    fseek(disque, 0, SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
    printf("D�fragmentation termin�e.\n");
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

    // Mettre � jour le disque
    fseek(disque, 0, SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
    printf("Fichier renomm� en %s.\n", nouveauNom);
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
            buffer.content.metadataTable.T[i].Nomdufichier[0] = '\0'; // Marquer comme supprim�
            break;
        }
    }

    // Mettre � jour le disque
    fseek(disque, 0, SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
    printf("Fichier %s supprim�.\n", nomFichier);
}

/* ---------------------------------LOF--------------------------------------------------------------------------*/


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

       // Récupérer les métadonnées du fichier
       fichiermetadonnes metadonnes;
       nomFichier=creerfichierCO(disque);
       int blocnecessaire = liremetadonnes(disque,nomFichier,1);
       int bloctrouve = 0; // Pour compter les blocs trouvés
       int adrPremierBloc = -1; // Variable pour stocker l'adresse du premier bloc
       int nbrbloctotal=obtenirNombreBlocs(disque,2);
       int nbrblocutiliser=obtenirNombreBlocs(disque,1);
       int blocPrecedent=-1;
       int place=-1;




       // Allouer les blocs nécessaires
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

    // Si c'est le premier bloc alloué
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
        // Chaînage des blocs : mise à jour du champ "next" du dernier bloc alloué

        fseek(disque, blocPrecedent * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);
        buffer.content.fileData.next= place;  // Mise à jour du champ "next"
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

    // Enregistrer l'adresse du bloc actuel pour le chaînage futur
    blocPrecedent=place ;
    bloctrouve++;
}

       // Si les blocs nécessaires n'ont pas été trouvés

       if (bloctrouve < blocnecessaire) {
           printf("Espace insuffisant pour allouer tous les blocs necessaires.\n");
           return;

           // Libération des blocs déjà alloués
           for (int i = 0; i < nbrbloctotal; i++) {
               if (buffer.content.allocation.tablelocation[i].etat == 1) {
                   metajourtableallocation(disque, i, 0);
                   printf("Bloc %d libere.\n", i);
               }
           }
           return;
       }

       // Mise à jour des métadonnées
       miseAJourMetadonnees(disque, nomFichier, 3, adrPremierBloc);
       mettreAJourNombreBlocs(disque,1,nbrblocutiliser+blocnecessaire);

       printf("Fichier charge avec succes.\n");
   }

  void defragmentationlof(FILE *disque, const char *nomFichier) {
    Bloc buffer;             // Buffer pour charger les blocs
    maladie temp[20];        // Tableau temporaire pour réorganiser les enregistrements
    int blocactuelle, blocsuivant; // Pointeurs pour le bloc courant et suivant
    int indexTemp = 0;       // Indice pour remplir le tableau temporaire
    int totalEnregistrements = 0; // Compteur pour les enregistrements valides
    int taillefichierblocs;  // Nombre de blocs utilisés après défragmentation
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

    // Étape 1 : Collecter tous les enregistrements valides
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

    // Calculer le nombre de blocs nécessaires après défragmentation
    taillefichierblocs = (indexTemp + 20 - 1) / 20; // Diviser en arrondissant vers le haut

    // Étape 2 : Réécrire les enregistrements dans les blocs nécessaires
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

        // Gestion du chaînage
        if (i == taillefichierblocs - 1) {
            buffer.content.fileData.next = -1; // Dernier bloc
        } else {
            blocsuivant = buffer.content.fileData.next; // Conserver le chaînage
        }

        // Écrire le bloc mis à jour
        fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
        if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Erreur : Échec d'écriture dans le bloc %d.\n", blocactuelle);
            return;
        }

        blocactuelle = buffer.content.fileData.next;
    }

    // Étape 3 : Libérer les blocs inutilisés dans la table d'allocation
    while (blocactuelle != -1) {
        fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
        if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Erreur : Impossible de lire le bloc %d.\n", blocactuelle);
            return;
        }

        // Libérer le bloc courant
        metajourtableallocation(disque, blocactuelle, 0);

        blocactuelle = buffer.content.fileData.next;
    }

    // Étape 4 : Mise à jour des métadonnées
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
            printf("Erreur : Le bloc %d n'est pas un bloc de données.\n", blocactuelle);
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
   maladie enrdecale;// l'enregistrement qui va decalé vers le bloc suivant est aussi le derniere enregistrement dans le bloc ou on a trouver la position
   maladie enr;// variable qui va engistré l'enregistrement qui va changer du bloc
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

   // Parcourir les blocs jusqu'au dernier ou atteindre le nombre de blocs utilisés
   buffer.typedebloc=2;
    for (int i = 0; i <nbrbloc ; i++) {
        fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
        if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Erreur : Impossible de lire le bloc %d.\n", blocactuelle);
            return; // Erreur
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

    // Détecter la position exacte pour l'insertion
    int position = buffer.content.fileData.nbrmaladie; // Par défaut, insérer à la fin
    for (int i = 0; i < buffer.content.fileData.nbrmaladie; i++) {
        if (m.id < buffer.content.fileData.T[i].id) { // Critère : ID croissant
            position = i;
            break; // Trouvé la position correcte
        }
    }

    printf("Position trouvée pour l'insertion : %d\n", position);

    // Décaler les enregistrements à partir de la position trouvée
    for (int j = buffer.content.fileData.nbrmaladie; j > position; j--) {
        buffer.content.fileData.T[j] = buffer.content.fileData.T[j - 1];
    }

    // Insérer le nouvel enregistrement à la position correcte
    buffer.content.fileData.T[position] = m;
    buffer.content.fileData.nbrmaladie++;

    // Mettre à jour les métadonnées
    miseAJourMetadonnees(disque, nomFichier, 2, nbrenregistrement + 1);

    // Écrire les changements dans le fichier disque
    fseek(disque, debut * sizeof(Bloc), SEEK_SET);
    if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
        printf("Erreur : Impossible d'écrire les modifications dans le bloc %d.\n", debut);
        return;
    }

    printf("Insertion dans le 1er bloc effectuée avec succès.\n");
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
               buffer.content.fileData.T[j] = buffer.content.fileData.T[j - 1]; // Décalage intra bloc
           }
           buffer.content.fileData.T[position] = m;
           buffer.content.fileData.nbrmaladie++;
           // Écrire les changements
           fseek(disque, blocdernier * sizeof(Bloc), SEEK_SET);
           fwrite(&buffer, sizeof(Bloc), 1, disque);
           printf("Insertion termine avec succes.\n");
           // Mise à jour des métadonnées
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
           // Charger le dernier bloc pour récupérer l'enregistrement à décaler
           fseek(disque, blocdernier * sizeof(Bloc), SEEK_SET);
           fread(&buffer, sizeof(Bloc), 1, disque);

           if(enrdecale.id>=m.id) {
                enrdecale = buffer.content.fileData.T[buffer.content.fileData.nbrmaladie - 1]; // Dernier enregistrement du bloc
           for (int j = buffer.content.fileData.nbrmaladie - 1; j > position; j--) {
               buffer.content.fileData.T[j] = buffer.content.fileData.T[j - 1];
           }

           buffer.content.fileData.T[position] = m; // Insérer le nouvel enregistrement
           buffer.content.fileData.nbrmaladie++;   // Mettre à jour le nombre d'enregistrements
           buffer.content.fileData.next = place; // Mettre à jour le chaînage

           // Écrire les modifications dans le dernier bloc

           fseek(disque, blocdernier * sizeof(Bloc), SEEK_SET);
           fwrite(&buffer, sizeof(Bloc), 1, disque);
           buffer.content.fileData.T[0] = enrdecale;

          } else{
               // Allouer un nouveau bloc pour le décalage
           fseek(disque, place * sizeof(Bloc), SEEK_SET);
           fread(&buffer, sizeof(Bloc), 1, disque);
   //verifier si le eng que en va inserer et superieur au tous les eng alors il va inserer dans le bloc allouer sinon le derniere eng de dernier bloc il va sauter
            buffer.content.fileData.T[0]=m;}
          // Déplacer l'enregistrement décalé dans le nouveau bloc


           buffer.content.fileData.nbrmaladie = 1;
           buffer.content.fileData.next = -1;

           // Écrire le nouveau bloc
           fseek(disque, place * sizeof(Bloc), SEEK_SET);
           fwrite(&buffer, sizeof(Bloc), 1, disque);

           buffer.typedebloc=3;
           // Mettre à jour la table d'allocation et les métadonnées
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

   // buffer est chargé le bloc ou en va inserer

   enrdecale=buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1];// l'enregistrement qui va decalé vers le bloc suivant est aussi le derniere enregistrement dans le bloc

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

   enr=buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1];// avant decalge en engistré le eng qui va sauter

     for(int i=20-1;i>0;i--) {// decaler pour vider la 1 case
       buffer.content.fileData.T[i]=buffer.content.fileData.T[i-1];
     }

     buffer.content.fileData.T[0]=enrdecale; // met le eng qui a sauter dans la 1 case
     enrdecale=enr;
     // ecrire les modification de bloc actuelle avant de  passé a le bloc suivant

     fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
     fwrite(&buffer, sizeof(Bloc), 1, disque);

     blocdernier=blocactuelle;//pour engistré l'adresse de dernier bloc
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
    adressemetadonnes adressetrouve = {-1, -1}; // Initialisation à "non trouvé"
    Bloc buffer;
    int blocactuelle = liremetadonnes(disque, nomFichier, 3); // Récupérer l'adresse du premier bloc

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

        // Vérification que c'est un bloc de données
        if (buffer.typedebloc != 2) {
            printf("Erreur : Le bloc %d n'est pas un bloc de données.\n", blocactuelle);
            break;
        }

        // Parcourir les enregistrements du bloc
        for (int j = 0; j < buffer.content.fileData.nbrmaladie; j++) {
            if (buffer.content.fileData.T[j].suprimelogiquement == 0 && buffer.content.fileData.T[j].id == ID) {
                adressetrouve.numerodebloc = blocactuelle;
                adressetrouve.index = j;
                printf("Enregistrement trouvé dans le bloc %d, index %d.\n", blocactuelle, j);
                return adressetrouve;
            }
        }

        blocactuelle = buffer.content.fileData.next; // Passer au bloc suivant
    }

    printf("Enregistrement avec ID %d introuvable dans le fichier %s.\n", ID, nomFichier);
    return adressetrouve; // Retourne {-1, -1} si l'enregistrement n'est pas trouvé
}



   void supprimerEnregistrementLogique(FILE *disque, const char *nomFichier, int ID) {
    // Structure pour stocker les données
    Bloc buffer;

    // Rechercher l'enregistrement par son ID
    adressemetadonnes adresse = rechercheenregistrement(disque, nomFichier, ID);

    // Vérifier si l'enregistrement existe
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

    // Modifier l'état de l'enregistrement pour le marquer comme supprimé
    buffer.content.fileData.T[adresse.index].suprimelogiquement = 1;

    // Écrire les modifications dans le fichier disque
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
        printf("Erreur : Échec de l'écriture dans le bloc %d.\n", adresse.numerodebloc);
        return;
    }

    // Confirmation de suppression
    printf("L'enregistrement avec ID %d a été supprimé logiquement du fichier %s.\n", ID, nomFichier);
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
       maladie m;// pour faire engistré les enregistrement qui va changer apres decalage

       adressemetadonnes adresse=rechercheenregistrement(disque,nomFichier,ID);

       if (adresse.numerodebloc == -1) {
           printf("Enregistrement introuvable : ID %d dans le fichier %s.\n", ID, nomFichier);
           return;
       }

      adressemetadonnes meta=recherchemetadonnes(disque,nomFichier);//recuperer l'adresse de metadonnes pour met a jour

      if (meta.numerodebloc == -1) {
           printf("Métadonnées introuvables pour le fichier : %s\n", nomFichier);
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
      fseek(disque, dernierbloc * sizeof(Bloc), SEEK_SET);// charger le bloc pour pointé sur bloc suivant
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

  // Décaler les éléments vers la gauche à partir de l'index
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
   fseek(disque,  blocactuelle* sizeof(Bloc), SEEK_SET);// charger le bloc ou on va suprimé
   fread(&buffer, sizeof(Bloc), 1, disque);

  // faire la supression

  // Décaler les éléments vers la gauche à partir de l'index
       for (int i = adresse.index; i < (buffer.content.fileData.nbrmaladie) - 1; i++) {
           buffer.content.fileData.T[i] = buffer.content.fileData.T[i + 1];
       }

      fseek(disque, debut * sizeof(Bloc), SEEK_SET); // engistré les modification apres changement de chainage
       fwrite(&buffer, sizeof(Bloc), 1, disque);

      miseAJourMetadonnees(disque, nomFichier, 2,nbrenregistrement--);//mise a jour metadonnes



  blocprecedent=adresse.numerodebloc;
   blocactuelle=buffer.content.fileData.next;

  while(buffer.content.fileData.next!=-1) // pour ne entré pas dans le derniere bloc pour deviser les cas
   {

  fseek(disque,blocactuelle* sizeof(Bloc), SEEK_SET);// charger le bloc suivante
   fread(&buffer, sizeof(Bloc), 1, disque);

  m=buffer.content.fileData.T[0];// le premiere enregistrement il rementera au bloc precedent

  for (int i = 0; i < (buffer.content.fileData.nbrmaladie) - 1; i++) { // fait le decalge parceque le premiere eng est l'element a ete suprimé qui va rementre au bloc precedent
           buffer.content.fileData.T[i] = buffer.content.fileData.T[i + 1];
       }

  fseek(disque,blocactuelle* sizeof(Bloc), SEEK_SET);// charger le bloc suivante
   fwrite(&buffer, sizeof(Bloc), 1, disque);//pour engitré les changement

  fseek(disque,blocprecedent* sizeof(Bloc), SEEK_SET);// charger le bloc suivante  pour remplir le derniere eng qui est vide apres le decalge
   fread(&buffer, sizeof(Bloc), 1, disque);

  buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1]=m;// le premiere enr de bloc actuelle dans derniere eng de bloc precedent

  fseek(disque, blocprecedent * sizeof(Bloc), SEEK_SET); // engistré les modification apres decalage vers la gauche
   fwrite(&buffer, sizeof(Bloc), 1, disque);

   blocprecedent=blocactuelle;
   blocactuelle=buffer.content.fileData.next;

  }


  // etrer dans le derniere bloc pour engistré le 1 eng qui va chargé dans le bloc precedent
   fseek(disque,blocactuelle* sizeof(Bloc), SEEK_SET);// car la boucle a    eté arreter  dans le derniere bloc
   fread(&buffer, sizeof(Bloc), 1, disque);

   m=buffer.content.fileData.T[0];

  fseek(disque,blocprecedent* sizeof(Bloc), SEEK_SET);// pour met le eng dans bloc avant dernier dans derniere place
   fread(&buffer, sizeof(Bloc), 1, disque);

  buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1]=m;

  // ecrire les changement

  fseek(disque, blocprecedent * sizeof(Bloc), SEEK_SET); // engistré les modification apres decalage
   fwrite(&buffer, sizeof(Bloc), 1, disque);
   }

  // separer les cas si le bloc vide alors en liberer derniere bloc sinon en fais derniere decalage dans le derniere bloc
   // cas 5: avec decalage et bloc vide
   if (blocvide==true && decalage==true) // changer chainage car le bloc est vide
   {



  buffer.content.fileData.next=-1;  // psq je suis dans l'avant derniere

  fseek(disque, blocprecedent * sizeof(Bloc), SEEK_SET); // engistré les modification de chainage
   fwrite(&buffer, sizeof(Bloc), 1, disque);


  metajourtableallocation(disque,blocactuelle,0);//mise a jour table d'allocation
   miseAJourMetadonnees(disque, nomFichier, 2,nbrbloc--);//mise a jour taille en bloc
   mettreAJourNombreBlocs(disque,1,nbrblocutil-1);

  printf("Suppression physique effectuée pour l'enregistrement ID %d.\n", ID);

  return;
   }
   else // cas 6 : avec decalage est pas de bloc libre
   {
  fseek(disque,blocactuelle* sizeof(Bloc), SEEK_SET);// pour met le eng dans bloc  dernier dans derniere place
   fread(&buffer, sizeof(Bloc), 1, disque);

  // le derniere eng dans bloc precedent est vide

  m=buffer.content.fileData.T[0];

  for (int i = 0; i < (buffer.content.fileData.nbrmaladie) - 1; i++) { // fait le decalge pparceque le premiere est l'element a ete suprimé
           buffer.content.fileData.T[i] = buffer.content.fileData.T[i + 1];
       }

  buffer.content.fileData.nbrmaladie--;

  fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
// engistré les modification apres le dernier decalage
   fwrite(&buffer, sizeof(Bloc), 1, disque);

  fseek(disque,blocprecedent* sizeof(Bloc), SEEK_SET);// pour met le 1 eng de bloc  dernier dans l'avant derniere bloc dans derniere place
   fread(&buffer, sizeof(Bloc), 1, disque);

  buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1]=m;

  fseek(disque,blocprecedent* sizeof(Bloc), SEEK_SET);// engistré les modification
   fwrite(&buffer, sizeof(Bloc), 1, disque);

  printf("Suppression physique effectuée pour l'enregistrement ID %d.\n", ID);

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



  // engistré les modification
       fseek(disque, adress.numerodebloc * sizeof(Bloc), SEEK_SET);
       fwrite(&buffer, sizeof(Bloc), 1, disque);

  printf("succes");

  return;
   }



  void suprimerFCO(FILE* disque, const char* nomFichier) {
       Bloc buffer;
       int dernierblocmeta = 1; // Dernier bloc de métadonnées
       int blocprecedent = -1;
       fichiermetadonnes dernierengmeta;
       int nbrbloc = liremetadonnes(disque, nomFichier, 1); // Nombre de blocs utilisés par le fichier
       int adrpremierbloc = liremetadonnes(disque, nomFichier, 3); // Adresse du premier bloc du fichier
       int nbrblocutil = obtenirNombreBlocs(disque, 1); // Nombre total de blocs utilisés
       bool vide = false; // Indicateur pour savoir si le dernier bloc sera vide après suppression

      adressemetadonnes adress = recherchemetadonnes(disque, nomFichier);

      if (adress.numerodebloc == -1) {
           printf("Erreur : Métadonnées non trouvées pour le fichier : %s\n", nomFichier);
           return;
       }

      // Chercher l'adresse du dernier bloc qui stocke les métadonnées
       while (1) {
           fseek(disque, dernierblocmeta * sizeof(Bloc), SEEK_SET);
           if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : Impossible de lire le bloc %d.\n", dernierblocmeta);
               return;
           }

          if (buffer.typedebloc != 1) {
               printf("Erreur : Le bloc %d n'est pas un bloc de métadonnées.\n", dernierblocmeta);
               return;
           }

          if (buffer.content.metadataTable.next == -1) break; // Dernier bloc atteint

          blocprecedent = dernierblocmeta;
           dernierengmeta = buffer.content.metadataTable.T[buffer.content.metadataTable.nbrMetadonnees - 1];
           dernierblocmeta = buffer.content.metadataTable.next;
       }

      // Vérifier si le dernier bloc sera vide après suppression
       if (buffer.content.metadataTable.nbrMetadonnees == 1) {
           vide = true;
       }

      // Supprimer la métadonnée et effectuer le décalage
       fseek(disque, adress.numerodebloc * sizeof(Bloc), SEEK_SET);
       if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
           printf("Erreur : Impossible de lire le bloc %d.\n", adress.numerodebloc);
           return;
       }

      for (int i = adress.index; i < buffer.content.metadataTable.nbrMetadonnees - 1; i++) {
           buffer.content.metadataTable.T[i] = buffer.content.metadataTable.T[i + 1];
       }

      buffer.content.metadataTable.nbrMetadonnees--; // Réduire le nombre de métadonnées

      // Si le dernier bloc devient vide, le libérer
       if (vide) {
           if (blocprecedent != -1) {
               fseek(disque, blocprecedent * sizeof(Bloc), SEEK_SET);
               if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
                  printf("Erreur : Impossible de lire le bloc %d.\n", blocprecedent);
                   return;
               }

              buffer.content.metadataTable.next = -1; // Mettre à jour le pointeur du bloc précédent

              fseek(disque, blocprecedent * sizeof(Bloc), SEEK_SET);
               if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
                   printf("Erreur : Échec de l'écriture dans le bloc %d.\n", blocprecedent);
                   return;
               }
           }

          // Libérer le dernier bloc
           memset(&buffer, 0, sizeof(Bloc));
           buffer.typedebloc = 0; // Bloc inutilisé
           fseek(disque, dernierblocmeta * sizeof(Bloc), SEEK_SET);
           if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : Échec de l'écriture dans le bloc %d.\n", dernierblocmeta);
               return;
           }

          metajourtableallocation(disque, dernierblocmeta, 0); // Libérer le dernier bloc
           mettreAJourNombreBlocs(disque, 1, nbrblocutil - 1);
       } else {
           // Réécrire le bloc avec le décalage
           fseek(disque, adress.numerodebloc * sizeof(Bloc), SEEK_SET);
           if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : Échec de l'écriture dans le bloc %d.\n", adress.numerodebloc);
               return;
           }

       }

      // Libérer les blocs de données du fichier
       int blocActuel = adrpremierbloc;
       while (blocActuel != -1) {
           fseek(disque, blocActuel * sizeof(Bloc), SEEK_SET);
           if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : Impossible de lire le bloc %d.\n", blocActuel);
               return;
           }

          int blocSuivant = buffer.content.fileData.next; // Adresse du bloc suivant
           memset(&buffer, 0, sizeof(Bloc));               // Vider le contenu du bloc
           buffer.typedebloc = 0;                          // Bloc inutilisé
           fseek(disque, blocActuel * sizeof(Bloc), SEEK_SET);
           if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : Échec de l'écriture dans le bloc %d.\n", blocActuel);
               return;
           }

          metajourtableallocation(disque, blocActuel, 0); // Libérer le bloc
           blocActuel = blocSuivant;
       }

      // Mettre à jour le nombre de blocs utilisés
       mettreAJourNombreBlocs(disque, 1, nbrblocutil - nbrbloc);

      printf("Fichier supprimé avec succès.\n");
       return;
   }


  void afficherMS(FILE *disque) {
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



/*----------------------------LO_F----------------------------------------------------------------------------*/



void creationL_OF(FILE *disque) {
    Bloc buffer;
    int ptDataBlock = -1;
    int i = 0;
    int metadataFound = 0;
    int previousBlock = -1;

    // Get file metadata (e.g., file size, name, etc.)
    fichiermetadonnes metadonnes;
    printf("Donner le nom du fichier : \n");
    scanf("%19s", metadonnes.Nomdufichier);

    printf("Donner la taille de fichier en enregistrements : \n");
    scanf("%d", &metadonnes.Taillefichierenregistrements);

    // Calculate the number of blocks needed
    int taille = metadonnes.Taillefichierenregistrements / 20 + (metadonnes.Taillefichierenregistrements % 20 != 0);
    metadonnes.Taillefichierblocs = taille;
    metadonnes.Taillefichierenregistrements = 0;

    printf("Starting file creation with %d blocks\n", taille);

    // Check if there is enough space
    if (!verifierEspaceSuffisant(disque, taille)) {
        printf("Espace insuffisant.\n");
        return;
    }

    // Find a metadata block with available space
    int metadataBlockIndex = 1;
    while (metadataBlockIndex != -1) {
        fseek(disque, metadataBlockIndex * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        if (buffer.typedebloc != 1) {
            printf("Block at index %d is not a metadata block (type = %d)\n", metadataBlockIndex, buffer.typedebloc);
            break;
        }

        if (buffer.content.metadataTable.nbrMetadonnees < 20) {
            printf("Found metadata block with space at index %d\n", metadataBlockIndex);
            metadataFound = 1;
            break;
        }
        metadataBlockIndex = buffer.content.metadataTable.next;
    }

    if (!metadataFound) {
        printf("No space available in metadata blocks.\n");
        return;
    }

    // Allocate and initialize data blocks
    for (i = 0; i < taille; i++) {
        // Find an empty block for data
        for (int j = 2; j <20; j++) {
            fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
            fread(&buffer, sizeof(Bloc), 1, disque);

            if (buffer.content.allocation.tablelocation[j].etat == 0) {
                ptDataBlock = j;
                printf("Found empty data block at index %d\n", ptDataBlock);
                break;
            }
        }

        if (ptDataBlock == -1) {
            printf("No available blocks in MS.\n");
            return;
        }

        // Initialize data block
        buffer.typedebloc = 2; // Set as data block
        buffer.content.fileData.nbrmaladie = 0;
        buffer.content.fileData.next = -1; // Initially, no next block

        // If this is not the first block, link it to the previous block
        if (previousBlock != -1) {
            fseek(disque, previousBlock * sizeof(Bloc), SEEK_SET);
            fread(&buffer, sizeof(Bloc), 1, disque);
            buffer.content.fileData.next = ptDataBlock; // Link previous block to this block
            fseek(disque, previousBlock * sizeof(Bloc), SEEK_SET);
            fwrite(&buffer, sizeof(Bloc), 1, disque);
        }

        // Write the current block
        fseek(disque, ptDataBlock * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);

        // Update allocation table
        fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        buffer.content.allocation.tablelocation[ptDataBlock].etat = 1;
        buffer.content.allocation.nbrblocutil++;

        fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);

        // Update previous block pointer
        previousBlock = ptDataBlock;
    }

    // Update metadata
    fseek(disque, metadataBlockIndex * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    buffer.content.metadataTable.T[buffer.content.metadataTable.nbrMetadonnees].Adrpremierbloc = previousBlock;
    buffer.content.metadataTable.nbrMetadonnees++;

    fseek(disque, metadataBlockIndex * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    // Add file metadata to the metadata block
    bool succes = ajoutermetadonnes(disque, metadonnes, taille);
    if (succes) {
        printf("Le fichier '%s' a ete cree avec succes.\n", metadonnes.Nomdufichier);
    } else {
        printf("Erreur : espace insuffisant pour creer le fichier '%s'.\n", metadonnes.Nomdufichier);
    }

    printf("File creation completed successfully.\n");
}




void defregmentation(FILE *disque, const char *nomFichier) {
    Bloc buffer;
    int blocactuelle, blocsuivant;
    int taillefichierblocs = 0;
    int totalEnregistrements = 0;

    int debut = liremetadonnes(disque, nomFichier, 3);

    blocactuelle = debut;

    if (debut == -1) {
        printf("Le fichier %s est introuvable.\n", nomFichier);
        return;
    }

    if (!verifierEspaceSuffisant(disque, 0)) {
        printf("Erreur : Bloc d'allocation invalide ou espace insuffisant pour la défragmentation.\n");
        return;
    }

    rewind(disque);

    while (blocactuelle != -1) {
        fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        Bloc newBuffer = {0};
        int count = 0;

        for (int i = 0; i < 20; i++) {
            if (!buffer.content.fileData.T[i].suprimelogiquement) {
                newBuffer.content.fileData.T[count++] = buffer.content.fileData.T[i];
            }
        }

        totalEnregistrements += count;

        newBuffer.content.fileData.nbrmaladie = count;

        blocsuivant = buffer.content.fileData.next;
        if (count > 0) {
            taillefichierblocs++;

            newBuffer.content.fileData.next = blocsuivant;

            fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
            fwrite(&newBuffer, sizeof(Bloc), 1, disque);
        } else {
            metajourtableallocation(disque, blocactuelle, 0);
        }

        blocactuelle = blocsuivant;
    }

    miseAJourMetadonnees(disque, nomFichier, 1, taillefichierblocs);
    miseAJourMetadonnees(disque, nomFichier, 2, totalEnregistrements);

    printf("La défragmentation a été réalisée avec succès.\n");
}




maladie insertHelper() {
    maladie m;

    printf("Enter the new information:\n");

    printf("ID: ");
    scanf("%d", &m.id);

    printf("Name: ");
    scanf(" %[^\n]", m.name);

    printf("Age: ");
    scanf("%d", &m.age);

    printf("Sexe: ");
    scanf(" %[^\n]", m.sexe);

    printf("Adresse: ");
    scanf(" %[^\n]", m.adresse);

    printf("Number of Visits: ");
    scanf("%d", &m.nmbrdevisite);

    m.suprimelogiquement = false;

    return m;
}

void insertDis(FILE *disque, const char* nomFichier) {
    Bloc buffer, prevBuffer;
    int lock;
    int i;
    int lastBlock = -1;
    maladie m;
    int nbrbloc=obtenirNombreBlocs(disque,2);

    m = insertHelper();

    if (!verifierEspaceSuffisant(disque, 1)) {
        printf("Echec : Espace insuffisant pour insérer le record.\n");
        return;
    }

    lock = liremetadonnes(disque, nomFichier, 3);

    for (i = lock; i < nbrbloc; i++) {
        fseek(disque, i * sizeof(Bloc), SEEK_SET);
        fread(&prevBuffer, sizeof(Bloc), 1, disque);

        if (prevBuffer.content.fileData.next == -1) {
            lastBlock = i;
            break;
        }
    }

    fseek(disque, lastBlock * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    if (buffer.content.fileData.nbrmaladie < 20) {
        buffer.content.fileData.T[buffer.content.fileData.nbrmaladie] = m;
        buffer.content.fileData.nbrmaladie++;

        fseek(disque, lastBlock * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);

        printf("Le record a ete insere avec succes dans le bloc %d.\n", lastBlock);

        int nbrBlocsUtilises = obtenirNombreBlocs(disque, 1);
        mettreAJourNombreBlocs(disque, 1, nbrBlocsUtilises + 1);

        return;
    }

    int newBlock = -1;
    for (i = 2; i < nbrbloc; i++) {
        Bloc buffer;
        fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        if (buffer.content.allocation.tablelocation[i].etat == 0) {
            newBlock = i;
            break;
        }
    }

    if (newBlock == -1) {
        printf("Aucun espace disponible pour insérer le nouveau record.\n");
        return;
    }

    buffer.content.fileData.next = newBlock;
    fseek(disque, lastBlock * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    metajourtableallocation(disque, newBlock, 1);

    buffer.content.fileData.nbrmaladie = 0;
    buffer.content.fileData.next = -1;
    buffer.content.fileData.T[buffer.content.fileData.nbrmaladie] = m;
    buffer.content.fileData.nbrmaladie++;

    fseek(disque, newBlock * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    printf("Le record a été inséré avec succès dans un nouveau bloc %d.\n", newBlock);

    int nbrBlocsUtilises = obtenirNombreBlocs(disque, 1);
    mettreAJourNombreBlocs(disque, 1, nbrBlocsUtilises + 1);
}





position researchDis(FILE *disque, int searchId, const char* nomFichier) {
    Bloc buffer;
    position res = {-1, -1};

    int numBloc = liremetadonnes(disque, nomFichier, 3);

    if (!verifierEspaceSuffisant(disque, 0)) {
        printf("Erreur : Bloc d'allocation invalide ou espace insuffisant.\n");
        return res;
    }

    while (numBloc != -1) {
        rewind(disque);
        fseek(disque, numBloc * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        if (buffer.typedebloc == 2) {
            for (int j = 0; j < buffer.content.fileData.nbrmaladie; j++) {
                if (buffer.content.fileData.T[j].id == searchId) {
                    res.deplacement = j;
                    res.numBloc = numBloc;
                    return res;
                }
            }
        }

        numBloc = buffer.content.fileData.next;
    }

    return res;
}






// nouuuuuuuuuuuuuuuuuuuuuuuuuuuur




bool deleteL_OF(FILE*disque,const char*nomFichier){
    adressemetadonnes adresse = recherchemetadonnes(disque, nomFichier);
    if (adresse.numerodebloc == -1) {
        printf("Erreur: Fichier introuvable.\n");
        return false;
    }

    Bloc buffer;
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    int currentBlock = buffer.content.metadataTable.T[adresse.index].Adrpremierbloc;

    while (currentBlock != -1) {
        fseek(disque, currentBlock * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);
        int nextBlock = buffer.content.fileData.next;

        metajourtableallocation(disque, currentBlock, 0);

        currentBlock = nextBlock;
    }

    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    for (int i = adresse.index; i < buffer.content.metadataTable.nbrMetadonnees - 1; i++) {
        buffer.content.metadataTable.T[i] = buffer.content.metadataTable.T[i + 1];
    }
    buffer.content.metadataTable.nbrMetadonnees--;

    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    int blocCount = obtenirNombreBlocs(disque, 1);
    mettreAJourNombreBlocs(disque, 1, blocCount - 1);

    printf("Fichier '%s' supprimé avec succès.\n", nomFichier);
    return true;

}




void suppLogique(FILE *disque, int searchId, const char *nomFichier) {
    position res = researchDis(disque, searchId, nomFichier);
    if (res.deplacement != -1) {
        Bloc buffer;
        fseek(disque, res.numBloc * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        buffer.content.fileData.T[res.deplacement].suprimelogiquement = true;

        rewind(disque);
        fseek(disque, res.numBloc * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);
    } else {
        printf("\nTHIS ID DOESN'T EXIST !! no need for this operation ");
    }
}



void suppPhysique(FILE *disque, const char *nomFichier) {
    defregmentation(disque, nomFichier);
}


//------------------------------------------- Fonction de compactage du fichier en réorganisant les blocs.-------------------------------------------------------------------

void compactage(FILE*disque) {

    printf("Compactage terminé. Nombre de blocs utilisés après compactage : %d\n" );
}





//------------------------------------------- Fonction qui vérifie s'il y a de l'espace contigu dans la mémoire centrale--------------------------------------




// Fonction de compactage proposée si l'espace est insuffisant
void proposerCompactage(FILE*disque) {
    printf("Espace insuffisant.  compactage en courants ...\n");

    // Appel de la fonction de compactage pour réorganiser les blocs et récupérer de l'espace libre
    compactage(disque);
}

// Fonction principale de gestion de l'espace avant une opération (création ou insertion)
void gererEspace(FILE*disque,int nbrblocrequise) {
    // Vérifier si l'espace libre est suffisant
    if (verifierEspaceSuffisant(disque,nbrblocrequise)) {
        printf("Espace suffisant pour effectuer l'opération.\n");
        return;  // L'espace est suffisant, on peut continuer l'opération
    }

    // Si l'espace est insuffisant, proposer un compactage
    proposerCompactage(disque);




    // Si l'espace reste insuffisant après compactage
    printf("Erreur : Espace insuffisant même après compactage. La mémoire secondaire est pleine.\n");

}

void afficherEtatMs(FILE*disque){
}




//--------le mennu interactif(le programme principale ----------------------------------------------------------------------------------------------------------------------------
int  main (){
    int modeoI,modeoG ;
    int choix;
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
    printf ("give the global organisation of ur file :\n");//demander le mode org globale
    scanf("%d",&modeoG);
     printf ("give the internel organisation of ur file : \n");//demander le mode d organisation interne
    scanf ("%d",&modeoI);
   //un switch pour le choix de l organisation
   if (modeoG == 0 && modeoI == 0) { // Chaîne Non Ordonnée
    do {

        printf("choisisser l'opeartion ");

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



 switch(choix) {
            case 1:
                InitMs(disque, 20);
                 printf("Memoire secondaire initialise avec succes.\n");
                break;
            case 2:
                printf("Création d'un fichier\n");
                // Créer le fichier en fonction des choix d'organisation

                chargerfichier(disque);

                break;
            case 3:
                // Afficher l'état de la mémoire secondaire
                afficherEtatMs(disque);
                break;
            case 4:
                printf("Afficher les détails de la mémoire secondaire :\n");
                 afficherMS(disque);
                break;
            case 5:
                printf("Recherche d'enregistrement\n");
                printf("Donnez le nom de fichier  ");
                scanf("%s",&nomFichier);
                printf("Donner id pour la recherche");
                scanf("%d",&ID);
                rechercheenregistrement(disque,nomFichier,ID);


                break;
            case 6:
                printf("Insertion d'enregistrement\n");
                printf("Donnez le nom de fichier  ");
                scanf("%s",&nomFichier);

                //  le fichier en fonction des choix d'organisation
                insertionenregistrement(disque,nomFichier);

                break;
            case 7:
                printf("Suppression d'enregistrement phisiqument \n");
                printf("Votre nom de fichier: ");
                scanf("%d", &nomFichier);
                // le fichier en fonction des choix d'organisation
                printf("Entrez l'ID de l'enregistrement a supprimer : ");
                scanf("%d", &ID);

                suprimerenregistrementphisique(disque,nomFichier,  ID);
                //suprimerenregistrementphisique(disque,nomFichier, ID);
                afficherEnregistrements(disque, nomFichier);

                break;
            case 8:
                printf("Défragmentation effectuée\n");
                // le fichier en fonction des choix d'organisation
                printf("Donner le nom de fichier ");
                scanf("%s",&nomFichier);
                defragmentationlof(disque,nomFichier);

                break;
            case 9:
                printf("Suppression de fichier\n");
                printf("donner  le nom du fichier");
                scanf("%s",nomFichier);
                // le fichier en fonction des choix d'organisation
                suprimerFCO(disque,nomFichier);

                break;
            case 10:
                printf("Renommage de fichier\n");
                printf("donner  le nom du fichier que  vous voulez change son nom");
                scanf("%s",&ancienNom);
                printf("donner le nouveau nom");
                scanf("%s",&nouveaunom);
                renommerfichierCO(disque, ancienNom, nouveaunom);

                break;
            case 11:
                printf("Compactage de la mémoire secondaire\n");
                // Appeler la fonction de compactage
                compactage( disque);
                break;
            case 12:
                printf("Mémoire secondaire vidée\n");
                ViderMs(disque);
                break;
            case  13:
                printf("suppression logique d'enregistrement");
                printf("Donner le nom de fichier");
                scanf("%s",&nomFichier);
                printf("Donner  le id de votre enregistrement : ");
                scanf("%d", &ID);
                supprimerEnregistrementLogique(disque,nomFichier,  ID);
                afficherEnregistrements(disque,nomFichier);
                 break;

            case 0:
                printf("Programme terminé !\n");
                break;
            default:
                printf("Choix invalide. Veuillez réessayer.\n");
                break;
        }
    } while (choix != 0);
   }
   if (modeoG == 0 &&  modeoI==1) {
    do{

        printf("choisisser l'opeartion ");

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



 switch(choix) {
            case 1:
                printf("Initialisation de la mémoire secondaire\n");
                InitMs(disque, 20);
                break;
            case 2:
                printf("Création d'un fichier\n");
                // Créer le fichier en fonction des choix d'organisation

                creationL_OF(disque);

                break;
            case 3:
                // Afficher l'état de la mémoire secondaire
                afficherEtatMs(disque);
                break;
            case 4:
                printf("Afficher les détails de la mémoire secondaire :\n");
                afficherMS(disque);
                break;
            case 5:
                printf("Recherche d'enregistrement\n");
                printf("Donnez le nom de fichier  ");
                scanf("%s",&nomFichier);
                printf("Donner ID pour la recherche");
                scanf("%d",&ID);
                researchDis(disque,ID,nomFichier);


                break;
            case 6:
                printf("Insertion d'enregistrement\n");
                 printf("Donnez le nom de fichier  ");
                 scanf("%s",&nomFichier);

                //  le fichier en fonction des choix d'organisation
                insertDis(disque,nomFichier);
                break;
            case 7:
                printf("Suppression d'enregistrement\n");
                printf("Votre nom de fichier: ");
                scanf("%d", &nomFichier);
                // le fichier en fonction des choix d'organisation
                //supretio physique
                  suppPhysique(disque,nomFichier);

                break;
            case 8:
                printf("Défragmentation effectuée\n");
                printf("Donner le nom de fichier");
                scanf("%s",&nomFichier);
                 defregmentation(disque,nomFichier);

                break;
            case 9:
                printf("Suppression de fichier\n");
                printf("donner  le nom du fichier");
                scanf("%s",&nomFichier);
                // le fichier en fonction des choix d'organisation
                deleteL_OF(disque,nomFichier);
                break;
            case 10:
                printf("Renommage de fichier\n");
                printf("donner  le nom du fichier que  vous voulez change son nom");
                scanf("%s",&ancienNom);
                printf("donner le nouveau nom");
                scanf
                ("%s",&nouveaunom);
               renommerfichierCO(disque, ancienNom, nouveaunom);

                break;
            case 11:
                printf("Compactage de la mémoire secondaire\n");
                // Appeler la fonction de compactage
                compactage( disque);
                break;
            case 12:
                printf("Mémoire secondaire vidée\n");
                ViderMs(disque);
                break;
            case  13:
                printf("suppression logique d'enregistrement");
               printf("Donner le nom de fichier");
                scanf("%s",&nomFichier);
                printf("Donner  le id de votre enregistrement : ");
                scanf("%d", &ID);
                suppLogique(disque ,ID,nomFichier);


            case 0:
                printf("Programme terminé !\n");
                break;
            default:
                printf("Choix invalide. Veuillez réessayer.\n");
                break;
        }
    } while (choix != 0);
   }

   if(modeoG==1 && modeoI==1){
do{
        printf("choisisser l'opeartion ");

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

// yousra wa9ila
 switch(choix) {
            case 1:
                printf("Initialisation de la mémoire secondaire\n");
                InitMs(disque, 20);
                break;
            case 2:
                printf("Création d'un fichier\n");
                // Créer le fichier en fonction des choix d'organisation

                chargerFileTNOF(disque );

                break;
            case 3:
                // Afficher l'état de la mémoire secondaire
                afficherEtatMs(disque);
                break;
            case 4:
                printf("Afficher les détails de la mémoire secondaire :\n");
                afficherMS(disque);
                break;
            case 5:
                printf("Recherche d'enregistrement\n");
                printf("Donnez le nom de fichier  ");
                scanf("%s",&nomFichier);
                printf("Donner ID pour la recherche");
                scanf("%d",&ID);
                rechercheParID(disque,ID);


                break;
            case 6:
                printf("Insertion d'enregistrement\n");
                printf("Donnez le nom de fichier  ");
                scanf("%s",&nomFichier);

                //  le fichier en fonction des choix d'organisation
                insererEnregistrement(disque,nomFichier);
                break;
            case 7:
                printf("Suppression d'enregistrement\n");
                printf("Votre nom de fichier: ");
                scanf("%d", &nomFichier);
                // le fichier en fonction des choix d'organisation
                //supretio physique

                printf("donner le id");
                scanf("%d",&ID);
                suppressionPhysique(disque,ID);

                break;
            case 8:
                printf("Défragmentation effectuée\n");
                printf("Donner le nom de fichier");
                // le fichier en fonction des choix d'organisation
                scanf("%s",&nomFichier);
                defragmentationyousra(disque,nomFichier);

                break;
            case 9:
                printf("Suppression de fichier\n");
                printf("donner  le nom du fichier");
                scanf("%s",&nomFichier);
                // le fichier en fonction des choix d'organisation
               supprimerFichier(disque,nomFichier);
                break;
            case 10:
                printf("Renommage de fichier\n");
                printf("donner  le nom du fichier que  vous voulez change son nom");
                scanf("%s",&ancienNom);
                printf("donner le nouveau nom");
                scanf("%s",&nomFichier);
                renommerfichierCO(disque, ancienNom, nouveaunom);

                break;
            case 11:
                printf("Compactage de la mémoire secondaire\n");
                // Appeler la fonction de compactage
                compactage(disque);
                break;
            case 12:
                printf("Mémoire secondaire vidée\n");
                ViderMs(disque);
                break;
            case  13:
                printf("suppretion logique d'enregistrement");

                scanf("%s",&nomFichier);

                printf("Donner  le id de votre enregistrement : ");
                scanf("%d", &ID);
                suppressionLogique(disque,ID);


            case 0:
                printf("Programme terminé !\n");
                break;
            default:
                printf("Choix invalide. Veuillez réessayer.\n");
        }
    } while (choix != 0);
   }
if(modeoG==1 && modeoI==0){
    do{

        printf("choisisser l'opeartion ");

        printf("\n--- Gestion de la Mémoire Secondaire ---\n");
        printf("1. Initialiser la mémoire secondaire\n");
        printf("2. Créer un fichier\n");
        printf("3. Afficher l'état de la mémoire secondaire\n");
        printf("4. Afficher les métadonnées des fichiers\n");
        printf("5. Rechercher un enregistrement\n");
        printf("6. Insérer un nouvel enregistrement\n");
        printf("7. Sufpprimer un enregistrement\n");
        printf("8. Défragmenter un fichier\n");
        printf("9. Supprimer un fichier\n");
        printf("10. Renommer un fichier\n");
        printf("11. Compacter la mémoire secondaire\n");
        printf("12. Vider la mémoire secondaire\n");
        printf("13. suppression logique de fichier\n");
        printf("0. Quitter\n");
        printf("Votre choix : ");
        scanf("%d", &choix);



 switch(choix) {
            case 1:
                printf("Initialisation de la mémoire secondaire\n");
                InitMs(disque, 20);
                break;
            case 2:
                printf("Création d'un fichier\n");
                // Créer le fichier en fonction des choix d'organisation

                chargerFileTNOF(disque);

                break;
            case 3:
                // Afficher l'état de la mémoire secondaire
             afficherEtatMs(disque);
                break;
            case 4:
                printf("Afficher les détails de la mémoire secondaire :\n");
                afficherMS(disque);
                break;
            case 5:
                printf("Recherche d'enregistrement\n");
                printf("Donnez le nom de fichier  ");
                scanf("%s",&nomFichier);
                printf("Donner id pour la recherche");
                scanf("%d",ID);
                rechercheFILETNOF(disque ,ID);


                break;
            case 6:
                printf("Insertion d'enregistrement\n");
                printf("Donnez le nom de fichier  ");
                scanf("%s",&nomFichier);

                //  le fichier en fonction des choix d'organisation
                InsertionfileTNOF(disque );
                break;
            case 7:
                printf("Suppression d'enregistrement\n");
                printf("Votre nom de fichier: ");
                scanf("%d", &nomFichier);
                // le fichier en fonction des choix d'organisation
                //supretio physique
                supprPhysiqueFileTNOF(disque ,ID );

                break;
            case 8:
                printf("Défragmentation effectuée\n");
                // le fichier en fonction des choix d'organisation
                scanf("%s",&nomFichier);
                defragmentationFileTNOF(disque,nomFichier);// khawty hdi li drtha lzm nom de fichier kifh diri defragmentation wnty m3ndkch fichier

                break;
            case 9:
                printf("Suppression de fichier\n");
                printf("donner  le nom du fichier");
                scanf("%s",nomFichier);
                // le fichier en fonction des choix d'organisation
                supprPhysiqueFileTNOF(disque ,ID );//supp du phichier yousraaaaa
                break;
            case 10:
                printf("Renommage de fichier\n");
                printf("donner  le nom du fichier que  vous voulez change son nom");
                scanf("%s",&ancienNom);
                printf("donner le nouveau nom");
                scanf("%s",&nouveaunom);
                renommerfichierCO(disque, ancienNom, nouveaunom);

                break;
            case 11:
                printf("Compactage de la mémoire secondaire\n");
                // Appeler la fonction de compactage
                compactage(disque);
                break;
            case 12:
                printf("Mémoire secondaire vidée\n");
                ViderMs(disque);
                break;
            case  13:
                printf("suppretion logique d'enregistrement");

                scanf("%s",&nomFichier);
                printf("Donner  le id de votre enregistrement : ");
                scanf("%d", &ID);
                supprPhysiqueFileTNOF(disque ,ID );


            case 0:
                printf("Programme terminé !\n");
                break;
            default:
                printf("Choix invalide. Veuillez réessayer.\n");
                break;
        }
    } while (choix != 0);

  }
return 0;
}

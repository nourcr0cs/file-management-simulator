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

   // Function to create a new file and its associated metadata in the system.
   char* creerfichierCO(FILE* disque){
        char* nomFichier=(char*)malloc(20*sizeof(char));
        if(!nomFichier){
            printf("Erreur : Allocation de mémoire échouée.\n");
            return NULL;
        }

        fichiermetadonnes metadonnes;
        rewind(disque);

        printf("Donner le nom du fichier : \n");
        scanf("%19s",metadonnes.Nomdufichier);

        printf("Donner la taille de fichier en enregistrement : \n");
        scanf("%d",&metadonnes.Taillefichierenregistrements);

        printf("Donner le mode d'organisation globale : \n");
        scanf("%d",&metadonnes.Modeorganisationglobale);

        printf("Donner le mode d'organisation interne : \n");
        scanf("%d",&metadonnes.Modeorganisationinterne);

        int taille=ceil((double)metadonnes.Taillefichierenregistrements/20)+1;

        metadonnes.Taillefichierblocs=taille;

        if(!verifierEspaceSuffisant(disque ,taille)){
            free(nomFichier);
            return NULL;
        }

        bool succes=ajoutermetadonnes(disque ,metadonnes ,taille);

        if(succes){
            printf("Le fichier '%s' a été créé avec succès.\n", metadonnes.Nomdufichier);
            strcpy(nomFichier ,metadonnes.Nomdufichier);
            return nomFichier;
        } else {
            printf("Erreur : espace insuffisant pour créer le fichier '%s'.\n", metadonnes.Nomdufichier);
            free(nomFichier);
            return NULL;
        }
   }
 // allocation of blocs and do the chainage
   void chargerfichier(FILE* disque) {

       Bloc buffer;
       char* nomFichier;
       rewind(disque);

       // Récupérer les métadonnées du fichier
       fichiermetadonnes metadonnes;
       nomFichier=creerfichierCO(disque);
       adressemetadonnes adresse = recherchemetadonnes(disque, nomFichier);
       int taille=liremetadonnes(disque,nomFichier,1);

       if (adresse.numerodebloc == -1) {
           printf("Fichier introuvable \n");
           return;  // Le fichier n'a pas été trouvé
       }
        printf("Chargement des métadonnées du fichier : %s\n", nomFichier);
       // Charger les métadonnées dans le buffer
       fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
       fread(&buffer, sizeof(Bloc), 1, disque);

       if (buffer.typedebloc != 1) {
           printf("Le bloc ne contient pas de métadonnées.\n");
           return;
       }

       metadonnes = buffer.content.metadataTable.T[adresse.index];

       int blocnecessaire = metadonnes.Taillefichierblocs;
       int bloctrouve = 0; // Pour compter les blocs trouvés
       int adrPremierBloc = -1; // Variable pour stocker l'adresse du premier bloc
       int nbrbloctotal=obtenirNombreBlocs(disque,2);
       int nbrblocutiliser=obtenirNombreBlocs(disque,1);
       // Lire la table d'allocation (stockée dans le premier bloc)
       fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
       fread(&buffer, sizeof(Bloc), 1, disque);

       if (buffer.typedebloc != 3) {
           printf("Le bloc d'allocation n'est pas valide.\n");
           return;
       }

       // Allouer les blocs nécessaires
       for (int i = 0; i < nbrbloctotal && bloctrouve < blocnecessaire; i++) {
       fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
       fread(&buffer, sizeof(Bloc), 1, disque);

       if (buffer.content.allocation.tablelocation[i].etat==0) {  // Si le bloc est vide
               metajourtableallocation(disque, i, 1);  // Marquer le bloc comme utilisé

               // Enregistrer l'adresse du premier bloc alloué
               if (adrPremierBloc == -1) {
                   adrPremierBloc = i;
               }else{
               // Chaînage des blocs : mise à jour du champ "next" du bloc précédent

                   fseek(disque, adrPremierBloc * sizeof(Bloc), SEEK_SET);
                   fread(&buffer, sizeof(Bloc), 1, disque);
                   buffer.content.fileData.next = i;
                   fseek(disque, adrPremierBloc * sizeof(Bloc), SEEK_SET);
                   fwrite(&buffer, sizeof(Bloc), 1, disque);
               }

               // Enregistrer l'adresse du bloc actuel pour le chaînage futur
               adrPremierBloc = i;
               bloctrouve++;
           }
       }

       // Si les blocs nécessaires n'ont pas été trouvés

       if (bloctrouve < blocnecessaire) {
           printf("Espace insuffisant pour allouer tous les blocs nécessaires.\n");
           return;

           // Libération des blocs déjà alloués
           for (int i = 0; i < nbrbloctotal; i++) {
               if (buffer.content.allocation.tablelocation[i].etat == 1) {
                   metajourtableallocation(disque, i, 0);
                   printf("Bloc %d libéré.\n", i);
               }
           }
           return;
       }

       // Mise à jour des métadonnées
       miseAJourMetadonnees(disque, nomFichier, 3, adrPremierBloc);
       mettreAJourNombreBlocs(disque,1,nbrblocutiliser+taille);

       printf("Fichier chargé avec succès.\n");
   }
// Recover spaced used by enregistrement deleted
   void defragmentation(FILE *disque, const char *nomFichier) {
       Bloc buffer;             // buffer pour charger les blocs
       maladie temp[20];        // tableau temporaire pour réorganiser les enregistrements
       int blocactuelle, blocsuivant; // pointeurs pour le bloc courant et suivant
       int indexTemp = 0;       // indice pour remplir le tableau temporaire
       int totalEnregistrements = 0; // compteur pour les enregistrements valides
       int taillefichierblocs;  // nombre de blocs utilisés après défragmentation
       rewind(disque);

       // obtenir le premier bloc et le nombre total d'enregistrements
       int debut = liremetadonnes(disque, nomFichier, 3);
       if (debut == -1) {
           printf("Erreur : Le fichier %s est introuvable.\n", nomFichier);
           return;
       }

       totalEnregistrements = liremetadonnes(disque, nomFichier, 2); // nombre total d'enregistrements
       if (totalEnregistrements == 0) {
           printf("Le fichier %s est vide. Aucune défragmentation nécessaire.\n", nomFichier);
           return;
       }

       // parcourir tous les blocs pour collecter les enregistrements valides
       blocactuelle = debut;
       while (blocactuelle != -1) {
          // charger le bloc courant
           fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
           fread(&buffer, sizeof(Bloc), 1, disque);

           // collecter les enregistrements valides dans le tableau temporaire
           for (int i = 0; i < buffer.content.fileData.nbrmaladie; i++) {
               if (!buffer.content.fileData.T[i].suprimelogiquement) {
                   temp[indexTemp++] = buffer.content.fileData.T[i];
               }
           }

           blocactuelle = buffer.content.fileData.next; // passer au bloc suivant
       }

       // calculer le nombre de blocs nécessaires
       taillefichierblocs = (indexTemp + 20 - 1) / 20;

       // écriture des blocs avec les enregistrements valides
       blocactuelle = debut;
       indexTemp = 0;
       for (int i = 0; i < taillefichierblocs; i++) {
           // charger ou initialiser un bloc existant
           fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
           fread(&buffer, sizeof(Bloc), 1, disque);

           // remplir le bloc avec les enregistrements valides
           int nbrEnregistrements = 0;
           while (indexTemp < totalEnregistrements && nbrEnregistrements < 20) {
               buffer.content.fileData.T[nbrEnregistrements++] = temp[indexTemp++];
           }
           buffer.content.fileData.nbrmaladie = nbrEnregistrements;

           // gestion du chaînage
           if (i == taillefichierblocs - 1) {
               buffer.content.fileData.next = -1; // dernier bloc
           } else {
               blocsuivant = buffer.content.fileData.next; // conserver le chaînage
           }

           // ecrire le bloc mis à jour
           fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
           fwrite(&buffer, sizeof(Bloc), 1, disque);

           // passer au bloc suivant
           blocactuelle = buffer.content.fileData.next;
       }

       // libérer les blocs inutilisés dans la table d'allocation
       while (blocactuelle != -1) {
           fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
           fread(&buffer, sizeof(Bloc), 1, disque);

         metajourtableallocation(disque, blocactuelle, 0); // libérer le bloc
           blocactuelle = buffer.content.fileData.next;
       }

       // mettre à jour les métadonnées
       miseAJourMetadonnees(disque, nomFichier, 1, taillefichierblocs); // taille en blocs
       miseAJourMetadonnees(disque, nomFichier, 2, totalEnregistrements); // nombre d'enregistrements

       printf("Défragmentation du fichier %s terminée avec succès.\n", nomFichier);
       return;
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


   adressemetadonnes adresse = recherchemetadonnes(disque, nomFichier);

   int debut=liremetadonnes(disque,nomFichier,3);// adresse de 1 bloc

   int nbrenregistrement=liremetadonnes(disque,nomFichier,2);// nombre total de enregistrement dans ce bloc

   int nbrbloc=liremetadonnes(disque,nomFichier,1);// nombre total de bloc

   int nbrbloctotal=obtenirNombreBlocs(disque,2);

   int nombrblocutil=obtenirNombreBlocs(disque,1);

   if (adresse.numerodebloc == -1) {
           printf("Fichier introuvable : %s\n", nomFichier);
           return;
       }



   defragmentation(disque,nomFichier);// pour recuperer les espaces inutiliser

   // mtnsaych compactage

   // verifier si il ya un espace pour un enregistrement  sinon on alouer un nouveaux bloc


   bool allouer = (nbrenregistrement % 20 == 0);

   // verifier si il ya un decalge (insertion dans 1 bloc

   blocdernier = debut;

   int maxIterations = 20; // Prevent infinite loops
   int iterations = 0;

   while (buffer.content.fileData.next != -1) {
       if (++iterations > maxIterations) {
           printf("Erreur : Boucle infinie détectée dans le chaînage.\n");
           break;
       }
       fseek(disque, blocdernier * sizeof(Bloc), SEEK_SET);
       if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
           printf("Erreur : Impossible de lire le bloc.\n");
           break;
       }
       blocdernier = buffer.content.fileData.next;
   }


   // lire le information de la  nouvelle maladie

   printf("ID : \n");
   scanf("%s",& m.id);

   printf("AGE : \n");
   scanf("%s",& m.age);

   printf("SEXE : \n");
   scanf("%s",& m.sexe);

   printf("ADRESSE : \n");
   scanf("%s",& m.adresse);

   m.nmbrdevisite=1;
   m.suprimelogiquement = 0;

   //  cas 1 :si l'enregistrement pour inserer est le  1 enregistrement dans le fichier(fichier vide

   if(nbrenregistrement==0)  {
   printf("Insertion dans un fichier vide.\n");
   fseek(disque, debut * sizeof(Bloc), SEEK_SET);
   fread(&buffer, sizeof(Bloc), 1, disque);


   buffer.typedebloc=2;
   buffer.content.fileData.T[0]=m;
   buffer.content.fileData.nbrmaladie = 1;
   buffer.content.fileData.next = -1;


   // ecrire les changement

   fseek(disque, debut * sizeof(Bloc), SEEK_SET);
   fwrite(&buffer, sizeof(Bloc), 1, disque);

   // mise a jour les metadonnes

   miseAJourMetadonnees(disque, nomFichier, 2,nbrenregistrement++);

   return ;
   }

   // Chercher la position d'insertion

   blocactuelle=debut;// pour passer a bloc suivant

   position=-1;//pour insertion a la fin si en ignore les cas de decalage

   bool positiontrouve=false;

   // parcourir les bloc

   while( (blocactuelle!=-1)  &&  (!positiontrouve) ){ // boucle pour chercher la position

   fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
   fread(&buffer, sizeof(Bloc), 1, disque);

   for(int i=0;i<buffer.content.fileData.nbrmaladie-1;i++){

   // comparer les id pour trouver la position

   if(m.id<buffer.content.fileData.T[i].id){

      position=i;
      positiontrouve=true;
      decalage = true; // Décalage nécessaire si l'insertion n'est pas à la fin

      break;

   }
   }

   if (!positiontrouve){
           blocactuelle=buffer.content.fileData.next;}
   }

   //verifier si adresse de l'enregistrement est dans le bloc derniere

   if(blocdernier==blocactuelle)
   {
       decalage=false;

   }
   else{decalage=true;}

   // cas sans decalge sans allocation
   if (!decalage && !allouer) {

            printf("Insertion dans un bloc existant sans décalage ni allocation.\n");

           fseek(disque, blocdernier * sizeof(Bloc), SEEK_SET);
           fread(&buffer, sizeof(Bloc), 1, disque);

           for (int j = buffer.content.fileData.nbrmaladie; j > position; j--) {
               buffer.content.fileData.T[j] = buffer.content.fileData.T[j - 1]; // Décalage
           }

           buffer.content.fileData.T[position] = m;
           buffer.content.fileData.nbrmaladie++;

           // Écrire les changements
           fseek(disque, blocdernier * sizeof(Bloc), SEEK_SET);
           fwrite(&buffer, sizeof(Bloc), 1, disque);

           printf("Insertion terminée avec succès.\n");

           // Mise à jour des métadonnées
           miseAJourMetadonnees(disque, nomFichier, 2, nbrenregistrement + 1);

           return;
       }

   // cas insertion  (dans le derniere bloc ) est avec allocation

   if(allouer&&!decalage){

       printf("insertion dans le derniere bloc  sans allocation d'un nouveau bloc.\n");

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
               printf("Espace insuffisant pour insérer un nouvel enregistrement.\n");
               return;
           }
           buffer.typedebloc=2;
           // Charger le dernier bloc pour récupérer l'enregistrement à décaler
           fseek(disque, blocdernier * sizeof(Bloc), SEEK_SET);
           fread(&buffer, sizeof(Bloc), 1, disque);

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

           // Allouer un nouveau bloc pour le décalage
           fseek(disque, place * sizeof(Bloc), SEEK_SET);
           fread(&buffer, sizeof(Bloc), 1, disque);
   //verifier si le eng que en va inserer et superieur au tous les eng alors il va inserer dans le bloc allouer sinon le derniere eng de dernier bloc il va sauter


          if(enrdecale.id>=m.id) {
           buffer.content.fileData.T[0] = enrdecale;

          } else{buffer.content.fileData.T[0]=m;}
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

           printf("Décalage avec allocation effectué avec succès.\n");
           return;
       }



   // cas avec decalage l'eng n'est pas dans le derniere bloc

   if(decalage){
   printf("Décalage intra-bloc sans allocation.\n");
   buffer.typedebloc=2;
   fseek(disque,blocactuelle* sizeof(Bloc), SEEK_SET); // charger le bloc suivant pour faire le decalage
   fread(&buffer, sizeof(Bloc), 1, disque);

   // buffer est chargé le bloc ou en va inserer

   enrdecale=buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1];// l'enregistrement qui va decalé vers le bloc suivant est aussi le derniere enregistrement dans le bloc

   //decaler les enregistrement dans  bloc ou en a trouver la position

   for(int j=20-1;j>position;j--){ // fb-1 car le 1 index array est 0


   buffer.content.fileData.T[j] = buffer.content.fileData.T[j - 1]; // pour vider l'espace de position

   }

   buffer.content.fileData.T[position]=m;

   miseAJourMetadonnees( disque,nomFichier, 2, nbrenregistrement++);

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

   if (verifierEspaceSuffisant(disque,1)) { // Pas de blocs libres

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

  }
   }
   return; // sortir car l 'insertion est faite

  }


  adressemetadonnes rechercheebregistrement(FILE*disque,const char*nomFichier,int ID){

  adressemetadonnes adressetrouve= {-1, -1} ;//adress d'enregistrement trouvé

  Bloc buffer;

  buffer.typedebloc=2;

  int blocactuelle;// pour parcourir les blocs

  bool find=false;


  adressemetadonnes adress=recherchemetadonnes(disque,nomFichier);

    // Vérifiez si le fichier existe
       if (adress.numerodebloc == -1) {
           printf("Fichier introuvable : %s\n", nomFichier);
           return adressetrouve;
       }

  blocactuelle=liremetadonnes(disque,nomFichier,3);// adresse de 1 bloc

  int nbrenregistrement=liremetadonnes(disque,nomFichier,2);// nombre total de enregistrement dans ce fichier

  int nbrbloc=liremetadonnes(disque,nomFichier,1);// nombre total de bloc

  // Parcourir les blocs
       while (blocactuelle != -1) {
           fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
           fread(&buffer, sizeof(Bloc), 1, disque);

          // Vérifiez si le bloc est de type fichier (type 2)
           if (buffer.typedebloc != 2) {
               printf("Erreur : Bloc inattendu (type %d).\n", buffer.typedebloc);
               return adressetrouve;
           }

  for(int i=blocactuelle;i<nbrbloc;i++){
      fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);// charger le bloc pur lire le tableau
      fread(&buffer, sizeof(Bloc), 1, disque);

     for(int j=0;j<buffer.content.fileData.nbrmaladie;j++) {

         if (buffer.content.fileData.T[j].id==ID){

                adressetrouve.numerodebloc=i;
                adressetrouve.index=j;
                find=true;
                return adressetrouve;

         }

     }

     blocactuelle=buffer.content.fileData.next;


  }
   }

  if(find==false){
       printf("l’enregistrement recherché n’existe pas.");
   }

  return adressetrouve;
   }


  void  suprimerenregistrement(FILE*disque,const char*nomFichier,int ID) {

       Bloc buffer;
        rewind(disque);
       int choixSuppression = 0;
       int dernierbloc;
       bool decalage=false;// pour voir si il ya un decalage des bloc
       bool blocvide=false;//si apres decalage le dernier bloc est vide
       int blocactuelle=-1;
       int blocprecedent=-1;//pour la supression avec decalge pour revenir dans le decalage
       maladie m;// pour faire engistré les enregistrement qui va changer apres decalage

      adressemetadonnes adresse=rechercheebregistrement(disque,nomFichier,ID);

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


    // Obtenir le choix de suppression
       do {
           printf("Quel type de suppression voulez-vous effectuer ?\n");
           printf("1. Suppression logique\n");
           printf("2. Suppression physique\n");
           printf("Entrez votre choix (1 ou 2) : ");
           if (scanf("%d", &choixSuppression) != 1 || (choixSuppression != 1 && choixSuppression != 2)) {
               printf("Choix invalide. Veuillez entrer 1 ou 2.\n");
               while (getchar() != '\n'); // Vider le buffer en cas d'entrée non valide
           }
       } while (choixSuppression != 1 && choixSuppression != 2);

      // confirmé le choix
       if (choixSuppression == 1) {
          printf("Vous avez choisi la suppression logique.\n");
       } else if (choixSuppression == 2) {
           printf("Vous avez choisi la suppression physique.\n");
       }

  if(choixSuppression==1)
   {
   printf("Vous avez choisi la suppression logique.\n");
   fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);// charger le 1 bloc pour chercher l'adresse de derniere bloc
   fread(&buffer, sizeof(Bloc), 1, disque);

  buffer.content.fileData.T[adresse.index].suprimelogiquement=1;

  fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);// engistré les changement
   fwrite(&buffer, sizeof(Bloc), 1, disque);

  printf("Suppression logique effectuée pour l'enregistrement ID %d.\n", ID);
           return;

  }

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



  int main() {
       int choix;
       int modeG, modeI;
       char nomFichier[20], ancienNom[20], nouveaunom[20];
       int ID;

     printf("Program started successfully!\n");

      FILE* disque = fopen("disque.bin", "r+b"); // Ouvrir le fichier pour lecture/écriture
       if (!disque) {
           disque = fopen("disque.bin", "w+b"); // Créer le fichier s'il n'existe pas
           if (!disque) {
               printf("Erreur : Impossible de créer le fichier disque.bin.\n");
               return 1;
           }
       }

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

          switch (choix) {
               case 1:
                   InitMs(disque, 20);
                   printf("Mémoire secondaire initialisée avec succès.\n");
                   break;

              case 2:
                   printf("Création d'un fichier\n");
                   printf("Votre choix d'organisation globale : ");
                   scanf("%d", &modeG);
                   printf("Votre choix d'organisation interne : ");
                   scanf("%d", &modeI);
                   chargerfichier(disque);
                   break;

              case 3:
                   printf("Affichage de l'état de la mémoire secondaire :\n");
                   afficherMS(disque);
                   break;

              case 4:
                  printf("Affichage des métadonnées des fichiers :\n");
                   // Ajouter une fonction pour afficher uniquement les métadonnées
                   break;

              case 5:
                   printf("Recherche d'enregistrement\n");
                   printf("Entrez le nom du fichier : ");
                   scanf("%s", nomFichier);
                   printf("Entrez l'ID de l'enregistrement à rechercher : ");
                   scanf("%d", &ID);
                   adressemetadonnes resultat = rechercheebregistrement(disque, nomFichier, ID);
                   if (resultat.numerodebloc == -1) {
                       printf("Enregistrement introuvable.\n");
                   } else {
                       printf("Enregistrement trouvé dans le bloc %d, index %d.\n",
                              resultat.numerodebloc, resultat.index);
                   }
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
                   printf("Entrez l'ID de l'enregistrement à supprimer : ");
                   scanf("%d", &ID);
                   suprimerenregistrement(disque, nomFichier, ID);
                   break;

              case 8:
                   printf("Défragmentation d'un fichier\n");
                   printf("Entrez le nom du fichier à défragmenter : ");
                   scanf("%s", nomFichier);
                   // Appeler la fonction de défragmentation ici
                   break;

              case 9:
                   printf("Suppression de fichier\n");
                   printf("Entrez le nom du fichier à supprimer : ");
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
                   printf("Compactage de la mémoire secondaire\n");
                   // Appeler la fonction de compactage ici
                   break;

              case 12:
                   printf("Vidage de la mémoire secondaire\n");
                   // Appeler la fonction de vidage ici
                   ViderMs(disque);
                   break;

              case 0:
                   printf("Programme terminé !\n");
                  break;

              default:
                   printf("Choix invalide. Veuillez réessayer.\n");
                  break;
           }
     } while (choix != 0);
     fclose(disque);
      return 0;
  }

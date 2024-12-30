#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <locale.h>


#define FB 10 
#define MAX_BLOCKS 50

typedef struct {
    char Nomdufichier[20];
    int Taillefichierblocs;
    int Taillefichierenregistrements;
    int Adrpremierbloc;  //Address of the pt block
    int Modeorganisationglobale; //0 for chained
    int Modeorganisationinterne;  //0 for ordered
} fichiermetadonnees;

typedef struct {
    int id;
    char name[15];
    int age;
    char sexe[10];
    char adresse[30];
    int nmbrdevisite;
    bool suprimelogiqument;
} maladie;

//linked list
typedef struct {
    maladie T[FB];
    int nbrmaladie;
    int next;
} BlocData;

typedef struct {
    int adrdebloc; //Address of the block
    int etat;      //0 = empty, 1 = full
} Tableallocation;

typedef struct {
    int nbrblocutil; //Number of blocks in use
    int nbrbloc;     //Total number of blocks
    Tableallocation tablelocation[30];
    int flg;
} MS;

//to locate which bloc and which record that represent the file infos
typedef struct {
    int numerodebloc;
    int index;
} adressemetadonnees;


typedef struct {
    fichiermetadonnees T[FB]; // Tableau de métadonnées
    int nbrMetadonnees;       // Nombre actuel de métadonnées dans ce bloc
    int next;
} BlocMetadonnees;

//union to store different types of content in a block 
//this is the main idea to minimize the storage gap
typedef union {
    BlocMetadonnees metadataTable;   //a metadataTable 
    BlocData fileData;            //the main file (file data/maladie records)
    Tableallocation allocation;   
} BlockContent;

//the whole block structure containing the union and block type (to access it directly)
typedef struct {
    BlockContent content;  //The union storing block content
    int typedebloc;        // 1 = metadataTable, 2 = file data, 3 = allocation
} Bloc;

//for research 
typedef struct {
    int numBloc;
    int deplacement;
} position;


void initMS(MS *ms, int nbrbloc) {
    ms->nbrbloc = nbrbloc;
    ms->nbrblocutil = 0;
    for (int i = 0; i < nbrbloc; i++) {
        ms->tablelocation[i].etat = 0; //All blocks initially empty
        ms->tablelocation[i].adrdebloc = -1; //No address assigned
    }
}


void initMetadonnees(FILE *disque, int i) {
    BlocMetadonnees metadataTable;
    Bloc buffer;
    //i is for num of record
    //infos that the user should enter
    printf("Enter the file name: ");
    scanf("%s", metadataTable.T[i].Nomdufichier);

    printf("Enter the global organization mode (0 for chained, 1 for contigue): ");
    scanf("%s", metadataTable.T[i].Modeorganisationglobale);

    printf("Do you want it to be ordored or non-ordored (0 for ordored, 1 for non-ordored): ");
    scanf("%s", metadataTable.T[i].Modeorganisationinterne);

    printf("Enter the number of records in total: ");
    scanf("%s", metadataTable.T[i].Taillefichierenregistrements);


    metadataTable.T[i].Adrpremierbloc = -1;

    //to copy directly an entire bloc = fread
    memcpy(&buffer.content.metadataTable.T[i], &metadataTable, sizeof(metadataTable));
    buffer.content.metadataTable.nbrMetadonnees++;
    fseek(disque, sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    printf("metadataTable initialized successfully.\n");
}


void initTableAllocation(MS* ms) {
    //first bloc (index 0) tae tableAlloc
    ms->tablelocation[0].adrdebloc = 0;
    ms->tablelocation[0].etat = 1;


    for (int i = 1; i < ms->nbrbloc; i++) {
        ms->tablelocation[i].adrdebloc = i;  
        ms->tablelocation[i].etat = 0;       
    }

    
}

void MAJtaballocation(MS *ms, int index, int etat) {
   
    if (index < ms->nbrbloc){
    ms->tablelocation[index].etat = etat; 
    ms->tablelocation[index].adrdebloc = index;
        if (etat == 0) {
            ms->nbrblocutil--; 
        }else {
            ms->nbrblocutil++;
        }
    }else {
        printf("MS is full !");
        return;
    }
    
}



void creationL_OF(FILE *disque, MS *ms, int nbrbloc) {
    Bloc buffer;
    int ptDataBlock = -1;
    int i = 0;
    int metadataFound = 0;

         //i asssumed the bloc with index 1 is for metadata 
        int metadataBlockIndex = 1;
        while (metadataBlockIndex != -1) {
        rewind(disque);
        fseek(disque, metadataBlockIndex * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        //we search if there is still some space in the first block to insert the new metaData
        if (buffer.content.metadataTable.nbrMetadonnees < FB) {
            initMetadonnees(disque, buffer.content.metadataTable.nbrMetadonnees);
            metadataFound = 1;
            break;
        }
        metadataBlockIndex = buffer.content.metadataTable.next;
    }

    // If no space in metadata, print error
    // but we may search later, if we have time for new blocks for metaData
    if (!metadataFound) {
        printf("No space available in metadata blocks.\n");
        return;
    }

    //skipping the pt two wla three (alloc + metadataTable), pour l'instant rni dyra 3 (whda alloc, zodj metadata)
    for ( i = 3; i < ms->nbrbloc; i++) {
        if (ms->tablelocation[i].etat == 0) {
            ptDataBlock = i;
            break;
        }
    }

    if (ptDataBlock == -1) {
        printf("No available blocks in MS.\n");
        return ;
    }

    //init (capable ndirha f fonction whdha)
    buffer.content.fileData.nbrmaladie = 0;
    buffer.content.fileData.next = -1;
    fseek(disque, ptDataBlock * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
    ms->tablelocation[ptDataBlock].etat = 1;



    //Update the metadataTable block with the new data block address (aussi capable fonc whdha )
    Bloc metadataTable;
    fseek(disque, sizeof(Bloc), SEEK_SET); // we assume metadataTable is in block index 1 -------- needs to be changed
    fread(&metadataTable, sizeof(Bloc), 1, disque); //buffer for metadonnees
    for (int j = 1; j < 3; j++){
        if (metadataTable.content.metadataTable.T[j].Adrpremierbloc == -1) {
            initMetadonnees(disque, j);
        metadataTable.content.metadataTable.T[j].Adrpremierbloc = ptDataBlock;
            break;
        }

    fseek(disque, j*sizeof(Bloc), SEEK_SET);
    fwrite(&metadataTable, sizeof(Bloc), 1, disque);


    //just to test if its a success
    printf("new data block at address: %d\n", ptDataBlock);

}
}




////////////////////////////----f----///////////////////////
adressemetadonnees recherchemetadonnees(FILE*disque,const char* nomfichier){
    Bloc buffer;
    adressemetadonnees resultat = {-1, -1};  // Initialisation a -1 pour indiquer non trouvé
    rewind(disque);
    
    for(int i = 1;i <= 2; i++) {
        fseek(disque,i*sizeof(Bloc),SEEK_SET);
        fread(&buffer,sizeof(Bloc),1,disque);

        if(buffer.typedebloc == 1) { 
            for(int j = 0;j < buffer.content.metadataTable.nbrMetadonnees; j++) {
                if(strcmp(buffer.content.metadataTable.T[j].Nomdufichier, nomfichier) == 0) {
                    resultat.index=j; 
                    resultat.numerodebloc=i;
                    return resultat;
                }
            }
        }
    }
    printf("le fichier n'existes pas");
    return resultat;

}



/*int lireCaracteristique(FILE *disque, const char *nomFichier, int caracteristique) {
    rewind(disque);
    Bloc buffer;
    adressemetadonnees addresse = recherchemetadonnees(disque, nomFichier);

    if (addresse.numerodebloc == -1) {
        printf("fichier introuvable");
        return -1;
    }

    fseek(disque, addresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    switch (caracteristique) {
        case 1:  
            return buffer.content.metadataTable.T[addresse.index].Taillefichierblocs;
        case 2:  
            return buffer.content.metadataTable.T[addresse.index].Taillefichierenregistrements;
        case 3:  
            return buffer.content.metadataTable.T[addresse.index].Adrpremierbloc;
        case 4:  
            return buffer.content.metadataTable.T[addresse.index].Modeorganisationglobale;
        case 5:  
            return buffer.content.metadataTable.T[addresse.index].Modeorganisationinterne;
        default:
            printf("Caracteristique non trouvé\n");
            return -1;
    }
}*/


int liremetadonnees(FILE*disque, const char* nomFichier, int caracteristique) {
    Bloc buffer;
    adressemetadonnees adresse;

    adresse = recherchemetadonnees(disque, nomFichier);
    
        if(adresse.numerodebloc == -1) {
        printf("fichier introuvable");
        }

      rewind(disque);

        fseek(disque,adresse.numerodebloc*sizeof(Bloc),SEEK_SET);
        fread(&buffer,sizeof(Bloc),1,disque);

    for (int i =0; i < FB ; i++) {
        if ( adresse.index == i) {
            switch (caracteristique)
    {        
                case 1:  
                    return buffer.content.metadataTable.T[i].Taillefichierblocs;
                case 2:  
                    return buffer.content.metadataTable.T[i].Taillefichierenregistrements;
                case 3:  
                    return buffer.content.metadataTable.T[i].Adrpremierbloc;
                case 4:  
                    return buffer.content.metadataTable.T[i].Modeorganisationglobale;
                case 5:  
                    return buffer.content.metadataTable.T[i].Modeorganisationinterne;
            
                default:
                    printf("Caracteristique non trouve\n");
                    return -1;
            }
        }
    }
}

void miseAJourMetadonnees(FILE* disque, const char* nomFichier, int champ, int nouvelleValeur) {
    // Recherche l'adresse des métadonnées du fichier
    adressemetadonnees adresse = recherchemetadonnees(disque, nomFichier);

    // Vérification si le fichier existe
    if (adresse.numerodebloc == -1) {
        printf("Fichier introuvable.\n");
        return;
    }

    // Charger le bloc contenant les métadonnées dans le buffer
    Bloc buffer;
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    // Vérification du type de bloc
    if (buffer.typedebloc != 1) {
        printf("Erreur : Le bloc trouvé ne contient pas de métadonnées.\n");
        return;
    }

    // Mise à jour du champ spécifié
    if (champ >= 1 && champ <= 5) {
        int* targetField = NULL;

        switch (champ) {
            case 1: targetField = &buffer.content.metadataTable.T[adresse.index].Taillefichierblocs; break;
            case 2: targetField = &buffer.content.metadataTable.T[adresse.index].Taillefichierenregistrements; break;
            case 3: targetField = &buffer.content.metadataTable.T[adresse.index].Adrpremierbloc; break;
            case 4: targetField = &buffer.content.metadataTable.T[adresse.index].Modeorganisationglobale; break;
            case 5: targetField = &buffer.content.metadataTable.T[adresse.index].Modeorganisationinterne; break;
        }

        if (targetField != NULL) {
            *targetField = nouvelleValeur;
            fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
            fwrite(&buffer, sizeof(Bloc), 1, disque);
            printf("Champ mis à jour avec succès.\n");
        }
    } else {
        printf("Champ non valide.\n");
    }
}


bool ajoutermetadonnees(FILE* disque, fichiermetadonnees metadonnes, int taille) {
    Bloc buffer;
    int blocActuel = 1;
    int prevBloc = 1;
    MS* ms;
    bool allouer = true;

    //
    if ((ms->nbrblocutil + taille) > ms->nbrbloc) {
        printf("Espace insuffisant pour créer le fichier.\n");
        return false;
    }

    //bloc with index 1 for metadata
    MAJtaballocation(ms, 1, 1);


    while (blocActuel != -1) {
        fseek(disque, blocActuel * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        if (buffer.content.metadataTable.nbrMetadonnees < FB) {
            buffer.content.metadataTable.T[buffer.content.metadataTable.nbrMetadonnees] = metadonnes;
            buffer.content.metadataTable.nbrMetadonnees++;

            fseek(disque, blocActuel * sizeof(Bloc), SEEK_SET);
            fwrite(&buffer, sizeof(Bloc), 1, disque);

            return true; 
        }

        prevBloc = blocActuel;
        blocActuel = buffer.content.metadataTable.next;
    }

    
    if (allouer) {
        if (ms->nbrbloc == ms->nbrblocutil) {
            printf("Espace insuffisant pour stocker le fichier !\n");
            return false; 
        }

        //find an empty bloc 
        int place = -1;
        for (int i = 0; i < ms->nbrbloc; i++) {
            if (ms->tablelocation[i].etat == 0) {
                place = i;
                break;
            }
        }

        if (place == -1) {
            printf("Erreur : Aucun bloc disponible.\n");
            return false;
        }

        MAJtaballocation(ms, place, 1);

        // Faire le chaînage
        fseek(disque, prevBloc * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);
        buffer.content.metadataTable.next = place;

        fseek(disque, prevBloc* sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);

        // Initialiser le nouveau bloc pour les métadonnées
        fseek(disque, place * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        buffer.content.metadataTable.T[0] = metadonnes;
        buffer.content.metadataTable.nbrMetadonnees = 1;
        buffer.content.metadataTable.next = -1;

        fseek(disque, place * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);

        return true; //success
    }

    return false; //echec 
}


void defregmentation(FILE *disque, MS *ms, const char *nomFichier) {
    Bloc buffer;
    int blocactuelle, blocsuivant;
    int taillefichierblocs = 0;
    int totalEnregistrements = 0;


    //first block of the file
    int debut = liremetadonnees(disque, nomFichier, 3);

    blocactuelle = debut;

    if (debut == -1) {
        printf("Le fichier %s est introuvable.\n", nomFichier);
        return;
    }

    rewind(disque);
    //copying file blocks into the buffer
    while (blocactuelle != -1) {
        fseek(disque, blocactuelle*sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        Bloc newBuffer = {0};
        int count = 0;

        //copy non-deleted elements
        for (int i = 0; i < FB ; i++) {
            if (!buffer.content.fileData.T[i].suprimelogiqument) {
                //overwrite, using a new buffer 
                newBuffer.content.fileData.T[count++] = buffer.content.fileData.T[i];
            }
        }

        totalEnregistrements += count;

        //updating the file content
        newBuffer.content.fileData.nbrmaladie = totalEnregistrements;
        //stocking next
        blocsuivant = buffer.content.fileData.next;
        if (count > 0) {
            taillefichierblocs++;
            //working on the new buffer 
            if (blocsuivant != -1) {
                newBuffer.content.fileData.next = blocsuivant;
            } else {
                newBuffer.content.fileData.next = -1;
}


            //modifications into MS, also it overwrites the prev buffer 
            fseek(disque, blocactuelle*sizeof(Bloc), SEEK_SET);
            fwrite(&newBuffer, sizeof(Bloc), 1, disque);
        } else {
            //unused blocks 
        MAJtaballocation(ms, blocactuelle, 0);
        }

        blocactuelle = blocsuivant;
    }
    miseAJourMetadonnees(disque, nomFichier, 1, taillefichierblocs);
    miseAJourMetadonnees(disque, nomFichier, 2, totalEnregistrements);
    
    //Debugger 
    printf("defregmentation functionality check.");
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
    
    //par defaut
    m.suprimelogiqument = false;

    return m;
}

void insertDis(FILE *disque, MS *ms, int nbrbloc, const char* nomFichier) {
    Bloc buffer, prevBuffer;
    int lock;
    int i, j;
    int lastBlock = -1;
    maladie m;

    //get record
    m = insertHelper();

    // njib adr ta3 lpremier w n93d nmchi hta nl9a next = -1 
    lock = liremetadonnees(disque, nomFichier, 3);
    
    
    //to get the adr of the last bloc then we start foll the .next of this bloc to the new bloc 
    for (i = lock; i < nbrbloc; i++) {
            fseek(disque,i * sizeof(Bloc), SEEK_SET);
            fread(&prevBuffer,sizeof(Bloc), 1, disque);

            if (prevBuffer.content.fileData.next == -1) {
                lastBlock = i; //Found the last block
                break;
            }

    }


    //we need to see if we add the record in the same bloc (FB)
    fseek(disque, lastBlock * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    if (buffer.content.fileData.nbrmaladie < FB) {
        //the next empty record
        buffer.content.fileData.T[buffer.content.fileData.nbrmaladie] = m;
        buffer.content.fileData.nbrmaladie++;

        // Write back the updated block
        fseek(disque, lastBlock * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);

        printf("Le record a ete insere avec succes dans le bloc %d.\n", lastBlock);
        return;
    }


    //if block is full, we need to find a new bloc
    int newBlock = -1;
    for (i = 2; i < nbrbloc; i++) {
        if (ms->tablelocation[i].etat == 0) {
            newBlock = i;
            break;
        }
    }

    if (newBlock == -1) {
        printf("Aucun espace disponible pour inserer le new record.\n");
        return;
    }

    // update the next of the last block (points to new block)
    buffer.content.fileData.next = newBlock;
    fseek(disque, lastBlock * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    //making the new block 
    MAJtaballocation(ms, newBlock, 1);
    /*
    ms->tablelocation[newBlock].etat = 1; 
    ms->tablelocation[newBlock].adrdebloc = newBlock;
    */
    buffer.content.fileData.nbrmaladie = 0;
    buffer.content.fileData.next = -1;
    buffer.content.fileData.T[buffer.content.fileData.nbrmaladie] = m;
    buffer.content.fileData.nbrmaladie++;

    //updae the MS
    fseek(disque, newBlock * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    printf("Le record a ete insere avec succes dans un nouveau bloc %d.\n", newBlock);
}



position researchDis(FILE *disque, int searchId, const char* nomFichier) {
    Bloc buffer;
    int recordFound = 0;
    position res;

    int numBloc = liremetadonnees(disque, nomFichier, 3);

    while (numBloc != -1) {  
        rewind(disque);
        fseek(disque, numBloc * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);
        //we check if it's a data FILE (==2)
        if (buffer.typedebloc == 2) {
            for (int j = 0; j < buffer.content.fileData.nbrmaladie; j++) {
                if (buffer.content.fileData.T[j].id == searchId) {
                    res.deplacement = j;
                    res.numBloc = numBloc;
                    recordFound = 1;
                    return res;
                }
            }
        }
        numBloc = buffer.content.fileData.next;
    }
    //id not found
    if (!recordFound) 
        res.deplacement = -1;
        return res;
        
}



void suppLogique(FILE *disque, int searchId, const char *nomFichier) {

    Bloc buffer;
    int currentBlock = -1;
    bool recordFound = false;
    
    position res = researchDis(disque, searchId, nomFichier);
    if (res.deplacement != -1 ) {

        fseek(disque, res.numBloc * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        buffer.content.fileData.T[res.deplacement].suprimelogiqument = true;

        rewind(disque);
        fseek(disque, res.numBloc * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);
    }
    else {
         printf("\nTHIS ID DOESN'T EXIST !! no need for this operation ");
    }

}


void suppPhysique(FILE *disque, MS *ms, const char *nomFichier) {
 
                defregmentation(disque, ms, nomFichier);
                // maybe i will add more later
}


void renameFile(FILE *disque, const char *nomFichier, const char *newName) {
    adressemetadonnees adress;
    Bloc buffer;

    adress = recherchemetadonnees(disque, nomFichier);

    fseek(disque, adress.numerodebloc *sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);
    //once found we copy it 
    strcpy(buffer.content.metadataTable.T[adress.index].Nomdufichier, newName);

    fseek(disque, adress.numerodebloc *sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    printf("File renamed to '%s'\n", newName);
}


int main() {
    FILE *disque = fopen("disk.dat", "w+b");
    if (!disque) {
        printf("Error: Unable to create disk.\n");
        return 1;
    }

    MS ms;
    initMS(&ms, MAX_BLOCKS);

    creationL_OF(disque, &ms, 10);


    fclose(disque);
    return 0;
}
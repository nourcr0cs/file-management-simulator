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

typedef struct MS {
    int nbrblocutil; //used blocs
    int nbrbloc; 
    int fb;
} MS;

typedef struct {
    fichiermetadonnees T[FB]; 
    int nbrMetadonnees;      
    int next;
} BlocMetadonnees;

//to locate which bloc and which record that represent the file infos
typedef struct {
    int numerodebloc;
    int index;
} adressemetadonnees;

typedef struct {
    Tableallocation tablelocation[FB];
    int nbrblocutil; 
    int nbrbloc; 
} BlocAllocation;

//union to store different types of content in a block 
//this is the main idea to minimize the storage gap
typedef union {
    BlocMetadonnees metadataTable;   //a metadataTable 
    BlocData fileData;            //the main file (file data/maladie records)
    BlocAllocation allocation;   
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

typedef struct {
    int blockNumber;
    Bloc originalContent;
} BlockBackup;

typedef struct {
    BlockBackup* backups;
    int backupCount;
    bool isActive;
} Transaction;


bool beginTransaction(Transaction* trans) {
    trans->backups = malloc(sizeof(BlockBackup) * MAX_BLOCKS);
    trans->backupCount = 0;
    trans->isActive = true;
    return true;
}

bool backupBlock(FILE* disque, Transaction* trans, int blockNum) {
    if (!trans->isActive) return false;
    
    Bloc buffer;
    fseek(disque, blockNum * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);
    
    trans->backups[trans->backupCount].blockNumber = blockNum;
    trans->backups[trans->backupCount].originalContent = buffer;
    trans->backupCount++;
    
    return true;
}

bool commitTransaction(Transaction* trans) {
    free(trans->backups);
    trans->backupCount = 0;
    trans->isActive = false;
    return true;
}

bool rollbackTransaction(FILE* disque, Transaction* trans) {
    if (!trans->isActive) return false;
    
    // restore all blocks to their original state
    for (int i = 0; i < trans->backupCount; i++) {
        fseek(disque, trans->backups[i].blockNumber * sizeof(Bloc), SEEK_SET);
        fwrite(&trans->backups[i].originalContent, sizeof(Bloc), 1, disque);
    }
    
    free(trans->backups);
    trans->backupCount = 0;
    trans->isActive = false;
    return true;
}

bool verifierEspaceSuffisant(FILE* disque, int nbrBlocsVoulu) {
    Bloc buffer;
    rewind(disque);
    buffer.typedebloc = 3;
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    if (buffer.typedebloc != 3) {
        printf("Erreur : Le bloc 0 n'est pas un bloc d'allocation.\n");
        return false;
    }

    int blocsLibres = buffer.content.allocation.nbrbloc - buffer.content.allocation.nbrblocutil;

    if (nbrBlocsVoulu > blocsLibres) {
        printf("Erreur : Espace insuffisant. %d blocs necessaires, %d disponibles.\n", nbrBlocsVoulu, blocsLibres);
        return false;
    }

    printf("Succes : Il y a suffisamment d'espace. %d blocs disponibles.\n", blocsLibres);
    return true;
}

int obtenirNombreBlocs(FILE* disque, int option) {
    Bloc buffer;
    buffer.typedebloc = 3;
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    if (buffer.typedebloc != 3) {
        printf("Erreur : Le bloc 0 n'est pas un bloc d'allocation.\n");
        return -1; 
    }

    BlocAllocation* allocation = &buffer.content.allocation;

    switch (option) {
        case 1: // num of used blocs
            return allocation->nbrblocutil;
        case 2: //total num of blocs
            return allocation->nbrbloc;
        default:
            printf("Erreur : Option invalide. Utilisez 1 pour blocs utilisés ou 2 pour blocs totaux.\n");
            return -1; 
    }
}

//Function to update the number of blocks used or total blocks
void mettreAJourNombreBlocs(FILE* disque, int option, int nouvelleValeur) {
    Bloc buffer;

    buffer.typedebloc = 3;

    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    BlocAllocation* allocation = &buffer.content.allocation;

    switch (option) {
        case 1: //upbdate nbr de blocs utils
            allocation->nbrblocutil = nouvelleValeur;
            break;
        case 2: //update total numb of blocs
            if (nouvelleValeur < allocation->nbrblocutil) {
                return; 
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
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    if (buffer.typedebloc != 3) {
        printf("le premier bloc n'est pas un bloc d'allocation.\n");
        return;
    }

    buffer.content.allocation.tablelocation[blocIndex].etat = etat;
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
}


void initTableAllocation(FILE *disque, MS *ms) {
    Bloc buffer;
    buffer.typedebloc = 3; 
    for (int i = 0; i < ms->nbrbloc; i++) {
        buffer.content.allocation.tablelocation[i].adrdebloc = i;
        buffer.content.allocation.tablelocation[i].etat = 0;
    }

    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    metajourtableallocation(disque, 0, 1);
}


void initMS(FILE *disque, MS *ms, int nbrbloc) {
    ms->nbrbloc = MAX_BLOCKS;
    ms->fb = FB;
    ms->nbrblocutil = 1;
    initTableAllocation(disque, ms);
}



/**
 * @brief Initializes the metadata for a new file
 * @param disque Pointer to the disk file
 * @param i Index in the metadata table
 * @throws None
 * @details This function prompts the user for file information and initializes
 *          the metadata block with the provided information. It handles:
 *          - File name
 *          - Organization mode
 *          - Record ordering
 *          - Number of records
 */


void initMetadonnees(FILE *disque, int i) {
    Bloc buffer;
    buffer.typedebloc = 1; //Set block type to metadata
    
    // Read existing block first
    fseek(disque, sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);
    
    //infos that the user should enter
    printf("Enter the file name: ");
    scanf("%s", buffer.content.metadataTable.T[i].Nomdufichier);

    printf("Enter the global organization mode (0 for chained, 1 for contigue): ");
    scanf("%d", &buffer.content.metadataTable.T[i].Modeorganisationglobale);

    printf("Do you want it to be ordored or non-ordored (0 for ordored, 1 for non-ordored): ");
    scanf("%d", &buffer.content.metadataTable.T[i].Modeorganisationinterne);

    printf("Enter the number of records in total: ");
    scanf("%d", &buffer.content.metadataTable.T[i].Taillefichierenregistrements);

    buffer.content.metadataTable.T[i].Adrpremierbloc = -1;
    buffer.content.metadataTable.nbrMetadonnees++;

    // Write back the updated block
    fseek(disque, sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    printf("Metadata initialized successfully.\n");
}

/**
 * @brief Creates a new file in the system
 * @param disque Pointer to the disk file
 * @param ms Pointer to the memory structure
 * @param nbrbloc Number of blocks available
 * @return void
 * @throws None
 * @details Creates a new file by:
 *          1. Checking for available space
 *          2. Finding an empty metadata slot
 *          3. Initializing metadata
 *          4. Allocating first data block
 *          5. Updating allocation table
 */
void creationL_OF(FILE *disque, MS *ms, int nbrbloc) {
    Bloc buffer;
    int ptDataBlock = -1;
    int i = 0;
    int metadataFound = 0;
    int taille = 1; // Define taille variable

    if ((ms->nbrblocutil + taille) > ms->nbrbloc) {
        printf("Espace insuffisant.\n");
        return;
    }
    //I asssumed the bloc with index 1 is for metadata 
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
    //If no space in metadata, print error
    //but we may search later, if we have time for new blocks for metaData
    if (!metadataFound) {
        printf("No space available in metadata blocks.\n");
        return;
    }
    //skipping the pt two wla three (alloc + metadataTable), pour l'instant rni dyra 3 (whda alloc, zodj metadata)
    for ( i = 3; i < ms->nbrbloc; i++) {
        if (ms->[i].etat == 0) {
            ptDataBlock = i;
            break;
        }
    }
    if (ptDataBlock == -1) {
        printf("No available blocks in MS.\n");
        return ;
    }
    buffer.content.fileData.nbrmaladie = 0;
    buffer.content.fileData.next = -1;
    fseek(disque, ptDataBlock * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

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
    // Calculate blocks needed based on records and blocking factor
    int facteur_blocage = FB; // Using FB constant
    fichiermetadonnees metadonnees = buffer.content.metadataTable.T[i];
    metadonnees.Taillefichierblocs = ceil((double)metadonnees.Taillefichierenregistrements / facteur_blocage) + 1;
    // Write back updated metadata
    fseek(disque, sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
}
}




///////////////////////////////////////////////-----------f------------///////////////////////////////////////
adressemetadonnees recherchemetadonnees(FILE*disque,const char* nomfichier){
    Bloc buffer;
    adressemetadonnees resultat = {-1, -1};  
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


void compactage(Tableallocation* blocAlloc) {
    int indexLibre = 0;  // L'indice du prochain bloc vide à remplir

    // Parcours de tous les blocs pour déplacer les blocs pleins
    for (int i = 0; i < 20; i++) {
        if (BlocAllocation->tablelocation[i].etat == 1) {  // Si le bloc est plein
            if (i != indexLibre) {  // Si ce n'est pas déjà à la bonne position
                // Déplacer le bloc plein vers la position vide
                BlocAllocation.tablelocation[indexLibre] = BlocAllocation.tablelocation[i];
                BlocAllocation.tablelocation[i]->etat = 0;  //Le bloc déplacé devient vide
            }
            indexLibre++;  //on passe à la prochaine case vide
        }
    }

    // Après le compactage, tous les blocs à partir de indexLibre seront vides
    for (int i = indexLibre; i < 20; i++) {
        BlocAllocation.tablelocation[i].etat = 0;  //Marquer les blocs comme vides
    }

    printf("La mémoire a été compactée.\n");
}




int liremetadonnees(FILE*disque, const char* nomFichier, int caracteristique) {
    Bloc buffer;
    adressemetadonnees adresse;

    adresse = recherchemetadonnees(disque, nomFichier);
    
        if(adresse.numerodebloc == -1) {
        printf("fichier introuvable");
        }

      rewind(disque);

        fseek(disque, adresse.numerodebloc*sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

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

    //Verification si le fichier existe
    if (adresse.numerodebloc == -1) {
        printf("Fichier introuvable.\n");
        return;
    }

    //Charger le bloc contenant les métadonnées dans le buffer
    Bloc buffer;
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    //Verification du type de bloc
    if (buffer.typedebloc != 1) {
        printf("Erreur : Le bloc trouve ne contient pas de metadonnees.\n");
        return;
    }

    //Mise a jour du champ specifie
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


bool ajoutermetadonnees(FILE* disque, MS* ms, fichiermetadonnees metadonnes, int taille) {
    Bloc buffer;
    int blocActuel = 1;
    int prevBloc = 1;
    bool allouer = true;

    if ((ms->nbrblocutil + taille) > ms->nbrbloc) {
        printf("Espace insuffisant pour creer le fichier.\n");
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
            if (buffer.content.allocation.tablelocation[i].etat == 0) {
                place = i;
                break;
            }
        }

        if (place == -1) {
            printf("Erreur : Aucun bloc disponible.\n");
            return false;
        }

        MAJtaballocation(ms, place, 1);

        //Faire le chaînage
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

    // Compactage: ensures all fragmented blocks are handled first
    compactage(ms);

    // Get the first block of the file
    int debut = liremetadonnees(disque, nomFichier, 3);

    blocactuelle = debut;

    if (debut == -1) {
        printf("Le fichier %s est introuvable.\n", nomFichier);
        return;
    }

    //verify if the metadata block is valid before proceeding
    if (!verifierEspaceSuffisant(disque, 0)) {
        printf("Erreur : Bloc d'allocation invalide ou espace insuffisant pour la défragmentation.\n");
        return;
    }

    rewind(disque);

    //copying file blocks into the buffer
    while (blocactuelle != -1) {
        fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        Bloc newBuffer = {0};
        int count = 0;

        //copy non-deleted elements
        for (int i = 0; i < FB; i++) {
            if (!buffer.content.fileData.T[i].suprimelogiqument) {
                // Overwrite, using a new buffer
                newBuffer.content.fileData.T[count++] = buffer.content.fileData.T[i];
            }
        }

        totalEnregistrements += count;

        //updating the file content
        newBuffer.content.fileData.nbrmaladie = totalEnregistrements;

        //stocking the next block
        blocsuivant = buffer.content.fileData.next;
        if (count > 0) {
            taillefichierblocs++;

            //working on the new buffer
            if (blocsuivant != -1) {
                newBuffer.content.fileData.next = blocsuivant;
            } else {
                newBuffer.content.fileData.next = -1;
            }

            //modifications into MS, also it overwrites the previous buffer
            compactage(ms);
            fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
            fwrite(&newBuffer, sizeof(Bloc), 1, disque);
        } else {
            // Unused blocks
            MAJtaballocation(ms, blocactuelle, 0);
        }

        blocactuelle = blocsuivant;
    }

    //updating metadata with the new block count and total records
    miseAJourMetadonnees(disque, nomFichier, 1, taillefichierblocs);
    miseAJourMetadonnees(disque, nomFichier, 2, totalEnregistrements);

    //debugger
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

    // Get the record
    m = insertHelper();

    // Verify if there is enough space before proceeding
    if (!verifierEspaceSuffisant(disque, 1)) {
        printf("Echec : Espace insuffisant pour insérer le record.\n");
        return;
    }

    // Get address of the first block of the target file, then we get to the next block by .next
    lock = liremetadonnees(disque, nomFichier, 3);

    //To get the address of the first block, then we start following the .next of this block to the new block
    for (i = lock; i < nbrbloc; i++) {
        fseek(disque, i * sizeof(Bloc), SEEK_SET);
        fread(&prevBuffer, sizeof(Bloc), 1, disque);

        if (prevBuffer.content.fileData.next == -1) {
            lastBlock = i; // Found the last block
            break;
        }
    }

    //We need to see if we add the record in the same block (FB)
    fseek(disque, lastBlock * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    if (buffer.content.fileData.nbrmaladie < FB) {
        // The next empty record
        buffer.content.fileData.T[buffer.content.fileData.nbrmaladie] = m;
        buffer.content.fileData.nbrmaladie++;

        //write back the updated block
        fseek(disque, lastBlock * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);

        printf("Le record a ete insere avec succes dans le bloc %d.\n", lastBlock);

        // update the number of blocks used
        int nbrBlocsUtilises = obtenirNombreBlocs(disque, 1);
        mettreAJourNombreBlocs(disque, 1, nbrBlocsUtilises + 1);

        return;
    }

    //if block is full, we need to find a new block
    int newBlock = -1;
    for (i = 2; i < nbrbloc; i++) {
        if (ms->tablelocation[i].etat == 0) {
            newBlock = i;
            break;
        }
    }

    if (newBlock == -1) {
        printf("Aucun espace disponible pour insérer le nouveau record.\n");
        return;
    }

    //update the next of the last block (points to new block)
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

    //update the MS
    fseek(disque, newBlock * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    printf("Le record a été inséré avec succès dans un nouveau bloc %d.\n", newBlock);

    // Update the total number of blocks used
    int nbrBlocsUtilises = obtenirNombreBlocs(disque, 1);
    mettreAJourNombreBlocs(disque, 1, nbrBlocsUtilises + 1);
}

/**
 * @brief Searches for a record in the file system
 * @param disque Pointer to the disk file
 * @param searchId ID of the record to find
 * @param nomFichier Name of the file to search in
 * @return position Structure containing block number and displacement
 * @throws None
 * @note Returns {-1, -1} if record is not found
 */
position researchDis(FILE *disque, int searchId, const char* nomFichier) {
    Bloc buffer;
    int recordFound = 0;
    position res;

    //adress of the first bloc of the target file
    int numBloc = liremetadonnees(disque, nomFichier, 3);

    if (!verifierEspaceSuffisant(disque, 0)) {
        printf("Erreur : Bloc d'allocation invalide ou espace insuffisant.\n");
        res.deplacement = -1;
        return res;
    }

    //traversing the blocs of the target file
    while (numBloc != -1) {
        rewind(disque);
        fseek(disque, numBloc * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        //Check if it's a data block (==2)
        if (buffer.typedebloc == 2) {
            for (int j = 0; j < buffer.content.fileData.nbrmaladie; j++) {
                if (buffer.content.fileData.T[j].id == searchId) {
                    // Record found
                    res.deplacement = j;
                    res.numBloc = numBloc;
                    recordFound = 1;
                    return res;
                }
            }
        }

        //next bloc
        numBloc = buffer.content.fileData.next;
    }

    //not found
    if (!recordFound) {
        res.deplacement = -1;
    }
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
}


void renameFile(FILE *disque, const char *nomFichier, const char *newName) {
    adressemetadonnees adress;
    Bloc buffer;

    adress = recherchemetadonnees(disque, nomFichier);

    fseek(disque, adress.numerodebloc *sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);
    //once found we introduce the new name to the matching file
    strcpy(buffer.content.metadataTable.T[adress.index].Nomdufichier, newName);

    fseek(disque, adress.numerodebloc *sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    printf("File renamed to '%s'\n", newName);
}


void afficherMemoireSecondaire(FILE* disque, int nombreBlocs) {
    Bloc buffer;

    printf("========== État de la Mémoire Secondaire ==========\n");
    for (int i = 0; i < nombreBlocs; i++) {
        // Lire le bloc à l'adresse i
        fseek(disque, i * sizeof(Bloc), SEEK_SET);
        if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Erreur : Impossible de lire le bloc %d.\n", i);
            continue;
        }

        // Afficher le type du bloc
        printf("Bloc %d : ", i);
        switch (buffer.typedebloc) {
            case 0:
                printf("Inutilisé\n");
                break;
            case 1:
                printf("Bloc de métadonnées\n");
                printf("  Nombre de fichiers : %d\n", buffer.content.metadataTable.nbrMetadonnees);
                printf("  Next : %d\n", buffer.content.metadataTable.next);
                break;
            case 2:
                printf("Bloc de données\n");
                printf("  Nombre d'enregistrements : %d\n", buffer.content.fileData.nbrmaladie);
                printf("  Next : %d\n", buffer.content.fileData.next);

                // Parcourir les enregistrements et afficher leur état
                for (int j = 0; j < buffer.content.fileData.nbrmaladie; j++) {
                    printf("    Enregistrement %d : ID = %d, Supprimé = %s\n",
                        j,
                        buffer.content.fileData.T[j].id,
                        buffer.content.fileData.T[j].suprimelogiqument ? "Oui" : "Non");
                }
                break;
            case 3:
                printf("Bloc de table d'allocation\n");
                printf("  Nombre total de blocs : %d\n", buffer.content.allocation.nbrbloc);
                printf("  Nombre de blocs utilisés : %d\n", buffer.content.allocation.nbrblocutil);
                printf("  Table d'allocation :\n");
                for (int j = 0; j < 20; j++) {
                    printf("    Bloc %d : %s\n",
                        buffer.content.allocation.tablelocation[j].adrdebloc,
                        buffer.content.allocation.tablelocation[j].etat == 1 ? "Alloué" : "Libre");
                }

                // Appeler obtenirNombreBlocs pour afficher plus de détails sur l'allocation
                printf("  Détails additionnels :\n");
                printf("    Blocs utilisés : %d\n", obtenirNombreBlocs(disque, 1));
                printf("    Blocs totaux : %d\n", obtenirNombreBlocs(disque, 2));
                break;
            default:
                printf("Type inconnu (%d)\n", buffer.typedebloc);
                break;
        }
    }

    // Débugger
    printf("========== Fin de l'état de la Mémoire Secondaire ==========\n");
}

void MAJtaballocation(MS *ms, int blocIndex, int etat) {
    if (blocIndex >= 0 && blocIndex < ms->nbrbloc) {
        ms->nbrblocutil += (etat == 1) ? 1 : -1;
        // Update allocation table state
        // This should update the actual allocation table in the disk
    }
}

bool deleteL_OF(FILE* disque, MS* ms, const char* nomFichier) {
    // Initialize transaction
    Transaction trans;
    if (!beginTransaction(&trans)) {
        printf("Erreur: Impossible de démarrer la transaction.\n");
        return false;
    }

    // Get file metadata
    adressemetadonnees adresse = recherchemetadonnees(disque, nomFichier);
    if (adresse.numerodebloc == -1) {
        printf("Erreur: Fichier introuvable.\n");
        return false;
    }

    // Backup metadata block
    backupBlock(disque, &trans, adresse.numerodebloc);

    // Read metadata to get first data block
    Bloc buffer;
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    int currentBlock = buffer.content.metadataTable.T[adresse.index].Adrpremierbloc;
    
    // Backup and free all data blocks
    while (currentBlock != -1) {
        // Backup current block before modification
        backupBlock(disque, &trans, currentBlock);
        
        // Read next block address before freeing current
        fseek(disque, currentBlock * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);
        int nextBlock = buffer.content.fileData.next;

        // Update allocation table
        metajourtableallocation(disque, currentBlock, 0);
        
        currentBlock = nextBlock;
    }

    // Clear metadata entry
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);
    
    // Shift remaining metadata entries
    for (int i = adresse.index; i < buffer.content.metadataTable.nbrMetadonnees - 1; i++) {
        buffer.content.metadataTable.T[i] = buffer.content.metadataTable.T[i + 1];
    }
    buffer.content.metadataTable.nbrMetadonnees--;

    // Write updated metadata block
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    // Update block count
    int blocCount = obtenirNombreBlocs(disque, 1);
    mettreAJourNombreBlocs(disque, 1, blocCount - 1);

    // If everything succeeded, commit the transaction
    if (commitTransaction(&trans)) {
        printf("Fichier '%s' supprimé avec succès.\n", nomFichier);
        return true;
    } else {
        // If something went wrong, rollback
        rollbackTransaction(disque, &trans);
        printf("Erreur: La suppression a échoué. Opération annulée.\n");
        return false;
    }
}

int main() {
    printf("Program started successfully!\n");

    FILE* disque = fopen("disque.bin", "r+b");
    if (!disque) {
        disque = fopen("disque.bin", "w+b");
        if (!disque) {
            printf("Error: Could not create disque.bin.\n");
            return 1;
        }
        
        // Initialize MS structure
        MS ms;
        ms.nbrbloc = MAX_BLOCKS;
        ms.fb = FB;
        ms.nbrblocutil = 1;
        
        // Initialize disk with allocation block
        initMS(disque, &ms, MAX_BLOCKS);
    }

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
                //créer le fichier en fonction des choix d'organisation
                break;
            case 3:
                //afficher l'état de la mémoire secondaire
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



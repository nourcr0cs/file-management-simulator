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
} MS;

//to locate which bloc and which record that represent the file infos
typedef struct {
    int numerodebloc;
    int index;
} adressemetadonnees;

//union to store different types of content in a block 
//this is the main idea to minimize the storage gap
typedef union {
    fichiermetadonnees metadata;   //a metadata file
    BlocData fileData;            //the main file (file data/maladie records)
    Tableallocation allocation;   
} BlockContent;

//the whole block structure containing the union and block type (to access it directly)
typedef struct {
    BlockContent content;  //The union storing block content
    int typedebloc;        // 1 = metadata, 2 = file data, 3 = allocation
} Bloc;


void initMS(MS *ms, int nbrbloc) {
    ms->nbrbloc = nbrbloc;
    ms->nbrblocutil = 0;
    for (int i = 0; i < nbrbloc; i++) {
        ms->tablelocation[i].etat = 0; //All blocks initially empty
        ms->tablelocation[i].adrdebloc = -1; //No address assigned
    }
}


void initMetadonnees(FILE *disque) {
    fichiermetadonnees metadata;
    Bloc buffer;

    //infos that the user should enter
    printf("Enter the file name: ");
    scanf("%s", metadata.Nomdufichier);

    printf("Enter the global organization mode (0 for chained, 1 for contigue): ");
    scanf("%s", metadata.Modeorganisationglobale);

    printf("Do you want it to be ordored or non-ordored (0 for ordored, 1 for non-ordored): ");
    scanf("%s", metadata.Modeorganisationinterne);

    printf("Enter the number of records in total: ");
    scanf("%s", metadata.Taillefichierenregistrements);


    metadata.Adrpremierbloc = -1;

    //to copy directly an entire bloc = fread
    memcpy(&buffer.content.metadata, &metadata, sizeof(metadata));

    fseek(disque, sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    printf("Metadata initialized successfully.\n");
}



void creationL_OF(FILE *disque, MS *ms, int nbrbloc) {
    Bloc buffer;
    int ptDataBlock = -1;
    int j =0;

    //9bl hna lzm tkon n init l alloc w metadonnees, nrmlm lokhrin ydiroha
    

    //skipping the pt two wla three (alloc + metadata), pour l'instant rni dyra 2
    for (int i = 2; i < ms->nbrbloc; i++) {
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



    //Update the metadata block with the pt data block address (aussi capable fonc whdha )
    Bloc metadata;
    fseek(disque, sizeof(Bloc), SEEK_SET); // we assume metadata is in block 1 -------- needs to be changed
    fread(&metadata, sizeof(Bloc), 1, disque); //buffer for metadonnees
    metadata.content.metadata.Adrpremierbloc = ptDataBlock;
    fseek(disque, sizeof(Bloc), SEEK_SET);
    fwrite(&metadata, sizeof(Bloc), 1, disque);


    //just to test if its a success
    printf("pt data block at address: %d\n", ptDataBlock);

}



adressemetadonnees recherchemetadonnees(FILE*disque,const char* nomfichier){
    Bloc buffer;
    adressemetadonnees resultat = {-1, -1};  // Initialisation a -1 pour indiquer non trouvé

    //chercher l'adresse de metadonnees dans les bloc 2 et 3
    for(int i = 2;i <= 3; i++) {
        fseek(disque,i*sizeof(Bloc),SEEK_SET);
        fread(&buffer,sizeof(Bloc),1,disque);

        if(buffer.typedebloc == 1) { //pour assurer que se bloc contient metadonnees 
            for(int j = 0;j < FB; j++) {
                if(strcmp(buffer.content.metadata.Nomdufichier, nomfichier) == 0) {// comparer si le nom de fichier courant c'est le meme que je cherche 
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

int lireCaracteristique(FILE *disque, const char *nomFichier, int caracteristique) {

    Bloc buffer;
    adressemetadonnees adresse = recherchemetadonnees(disque, nomFichier);

    if (adresse.numerodebloc == -1) {
        printf("fichier introuvable");
        return -1;
    }

    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    switch (caracteristique) {
        case 1:  
            return buffer.content.metadata.Taillefichierblocs;
        case 2:  
            return buffer.content.metadata.Taillefichierenregistrements;
        case 3:  
            return buffer.content.metadata.Adrpremierbloc;
        case 4:  
            return buffer.content.metadata.Modeorganisationglobale;
        case 5:  
            return buffer.content.metadata.Modeorganisationinterne;
        default:
            printf("Caracteristique non trouvé\n");
            return -1;
    }
}


int liremetadonnees(FILE*disque, const char* nomFichier, int caracteristique) {
    Bloc buffer;
    adressemetadonnees adresse;

    adresse = recherchemetadonnees(disque, nomFichier);

        if(adresse.numerodebloc == -1) {
        printf("fichier introuvable");
        }

        fseek(disque,adresse.numerodebloc*sizeof(Bloc),SEEK_SET);
        fread(&buffer,sizeof(Bloc),1,disque);

    for (int i =0; i < FB ; i++) {
        if ( adresse.index == i) {
            switch (caracteristique)
    {        
                case 1:  
                    return buffer.content.metadata.Taillefichierblocs;
                case 2:  
                    return buffer.content.metadata.Taillefichierenregistrements;
                case 3:  
                    return buffer.content.metadata.Adrpremierbloc;
                case 4:  
                    return buffer.content.metadata.Modeorganisationglobale;
                case 5:  
                    return buffer.content.metadata.Modeorganisationinterne;
            
                default:
                    printf("Caracteristique non trouve\n");
                    return -1;
            }
        }
    }
}

void insertDis(FILE *disque, MS *ms, maladie *record, int nbrbloc, const char* nomFichier) {
    Bloc buffer, prevBuffer;
    int lock;
    int i, j;
    int lastBlock = -1;

  
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

    if (buffer.content.fileData.nbrmaladie <= FB) {
        buffer.content.fileData.T[buffer.content.fileData.nbrmaladie] = *record;
        buffer.content.fileData.nbrmaladie++;

        // Write back the updated block
        fseek(disque, lastBlock * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);

        printf("Le record a ete insere avec succes dans le bloc %d.\n", lastBlock);
        return;
    }


    //find a new bloc
    int newBlock = -1;
    for (i = 0; i < nbrbloc; i++) {
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
    ms->tablelocation[newBlock].etat = 1; 
    ms->tablelocation[newBlock].adrdebloc = newBlock;
    buffer.content.fileData.nbrmaladie = 0;
    buffer.content.fileData.next = -1;
    buffer.content.fileData.T[buffer.content.fileData.nbrmaladie] = *record;
    buffer.content.fileData.nbrmaladie++;

    //updae the MS
    fseek(disque, newBlock * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    printf("Le record a ete insere avec succès dans un nouveau bloc %d.\n", newBlock);
}





bool researchDis(FILE *disque, MS *ms, int searchId, const char* nomFichier, int* pt, int* indx) {
    Bloc buffer;
    int recordFound = 0;

    *pt = lireCaracteristique(disque, nomFichier, 3);

    while (*pt != -1) {  
        fseek(disque, *pt * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);
        //we check if it's a data FILE (==2)
        if (buffer.typedebloc == 2) {
            for (int j = 0; j < FB; j++) {
                if (buffer.content.fileData.T[j].id == searchId) {
                    *indx = j;
                    printf("Record found in block %d, at index %d.\n", &pt , j);
                    recordFound = 1;
                    break;
                }
            }
        }
        if (recordFound) return true;
        *pt = buffer.content.fileData.next;
    }
    // id not found
    if (!recordFound) 
        return false;
}



void suppLogique(FILE *disque, MS *ms, int searchId, const char *nomFichier) {

    Bloc buffer;
    int currentBlock = -1;
    bool recordFound = false;
    int pt = 0;
    int indx = 0;
    
    if (researchDis(disque, ms, searchId, nomFichier, &pt, &indx)) {
        fseek(disque, pt * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        buffer.content.fileData.T[indx].suprimelogiqument = true;

        fseek(disque, pt * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);
    }
    else {
         printf("\nTHIS ID DOESN'T EXIST ! ");
    }

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
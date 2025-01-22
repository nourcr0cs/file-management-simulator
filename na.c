#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define FB 10
#define MAX_BLOCKS 50

typedef struct {
    char Nomdufichier[20];
    int Taillefichierblocs;
    int Taillefichierenregistrements;
    int Adrpremierbloc;  // Address of the first block
    int Modeorganisationglobale; // 0 for chained
    int Modeorganisationinterne;  // 0 for ordered
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

typedef struct {
    maladie T[FB];
    int nbrmaladie;
    int next;
} BlocData;

typedef struct {
    int adrdebloc; // Address of the block
    int etat;      // 0 = empty, 1 = full
} Tableallocation;

typedef struct {
    fichiermetadonnees T[FB];
    int nbrMetadonnees;
    int next;
} BlocMetadonnees;

typedef struct {
    Tableallocation tablelocation[MAX_BLOCKS];
    int nbrblocutil;
    int nbrbloc;
} BlocAllocation;

typedef union {
    BlocMetadonnees metadataTable;
    BlocData fileData;
    BlocAllocation allocation;
} BlockContent;

typedef struct {
    BlockContent content;
    int typedebloc; // 1 = metadataTable, 2 = file data, 3 = allocation
} Bloc;

typedef struct {
    int numerodebloc;
    int index;
} adressemetadonnees;

typedef struct {
    int numBloc;
    int deplacement;
} position;

// Function declarations
bool verifierEspaceSuffisant(FILE* disque, int nbrBlocsVoulu);
int obtenirNombreBlocs(FILE* disque, int option);
void mettreAJourNombreBlocs(FILE* disque, int option, int nouvelleValeur);
void metajourtableallocation(FILE* disque, int blocIndex, int etat);
void initMS(FILE *disque, int nbrbloc);
void creationL_OF(FILE *disque, int nbrbloc);
adressemetadonnees recherchemetadonnees(FILE* disque, const char* nomFichier);
int liremetadonnees(FILE* disque, const char* nomFichier, int caracteristique);
void miseAJourMetadonnees(FILE* disque, const char* nomFichier, int champ, int nouvelleValeur);
bool ajoutermetadonnees(FILE* disque, fichiermetadonnees metadonnes, int taille);
void defregmentation(FILE *disque, const char *nomFichier);
maladie insertHelper();
void insertDis(FILE *disque, int nbrbloc, const char* nomFichier);
position researchDis(FILE *disque, int searchId, const char* nomFichier);
void suppLogique(FILE *disque, int searchId, const char *nomFichier);
void suppPhysique(FILE *disque, const char *nomFichier);
void renameFile(FILE *disque, const char *nomFichier, const char *newName);
void afficherMemoireSecondaire(FILE* disque, int nombreBlocs);
bool deleteL_OF(FILE* disque, const char* nomFichier);

// Function definitions
bool verifierEspaceSuffisant(FILE* disque, int nbrBlocsVoulu) {
    Bloc buffer;
    rewind(disque);
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    if (buffer.typedebloc != 3) {
        printf("Erreur : Le bloc 0 n'est pas un bloc d'allocation.\n");
        return false;
    }

    int blocsLibres = buffer.content.allocation.nbrbloc - buffer.content.allocation.nbrblocutil;

    if (nbrBlocsVoulu > blocsLibres) {
        printf("Erreur : Espace insuffisant. %d blocs nécessaires, %d disponibles.\n", nbrBlocsVoulu, blocsLibres);
        return false;
    }

    printf("Succès : Il y a suffisamment d'espace. %d blocs disponibles.\n", blocsLibres);
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
        case 1: // Number of used blocks
            return allocation->nbrblocutil;
        case 2: // Total number of blocks
            return allocation->nbrbloc;
        default:
            printf("Erreur : Option invalide. Utilisez 1 pour blocs utilisés ou 2 pour blocs totaux.\n");
            return -1;
    }
}

void mettreAJourNombreBlocs(FILE* disque, int option, int nouvelleValeur) {
    Bloc buffer;

    buffer.typedebloc = 3;

    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    BlocAllocation* allocation = &buffer.content.allocation;

    switch (option) {
        case 1: // Update number of used blocks
            allocation->nbrblocutil = nouvelleValeur;
            break;
        case 2: // Update total number of blocks
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
        printf("Le premier bloc n'est pas un bloc d'allocation.\n");
        return;
    }

    buffer.content.allocation.tablelocation[blocIndex].etat = etat;
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
}

void initMS(FILE *disque, int nbrbloc) {
    Bloc buffer;

    // Initialize the allocation table
    buffer.typedebloc = 3; // Allocation block type
    buffer.content.allocation.nbrbloc = MAX_BLOCKS; // Total number of blocks
    buffer.content.allocation.nbrblocutil = 1; // Block 0 is used for allocation table

    // Mark block 0 as used (allocation table)
    buffer.content.allocation.tablelocation[0].adrdebloc = 0;
    buffer.content.allocation.tablelocation[0].etat = 1;

    // Mark block 1 as used (metadata block)
    buffer.content.allocation.tablelocation[1].adrdebloc = 1;
    buffer.content.allocation.tablelocation[1].etat = 1;

    // Mark all other blocks as free
    for (int i = 2; i < MAX_BLOCKS; i++) {
        buffer.content.allocation.tablelocation[i].adrdebloc = i;
        buffer.content.allocation.tablelocation[i].etat = 0;
    }

    // Write the allocation table to disk at index 0
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
        printf("Error writing allocation table\n");
        return;
    }

    // Initialize the first metadata block at index 1
    Bloc metadataBlock;
    metadataBlock.typedebloc = 1; // Metadata block type
    metadataBlock.content.metadataTable.nbrMetadonnees = 0;
    metadataBlock.content.metadataTable.next = -1; // No next block initially

    // Write the metadata block to disk at index 1
    fseek(disque, 1 * sizeof(Bloc), SEEK_SET);
    if (fwrite(&metadataBlock, sizeof(Bloc), 1, disque) != 1) {
        printf("Error writing metadata block at index 1\n");
        return;
    }

    // Initialize all other blocks as unused
    Bloc unusedBlock;
    unusedBlock.typedebloc = 0; // Unused block type
    for (int i = 2; i < MAX_BLOCKS; i++) {
        fseek(disque, i * sizeof(Bloc), SEEK_SET);
        if (fwrite(&unusedBlock, sizeof(Bloc), 1, disque) != 1) {
            printf("Error initializing block %d\n", i);
            return;
        }
    }

    printf("Initialized all blocks successfully.\n");
}

void creationL_OF(FILE *disque, int nbrbloc) {
    Bloc buffer;
    int ptDataBlock = -1;
    int i = 0;
    int metadataFound = 0;

    printf("Starting file creation with %d blocks\n", nbrbloc);

    // Check if there is enough space
    if (!verifierEspaceSuffisant(disque, nbrbloc)) {
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

        if (buffer.content.metadataTable.nbrMetadonnees < FB) {
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

    // Find an empty block for data
    for (i = 2; i < MAX_BLOCKS; i++) {
        fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        if (buffer.content.allocation.tablelocation[i].etat == 0) {
            ptDataBlock = i;
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
    buffer.content.fileData.next = -1;

    fseek(disque, ptDataBlock * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    // Update metadata
    fseek(disque, metadataBlockIndex * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    buffer.content.metadataTable.T[buffer.content.metadataTable.nbrMetadonnees].Adrpremierbloc = ptDataBlock;
    buffer.content.metadataTable.nbrMetadonnees++;

    fseek(disque, metadataBlockIndex * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    // Update allocation table
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    buffer.content.allocation.tablelocation[ptDataBlock].etat = 1;
    buffer.content.allocation.nbrblocutil++;

    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    printf("File creation completed successfully.\n");
}

// Other functions remain unchanged...
void compactage(Tableallocation* blocAlloc) {
    int indexLibre = 0;  // L'indice du prochain bloc vide à remplir

    // Parcours de tous les blocs pour déplacer les blocs pleins
    for (int i = 0; i < 20; i++) {
        if (blocAlloc[i].etat == 1) {  // Si le bloc est plein
            if (i != indexLibre) {  // Si ce n'est pas déjà à la bonne position
                // Déplacer le bloc plein vers la position vide
                blocAlloc[indexLibre] = blocAlloc[i];
                blocAlloc[i].etat = 0;  // Le bloc déplacé devient vide
            }
            indexLibre++;  // On passe à la prochaine case vide
        }
    }

    // Après le compactage, tous les blocs à partir de indexLibre seront vides
    for (int i = indexLibre; i < 20; i++) {
        blocAlloc[i].etat = 0;  // Marquer les blocs comme vides
    }

    printf("La mémoire a été compactée.\n");
}

adressemetadonnees recherchemetadonnees(FILE* disque, const char* nomFichier) {
    adressemetadonnees adresse = {-1, -1};
    Bloc buffer;
    int blocActuel = 1;

    while (blocActuel != -1) {
        fseek(disque, blocActuel * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        if (buffer.typedebloc == 1) {
            for (int i = 0; i < buffer.content.metadataTable.nbrMetadonnees; i++) {
                if (strcmp(buffer.content.metadataTable.T[i].Nomdufichier, nomFichier) == 0) {
                    adresse.numerodebloc = blocActuel;
                    adresse.index = i;
                    return adresse;
                }
            }
            blocActuel = buffer.content.metadataTable.next;
        } else {
            break;
        }
    }

    return adresse;
}

int liremetadonnees(FILE* disque, const char* nomFichier, int caracteristique) {
    Bloc buffer;
    adressemetadonnees adresse;

    adresse = recherchemetadonnees(disque, nomFichier);

    if(adresse.numerodebloc == -1) {
        printf("fichier introuvable");
        return -1;
    }

    rewind(disque);

    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    for (int i = 0; i < FB; i++) {
        if (adresse.index == i) {
            switch (caracteristique) {
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
    return -1;
}

void miseAJourMetadonnees(FILE* disque, const char* nomFichier, int champ, int nouvelleValeur) {
    adressemetadonnees adresse = recherchemetadonnees(disque, nomFichier);

    if (adresse.numerodebloc == -1) {
        printf("Fichier introuvable.\n");
        return;
    }

    Bloc buffer;
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    if (buffer.typedebloc != 1) {
        printf("Erreur : Le bloc trouve ne contient pas de metadonnees.\n");
        return;
    }

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
    bool allouer = true;

    if ((obtenirNombreBlocs(disque, 1) + taille) > obtenirNombreBlocs(disque, 2)) {
        printf("Espace insuffisant pour creer le fichier.\n");
        return false;
    }

    metajourtableallocation(disque, 1, 1);

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
        if (obtenirNombreBlocs(disque, 1) == obtenirNombreBlocs(disque, 2)) {
            printf("Espace insuffisant pour stocker le fichier !\n");
            return false;
        }

        int place = -1;
        for (int i = 0; i < MAX_BLOCKS; i++) {
            Bloc buffer;
            fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
            fread(&buffer, sizeof(Bloc), 1, disque);

            if (buffer.content.allocation.tablelocation[i].etat == 0) {
                place = i;
                break;
            }
        }

        if (place == -1) {
            printf("Erreur : Aucun bloc disponible.\n");
            return false;
        }

        metajourtableallocation(disque, place, 1);

        fseek(disque, prevBloc * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);
        buffer.content.metadataTable.next = place;

        fseek(disque, prevBloc * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);

        fseek(disque, place * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        buffer.content.metadataTable.T[0] = metadonnes;
        buffer.content.metadataTable.nbrMetadonnees = 1;
        buffer.content.metadataTable.next = -1;

        fseek(disque, place * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);

        return true;
    }

    return false;
}

void defregmentation(FILE *disque, const char *nomFichier) {
    Bloc buffer;
    int blocactuelle, blocsuivant;
    int taillefichierblocs = 0;
    int totalEnregistrements = 0;

    int debut = liremetadonnees(disque, nomFichier, 3);

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

        for (int i = 0; i < FB; i++) {
            if (!buffer.content.fileData.T[i].suprimelogiqument) {
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

    m.suprimelogiqument = false;

    return m;
}

void insertDis(FILE *disque, int nbrbloc, const char* nomFichier) {
    Bloc buffer, prevBuffer;
    int lock;
    int i;
    int lastBlock = -1;
    maladie m;

    m = insertHelper();

    if (!verifierEspaceSuffisant(disque, 1)) {
        printf("Echec : Espace insuffisant pour insérer le record.\n");
        return;
    }

    lock = liremetadonnees(disque, nomFichier, 3);

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

    if (buffer.content.fileData.nbrmaladie < FB) {
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

    int numBloc = liremetadonnees(disque, nomFichier, 3);

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

void suppLogique(FILE *disque, int searchId, const char *nomFichier) {
    position res = researchDis(disque, searchId, nomFichier);
    if (res.deplacement != -1) {
        Bloc buffer;
        fseek(disque, res.numBloc * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        buffer.content.fileData.T[res.deplacement].suprimelogiqument = true;

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

void renameFile(FILE *disque, const char *nomFichier, const char *newName) {
    // Check if the file exists
    adressemetadonnees adresse = recherchemetadonnees(disque, nomFichier);
    if (adresse.numerodebloc == -1) {
        printf("Erreur : Le fichier '%s' n'existe pas.\n", nomFichier);
        return;
    }

    // Proceed with renaming the file
    Bloc buffer;
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    // Update the file name
    strcpy(buffer.content.metadataTable.T[adresse.index].Nomdufichier, newName);

    // Write the updated metadata back to disk
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    printf("Fichier renommé en '%s'\n", newName);
}

void afficherMemoireSecondaire(FILE* disque, int nombreBlocs) {
    Bloc buffer;

    printf("========== État de la Mémoire Secondaire ==========\n");
    for (int i = 0; i < nombreBlocs; i++) {
        fseek(disque, i * sizeof(Bloc), SEEK_SET);
        if (fread(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Bloc %d : Non initialisé ou corrompu\n", i);
            continue;
        }

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

                // Print all blocks in the allocation table (up to MAX_BLOCKS)
                for (int j = 0; j < MAX_BLOCKS; j++) {
                    printf("    Bloc %d : %s\n",
                        buffer.content.allocation.tablelocation[j].adrdebloc,
                        buffer.content.allocation.tablelocation[j].etat == 1 ? "Alloué" : "Libre");
                }

                printf("  Détails additionnels :\n");
                printf("    Blocs utilisés : %d\n", obtenirNombreBlocs(disque, 1));
                printf("    Blocs totaux : %d\n", obtenirNombreBlocs(disque, 2));
                break;
            default:
                printf("Type inconnu (%d)\n", buffer.typedebloc);
                break;
        }
    }

    printf("========== Fin de l'état de la Mémoire Secondaire ==========\n");
}

void MAJtaballocation(FILE* disque, int blocIndex, int etat) {
    Bloc buffer;
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    if (buffer.typedebloc != 3) {
        printf("Le premier bloc n'est pas un bloc d'allocation.\n");
        return;
    }

    buffer.content.allocation.tablelocation[blocIndex].etat = etat;
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
}

bool deleteL_OF(FILE* disque, const char* nomFichier) {
    adressemetadonnees adresse = recherchemetadonnees(disque, nomFichier);
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

void viderMemoireSecondaire(FILE* disque) {
    Bloc buffer;

    // Initialize all blocks as unused
    buffer.typedebloc = 0; // Unused block type
    for (int i = 0; i < MAX_BLOCKS; i++) {
        fseek(disque, i * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);
    }

    // Reinitialize the allocation table
    buffer.typedebloc = 3; // Allocation block type
    buffer.content.allocation.nbrbloc = MAX_BLOCKS;
    buffer.content.allocation.nbrblocutil = 1; // Block 0 is used for allocation table

    // Mark block 0 as used (allocation table)
    buffer.content.allocation.tablelocation[0].adrdebloc = 0;
    buffer.content.allocation.tablelocation[0].etat = 1;

    // Mark block 1 as used (metadata block)
    buffer.content.allocation.tablelocation[1].adrdebloc = 1;
    buffer.content.allocation.tablelocation[1].etat = 1;

    // Mark all other blocks as free
    for (int i = 2; i < MAX_BLOCKS; i++) {
        buffer.content.allocation.tablelocation[i].adrdebloc = i;
        buffer.content.allocation.tablelocation[i].etat = 0;
    }

    // Write the allocation table to disk at index 0
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    // Initialize the first metadata block at index 1
    Bloc metadataBlock;
    metadataBlock.typedebloc = 1; // Metadata block type
    metadataBlock.content.metadataTable.nbrMetadonnees = 0;
    metadataBlock.content.metadataTable.next = -1; // No next block initially

    // Write the metadata block to disk at index 1
    fseek(disque, 1 * sizeof(Bloc), SEEK_SET);
    fwrite(&metadataBlock, sizeof(Bloc), 1, disque);

    printf("Vidage de la mémoire secondaire terminé.\n");
}


void compactageMemoireSecondaire(FILE* disque) {
    Bloc buffer;
    Tableallocation tableAllocation[MAX_BLOCKS];
    int usedBlocks[MAX_BLOCKS];
    int usedCount = 0;

    // Read the allocation table
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    if (buffer.typedebloc != 3) {
        printf("Erreur : Le premier bloc n'est pas un bloc d'allocation.\n");
        return;
    }

    // Copy the allocation table
    for (int i = 0; i < MAX_BLOCKS; i++) {
        tableAllocation[i] = buffer.content.allocation.tablelocation[i];
    }

    // Identify used blocks
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (tableAllocation[i].etat == 1) {
            usedBlocks[usedCount++] = i;
        }
    }

    // Compact used blocks to the beginning of the file
    for (int i = 0; i < usedCount; i++) {
        if (usedBlocks[i] != i) {
            // Move the used block to the new position
            fseek(disque, usedBlocks[i] * sizeof(Bloc), SEEK_SET);
            fread(&buffer, sizeof(Bloc), 1, disque);

            fseek(disque, i * sizeof(Bloc), SEEK_SET);
            fwrite(&buffer, sizeof(Bloc), 1, disque);

            // Update the allocation table
            tableAllocation[i].etat = 1;
            tableAllocation[usedBlocks[i]].etat = 0;
        }
    }

    // Write the updated allocation table back to disk
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    for (int i = 0; i < MAX_BLOCKS; i++) {
        buffer.content.allocation.tablelocation[i] = tableAllocation[i];
    }

    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    printf("Compactage de la mémoire secondaire terminé.\n");
}



int main() {
    int choix;
    int modeG, modeI;
    char nomFichier[20], ancienNom[20], nouveaunom[20];
    int ID;

    printf("Program started successfully!\n");

    FILE* disque = fopen("disque.bin", "r+b"); // Open the file for reading/writing
    if (!disque) {
        disque = fopen("disque.bin", "w+b"); // Create the file if it doesn't exist
        if (!disque) {
            printf("Erreur : Impossible de créer le fichier disque.bin.\n");
            return 1;
        }
    }

    do {
        printf("\n--- Gestion de la Memoire Secondaire ---\n");
        printf("1. Initialiser la memoire secondaire\n");
        printf("2. Creer un fichier\n");
        printf("3. Afficher l'etat de la memoire secondaire\n");
        printf("4. Afficher les metadonnees des fichiers\n");
        printf("5. Rechercher un enregistrement\n");
        printf("6. Inserer un nouvel enregistrement\n");
        printf("7. Supprimer un enregistrement\n");
        printf("8. Defragmenter un fichier\n");
        printf("9. Supprimer un fichier\n");
        printf("10. Renommer un fichier\n");
        printf("11. Compacter la memoire secondaire\n");
        printf("12. Vider la mémoire secondaire\n");
        printf("0. Quitter\n");
        printf("Votre choix : ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                initMS(disque, 20); // Initialize the secondary memory
                printf("Memoire secondaire initialisee avec succes.\n");
                break;

            case 2:
                printf("Creation d'un fichier\n");
                printf("Votre choix d'organisation globale (0 pour chainee) : ");
                scanf("%d", &modeG);
                printf("Votre choix d'organisation interne (1 pour non-ordonnee) : ");
                scanf("%d", &modeI);
                if (modeG == 0) {
                    printf("Creation d'un fichier en mode chaine.\n");
                } else if (modeG == 1) {
                    printf("Creation d'un fichier en mode non ordonné.\n");
                } else {
                    printf("Choix d'organisation invalide.\n");
                    break;
                }
                creationL_OF(disque, 1); // Create a file (1 block for simplicity)
                break;

            case 3:
                printf("Affichage de l'etat de la memoire secondaire :\n");
                afficherMemoireSecondaire(disque, MAX_BLOCKS); // Display the state of the secondary memory
                break;

            case 4:
                printf("Affichage des metadonnees des fichiers :\n");
                // Add a function to display metadata (if needed)
                break;

            case 5:
                printf("Recherche d'enregistrement\n");
                printf("Entrez le nom du fichier : ");
                scanf("%s", nomFichier);
                printf("Entrez l'ID de l'enregistrement a rechercher : ");
                scanf("%d", &ID);
                position pos = researchDis(disque, ID, nomFichier);
                if (pos.deplacement != -1) {
                    printf("Enregistrement trouvé dans le bloc %d, position %d\n", pos.numBloc, pos.deplacement);
                } else {
                    printf("Enregistrement non trouvé.\n");
                }
                break;

            case 6:
                printf("Insertion d'enregistrement\n");
                printf("Entrez le nom du fichier : ");
                scanf("%s", nomFichier);
                insertDis(disque, MAX_BLOCKS, nomFichier);
                break;

            case 7:
                printf("Suppression d'enregistrement\n");
                printf("Entrez le nom du fichier : ");
                scanf("%s", nomFichier);
                printf("Entrez l'ID de l'enregistrement à supprimer : ");
                scanf("%d", &ID);
                suppLogique(disque, ID, nomFichier);
                break;

            case 8:
                printf("Defragmentation d'un fichier\n");
                printf("Entrez le nom du fichier a defragmenter : ");
                scanf("%s", nomFichier);
                defregmentation(disque, nomFichier);
                break;

            case 9:
                printf("Suppression de fichier\n");
                printf("Entrez le nom du fichier a supprimer : ");
                scanf("%s", nomFichier);
                deleteL_OF(disque, nomFichier);
                break;

            case 10:
                printf("Renommage de fichier\n");
                printf("Entrez le nom actuel du fichier : ");
                scanf("%s", ancienNom);
                printf("Entrez le nouveau nom du fichier : ");
                scanf("%s", nouveaunom);
                renameFile(disque, ancienNom, nouveaunom);
                break;

            case 11:
                printf("Compactage de la memoire secondaire\n");
                compactageMemoireSecondaire(disque);
                break;

            case 12:
                printf("Vidage de la memoire secondaire\n");
                viderMemoireSecondaire(disque);
                break;

            case 0:
                printf("Programme termine !\n");
                break;

            default:
                printf("Choix invalide. Veuillez réessayer.\n");
                break;
        }
    } while (choix != 0);

    fclose(disque); // Close the disk file
    return 0;
}

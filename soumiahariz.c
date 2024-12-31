#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
typedef struct 
{
  char Nomdufichier[20];
  int Taillefichierblocs;
  int Taillefichierenregistrements;
  int Adrpremierbloc;  //adresse  du 1 bloc 
  int Modeorganisationglobale; // si est =0 alors chaine
  int Modeorganisationinterne;  // si est=0 alors ordone
}fichiermetadonnes;


typedef struct 
{
  int id;               
  char name[15];  
  int age;
  char sexe[10];
  char adresse[30];
  int nmbrdevisite;
  int suprimelogiqument;// 1 si est suprimé logiquement

}maladie;
typedef struct {
    int adrdebloc; // adresse de bloc
    int etat;      // si vide = 0 pleine = 1
} Tableallocation;
typedef struct {
    int nbrblocutil; // nombre de bloc utilise
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
typedef struct 
{
    maladie T[20];
    int nbrmaladie;
    int next;
   
}BlocData;

typedef struct {
    union {
        BlocMetadonnees metadataTable;
        BlocData fileData;
        BlocAllocation allocation;
    } content;
    int typedebloc; // 1 = metadata, 2 = file data, 3 = allocation
} Bloc;

void metajourtableallocation(FILE* disque, int blocIndex, int etat) {
    Bloc buffer;
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    if (buffer.typedebloc != 3) {
        printf("Erreur : Le premier bloc n'est pas un bloc d'allocation.\n");
        return;
    }

    // Update the allocation table entry for the specified blocIndex
    buffer.content.allocation.tablelocation[blocIndex].etat = etat;

    // Write the updated allocation table back to the first block
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
}

void CreeTableAllocation( FILE* disque) {
    Bloc buffer;
    buffer.typedebloc=3;
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    // Write the initial allocation table in MC
    for (int i = 0; i < buffer.content.allocation.ms.nbrbloc; i++) {
        buffer.content.allocation.tablelocation[i].adrdebloc = i;
        buffer.content.allocation.tablelocation[i].etat = 0;
    }

    // Write the initial allocation table to the first block
     fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    // Mark the first block as allocated
    metajourtableallocation(disque, 0, 1);
}

void ViderMs(FILE* disque) {
    Bloc buffer;
    buffer.typedebloc=3;
      fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
      fread(&buffer, sizeof(Bloc), 1, disque);

    buffer.content.allocation.ms.nbrblocutil = 1; // nombre de bloc utilise
   // Write the initial allocation table in MC
    for (int i = 0; i < buffer.content.allocation.ms.nbrbloc; i++) {
        buffer.content.allocation.tablelocation[i].adrdebloc = i;
        buffer.content.allocation.tablelocation[i].etat = 0;
    }

    // Write the initial allocation table to the first block
     fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    // Mark the first block as allocated
    metajourtableallocation(disque, 0, 1);
}

void InitMs( FILE* disque) {
    Bloc buffer;
    buffer.typedebloc=3;
     fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
      fread(&buffer, sizeof(Bloc), 1, disque);

    buffer.content.allocation.ms.nbrbloc = 20;
    buffer.content.allocation.ms.FB = 20; // Nombre maximum d'enregistrements dans un bloc c'est le facteur du blocage
    buffer.content.allocation.ms.nbrblocutil = 1;
    CreeTableAllocation( disque);
    // Write the initial allocation table to the first block
     fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

}

// Main function for testing
int main() {
    MS ms;
    FILE *disque = fopen("filedisque.bin", "wb+");
    if (disque == NULL) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    InitMs( disque);
    printf("Initialisation de MS et création de la table d'allocation.\n");

    ViderMs(disque);
    printf("MS vidé.\n");

    // Test updating the allocation table
    metajourtableallocation(disque, 5, 1); // Mark bloc 5 as used
    printf("Bloc 5 marqué comme utilisé.\n");

    fclose(disque);
    return 0;
}



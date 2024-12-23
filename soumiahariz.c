#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>


#define FB 10 // Nombre maximum d'enregistrements dans un bloc c'est le facteur du blocage

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

typedef struct 
{
    maladie T[FB];
    int nbrmaladie;
    int next;
   
}Bloc;



typedef struct
{
  int adrdebloc; // adresse de bloc
  int etat;      // si vide = 0 pleine = 1
}Tableallocation;

typedef struct 
{
  
  int nbrblocutil; //nombre de bloc utilise
  int nbrbloc;
  Tableallocation tablelocation[30];
}MS;

typedef struct 
{
 
 int numerodebloc;
 int index;

}adressemetadonnes;

typedef struct {
    fichiermetadonnes T[FB]; // Tableau de métadonnées
    int nbrMetadonnees;       // Nombre actuel de métadonnées dans ce bloc
    int next;
} BlocMetadonnees;



typedef struct {
    union {
        BlocMetadonnees metadataTable;
        Bloc fileData;
        Tableallocation allocation;
        
    } content;
    int typedebloc; // 1 = metadata, 2 = file data, 3 = allocation
} Bloc;
//création de table d'allocation
Tableallocation CréeTableAllocation(MS ms) {
    Tableallocation TA;
    malloc T =ms.nbrbloc*sizeof(TA);
for (int i = 0; i < ms.nbrbloc; i++)
{
   TA.adrdebloc=i;
   TA.etat=0;
}
return TA;
}
void InitMs(MS ms){
    Bloc Buffer , TA ;
    ms.nbrbloc=20;
    ms.FB=20;
    ms.tablelocation= CréeTableAllocation(ms);
    for (int i = 0; i < ms.nbrbloc; i++)
    {
        Buffer[i]=TA[i];

    }
    fwrite(&Buffer,sizeof(TA),fileallooc);
}
void ViderMs(fileno*f,MS ms){
Tableallocation Buffer;
fread(&Buffer,sizeof(TA),filealloo);
for (int i = 0; i < ms.nbrbloc; i++)
{
   Buffer.etat=0;
}

}
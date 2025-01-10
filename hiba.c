#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

// Définir une constante pour la taille maximale des blocs
//#define FB 20          // FB : Taille fixe d'un bloc (10 enregistrements par bloc maximum)
//---------------------------------------------LES STRUCTURES NECESSAIRES--------------------------------------------------
#define facteur_blocage 20

typedef struct
{
  char  Nomdufichier[20];
  int Taillefichierblocs;
  int Taillefichierenregistrements;
  int Adrpremierbloc;  //adresse  du 1 bloc
  int Modeorganisationglobale; // si est =0 alors chaine
  int Modeorganisationinterne;  // si est=0 alors ordone
  int metadonneeActuel;// besoin de TNOF
}fichiermetadonnes;


typedef struct
{
  int id;
  char name[15];
  int age;
  char sexe[10];
  char adresse[30];
  int nmbrdevisite;
  int suprimelogiqument; // 1 si est suprimé logiquement

}maladie;

typedef struct
{
    maladie T[facteur_blocage];
    int nbrmaladie;
    int next;

}BlocData;



typedef struct
{
  int adrdebloc; // adresse de bloc
  int etat;
     // si vide = 0 pleine = 1
}Tableallocation;

typedef struct
{

  int nbrblocutil; //nombre de bloc utilise
  int nbrbloc; // Nombre total de blocs
  int fb;

}MS;

typedef struct // BESOIN DE L__F
{

 int numerodebloc;
 int index;

}adressemetadonnes;

typedef struct
 {
    fichiermetadonnes T[facteur_blocage]; // Tableau de métadonnées
    int nbrMetadonnees;       // Nombre actuel de métadonnées dans ce bloc
    int metadonneeActuel;// besoin de TNOF
    int next; // besoin de L__F
} BlocMetadonnees;

typedef struct
 {
    Tableallocation tablelocation[facteur_blocage];
    MS ms;
    int nbrblocutil; //nombre de bloc utilise
    int nbrbloc;
} BlocAllocation;

typedef struct
{
    union {
        BlocMetadonnees metadataTable;
        BlocData fileData;
        BlocAllocation allocation;

    } content;
    int typedebloc; // 1 = metadata, 2 = file data, 3 = allocation
} Bloc;
//-----------------------------------------------------besoin de TNOF-------------------------------------------
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
//-----------------------------trouver le bloc libre pour la creation ----------------
int trouverPremierBlocLibre(FILE *disque) {
    Bloc buffer;
    
    // Lire le bloc d'allocation (premier bloc)
    rewind(disque);
    fread(&buffer, sizeof(Bloc), 1, disque);
    
    if (buffer.typedebloc != 3) {
        printf("Erreur : Le premier bloc n'est pas un bloc d'allocation.\n");
        return -1;
    }
    
    // Parcourir la table d'allocation
    for (int i = 0; i < buffer.content.allocation.ms.nbrbloc; i++) {
        if (buffer.content.allocation.tablelocation[i].etat == 0) {
            // Bloc libre trouvé
            return i;
        }
    }
    
    // Aucun bloc libre trouvé
    printf("Aucun bloc libre n'a été trouvé.\n");
    return -1;
}

// -------------------------------------compactage ------------------------
void compactageDisque(FILE *disque) {
    Bloc buffer, bufferTemp;
    int blocLibre = 0;
    int blocCourant = 0;
    
    // Lire le bloc d'allocation
    fseek(disque, 0, SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);
    
    if (buffer.typedebloc != 3) {
        printf("Erreur : Le premier bloc n'est pas un bloc d'allocation.\n");
        return;
    }
    
    int nbrTotalBlocs = buffer.content.allocation.ms.nbrbloc;
    
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
    buffer.content.allocation.ms.nbrblocutil = blocLibre;
    
    // Réécrire le bloc d'allocation mis à jour
    fseek(disque, 0, SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);
    
    printf("Compactage terminé. %d blocs utilisés après compactage.\n", blocLibre);
}
//==========================================our program======================================
//------------------------------------FONCTIONS INDEPANDANTES-----------------------------
//----------------------------MISE A JOUR DE TABLE D'ALLOCATION -------------------------------

void metajourtableallocation (FILE* disque, int blocIndex, int etat)  {
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
//------------------------------------CREATION DE TABLE D'ALLOCATION--------------------------------
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


//-----------------------------------------------
void afficherEtatMemoire(BlocAllocation *blocAlloc) {
    printf("Etat de la mémoire secondaire (avec des boîtes):\n\n");

    // Afficher les informations globales
    printf("Nombre total de blocs: %d\n", blocAlloc->ms.nbrbloc);
    printf("Nombre de blocs utilisés: %d\n", blocAlloc->ms.nbrblocutil);
    printf("FB (autre info): %d\n\n", blocAlloc->ms.fb);

    // Affichage des blocs avec des boîtes autour de chaque bloc
    for (int i = 0; i < 20; i++) {
        const char* etat = (blocAlloc->tablelocation[i].etat == 0) ? "Vide" : "Plein";

        int largeur = 20 + 2 + 2 * strlen(etat); // pour l'adresse + état, ajustable selon la taille du texte
        printf("Bloc %d:\n", i + 1);
        printf("╔%.*s╗\n", largeur, "====================================="); // ligne du dessus (boîte)
        printf("║ Adresse: %d %*s║\n", blocAlloc->tablelocation[i].adrdebloc, largeur - 15, " "); // Adresse
        printf("║ Etat   : %s %*s║\n", etat, largeur - strlen(etat) - 12, " "); // Etat
        printf("╚%.*s╝\n", largeur, "====================================="); // ligne du dessous (boîte)
        printf("\n"); // Saut de ligne entre chaque bloc
    }
}
//------------------------------fct gestion de l'espace -------------------------
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

//===============================================================================================================

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
 void chargerFileTNOF(FILE *disque  ){

 fichiermetadonnes MT = creeFileTNOF();

  // deffirence with * and without
  int blocVolu  =  MT.Taillefichierenregistrements / 20;
    Bloc buffer;
    maladie malade;
    int cpt=-1;
    int blocNbr;
 
   int nbrB=0;//nombre de bloc
   int nbrE=0;//nombre de enregistrement  nbr de blocs suffisants =nbrElement\tailleBloc
   if (!verifierEspaceSuffisant(disque,blocVolu)) {
    compactageDisque(disque);
   }          
    rewind(disque);
    int indexBloc = trouverPremierBlocLibre(disque);
    fread(&buffer, sizeof(buffer), indexBloc+1 , disque);
    // pour metre le curseur au bonne position 
   int nbrMalade = 0;
  do {

    for(int i=0;i<facteur_blocage;i++) {
    nbrMalade++;
    printf("----------------------------------");
    printf("entrer la reference de malade %d:\n",nbrMalade);
    scanf("%d",&malade.id);
    if(malade.id==-1) break; // si id=-1 alors on arrete de remplir le bloc
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
    
    malade.suprimelogiqument=0;
    buffer.content.fileData.T[i].id =malade.id;//id
    copyString(buffer.content.fileData.T[i].name, malade.name);//name
    copyString(buffer.content.fileData.T[i].sexe,malade.sexe);//sexe
    buffer.content.fileData.T[i].age =malade.age;//age
    buffer.content.fileData.T[i].nmbrdevisite =malade.nmbrdevisite;//nbrvisite
    copyString(buffer.content.fileData.T[i].adresse , malade.adresse);//adress
    buffer.content.fileData.T[i].suprimelogiqument =malade.suprimelogiqument;

     }
       nbrB =nbrB+1;
       buffer.typedebloc=2;
       fwrite(&buffer, sizeof(buffer), 1, disque);
       metajourtableallocation ( disque, nbrB, 1); // 1 est plain
       buffer.content.allocation.nbrbloc++;
      if (cpt==1) blocNbr = buffer.content.allocation.nbrbloc++;
      } while (nbrMalade != MT.Taillefichierenregistrements );


    buffer.content.metadataTable.nbrMetadonnees++;
    buffer.typedebloc=1;
   int indexMeta= buffer.content.metadataTable.nbrMetadonnees-1;
   if (indexMeta==facteur_blocage) { // le cas de table d'allocation est plaine
     indexMeta = 0;
     metajourtableallocation (disque, blocNbr+nbrB , 1);
    };
   buffer.content.metadataTable.T[indexMeta].Taillefichierblocs=nbrB;
   buffer.content.metadataTable.T[indexMeta].Taillefichierenregistrements=nbrE;
   copyString(buffer.content.metadataTable.T[indexMeta].Nomdufichier , MT.Nomdufichier);
   buffer.content.metadataTable.T[indexMeta].Modeorganisationglobale= 1;
   buffer.content.metadataTable.T[indexMeta].Modeorganisationinterne=1;
   buffer.content.metadataTable.T[indexMeta].Adrpremierbloc=blocNbr;
   buffer.content.metadataTable.metadonneeActuel=indexMeta;
   nbrB++;

   fwrite(&buffer, sizeof(buffer), 1, disque);
   buffer.content.allocation.nbrbloc++;
   printf("le fichier est crée avec succès ");
   }
 
//-------------------------------INSERTION TNOF---------------------------------
  void InsertionfileTNOF(FILE *disque  ,maladie newmalade ){
    Bloc buffer;
    rewind(disque);
    fread(&buffer, sizeof(buffer), 1, disque);
    //   les informations de nouveau malade
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
    newmalade.suprimelogiqument=0;

   
    buffer.content.fileData.T[iend+1].id =newmalade.id;//id
    copyString(buffer.content.fileData.T[iend+1].name, newmalade.name);//name
    copyString(buffer.content.fileData.T[iend+1].sexe, newmalade.sexe);//sexe
    buffer.content.fileData.T[iend+1].age =newmalade.age;//age
    buffer.content.fileData.T[iend+1].nmbrdevisite =newmalade.nmbrdevisite;//nbrvisite
    copyString(buffer.content.fileData.T[iend+1].adresse, newmalade.adresse);//adress
    buffer.content.fileData.T[iend+1].suprimelogiqument =newmalade.suprimelogiqument;

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
    buffer.content.fileData.T[i].suprimelogiqument =newmalade.suprimelogiqument;
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
    for(int i=0;i<facteur_blocage;i++) {
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
buffer.content.fileData.T[p.deplacment].suprimelogiqument =1; //1 supprimer logiquement
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
if (dep != facteur_blocage-1) { 
for ( i=dep ; i<facteur_blocage-2; i++){ //decalage des element dans le bloc
    buffer.content.fileData.T[i].id =buffer.content.fileData.T[i+1].id ;//id
    copyString(buffer.content.fileData.T[i].name,buffer.content.fileData.T[i+1].name);//name
    copyString(buffer.content.fileData.T[i].sexe,buffer.content.fileData.T[i+1].sexe);//sexe
    buffer.content.fileData.T[i].age = buffer.content.fileData.T[i+1].age ;//age
    buffer.content.fileData.T[i].nmbrdevisite =buffer.content.fileData.T[i+1].nmbrdevisite;//nbrvisite
    copyString(buffer.content.fileData.T[i].adresse,buffer.content.fileData.T[i+1].adresse);//adress
    buffer.content.fileData.T[i].suprimelogiqument =buffer.content.fileData.T[i+1].suprimelogiqument ;

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
buffer.content.fileData.T[i+1].suprimelogiqument = buffer2.content.fileData.T[j-1].suprimelogiqument ;

int depl=p.deplacment;
if (depl == facteur_blocage){  //-----le cas suppresion de dernier element
int nombreElements = facteur_blocage; 
    // Réduire le nombre d'éléments
    nombreElements--;
 //demunation de taille logique de T
buffer.content.fileData.T[nombreElements].id =buffer2.content.fileData.T[j-1].id ;//id
copyString(buffer.content.fileData.T[nombreElements].name , buffer2.content.fileData.T[j-1].name);//name
copyString(buffer.content.fileData.T[nombreElements].sexe , buffer2.content.fileData.T[j-1].sexe);//sexe
buffer.content.fileData.T[nombreElements].age =buffer2.content.fileData.T[j-1].age ;//age
buffer.content.fileData.T[nombreElements].nmbrdevisite =buffer2.content.fileData.T[j-1].nmbrdevisite ;//nbrvisite
copyString(buffer.content.fileData.T[nombreElements].adresse , buffer2.content.fileData.T[j-1].adresse);//adress
buffer.content.fileData.T[nombreElements].suprimelogiqument = buffer2.content.fileData.T[j-1].suprimelogiqument ;
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
      int metaAct = buffer.content.metadataTable.metadonneeActuel;
      if (buffer.content.metadataTable.T[metaAct].Adrpremierbloc==nbrB)  test = 0;
      if (buffer.content.metadataTable.metadonneeActuel && test == 0 ){
         // on est dans le fichier metadonnee qui corespond aux ce fichier
         buffer.content.metadataTable.T[metaAct].Taillefichierenregistrements--;
        if (blocVide==1) {
           buffer.content.metadataTable.T[metaAct].Taillefichierblocs++;
           metajourtableallocation (disque, nbrB, 0); // bloc sera vide

    }
    }
    }
    };

 printf("---------------------le malade %d est supprimé physiquement---------------------\n", buffer.content.fileData.T[i].id);

}// tout les cas sont traiter normalement

// ---------------------------------------Defragmentation TNOF -----------------------
void defragmentationFileTNOF(FILE *disque ){
  Bloc buffer;
rewind(disque);
while (!feof(disque)){
fread(&buffer,sizeof(buffer),1,disque);
for (int i=0;i<facteur_blocage;i++){
  if (buffer.content.fileData.T[i].suprimelogiqument== 1 || buffer.content.fileData.T[i].id ==-2){
     int supp = buffer.content.fileData.T[i].id ;
    supprPhysiqueFileTNOF(disque , supp );
}
}
}}

 // ---------------------------------------Suppression d'un fichier TNOF  -----------------------
 // -------------------------------------suppression physique-----------------------------


 void supprPhysiqueDeFichierTNOF(FILE *disque , char filename[]){
  // 1 = metadata, 2 = file data, 3 = allocation
 Bloc buffer;
 int adr,taille;
 int nbrB=-1;
 rewind(disque);
 while (!feof(disque)){
  fread(&buffer, sizeof(buffer),1,disque);
  nbrB++;
  if (buffer.typedebloc==1){ // on a effacer les metadonnee de fichier
    for (int i=0;i<facteur_blocage;i++){
      if (strcmp(buffer.content.metadataTable.T[i].Nomdufichier,filename)==0){
         buffer.content.metadataTable.nbrMetadonnees--;
         adr = buffer.content.metadataTable.T[i].Adrpremierbloc ;
         taille = buffer.content.metadataTable.T[i].Taillefichierblocs;
        for (int j=i;j<facteur_blocage;j++){
          buffer.content.metadataTable.T[j]=buffer.content.metadataTable.T[j+1];
        }

      }
    }

  }

  if( buffer.typedebloc == 2 && nbrB == adr){ //on a trouver le fichier a supprimer
   for (int i=0; i<facteur_blocage; i++){
    buffer.content.fileData.T[i].id = -2;//id
    buffer.content.fileData.T[i].suprimelogiqument =1;
   }
  fwrite(&buffer, sizeof(buffer),taille,disque); //tout les blocs de fichier perdent leur contenue
  defragmentationFileTNOF(disque );
  for(int j= nbrB;j<(nbrB+taille);j++) {
  metajourtableallocation (disque, j, 0); // bloc sera vide
  }
  compactageDisque(disque); 
  //========================COMPACTAGE HERE SIS=============================
 //compactage(disque,&buffer.content.allocation.ms);
 printf("---------------------le fichier %s est supprimé physiquement---------------------\n", filename);
   return;
 }

 }

 }


//-----------------------------------------MAIN PROGRAM --------------------------------------


int main() {

    int choix ,choix1;
    int modeG;
    int modeI;
    maladie newmalade;
    int id;
    char filename[30];

     FILE* disque = fopen("disque.bin", "r+b"); // Ouvrir le fichier pour lecture/écriture
     CreeTableAllocation( disque);

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

        switch(choix) {
            case 1:
                printf("Initialisation de la mémoire secondaire\n");
                // Appeler la fonction d'initialisation ici
                break;
            case 2:
                printf("Création d'un fichier\n");
                printf("Votre choix d'organisation globale: ");
                scanf("%d", &modeG); // 0 chainee
                printf("Votre choix d'organisation interne: ");
                scanf("%d", &modeI); // 0 ordonnee
                // mode TNOF
                if (modeG !=0 && modeI !=0 )  chargerFileTNOF(disque);

                // Créer le fichier en fonction des choix d'organisation
                break;
            case 3:
                // Afficher l'état de la mémoire secondaire
                afficherEtatMemoire(NULL); // Exemple d'appel
                break;
            case 4:
                printf("Affichage des métadonnées du fichier :\n");
                // Afficher les métadonnées
                break;
            case 5:
                printf("Recherche d'enregistrement\n");
                // mode TNOF
                printf("entrer la reference de malade que vous voulez inserer ");
                scanf("%d", &id);
                if (modeG !=0 && modeI !=0 )  rechercheFILETNOF(disque ,id);
                break;
            case 6:
                printf("Insertion d'enregistrement\n");
                InsertionfileTNOF(disque  ,newmalade );
                break;
            case 7:
                printf("Suppression d'enregistrement\n");
                printf("entrer votre choix de suppression \n 1.physique \n 2.logique \n ");
                scanf("%d", &choix1);
                 printf("entrer la reference de malade que vous voulez supprimer \n");
                 scanf("%d", &id);
                if (choix1 == 2) supprLogiqueFileTNOF(disque,id);
                if (choix1 == 1) supprPhysiqueFileTNOF(disque,id);

                break;
            case 8:
                printf("Défragmentation effectuée\n");
                break;
            case 9:
                printf("Suppression de fichier\n");
                printf("entrer le nom du fichier que vous voulez supprimer \n");
                scanf("%s",filename ); // nom du fichier
                supprPhysiqueDeFichierTNOF(disque,filename);
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
    
     fclose(disque);
    return 0;
}
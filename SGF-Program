#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>


#define facteur_blocage 20
    typedef struct {
        char Nomdufichier[20];
        int Taillefichierblocs;
        int Taillefichierenregistrements;
        int Adrpremierbloc; // Adresse du premier bloc
        int Modeorganisationglobale; // Si = 0 alors chaîne
        int Modeorganisationinterne; // Si = 0 alors ordonné
         int metadonneeActuel;// besoin de TNOF
    } fichiermetadonnes;

    typedef struct {
        int id;
        char name[15];
        int age;
        char sexe[10];
        char adresse[30];
        int nmbrdevisite;
        int suprimelogiquement; // 1 si supprimé logiquement
        int suprimelogiqument; // 1 si supprimé
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
typedef struct
{

  int nbrblocutil; //nombre de bloc utilise
  int nbrbloc ; // Nombre total de blocs
  int fb;

}MS;
    typedef struct {
        fichiermetadonnes T[20]; // Tableau de métadonnées
        int nbrMetadonnees; // Nombre actuel de métadonnées dans ce bloc
        int next; // Pour chaîner les blocs de métadonnées
        int metadonneeActuel;// besoin de TNOF
    } BlocMetadonnees;

    typedef struct {
        Tableallocation tablelocation[20];
        int nbrblocutil; // Nombre de blocs utilisés
        int nbrbloc; // Nombre total de blocs
        MS ms;
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


//for research 
typedef struct {
    int numBloc;
    int deplacement;
} position;


 // Function to check if there is enough space for the requested number of blocks
    bool verifierEspaceSuffisant(FILE* disque, int nbrBlocsVoulu) {
        Bloc buffer;
        rewind(disque);
        buffer.typedebloc=3;
        fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        if (buffer.typedebloc != 3) {
            printf("Erreur : Le bloc 0 nest pas un bloc dallocation.\n");
            return false;
        }

        int blocsLibres = buffer.content.allocation.nbrbloc - buffer.content.allocation.nbrblocutil;

       if (nbrBlocsVoulu > blocsLibres) {
           printf("Erreur : Espace insuffisant. %d blocs necessaires, %d disponibles.\n", nbrBlocsVoulu,blocsLibres);
           return false;
       }

       printf("Succes : Il y a suffisamment despace. %d blocs disponibles.\n", blocsLibres);
       return true;
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
           printf("Erreur : echec d'ecriture dans le bloc 0.\n");
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
           printf("Erreur : echec decriture dans le bloc 0.\n");
           return;
       }


       // Mark the first block as allocated

        printf("succes");
   }
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
 void chargerFileTNOF(FILE *disque  ){

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
   if (enre > facteur_blocage)
     enre = enre / facteur_blocage + enre % facteur_blocage ;
 
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
    
    malade.suprimelogiqument=0;
    buffer.content.fileData.T[i].id =malade.id;//id
    copyString(buffer.content.fileData.T[i].name, malade.name);//name
    copyString(buffer.content.fileData.T[i].sexe,malade.sexe);//sexe
    buffer.content.fileData.T[i].age =malade.age;//age
    buffer.content.fileData.T[i].nmbrdevisite =malade.nmbrdevisite;//nbrvisite
    copyString(buffer.content.fileData.T[i].adresse , malade.adresse);//adress
    buffer.content.fileData.T[i].suprimelogiqument =malade.suprimelogiqument;
    i++;
    if (i==facteur_blocage-1) i=0;
     
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
   rewind (disque);
   fread(&buffer, sizeof(buffer), 1, disque);
   buffer.content.allocation.ms.nbrbloc++;
   printf("le fichier est crée avec succès ");
   }
 
//-------------------------------INSERTION TNOF---------------------------------
  void InsertionfileTNOF(FILE *disque  ,maladie newmalade ){
    Bloc buffer;
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
           buffer.content.metadataTable.T[metaAct].Taillefichierblocs--;
           metajourtableallocation (disque, nbrB, 0); // bloc sera vide
}}}};
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
        }}}}
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
 printf("---------------------le fichier %s est supprimé physiquement---------------------\n", filename);
   return;
 }

 }

 }
//_____________________________________________________________FIN DE MODE ___________________________________________________
//--------------------------------------------------------------------------------------------------------------------------
   

   // Function to obtain the number of blocks based on the option provided
   int obtenirNombreBlocs(FILE* disque, int option) {
       Bloc buffer;
       buffer.typedebloc=3;
       fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
       fread(&buffer, sizeof(Bloc), 1, disque);

       if (buffer.typedebloc != 3) {
           printf("Erreur : Le bloc 0 nest pas un bloc dallocation.\n");
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

   

   

   // Function to clear memory space by resetting block usage counts and allocation table
   void ViderMs(FILE* disque) {
    Bloc buffer = {0}; // Initialiser un bloc vide (toutes valeurs à 0/null)
    int nombreBlocs = obtenirNombreBlocs(disque, 2); // Obtenir le nombre total de blocs

    // Parcourir tous les blocs et les réinitialiser
    for (int i = 0; i < nombreBlocs; i++) {
        fseek(disque, i * sizeof(Bloc), SEEK_SET);
        if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
            printf("Erreur : Impossible de reinitialiser le bloc %d.\n", i);
            return;
        }
    }

    printf("Tous les blocs ont ete reinitialisés a NULL.\n");
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
               printf("Erreur : echec decriture dans le bloc %d.\n", 1);
               return;
           }

         buffer.content.metadataTable.nbrMetadonnees = 0;
          buffer.content.metadataTable.next = -1;

        fseek(disque, 2 * sizeof(Bloc), SEEK_SET);
           if (fwrite(&buffer, sizeof(Bloc), 1, disque) != 1) {
               printf("Erreur : echec decriture dans le bloc %d.\n", 2);
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

              printf("Metadonnees ajoutees avec succes au bloc %d.\n", blocactuelle);
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
               printf("Erreur : Le bloc %d nest pas un bloc de metadonnees.\n", blocActuel);
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

       printf("Metadonnees non trouvees pour le fichier : %s\n", nomfichier);
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
                printf("Caracteristique non trouvee\n");
                return -1;
        }
   }

   // Function to update metadata after insertion or deletion.
   void miseAJourMetadonnees(FILE* disque,const char* nomFichier,int champ,int nouvelleValeur){
        adressemetadonnes adresse=recherchemetadonnes(disque ,nomFichier );
        rewind(disque);
        if(adresse.numerodebloc==-1){
            printf("Fichier introuvable pour mise a jour des metadonnees.\n");
            return ;
        }

        Bloc buffer ;
        fseek(disque ,adresse.numerodebloc*sizeof(Bloc ),SEEK_SET );

        fread(&buffer,sizeof(Bloc ),1 ,disque );

        if(buffer.typedebloc!=1){
            printf("Erreur : Le bloc trouve ne contient pas de metadonnees.\n");
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


#include<stdio.h>
#include<stdio.h>
#include<string.h>
//-------------------------------------------les structures nesessaires --------------------------------
// FILE STRUCT 
typedef struct element {
  int  id;
char eleName[30];
float price;
int nbr;
char etat[30];
}produit;
// META STRUCT 
 typedef struct Meta_ele{  //typedef
    const char FName[20];
    int nbrBloc;
    int nbrRecord;
    produit *adrele1;//matensaych l'adress 
     char OrgGlobale[10];
    char OrgInterne[12];
 }MetaTNOF;
 // SEARCH STRUCT 
 typedef struct position {
    int blocNbr;
    int deplacment;

  }pos;
//L'UNION C'EST le bloc !!
//a reflichir
 union bloc{
  struct  position pos;
  produit    Tbloc[6];
  struct  Meta_ele MetaTNOF ; 
  // struct tableau d'allocation 
};
//-----------------------CREATION DE FICHIER --------------------------
MetaTNOF creeFileTNOF(FILE *file){
 MetaTNOF MT;
 // int FB;
  //char filename[40];
  //char OrgInterne[12];
 // char OrgGlobale[10];
printf("entrer le nom de fichier");
scanf("%s",&MT.FName);
printf("entrer le nombres d'enregistrements ");
scanf("%d",&MT.nbrRecord); 
printf("entrer le mode d'organaisation interne");
scanf("%s",&MT.OrgInterne);
printf("entrer le mode d'organaisation globale");
scanf("%s",&MT.OrgGlobale);
return MT;
}
//-------------------------CHARGEMENT DE FICHIER ----------------------------------
// fb nbr maximum d'enregistrement dans un bloc 
 void chargerFileTNOF(FILE *file ,char *filename[30]  ){ 
  MetaTNOF MT = creeFileTNOF(file);
  // deffirence with * and without
   struct element produit;
   union bloc buffer;
   //struct Meta_ele MetaTNOF;
   int nbrB=0;//nombre de bloc
   int nbrE=0;//nombre de enregistrement
   //condition if there is spase /fonction predifinie fct gestion d'espace if memoire plaine = true then break 
 int i=0;
 file = fopen(filename,"w");
  do { 
        
    for(int i=0;i<6;i++) {    
    printf("entrer la reference :\n");
    scanf("%d",&produit.id);
    if (produit.id!=-1) break;  // si id=-1 alors on arrete de remplir le bloc
    printf("entrer le nom :\n");
    scanf("%s",&produit.eleName);
    printf("entrer le prix :\n");
    scanf("%f",&produit.price);
    printf("entrer le nombre :\n");
    scanf("%d",&produit.nbr);
    printf("entrer l'etat :\n");
    scanf("%s",&produit.etat);
    buffer.Tbloc[i].id =produit.id;
    buffer.Tbloc[i].eleName =produit.eleName;
    buffer.Tbloc[i].price =produit.price;
    buffer.Tbloc[i].nbr =produit.nbr;
    buffer.Tbloc[i].etat =produit.etat;
     }
      nbrB =nbrB+1;
      fwrite(&buffer, sizeof(buffer), 1, file);
      } while (produit.id!=-1 );
  
    
  fclose(file);
   
    const char *f=filename;//recuperer l'adress de premier enregistrement 
    file = fopen(f,"r");
    rewind(f);
    f=(void *)file;
    fclose(file);
   
   buffer.MetaTNOF.FName = filename;
   buffer.MetaTNOF.nbrBloc = nbrB;
   buffer.MetaTNOF.nbrRecord=nbrE;
   buffer.MetaTNOF.adrele1=f;
   buffer.MetaTONF.OrgGlobale="continue";
   buffer.MetaTNOF.OrgInterne="non ordonee";
    char bigMeta [25];
    strcpy(bigMeta , filename);
    strcat(bigMeta,"Meta");
   FILE *fileMeta = fopen(bigMeta,"w");
   fwrite(&buffer, sizeof(buffer), 1, fileMeta);
   fclose(fileMeta);
 }
//-------------------------------INSERTION ---------------------------------
  void InsertionfileTNOF(FILE *file ,char *filename[30] ,produit pr ){
    union bloc buffer;
    char bigMeta [25];
     FILE *f = fopen(filename,"a+");
    const char *f=filename; // to cheak later 
    f=(void *)filename;
   FILE *fileMeta =fopen(bigMeta,"r");
   fread(&buffer, sizeof(buffer), 1, bigMeta);
    int iend =buffer.MetaTNOF.nbrRecord;
    if (iend/ buffer.MetaTNOF.nbrBloc!=0){   // il y a espace 
    fseek(f,0,SEEK_END);
    //fseek(f, -sizeof(produit), SEEK_END); to cheak later
     //fread(&buffer, sizeof(buffer), 1, filename);
      
    buffer.Tbloc[iend+1].id = pr.id;
    buffer.Tbloc[iend+1].eleName = pr.eleName;
    buffer.Tbloc[iend+1].price = pr.price;
    buffer.Tbloc[iend+1].nbr = pr.nbr;
    buffer.Tbloc[iend+1].etat = pr.etat;
    fwrite(&buffer, sizeof(buffer), 1, f); 
    
    } else {  int i=0; // il n 'y pas d'espace 
    buffer.Tbloc[i].id = pr.id;
    buffer.Tbloc[i].eleName = pr.eleName;
    buffer.Tbloc[i].price = pr.price;
    buffer.Tbloc[i].nbr = pr.nbr;
    buffer.Tbloc[i].etat = pr.etat;
    // besoin de table d'allocation 
    // -----------------------------------les cas possibles :
    //1.bloc prochain plain --> decalage 
    //2.bloc prochaine est vide --> slekna 
    } 
    // tu n'a pas traiter le cas de le fichier est plain !!!
    fclose(f);
    buffer.MetaTNOF.nbrRecord = buffer.MetaTNOF.nbrRecord+1;
    char bigMeta [25];
    FILE *fileMeta = fopen(bigMeta,"a+");
    fwrite(&buffer, sizeof(buffer), 1, fileMeta);   // to cheak later !!!
    fclose(fileMeta);
  }
 //---------------------------------------LA RECHERCHE -----------------------
  pos rechercheFILETNOF(FILE *file ,char filename[30], int id){
   //char filename[20]; 
   union bloc buffer;
   int nbrB=0;
   FILE *file = fopen(filename,"r");
   //int position =1;
   while ((fread(&buffer, sizeof(buffer), 1, file))) {
     nbrB=nbrB++;
    for(int i=0;i<6;i++) {    
    if(buffer.Tbloc[i].id == id) {
        buffer.pos.blocNbr=nbrB;
        buffer.pos.deplacment=i;
        fclose(file);
        return buffer.pos;
    }
    }
    }
    fclose(file);
   printf("l'element n'existe pas dans le fichier");
    buffer.pos.blocNbr=-1;
    buffer.pos.deplacment=-1;
    return buffer.pos;
  }
  // LE CAS DE NOMBRE DE BLOC A REFLICHIR !! LE NBR DE 1ER BLOC N'EST PAS TOUJOURS 1

// -------------------------------------LA SUPPRESSION -------------------------------
 //------------------------------------------------------n'oublie pas le fichier de meta donnee 
// 1 ----------------SUPPRESSION LOGIQUE
void supprLogiqueFileTNOF(FILE*file ,char filename[30], int idSuppr){
union bloc buffer;
int pt=-1;
pos p = rechercheFILETNOF(file,filename,idSuppr);
if (buffer.pos.blocNbr==-1 && buffer.pos.deplacment==-1){
printf("ERROR");
}
FILE *file = fopen(filename,"a+");
rewind(file);
while (pt!=buffer.pos.blocNbr){
fread(&buffer,sizeof(buffer),1,file);
pt=pt+1;
}
buffer.Tbloc[buffer.pos.deplacment].id=-1;
fwrite(&buffer,sizeof(buffer),1,file);

}

// 2 ----------------SUPPRESSION PHYSIQUE
void supprPhysiqueFileTNOF(FILE*file ,char filename[30] ,int idSuppr){
union bloc buffer;
produit pr;
int pt=-1;
pos p = rechercheFILETNOF(file,filename,idSuppr);
if (buffer.pos.blocNbr==-1 && buffer.pos.deplacment==-1){
printf("ERROR");
}
FILE *file = fopen(filename,"a+");
rewind(file);
do{
fread(&buffer,sizeof(buffer),1,file);
pt=pt+1;
}while (pt!=buffer.pos.blocNbr);
// decalage intraBloc 
int dep = buffer.pos.deplacment;
for (int i=dep ; i<5 ; i++){
buffer.Tbloc[i]=buffer.Tbloc[i+1];
}

// decalage intreBloc 
fseek(file, -1,SEEK_END);
fread(&buffer,sizeof(buffer),1,file);
int i=0;
while(buffer.Tbloc[i].id!='\0'){
  i=i++;
}

pr.eleName[30]=buffer.Tbloc[i].eleName[30];
pr.price=buffer.Tbloc[i].price;
pr.id=buffer.Tbloc[i].id;
pr.etat[30]=buffer.Tbloc[i].etat[30];
pr.nbr=buffer.Tbloc[i].nbr;
rewind(file);
pt=-1;
do{ //on a dans la position qu'on veut 
fread(&buffer,sizeof(buffer),1,file);
pt=pt+1;
}while (pt!=buffer.pos.blocNbr);
buffer.Tbloc[6]=pr;
fwrite(&buffer,sizeof(buffer),1,file);

fclose(file);
}// tout les cas sont traiter normalement 

// ---------------------------------------Defragmentation  -----------------------
void defragmentationFileTNOF(FILE *file , char filename[30]){
  union bloc buffer;
  FILE *file = fopen(filename,"a");
while (!feof(file)){
fread(&buffer,sizeof(buffer),1,file);
for (int i=0;i<6;i++){
  if (buffer.Tbloc[i].id==-1){
    supprPhysiqueFileTNOF(file , filename, -1);
}
}
}
fclose(file);
 }
// modifications later '-' 
 int main(){
  //---dans main on met un compteur de la supprision logique 
    char filename[20];
    //FILE *file = fopen();
//creeFileTNOF(file);
   printf("enter file name  \n") ;
   scanf("%s",&filename);
   return 0;
 }
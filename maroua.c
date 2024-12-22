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

bool ajoutermetadonnes(FILE*disque,fichiermetadonnes metadonnes,int taille)
{
Bloc buffer;

int blocactuelle=1;
int blocprecedent=1;
MS*ms;
bool allouer;

if((ms->nbrblocutil+taille)>(ms->nbrbloc))// calcuer si il ya un espace suffisant pour ajouter le fichier 
{
    printf("espace insufisant pour creer le fichier ");
    return false;
}

Metajourtaballocation(disque,1,1);// reserver le 1 bloc pour meta donnes

while(blocactuelle!=-1)
{
  fseek(disque,blocactuelle*sizeof(Bloc),SEEK_SET);
  fread(&buffer,sizeof(Bloc),1,disque);

  if(buffer.content.metadataTable.nbrMetadonnees<FB)
  {
    buffer.content.metadataTable.T[buffer.content.metadataTable.nbrMetadonnees]=metadonnes;
    buffer.content.metadataTable.nbrMetadonnees++;

    fseek(disque,blocactuelle*sizeof(Bloc),SEEK_SET);
    fwrite(&buffer,sizeof(Bloc),1,disque);

    allouer=false;
    return true;
  }
  else
  {
     blocprecedent=blocactuelle;
     blocactuelle=buffer.content.metadataTable.next;

  }
}

if(allouer)
{

if((ms->nbrbloc)==(ms->nbrblocutil)) // comparer nbr de bloc de fichier a nbr de bloc vide dans ms 
{
   printf("warning : Espace insuffisant pour stoker le fichier !! \n");
   return false ;//quiter la fonction pas d'espace
}

// resrver un bloc pour fichier metadonnes 
int place=1 ;
for(int i=0;i<ms->nbrbloc;i++)
{
    if(ms->tablelocation[i].etat==0)
    {
       place = i; // la place vide est le bloc  ou en va stoker les metadonnes
       break;
    }
    

}

Metajourtaballocation(disque,place,1);

//faire le chainage

fseek(disque,blocprecedent*sizeof(Bloc),SEEK_SET);// pour charger le bloc derniere 
fread(&buffer,sizeof(Bloc),1,disque);

buffer.content.metadataTable.next=place;

fseek(disque,blocprecedent*sizeof(Bloc),SEEK_SET); 
fwrite(&buffer,sizeof(Bloc),1,disque);

// inserer le nouvelle eng dans le bloc allouer

fseek(disque,place*sizeof(Bloc),SEEK_SET);// pour charger le bloc derniere 
fread(&buffer,sizeof(Bloc),1,disque);

buffer.content.metadataTable.T[0]=metadonnes;
buffer.content.metadataTable.nbrMetadonnees++;
buffer.content.metadataTable.next=-1;

fseek(disque,place*sizeof(Bloc),SEEK_SET);// pour charger le bloc derniere 
fwrite(&buffer,sizeof(Bloc),1,disque);

return true;



}


return false;
}







adressemetadonnes recherchemetadonnes(FILE*disque,const char* nomfichier)
{

Bloc buffer;
adressemetadonnes resultat = {-1, -1};  // Initialisation a -1 pour indiquer non trouvé

// chercher l'adresse de metadonnes dans les bloc 2 et 3

for(int i =2;i<=3;i++)
{
fseek(disque,i*sizeof(Bloc),SEEK_SET);
fread(&buffer,sizeof(Bloc),1,disque);

if(buffer.typedebloc == 1) // pour assurer que se bloc contient metadonnes 
{

  for(int j=0;j<FB;j++)
  {
     if(strcmp(buffer.content.metadataTable.T[j].Nomdufichier, nomfichier) == 0)// comparer si le nom de fichier courant c'est le meme que je cherche 
     {
       resultat.index=j;
       resultat.numerodebloc=i;
       
       return resultat;

     }
      

  }


}



}


  printf("le fichier est introvable");
  return resultat;


}


int liremetadonnes(FILE*disque,const char* nomFichier, int caracteristique )
{


Bloc buffer;
adressemetadonnes adresse=recherchemetadonnes(disque,nomFichier);

if(adresse.numerodebloc==-1) // verifier si le fichier exist 

{
   printf("fichier introuvable");
   

}

 // charger le bloc contenant les métadonnees

fseek(disque,adresse.numerodebloc*sizeof(Bloc),SEEK_SET);
fread(&buffer,sizeof(Bloc),1,disque);

switch (caracteristique)
  {
        case 1:  // taille en blocs
            return buffer.content.metadataTable.T[adresse.index].Taillefichierblocs;
        case 2:  // taille en enregistrements
            return buffer.content.metadataTable.T[adresse.index].Taillefichierenregistrements;
        case 3:  // adresse du premier bloc
            return buffer.content.metadataTable.T[adresse.index].Adrpremierbloc;
        case 4:  // mode d'organisation globale
            return buffer.content.metadataTable.T[adresse.index].Modeorganisationglobale;
        case 5:  // mode d'organisation interne
            return buffer.content.metadataTable.T[adresse.index].Modeorganisationinterne;
        case 6:
            return buffer.content.metadataTable.T[adresse.index].Nomdufichier;
       
        default:
            printf("Caracteristique non trouvé\n");
            return -1;
    }
}

// fonction pour mise a jour les metadonnes apres insertion ou supression 

void miseAJourMetadonnees(FILE* disque, const char* nomFichier, int champ, int nouvelleValeur) {
    //  recherche l'adresse  des métadonnées de ce fichier 
    adressemetadonnes adresse = recherchemetadonnes(disque, nomFichier);

// verification si le fichier exist
    if (adresse.numerodebloc == -1) {
        printf("fichier introuvable.\n");
        return;
    }

 // charger le bloc contenant les métadonnées dans buffer 
    Bloc buffer;
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);


//verification si ce bloc stoké des metadennes
    if (buffer.typedebloc != 1) {
        printf("Erreur : Le bloc trouvé ne contient pas de métadonnées.\n");
        return;
    }


    switch (champ) {
        case 1:  // taille en blocs
            buffer.content.metadataTable.T[adresse.index].Taillefichierblocs = nouvelleValeur;
            break;
        case 2:  // taille en enregistrements
            buffer.content.metadataTable.T[adresse.index].Taillefichierenregistrements = nouvelleValeur;
            break;
        case 3:  // adresse du premier bloc
            buffer.content.metadataTable.T[adresse.index].Adrpremierbloc = nouvelleValeur;
            break;
        case 4:  // mode d'organisation globale
            buffer.content.metadataTable.T[adresse.index].Modeorganisationglobale = nouvelleValeur;
            break;
        case 5:  // mode d'organisation interne
            buffer.content.metadataTable.T[adresse.index].Modeorganisationinterne = nouvelleValeur;
            break;
        
        default:
            printf("Champ non valide.\n");
            return;
    }

    // ecrire les modification 
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

   
}


void creerfichierCO(FILE*disque,MS*ms,int nbrbloc)
{

// nbrbloc c'est le nombre de bloc entré par utilisateur 
  Bloc buffer; 
  fichiermetadonnes*metadonnes;
  
// function pour creer le fichier metadones d'un fichier 
// lire les information du fichier 
printf("Donner le nom du fichier : \n");
scanf("%s",metadonnes->Nomdufichier);

printf("Donner la taille de fichier en enregistrement : \n");
scanf("%d",metadonnes->Taillefichierenregistrements);

printf("Donner le mode d'organisation globale : \n");
scanf("%d",metadonnes->Modeorganisationglobale);

printf("Donner le mode d'organisation interne : \n");
scanf("%d",metadonnes->Modeorganisationinterne);

 int taille = ceil((double)metadonnes->Taillefichierenregistrements / FB)+1; //calculer le nombre de bloc qui stoke les enregistrement de fichier
 metadonnes->Taillefichierblocs=taille;

 if((ms->nbrblocutil+taille)<ms->nbrbloc)
 {
     printf("espace insufisant");

 }

bool succes;
succes =ajoutermetadonnes(disque,*metadonnes,taille);

if(succes)
{
printf("le fichier a été creer avec succes");
}
else 
{printf("espace insufisant");}
return;
}


void chargerfichier(FILE* disque, MS* ms, const char* nomFichier) {
    Bloc buffer;

    // Récupérer les métadonnées du fichier
    fichiermetadonnes metadonnes;
    adressemetadonnes adresse = recherchemetadonnes(disque, nomFichier);

    if (adresse.numerodebloc == -1) {
        printf("Fichier introuvable \n");
        return;  // Le fichier n'a pas été trouvé
    }

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

    // Lire la table d'allocation (stockée dans le premier bloc)
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    if (buffer.typedebloc != 3) {
        printf("Le bloc d'allocation n'est pas valide.\n");
        return;
    }

    // Allouer les blocs nécessaires
    for (int i = 0; i < ms->nbrbloc && bloctrouve < blocnecessaire; i++) {
        if (ms->tablelocation[i].etat == 0) {  // Si le bloc est vide
            metajourtableallocation(disque, i, 1);  // Marquer le bloc comme utilisé

            // Enregistrer l'adresse du premier bloc alloué
            if (adrPremierBloc == -1) {
                adrPremierBloc = i;
            }

            // Mise à jour des métadonnées
            miseAJourMetadonnees(disque, nomFichier, 3, i);

            // Chaînage des blocs : mise à jour du champ "next" du bloc précédent
            if (adrPremierBloc != -1 && i != adrPremierBloc) {
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

    if (bloctrouve < blocnecessaire) {
        printf("Espace insuffisant pour allouer tous les blocs nécessaires.\n");
        return;
    }

    printf("Fichier chargé avec succès.\n");
}




void insertionenregistrement(FILE*disque,MS*ms,fichiermetadonnes*metadonnes,const char* nomFichier)
{

Bloc buffer;
maladie m; // maladie  qui en veux inserer
maladie enrdecale;// l'enregistrement qui va decalé vers le bloc suivant est aussi le derniere enregistrement dans le bloc ou on a trouver la position 
maladie enr;// variable qui va engistré l'enregistrement qui va changer du bloc
Bloc adressedubloc;//pour lire l'adresse de bloc 
int blocdernier;//adresse de derniere bloc
int blocactuelle;

adressemetadonnes adresse = recherchemetadonnes(disque, nomFichier);

int debut=liremetadonnes(disque,nomFichier,3);// adresse de 1 bloc 

int nbrenregistrement=liremetadonnes(disque,nomFichier,2);// nombre total de enregistrement dans ce bloc 

int nbrbloc=liremetadonnes(disque,nomFichier,1);// nombre total de bloc 



defregmentation();// pour recuperer les espaces inutiliser

// mtnsaych compactage 

// verifier si il ya un espace pour un enregistrement  sinon on alouer un nouveaux bloc 

bool allouer;
if(nbrenregistrement-nbrbloc*FB==0)
{
  allouer=true;
}
else 
{
  allouer=false;
}

// verifier si il ya un decalge (insertion dans 1 bloc
bool decalage;


 blocdernier = debut;

while(buffer.content.fileData.next!=-1)
{
   
   fseek(disque, blocdernier * sizeof(Bloc), SEEK_SET);// charger le bloc pour pointé sur bloc suivant 
   fread(&buffer, sizeof(Bloc), 1, disque); 
   blocdernier=buffer.content.fileData.next;

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
m.suprimelogiqument = 0;

//  cas 1 :si l'enregistrement pour inserer est le  1 enregistrement dans le fichier 

if(nbrenregistrement==0)
{
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

 blocactuelle=debut;// pour passer a bloc suivant 

int position=-1;//pour insertion a la fin si en ignore les cas de decalage 

bool positiontrouve=false;

// parcourir les bloc 

while( (blocactuelle!=-1)  &&  (!positiontrouve) )// boucle pour chercher la position
{

fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
fread(&buffer, sizeof(Bloc), 1, disque);

for(int i=0;i<buffer.content.fileData.nbrmaladie-1;i++)
{
 
 // comparer les id pour trouver la position 

 if(m.id<buffer.content.fileData.T[i].id)
 {
   position=i;
   positiontrouve=true;
   break;

 }
}

 if (positiontrouve==false)
 {blocactuelle=buffer.content.fileData.next;}


}

//verifier si adresse de l'enregistrement est dans le bloc derniere 

if(blocdernier==blocactuelle)
{
    decalage=false;

}
else{decalage=true;}

// cas sans decalge sans allocation 
 if (!decalage && !allouer) {
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

        // Mise à jour des métadonnées
        miseAJourMetadonnees(disque, nomFichier, 2, nbrenregistrement + 1);
        return;
    }

// cas insertion  (dans le derniere bloc ) est avec allocation



if(allouer==true)

{
int place=1 ;
for(int i=0;i<ms->nbrbloc;i++)
{
    if(ms->tablelocation[i].etat==0)
    {
       place = i; // la place vide est le bloc  ou en va inserer le noveux enregistrement
       break;
    }
}

// met a jour table d'allocation 

Metajourtaballocation(ms,place,1);

if(decalage==false)
{
fseek(disque,blocdernier* sizeof(Bloc), SEEK_SET); // charger le bloc suivant pour faire le decalage 
fread(&buffer, sizeof(Bloc), 1, disque);

// verifier si le eng a inserer est superieure a tous les eng
if(buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1].id<m.id)
{
  

buffer.content.fileData.next=place;// mise a jour le chainage

// inserer le eng 

fseek(disque,place* sizeof(Bloc), SEEK_SET);  
fread(&buffer, sizeof(Bloc), 1, disque);

buffer.content.fileData.T[0]=m;
buffer.content.fileData.nbrmaladie++;

miseAJourMetadonnees( disque,nomFichier, 2, nbrenregistrement++);
miseAJourMetadonnees( disque,nomFichier, 3, nbrbloc++);

ms->nbrblocutil++; //mise a jour nombre de bloc

return;

}
}
// insertion dans bloc derniere avec allocation est avec decalage dans le derniere bloc

else if(decalage==true&&allouer==true)
{
// inserer le derniere enrg dans le dernier bloc dans le bloc allouer 
 
fseek(disque,blocdernier* sizeof(Bloc), SEEK_SET);  
fread(&buffer, sizeof(Bloc), 1, disque);

enrdecale=buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1];

for(int j=FB-1;j>position;j--) // fb-1 car le 1 index array est 0
{

buffer.content.fileData.T[j] = buffer.content.fileData.T[j - 1]; // pour vider l'espace de position 

}

buffer.content.fileData.T[0]=m;

// ecrire les changement 

fseek(disque,blocdernier* sizeof(Bloc), SEEK_SET);  
fwrite(&buffer, sizeof(Bloc), 1, disque);

// inserer le eng decalé dans le bloc allouer

fseek(disque,place* sizeof(Bloc), SEEK_SET);  
fread(&buffer, sizeof(Bloc), 1, disque);

buffer.content.fileData.T[0]=enrdecale;
buffer.content.fileData.nbrmaladie++;

fseek(disque,place* sizeof(Bloc), SEEK_SET);  
fwrite(&buffer, sizeof(Bloc), 1, disque);

return;
}} // fin de allocation 

// cas de insertion dans derniere bloc sans allocation
if(decalage==false&&allouer==false)
{

fseek(disque,blocdernier* sizeof(Bloc), SEEK_SET);  
fread(&buffer, sizeof(Bloc), 1, disque);

for(int j=FB-1;j>position;j--) // fb-1 car le 1 index array est 0
{

buffer.content.fileData.T[j] = buffer.content.fileData.T[j - 1]; // pour vider l'espace de position 

}

buffer.content.fileData.T[0]=m;

// ecrire les changement 

fseek(disque,blocdernier* sizeof(Bloc), SEEK_SET);  
fwrite(&buffer, sizeof(Bloc), 1, disque);


return;

}
// cas avec decalage

if(decalage==true)
{

fseek(disque,blocactuelle* sizeof(Bloc), SEEK_SET); // charger le bloc suivant pour faire le decalage 
fread(&buffer, sizeof(Bloc), 1, disque);
// buffer est chargé le bloc ou en va inserer

 enrdecale=buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1];// l'enregistrement qui va decalé vers le bloc suivant est aussi le derniere enregistrement dans le bloc

//decaler les enregistrement dans  bloc ou en a trouver la position  

for(int j=FB-1;j>position;j--) // fb-1 car le 1 index array est 0
{

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


while(blocactuelle!=-1) 
{

 fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET); // charger le bloc suivant pour faire le decalage 
 fread(&buffer, sizeof(Bloc), 1, disque);

enr=buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1];// avant decalge en engistré le eng qui va sauter

  for(int i=FB-1;i>0;i--) // decaler pour vider la 1 case 
  {
    
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


// verifier si quand en fait un decalage il faut allouer un nouveux bloc  pour stoker l'enregistrement qui a decaler
if(allouer==true)
{

// cas : allouer un nouveaux bloc apres le decalage 

// voir si il ya un  espace pour allouer un noveaux bloc 

if (ms->nbrblocutil >= ms->nbrbloc) // Pas de blocs libres
// comparer si il ya un bloc   vide  
{
   printf(" Espace insuffisant pour insere l'enregistrement  !! \n"); // le cas de pas de espace dans la ms
   return;//quiter la fonction pas d'espace
}

  // alouer un nouveux bloc 
int place=1 ;
for(int i=0;i<ms->nbrbloc;i++)
{
    if(ms->tablelocation[i].etat==0)
    {
       place = i; // la place vide est le bloc  ou en va inserer le noveux enregistrement
       break;
    }
}

// met a jour table d'allocation 

Metajourtaballocation(ms,place,1);

// met a jour le chainage pour ajouter le noveaux bloc 

fseek(disque, blocactuelle* sizeof(Bloc), SEEK_SET);
fread(&buffer, sizeof(Bloc), 1, disque);

buffer.content.fileData.next=place;

enr=buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1];




// ecrire les changement

fseek(disque, blocactuelle* sizeof(Bloc), SEEK_SET);
fwrite(&buffer, sizeof(Bloc), 1, disque);

// charger le noveux bloc dans buffer pour stoker le nouveux enregistrement

fseek(disque, place * sizeof(Bloc), SEEK_SET);
fread(&buffer, sizeof(Bloc), 1, disque);

buffer.content.fileData.T[0]=enr;// ecrire le eng qui a sauter dans bloc allouer est qui a ete le derniere eng dans le fichier 
// ecrire les changement 

fseek(disque, place * sizeof(Bloc), SEEK_SET);
fwrite(&buffer, sizeof(Bloc), 1, disque); 

miseAJourMetadonnees( disque,nomFichier, 3, nbrbloc++);
ms->nbrbloc++;

}

return; // sortir car l 'insertion est faite
printf("Espace insuffisant pour insérer un enregistrement.\n");
}}


adressemetadonnes rechercheebregistrement(FILE*disque,const char*nomFichier,int ID)
{
 
 adressemetadonnes adressetrouvé;//adress d'enregistrement trouvé

 Bloc buffer; 

 buffer.typedebloc=2;

 int blocactuelle;// pour parcourir les blocs 

 bool find;

 adressemetadonnes adress=recherchemetadonnes(disque,nomFichier);

int blocactuelle=liremetadonnes(disque,nomFichier,3);// adresse de 1 bloc 

int nbrenregistrement=liremetadonnes(disque,nomFichier,2);// nombre total de enregistrement dans ce fichier

int nbrbloc=liremetadonnes(disque,nomFichier,1);// nombre total de bloc 

for(int i=blocactuelle;i<nbrbloc;i++)
{
   fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);// charger le bloc pur lire le tableau
   fread(&buffer, sizeof(Bloc), 1, disque);

   for(int j=0;j<buffer.content.fileData.nbrmaladie;j++)
   {

       if (buffer.content.fileData.T[j].id==ID)
       {

             adressetrouvé.numerodebloc=i;
             adressetrouvé.index=j;
             find=true;

             return adressetrouvé;

       }

   }

   blocactuelle=buffer.content.fileData.next;


}

if(find=false)
{
    printf("l’enregistrement recherché n’existe pas.");
}


}


void  suprimerenregistrement(FILE*disque,MS*ms,const char*nomFichier,int ID)
{
  
     Bloc buffer;
    int choixSuppression = 0;
    int dernierbloc;
    bool decalage;// pour voir si il ya un decalage des bloc 
    bool blocvide;//si apres decalage le dernier bloc est vide
    int blocactuelle;
    int blocprecedent;//pour la supression avec decalge pour revenir dans le decalage
    maladie m;// pour faire engistré les enregistrement qui va changer apres decalage

    adressemetadonnes adresse=rechercheebregistrement(disque,nomFichier,ID);
    adressemetadonnes meta=recherchemetadonnes(disque,nomFichier);//recuperer l'adresse de metadonnes pour met a jour
    int nbrenregistrement=liremetadonnes(disque,nomFichier,2);// nombre total de enregistrement dans ce fichier
    int nbrbloc=liremetadonnes(disque,nomFichier,1);// nombre total de bloc
    int debut=liremetadonnes(disque,nomFichier,3);// adresse de 1 bloc  


     // repeter jusqua le choix est valide
    do {
        
        printf("Quel type de suppression voulez-vous effectuer ?\n");
        printf("1. Suppression logique\n");
        printf("2. Suppression physique\n");
        printf("Entrez votre choix (1 ou 2) : ");

        // lire le choix de user
        scanf("%d", &choixSuppression);

        // verifier si est valide
        if (choixSuppression != 1 && choixSuppression != 2) {
            printf("Choix invalide. Veuillez entrer 1 pour suppression logique ou 2 pour suppression physique.\n");
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
    
fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);// charger le 1 bloc pour chercher l'adresse de derniere bloc
fread(&buffer, sizeof(Bloc), 1, disque);

buffer.content.fileData.T[adresse.index].suprimelogiqument=1;

fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);// engistré les changement 
fwrite(&buffer, sizeof(Bloc), 1, disque);


}
else // supression phisique 
{

// chercher l'adresse de derniere bloc dans le fichier  pour voir si il ya un decalage est si apres supression le bloc sera vide 

int dernierbloc=liremetadonnes(disque,nomFichier,3);// adresse de 1 bloc 

while(dernierbloc!=-1)
{
   fseek(disque, dernierbloc * sizeof(Bloc), SEEK_SET);// charger le bloc pour pointé sur bloc suivant 
   fread(&buffer, sizeof(Bloc), 1, disque); 
   dernierbloc=buffer.content.fileData.next;

}

//verifier si adresse de l'enregistrement est dans le bloc derniere 

if(dernierbloc==adresse.numerodebloc)
{
    decalage=false;

}
else{decalage=true;}

// verifier si apres la suppresion le dernier bloc devient vide

if((buffer.content.fileData.nbrmaladie-1)==0)
{

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
    
    
     
// cas 2 : apres supression dans le derniere bloc il devient vide 

if(blocvide==true && decalage==false)
{
    fseek(disque,  debut* sizeof(Bloc), SEEK_SET);// charger le 1 bloc pour charché l'adresse de l'avant derniere bloc
    fread(&buffer, sizeof(Bloc), 1, disque);

    // boucle pour pointé sur l'avant derniere bloc pour changer le chainage

   while(buffer.content.fileData.next!=-1)
   {

    fseek(disque,  debut* sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    debut=buffer.content.fileData.next;

   }

   buffer.content.fileData.next=-1;// buffer est charger l'avant dernier bloc en mais ce bloc le dernier
    fseek(disque, debut * sizeof(Bloc), SEEK_SET); // engistré les modification apres changement de chainage 
    fwrite(&buffer, sizeof(Bloc), 1, disque);
    miseAJourMetadonnees(disque, nomFichier, 2,nbrbloc--);//mise a jour taille en bloc 
    Metajourtaballocation(ms,dernierbloc,0);// met le bloc suprimé vide dans table d'allocation 
   

}
return;
}

//cas 3 : avec decalage  

if(decalage==true )
{

fseek(disque,  adresse.numerodebloc* sizeof(Bloc), SEEK_SET);// charger le bloc ou on va suprimé 
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
if (blocvide==true && decalage==true)// changer chainage car le bloc est vide  
{

// buffer et charge le bloc avant derniere

buffer.content.fileData.next=-1;// psq je suis dans l'avant derniere

fseek(disque, blocprecedent * sizeof(Bloc), SEEK_SET); // engistré les modification de chainage
fwrite(&buffer, sizeof(Bloc), 1, disque);


Metajourtaballocation(ms,blocactuelle,0);//mise a jour table d'allocation
miseAJourMetadonnees(disque, nomFichier, 2,nbrbloc--);//mise a jour taille en bloc 


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

fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET); // engistré les modification apres le dernier decalage
fwrite(&buffer, sizeof(Bloc), 1, disque); 

fseek(disque,blocprecedent* sizeof(Bloc), SEEK_SET);// pour met le 1 eng de bloc  dernier dans l'avant derniere bloc dans derniere place 
fread(&buffer, sizeof(Bloc), 1, disque);

buffer.content.fileData.T[buffer.content.fileData.nbrmaladie-1]=m;

fseek(disque,blocprecedent* sizeof(Bloc), SEEK_SET);// engistré les modification
fwrite(&buffer, sizeof(Bloc), 1, disque);



return;

}

}
}

void defragmentation(FILE *disque, MS *ms, const char *nomFichier) {
    Bloc buffer;             // buffer pour charger les blocs
    maladie temp[FB];        // tableau temporaire pour réorganiser les enregistrements
    int blocactuelle, blocsuivant; // pointeurs pour le bloc courant et suivant
    int indexTemp = 0;       // indice pour remplir le tableau temporaire
    int totalEnregistrements = 0; // compteur pour les enregistrements valides
    int taillefichierblocs;  // nombre de blocs utilisés après défragmentation

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
            if (!buffer.content.fileData.T[i].suprimelogiqument) {
                temp[indexTemp++] = buffer.content.fileData.T[i];
            }
        }

        blocactuelle = buffer.content.fileData.next; // passer au bloc suivant
    }

    // calculer le nombre de blocs nécessaires
    taillefichierblocs = (indexTemp + FB - 1) / FB;

    // eéécriture des blocs avec les enregistrements valides
    blocactuelle = debut;
    indexTemp = 0;
    for (int i = 0; i < taillefichierblocs; i++) {
        // charger ou initialiser un bloc existant
        fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        // remplir le bloc avec les enregistrements valides
        int nbrEnregistrements = 0;
        while (indexTemp < totalEnregistrements && nbrEnregistrements < FB) {
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

        Metajourtaballocation(ms, blocactuelle, 0); // libérer le bloc
        blocactuelle = buffer.content.fileData.next;
    }

    // mettre à jour les métadonnées
    miseAJourMetadonnees(disque, nomFichier, 1, taillefichierblocs); // taille en blocs
    miseAJourMetadonnees(disque, nomFichier, 2, totalEnregistrements); // nombre d'enregistrements

    printf("Défragmentation du fichier %s terminée avec succès.\n", nomFichier);
}

void renommerfichierCO(FILE*disque,const char*nomFichier)
{

adressemetadonnes adress;
Bloc buffer;


adress=recherchemetadonnes(disque,nomFichier);

buffer.typedebloc=1;

   fseek(disque, adress.numerodebloc * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

//copier le noveau mot 
      strcpy(buffer.content.metadataTable.T[adress.index].Nomdufichier, nomFichier);


// engistré les modification 
    fseek(disque, adress.numerodebloc * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

printf("succes");

return;
}

void suprimerFCO(FILE*disque,const char*nomFichier)
{

Bloc buffer;
MS*ms;
int dernierblocmeta=1;//derniere bloc metadonnes pour la supression
int blocprecedent;
fichiermetadonnes dernierengmeta;
int nbrbloc=liremetadonnes(disque,nomFichier,1); // pour engistrer les info avant la supresssion de metadatta
int adrpremierbloc=liremetadonnes(disque,nomFichier,3);

bool vide;//pour liberer le bloc qui sera vide

adressemetadonnes adress=recherchemetadonnes(disque,nomFichier);

// chercher l'adress de derniere bloc qui stoke metadonnes

while(1)
{
    fseek(disque, dernierblocmeta * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);
    if (buffer.content.metadataTable.next == -1) break;//pour sortir de la boucle
    blocprecedent=dernierblocmeta;
    dernierengmeta=buffer.content.metadataTable.T[buffer.content.metadataTable.nbrMetadonnees-1];// engistré le derniere eng de metadonnes
    dernierblocmeta=buffer.content.metadataTable.next;

}
// verifier si apres supression de metadata le dernier bloc sera vide 

if(buffer.content.metadataTable.nbrMetadonnees==1)
{
    vide=true;
}else{vide=false;}

fseek(disque, adress.numerodebloc * sizeof(Bloc), SEEK_SET);//pour suprimer metadonnes est decalé
fread(&buffer, sizeof(Bloc), 1, disque);



for (int i = adress.index; i < buffer.content.metadataTable.nbrMetadonnees - 1; i++) {

        buffer.content.metadataTable.T[i] = buffer.content.metadataTable.T[i + 1];
    }

    buffer.content.metadataTable.T[adress.index]=dernierengmeta;//met a la place de bloc que en va suprimé le dernier eng de meta pour eviter le decalage
    
 if(vide){   // suprimer le bloc 

    fseek(disque, blocprecedent * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    buffer.content.metadataTable.next=-1;

// ecrire les changement 
     fseek(disque, blocprecedent * sizeof(Bloc), SEEK_SET);
     fwrite(&buffer, sizeof(Bloc), 1, disque);

//mise a jour table d'allocation

Metajourtaballocation(ms,dernierblocmeta, 0);

 }

 // met les bloc de fichier vide 

    int blocActuel = adrpremierbloc;
    while (blocActuel != -1) {
        fseek(disque, blocActuel * sizeof(Bloc), SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, disque);

        int blocSuivant = buffer.content.fileData.next; // ddresse de bloc suivante
        memset(&buffer, 0, sizeof(Bloc));               // vider le contenu de bloc
        buffer.typedebloc = 2;                          // type de Bloc est filedata
        fseek(disque, blocActuel * sizeof(Bloc), SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, disque);

        Metajourtaballocation(ms, blocActuel, 0); // Libérer le bloc
        blocActuel = blocSuivant;
    }

    printf("Fichier supprimé avec succes.\n");
    return;
 }

































































































int main() {
   
    
}

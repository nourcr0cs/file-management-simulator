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
  bool suprimelogiqument;

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
    union {
        fichiermetadonnes metadata;
        Bloc fileData;
        Tableallocation allocation;
    } content;
    int typedebloc; // 1 = metadata, 2 = file data, 3 = allocation
} Bloc;

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
     if(strcmp(buffer.content.metadata.Nomdufichier, nomfichier) == 0)// comparer si le nom de fichier courant c'est le meme que je cherche 
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
            return buffer.content.metadata.Taillefichierblocs;
        case 2:  // taille en enregistrements
            return buffer.content.metadata.Taillefichierenregistrements;
        case 3:  // adresse du premier bloc
            return buffer.content.metadata.Adrpremierbloc;
        case 4:  // mode d'organisation globale
            return buffer.content.metadata.Modeorganisationglobale;
        case 5:  // mode d'organisation interne
            return buffer.content.metadata.Modeorganisationinterne;
       
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
            buffer.content.metadata.Taillefichierblocs = nouvelleValeur;
            break;
        case 2:  // taille en enregistrements
            buffer.content.metadata.Taillefichierenregistrements = nouvelleValeur;
            break;
        case 3:  // adresse du premier bloc
            buffer.content.metadata.Adrpremierbloc = nouvelleValeur;
            break;
        case 4:  // mode d'organisation globale
            buffer.content.metadata.Modeorganisationglobale = nouvelleValeur;
            break;
        case 5:  // mode d'organisation interne
            buffer.content.metadata.Modeorganisationinterne = nouvelleValeur;
            break;
        default:
            printf("Champ non valide.\n");
            return;
    }

    // ecrire les modification 
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fwrite(&buffer, sizeof(Bloc), 1, disque);

   
}









































void creerfichier(FILE*disque,MS*ms,fichiermetadonnes*metadonnes,int nbrbloc)
{

// nbrbloc c'est le nombre de bloc entré par utilisateur 
  Bloc buffer; 
  
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

// voir si il ya un  espace pour stoker le fichier 

if(taille>(ms->nbrbloc-ms->nbrblocutil)) // comparer nbr de bloc de fichier a nbr de bloc vide dans ms 
{
   printf("warning : Espace insuffisant pour stoker le fichier !! \n");
   return;//quiter la fonction pas d'espace
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

//verifier si en a trouver un bloc libre

if(place==1)
{
    printf("warning : Espace insuffisant aucun bloc libre pour stoker les metadonnes ");
    return;//quiter  la function 
}

// met a jour table d'allocation 

Metajourtaballocation(ms,place,1);

// met a jour nombre enregistrement de bloc de metadonnes 
buffer.typedebloc=1;
buffer.content.metadata=*metadonnes; // remplir la partie contenu du bloc avec les métadonnées du fichier


// stoker les metadonnes dans le bloc vide 
//met pointeur dans 1 enregistrement dans le bloc pour stoker les metadonnes dans la ms 
fseek(disque, place*sizeof(Bloc), SEEK_SET);
// ecrire contenu de buffer dans ms 
fwrite(&buffer,sizeof(Bloc),1,disque);

printf("le fichier a été creer avec succes");


}


void chargerfichier(FILE* disque, MS* ms, const char* nomFichier)

{

Bloc buffer;

 //  récupérer les métadonnées du fichier
    fichiermetadonnes metadonnes;
    adressemetadonnes adresse = recherchemetadonnes(disque, nomFichier);
    
    if (adresse.numerodebloc == -1) {
        printf("fichier introuvable \n");
        return -1;  // Le fichier n'a pas été trouvé
    }

    // charger les métadonnées du bloc donne le buffer
    
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    fread(&buffer, sizeof(Bloc), 1, disque);

    if (buffer.typedebloc != 1) {
        printf("Le bloc ne contient pas de métadonnées.\n");
        return -1; 
    }

metadonnes = buffer.content.metadata; // copier les metadonnees du bloc buffer dans la variable metadonnes

int blocnecessaire=metadonnes.Taillefichierblocs;   

int bloctrouvé=0;//pour chercher des bloc vide 

int adrPremierBloc = -1;// variable pour stoker ladresse du premier bloc dans le fichier pour mise a jour les metadonnes 

//  chercher des blocs libres dans la table d'allocation (le premier bloc de la table)
    fseek(disque, 0 * sizeof(Bloc), SEEK_SET);  // table d'allocation est dans le premier bloc
    fread(&buffer, sizeof(Bloc), 1, disque);

   if (buffer.typedebloc != 3) // verifier si ce bloc est stoker la structure de la table dalloction 
    {
        printf("Le bloc d'allocation n'est pas valide.\n");
        return -1;  
    }

     // allouer les blocs necessaires pour les enregistrements
for (int i = 0; i < ms->nbrbloc && bloctrouvé < blocnecessaire; i++) {
  if (ms->tablelocation[i].etat == 0) 
  {  // si le bloc est vide
       ms->tablelocation[i].etat = 1;  // marquer le bloc comme utilisé

     // verifier si  c'est le premier bloc alloué, enregistrer son adresse
  if (adrPremierBloc == -1) {
                adrPremierBloc = i;  // Enregistrer l'adresse du premier bloc
            }

      // allouer l'enregistrement dans le bloc
    fseek(disque, i * sizeof(Bloc), SEEK_SET); // met le pointeur donne le bloc vide trouver 
    fread(&buffer, sizeof(Bloc), 1, disque); // charger dans buffer pour faire le chainage 

    // chaînage des blocs  mettre à jour le champ next du bloc courant
    if (adrPremierBloc != -1) 
    
    {  
      // mettre à jour l'adresse du bloc précédent 
      if (i != adrPremierBloc) // verifier si le ce bloc n'est pas le 1 pour faire chainage avec le precedent
      {
      fseek(disque, adrPremierBloc * sizeof(Bloc), SEEK_SET);
      fread(&buffer, sizeof(Bloc), 1, disque);
       buffer.content.fileData.next = i;  // Chaînage du bloc précédent
      fseek(disque, adrPremierBloc * sizeof(Bloc), SEEK_SET);
      fwrite(&buffer, sizeof(Bloc), 1, disque);
      }

    }

            // Enregistrer l'adresse du bloc actuel pour le chaînage futur
            adrPremierBloc = i;
            bloctrouvé++;
        }
    }

    // Vérifier tous les blocs nessecaire sont allouer
    if (bloctrouvé < blocnecessaire) {
        printf("Espace insuffisant \n");
        return;
    }

    // mettre a jour les metadonnées avec l'adresse du premier bloc alloue
    metadonnes.Adrpremierbloc = adrPremierBloc;  // mettre a jour l'adresse du premier bloc alloue

    // sauvegarder les métadonnées mise a jour
    fseek(disque, adresse.numerodebloc * sizeof(Bloc), SEEK_SET);
    buffer.typedebloc = 1;
    buffer.content.metadata = metadonnes;
    fwrite(&buffer, sizeof(Bloc), 1, disque);

    printf("Le fichier a été charge et les blocs ont été alloués avec succès.\n");
}



void insertionenregistrement(FILE*disque,MS*ms,fichiermetadonnes*metadonnes,const char* nomFichier)
{

Bloc buffer;
maladie m; // maladie  qui en veux inserer
maladie enrdecale;// l'enregistrement qui va decalé vers le bloc suivant est aussi le derniere enregistrement dans le bloc ou on a trouver la position 
maladie enr;// variable qui va engistré l'enregistrement qui va changer du bloc
Bloc adressedubloc;//pour lire l'adresse de bloc 

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
m.suprimelogiqument = false;

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

// cas 2: en va inserer dans un  bloc qui a un bloc suivant (next!=-1)

int blocactuelle=debut;// pour passer a bloc suivant 

int position=-1;//pour insertion a la fin si en ignore les cas de decalage 

bool positiontrouve=false;

// parcourir les bloc 

while( (blocactuelle!=-1)  &&  (positiontrouve==false)) // boucle pour chercher la position
{

fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
fread(&buffer, sizeof(Bloc), 1, disque);

for(int i=0;i<buffer.content.fileData.nbrmaladie;i++)
{
 
 // comparer les id pour trouver la position 

 if(m.id<buffer.content.fileData.T[i].id)
 {
   position=i;
   positiontrouve=true;
   break;

 }
}

 if (!positiontrouve)
 {blocactuelle=buffer.content.fileData.next;}


}

 enr=buffer.content.fileData.T[FB-1];// l'enregistrement qui va decalé vers le bloc suivant est aussi le derniere enregistrement dans le bloc

//decaler les enregistrement dans  bloc ou en a trouver la position  

for(int j=FB-1;j>position;j--) // fb-1 car le 1 index array est 0
{

buffer.content.fileData.T[j] = buffer.content.fileData.T[j - 1]; // pour vider l'espace de position 

}

buffer.content.fileData.T[position]=m;

// ecrire les changement 

fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
    
fwrite(&buffer, sizeof(Bloc), 1, disque);

blocactuelle=buffer.content.fileData.next;



// dacaler les enregistrement qui sont dans les bloc suivant (decalage inter est intra bloc)
//les enregistrement  qui va changer de bloc sont just les derniere enregistrement dans les bloc apres le bloc ou on a inserer 
// en vais faire decalage est on arrete dans le derniere bloc de fichier (next=-1)


do {

  // charger le bloc pour faire un decalage est met le drenier enregistrement de bloc prcedent dans la index 0 
  fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
  fread(&buffer, sizeof(Bloc), 1, disque);

enrdecale=buffer.content.fileData.T[FB-1];

  for(int i=FB-1;i>0;i--)
  {
    
    buffer.content.fileData.T[i]=buffer.content.fileData.T[i-1];

  }

  buffer.content.fileData.T[0]=enrdecale;

  // ecrire les modification de bloc actuelle avant de  passé a le bloc suivant

  fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
  fwrite(&buffer, sizeof(Bloc), 1, disque);

  blocactuelle=buffer.content.fileData.next;

  

}while(buffer.content.fileData.next!=-1);

// verifier si quand en fait un decalage il faut allouer un nouveux bloc  pour stoker l'enregistrement qui a decaler
if(allouer=true)
{

// cas 4 : allouer un nouveaux bloc apres le decalage 

// voir si il ya un  espace pour allouer un noveaux bloc 

if(0>(ms->nbrbloc-ms->nbrblocutil)) // comparer si il ya un bloc   vide 
{
   printf("warning : Espace insuffisant pour insere l'enregistrement  !! \n");
   return;//quiter la fonction pas d'espace
}

// alouer un nouveux bloc 
int place=1 ;
for(int i=0;i<ms->nbrbloc;i++)
{
    if(ms->tablelocation[i].etat==0)
    {
       place = i; // la place vide est le bloc  ou en va inerer le noveux enregistrement
       break;
    }
    

}

//verifier si en a trouver un bloc libre le cas 5 espace insufisant 

if(place==1)
{
    printf("warning : Espace insuffisant aucun bloc libre pour stoker les metadonnes ");
    return;//quiter  la function 
}

// met a jour table d'allocation 

Metajourtaballocation(ms,place,1);

// charger le noveux bloc dans buffer pour stoker le nouveux enregistrement

fseek(disque, place * sizeof(Bloc), SEEK_SET);
fread(&buffer, sizeof(Bloc), 1, disque);

if(m.id>enrdecale.id) //verifier si le nouveux enregistrement est superieur a tous les enregistrement
{
  buffer.content.fileData.T[0]=enrdecale;
} 
else {

buffer.content.fileData.T[0]=m;

}

// ecrire les changement 

fseek(disque, place * sizeof(Bloc), SEEK_SET);
fwrite(&buffer, sizeof(Bloc), 1, disque); 

// met a jour le chainage est ajouter le bloc allouer 

fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
fread(&buffer, sizeof(Bloc), 1, disque);

buffer.content.fileData.next=place;

fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
fwrite(&buffer, sizeof(Bloc), 1, disque);





}

buffer.typedebloc=1;// pour chnger metadonnes

miseAJourMetadonnees( disque,nomFichier, 2, nbrenregistrement++);// just le nombre de enregistrement qui a changé dans ce cas 

return; // sortir car l 'insertion est faite

/* cas 5 : insertion dans le derniere bloc 
 
 
 buffer.typedebloc=2;
 
 fseek(disque, debut * sizeof(Bloc), SEEK_SET);
 fread(&buffer, sizeof(Bloc), 1, disque);

 blocactuelle=debut;

// boucle pour aller a le derniere bloc
do
{
  
fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
fread(&buffer, sizeof(Bloc), 1, disque);

blocactuelle=buffer.content.fileData.next;

} while (buffer.content.fileData.next!=-1);

blocactuelle=buffer.content.fileData.next; // stoker l'adresse de derniere bloc car la loop stop dans l'avant derniere 

fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
fread(&buffer, sizeof(Bloc), 1, disque);

// chercher la position ou en va inserer dans le derniere bloc 


for(int i=FB-1;i<FB;i++)
{
  if(m.id<buffer.content.fileData.T[i].id)
 {
   position=i;
   positiontrouve=true;
   break;

 }
}
 
 enrdecale=buffer.content.fileData.T[FB-1]; // engistreé le derniere enregistrement dans le fichier 
 //verifier si il ya une allocation 

if(allouer=false)
 {

// decaler pour inserer 

for(int j=FB-1;j>position;j--) // fb-1 car le 1 index array est 0
{

buffer.content.fileData.T[j] = buffer.content.fileData.T[j - 1]; // pour vider l'espace de position 

}

buffer.content.fileData.T[position]=m;

fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
fwrite(&buffer, sizeof(Bloc), 1, disque);


buffer.typedebloc=1;// pour chnger metadonnes

miseAJourMetadonnees( disque,nomFichier, 2, nbrenregistrement++);// just le nombre de enregistrement qui a changé dans ce cas 

printf("insertion avec succes");

return;


}
else // faire une allocation 
{

if(enrdecale<m.id) //cas si le nouveux enregistrement soit  apres le dernier enregistrement dans le cas de allocation d'un bloc 
{

// charger le noveux bloc dans buffer pour stoker le nouveux enregistrement

fseek(disque, place * sizeof(Bloc), SEEK_SET);
fread(&buffer, sizeof(Bloc), 1, disque);

buffer.content.fileData.T[0]=m; // stoker l'enregistrement dans l'index 0

// ecrire les changement 

fseek(disque, place * sizeof(Bloc), SEEK_SET);
fwrite(&buffer, sizeof(Bloc), 1, disque); 

// met a jour le chainage est ajouter le bloc allouer 

fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
fread(&buffer, sizeof(Bloc), 1, disque);

buffer.content.fileData.next=place;

fseek(disque, blocactuelle * sizeof(Bloc), SEEK_SET);
fwrite(&buffer, sizeof(Bloc), 1, disque);

return;



}

buffer.typedebloc=1;// pour chnger metadonnes

miseAJourMetadonnees( disque,nomFichier, 2, nbrenregistrement++);// just le nombre de enregistrement qui a changé dans ce cas 

return; // sortir car l 'insertion est faite*/


 printf("Espace insuffisant pour insérer un enregistrement.\n");










}
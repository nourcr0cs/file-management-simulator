#ifndef L_OF_H
#define L_OF_H

void initMS(MS *ms, int nbrbloc);

void initMetadonnees(FILE *disque, int i);

void initTableAllocation(MS* ms);

void creationL_OF(FILE *disque, MS *ms, int nbrbloc);

adressemetadonnees recherchemetadonnees(FILE*disque,const char* nomfichier);

int liremetadonnees(FILE*disque, const char* nomFichier, int caracteristique);

bool ajoutermetadonnees(FILE* disque, fichiermetadonnees metadonnes, int taille);

void miseAJourMetadonnees(FILE* disque, const char* nomFichier, int champ, int nouvelleValeur);

void defregmentation(FILE *disque, MS *ms, const char *nomFichier);

void MAJtaballocation(MS *ms, int index, int etat);

maladie insertHelper();

void insertDis(FILE *disque, MS *ms, int nbrbloc, const char* nomFichier);

position researchDis(FILE *disque, int searchId, const char* nomFichier);

void suppLogique(FILE *disque, int searchId, const char *nomFichier);

void suppPhysique(FILE *disque, MS *ms, const char *nomFichier);

void renameFile(FILE *disque, const char *nomFichier, const char *newName);

#endif 
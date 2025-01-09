#include <stdio.h>
//fonction changer nom du fichier
void changeFileName(FILE* disque , const char* namenewfile , const char* filename ){
   adressemetadonnes add= recherchemetadonnes(disque ,filename );
  
   bloc = buff;
//deplacerr vers le bloc en question
   fseek(file, add.numerodebloc * sizeof(bloc) , SEEK_SET);
   fread(&buff , sizeof(bloc) , 1 , disque );
//deplacer vers la case qui conteint le metadonne de notre fichier 
buff.content.mettaTable.T[add.index].Nomdufichier = namenewfile ;//cmodifier le nom du fichier   

}


//compactage de la ms

void compactage(TableAllocation* blocAlloc) {
    int indexLibre = 0;  // L'indice du prochain bloc vide à remplir

    // Parcours de tous les blocs pour déplacer les blocs pleins
    for (int i = 0; i < 20; i++) {
        if (blocAlloc->tablelocation[i].etat == 1) {  // Si le bloc est plein
            if (i != indexLibre) {  // Si ce n'est pas déjà à la bonne position
                // Déplacer le bloc plein vers la position vide
                blocAlloc->tablelocation[indexLibre] = blocAlloc->tablelocation[i];
                blocAlloc->tablelocation[i].etat = 0;  // Le bloc déplacé devient vide
            }
            indexLibre++;  // On passe à la prochaine case vide
        }
    }

    // Après le compactage, tous les blocs à partir de indexLibre seront vides
    for (int i = indexLibre; i < 20; i++) {
        blocAlloc->tablelocation[i].etat = 0;  // Marquer les blocs comme vides
    }

    printf("La mémoire a été compactée.\n");
}


    



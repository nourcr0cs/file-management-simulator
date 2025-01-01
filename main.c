#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FB 10

// Déclarations des structures (comme fournies dans la requête)

// Prototypes des fonctions


int main() {
    int choix;
    int modeG;
    int modeI;
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
                  printf("initialisation de la memoir secondaire");
                ///soumia function de initms
                break;
            case 2://creation dun fichier
                printf("creation d'un fichier");
                printf("Votre choix d'organisation globale: ");
                scanf("%d", &modeG);
                printf("Votre choix d'organisation interne: ");
                scanf("%d", &modeI);
                if(modeG == 0 ){
                    if(modeI == 0){
                        //le nom de la function creation org=chaine,ordonne
                    }
                    else{
                        //le nom de la function creation org=chaine,nomordonne
                    }
                }
                else
                {
                   if(modeI == 0){
                        //le nom de la function creation org=conti,ordonne
                    }
                    else{
                        //le nom de la function creation org=conti,nomordonne
                    } 
                }
                printf("Fichier telecharger ")
                break;
            case 3:
                // Afficher l'état de la mémoire secondaire
                printf("etat de la MS");
                rewind(ms);
                fread(&buff , sizeof(bloc) , 1, ms );//metre la table d alocation dans mc
                printf("\n--- Etat de la mémoire secondaire ---\n");
                for (int i = 0; i < ms.nbrbloc; i++) {
                    if (ms.tablelocation[i].etat == 0) {
                        printf("Bloc %d : Libre\n", i);
                    } else {
                        printf("Bloc %d : Occupé\n", i);
                    }
                }
                break;
            case 4:
                printf("Affichage de meta donne de fichier :");
                //recherche meta donne marwa
                //afficher metta donne marwa
                break;
            case 5://recherche enregistrement 
                 printf("Votre choix d'organisation globale: ");
                scanf("%d", &modeG);
                printf("Votre choix d'organisation interne: ");
                scanf("%d", &modeI);
                if(modeG == 0 ){
                    if(modeI == 0){
                        //le nom de la function  org=chaine,ordonne
                    }
                    else{
                        //le nom de la function org=chaine,nomordonne
                    }
                }
                else
                {
                   if(modeI == 0){
                        //le nom de la function org=conti,ordonne
                    }
                    else{
                        //le nom de la function  org=conti,nomordonne
                    } 
                }
                break;
            case 6:
                //insererEnregistrement
                 printf("Votre choix d'organisation globale: ");
                scanf("%d", &modeG);
                printf("Votre choix d'organisation interne: ");
                scanf("%d", &modeI);
                if(modeG == 0 ){
                    if(modeI == 0){
                        //le nom de la function  org=chaine,ordonne
                    }
                    else{
                        //le nom de la function  org=chaine,nomordonne
                    }
                }
                else
                {
                   if(modeI == 0){
                        //le nom de la function org=conti,ordonne
                    }
                    else{
                        //le nom de la function org=conti,nomordonne
                    } 
                }
                printf("enregistrement inseret");
                break;
            case 7:
                //supprimerEnregistrement
                 printf("Votre choix d'organisation globale: ");
                scanf("%d", &modeG);
                printf("Votre choix d'organisation interne: ");
                scanf("%d", &modeI);
                if(modeG == 0 ){
                    if(modeI == 0){
                        //le nom de la function  org=chaine,ordonne
                    }
                    else{
                        //le nom de la function org=chaine,nomordonne
                    }
                }
                else
                {
                   if(modeI == 0){
                        //le nom de la function  org=conti,ordonne
                    }
                    else{
                        //le nom de la function  org=conti,nomordonne
                    } 
                }
                printf("enregistrement suprimee");
                break;
            case 8:
                //defragmenterFichier
                 printf("Votre choix d'organisation globale: ");
                scanf("%d", &modeG);
                printf("Votre choix d'organisation interne: ");
                scanf("%d", &modeI);
                if(modeG == 0 ){
                    if(modeI == 0){
                        //le nom de la function org=chaine,ordonne
                    }
                    else{
                        //le nom de la function  org=chaine,nomordonne
                    }
                }
                else
                {
                   if(modeI == 0){
                        //le nom de la function n org=conti,ordonne
                    }
                    else{
                        //le nom de la function  org=conti,nomordonne
                    } 
                }
                printf("defragementation faite ");
                break;
            case 9:
            //supp fichier 
                printf("Votre choix d'organisation globale: ");
                scanf("%d", &modeG);
                printf("Votre choix d'organisation interne: ");
                scanf("%d", &modeI);
                if(modeG == 0 ){
                    if(modeI == 0){
                        //le nom de la function  org=chaine,ordonne
                    }
                    else{
                        //le nom de la function  org=chaine,nomordonne
                    }
                }
                else
                {
                   if(modeI == 0){
                        //le nom de la function  org=conti,ordonne
                    }
                    else{
                        //le nom de la function  org=conti,nomordonne
                    } 
                }
                printf("Fichier suprimee ");
                break;
            case 10:
                //function renommer fichier
                break;
            case 11:
                compacterMS();
                printf("compactage faits");
                break;
            case 12:
                viderMS();
                printf("MS vider");
                break;
            case 0:
                printf("Programe termine !\n");
                break;
            default:
                printf("Choix invalide. Veuillez réessayer.\n");
        }
    } while (choix != 0);

    return 0;
}


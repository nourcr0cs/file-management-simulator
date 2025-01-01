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

compactMS(MS* ms   ){
rewine( ms );
bloc buff1,buff2;
int i ,j;//i parcours la tablallocation  et j on l utilse en cas de deblacement
 fread(buff1, ms.nbrbloc* sizeof(Tableallocation),1,ms);
 //tant que on est pas arriver a la fin de la ms 
 while (i < ms.nbrbloc){
    if ( ms.tableallocation[i].etat == 0 ){
        for(j=i;j<ms.nbrbloc;j++){
        fseek(ms,ms.tableallocation[j+1].adrbloc,SEEK_SET);
        fread(&buff2,sizeof(bloc),1,ms);
        fseek(ms,- sizeof(bloc),SEEK_CUR); //declacer toute les add de tous les block pour complet le vide et avoir un vide a la fin
        fwrite(&buff2,ms.tableallocation[j].adrbloc,sizeof(bloc),1,ms);
       //metre ajour l'etat du bloc qui doits etres l etat de son prochain vue les decalges a droit
       ms.tablealocation[j].etat=ms.tablealocation[j+1].etat;
     }

    }
     i++;
    }
 }
}
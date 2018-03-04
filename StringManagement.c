#include "StringManagement.h"

///============================================

void flush(){
    int c;
    while (((c = getchar()) != '\n') && (c != EOF));
}

///============================================

void clean(char* chaine)
{
    char* p = strchr(chaine, '\n');

    if (p!=NULL)
    {
        *p = 0;
    }

    else
    {
        flush();
    }

}

///============================================

char** split(const char* chain,char sep){

    int i=-1,cpt=0;

    //indexes stores the indexes of the separators,
    //there is always a separator at chain[-1] and at chain[strlen(chain)]
    int indexes[MAX_SIZE];
    indexes[0]=-1;

    while(chain[++i]!=0){
        if(chain[i]==sep){
            cpt++;
            indexes[cpt]=i;
        }
    }
    indexes[cpt+1]=strlen(chain);

    //the array to fill, ends with a NULL pointer
    char** chain2=malloc(sizeof(char*)*(cpt+2)); // cpt separators => cpt+1 elements and a NULL pointer

    //filling chain2
    for(int j=0;j<(cpt+1);j++){

        chain2[j]=malloc(sizeof(char)*(indexes[j+1]-indexes[j]));
        for(int k=0;k<(indexes[j+1]-indexes[j])-1;k++){
            chain2[j][k]=chain[indexes[j]+1+k];
        }
        chain2[j][indexes[j+1]-indexes[j]-1]=0;

    }
    chain2[cpt+1]=NULL;

    return chain2;

}

///============================================

int contains(int len,char** stringArray,const char* str){

    int i=0;
    while(i<len){
        if(strcmp(stringArray[i++],str)==0) return 1;

    }

    return 0;
}

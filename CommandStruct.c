#include "CommandStruct.h"

///=============================================

Commande* createCommande(){
    //res is the returned Commande
    Commande* res=malloc(sizeof(Commande));

    res->prev=NULL;
    res->next=NULL;

    res->command=malloc(sizeof(char*)*MAX_SIZE);

    res->built_in=0;
    res->piped=0;
    res->anded=0;
    res->ored=0;
    res->zombie=0;
    res->end_status=0;
    res->nCO=0;
    res->nCI=0;
    res->irarg=NULL;
    res->orarg=NULL;

    return res;

}

///============================================

Commande* next(Commande* current_command,int* p_index){

    //the last element of the command is set to NULL for the execute function
    current_command->command[*p_index]=NULL;

    Commande* next_command=createCommande();

    current_command->next=next_command;
    next_command->prev=current_command;

    *p_index=0;

    return next_command;
}

///============================================

void destroyCommande(Commande *cmd){

    //return to the beginning
    while(cmd->prev!=NULL){
        cmd=cmd->prev;
    }

    while(cmd!=NULL){
        int i=0;

        //free or ir and or
        if (cmd->irarg!=NULL) free(cmd->irarg);
        if(cmd->orarg!=NULL) free(cmd->orarg);
        //free of char** command
        while(cmd->command[i]!=NULL){
            #ifdef DEBUG
            fprintf(stderr,GREEN"destroy  command[%d]: %s"WHITE"\n",i,cmd->command[i]);
            #endif
            free(cmd->command[i]);
            ++i;
        }
        free(cmd->command[i]);
        free(cmd->command);

        if(cmd->next!=NULL){
            cmd=cmd->next;
        }
        else{
            free(cmd);
            cmd=NULL;
        }

        if (cmd!=NULL && cmd->prev!=NULL) free(cmd->prev);

    }

}

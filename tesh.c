#include "tesh.h"

pid_t checkedFork(){
    pid_t res=fork();
    if(res==-1){
        fprintf(stderr,"[%d]Forked failed ERRNO : %s\n",getpid(),strerror(errno));
        return -1;
    }
    else{
        return res;
    }
}
///===============================================================

int isBuiltIn(const char* command){

    return (strcmp(command,"cd")==0 || strcmp(command,"exit")==0|| strcmp(command,"fg")==0);
}

///===============================================================

int parsing(Commande* current_command,const char* command){

    //TODO make split more robust
    char** csplit=split(command,' ');

    int addToInputArgs=0;
    int addToOutputArgs=0;
    int i=0;
    int index=0;

    #ifdef DEBUG
    fprintf(stderr,BLUE"[%d][In function parsing] parsing command : %s"WHITE"\n",getpid(),command);
    #endif // DEBUG

    while(csplit[i]!=NULL){
        addToInputArgs=0;
        addToOutputArgs=0;

        if(csplit[i]!=NULL && strcmp(csplit[i],"&&")==0){

            current_command=next(current_command,&index);
            current_command->anded=1;
            ++i;

        }
        if(csplit[i]!=NULL && strcmp(csplit[i],"&")==0){

            current_command->zombie=1;
            ++i;

        }
        if(csplit[i]!=NULL && strcmp(csplit[i],"||")==0){
            current_command=next(current_command,&index);
            current_command->ored=1;
            ++i;
        }
        if(csplit[i]!=NULL && strcmp(csplit[i],";")==0){
            current_command=next(current_command,&index);
            ++i;
        }
        if(csplit[i]!=NULL && strcmp(csplit[i],"<")==0){
            addToInputArgs=1;
            current_command->nCI=1;
            ++i;
        }
        if(csplit[i]!=NULL && strcmp(csplit[i],"<<")==0){
             //TODO si on a le temps

            ++i;
        }
        if(csplit[i]!=NULL && strcmp(csplit[i],">")==0){
            addToOutputArgs=1;
            ++i;
            current_command->nCO=1;
        }
        if(csplit[i]!=NULL && strcmp(csplit[i],">>")==0){
            addToOutputArgs=1;
            current_command->nCO=2;
            ++i;;
        }
        if(csplit[i]!=NULL && strcmp(csplit[i],"|")==0){
            current_command->piped=1;
            current_command=next(current_command,&index);
            ++i;
        }

        if(csplit[i]==NULL) break;

        if(isBuiltIn(csplit[i])){
            current_command->built_in=1;
        }
        if(addToInputArgs){
            current_command->irarg=malloc(sizeof(char)*MAX_SIZE);
            strcpy(current_command->irarg,csplit[i]);
        }
        else if(addToOutputArgs){
            current_command->orarg=malloc(sizeof(char)*MAX_SIZE);
            strcpy(current_command->orarg,csplit[i]);
        }
        else{
            current_command->command[index]=malloc(sizeof(char)*MAX_SIZE);
            strcpy(current_command->command[index],csplit[i]);
            ++index;
        }

        ++i;

    }

    //each command line end with a NULL pointer, exec requires it
    current_command->command[index]=NULL;

    //freeing the memory before exiting
    int j=0;
    while(csplit[j]!=NULL){
        free(csplit[j]);
        ++j;
    }
    free(csplit[j]);
    free(csplit);

    return 0;

}

///===============================================================

int cd(const char* new_directory){

    int absolute_path=0;


    //TODO si possible remplacer par "~"? a demander au prof
    if(new_directory==NULL || new_directory[0]==0){

        cd("/home");
        cd(getenv("USER"));
        return 0;
    }

    if(new_directory[0]=='/'){
        absolute_path=1;
    }

    if(!absolute_path){
        char* status=NULL;
        char* directory=malloc(sizeof(char)*MAX_SIZE);
        status = getcwd(directory,MAX_SIZE-1);
        if(status==NULL){
            fprintf(stderr,"[%d] In function cd, error in getcwd ERRNO : %s\n",getpid(),strerror(errno));
        }
        strcat(directory,"/");

        int a=chdir(strcat(directory,new_directory));

        free(directory);

        return a;
    }
    else{

        return chdir(new_directory);
    }

    return -1;

}

///===============================================================

int fg(pid_t pid)
{
    int status=0;
    waitpid(pid,&status,0);

    fprintf(stdout,"[%d->%d]\n",pid,status);
    fflush(stdout);

    return status;
}
///===============================================================

int execute(Commande* cmd,char* hostname,char* current_directory){

    //if exec is 0 the command should nor be executed
    int exec=1;
    #ifdef DEBUG
    fprintf(stderr,BLUE"[%d][In function execute]  irarg: %s orarg: %s anded: %d prevstatus : %d command : %s"WHITE"\n"
             ,getpid(),cmd->irarg,cmd->orarg,cmd->anded,(cmd->prev==NULL)?1:cmd->prev->end_status,cmd->command[0]);
    #endif // DEBUG

    if(cmd->anded && (cmd->prev->end_status==0)){
        exec=0;
        //cmd->end_status=(cmd->prev->end_status && 0);
        if(cmd->built_in){
            cmd->end_status=0;
            return -1;
        }
        exit(-1);
    }
    if(cmd->ored  && (cmd->prev->end_status==1)){
        exec=0;
        //cmd->end_status=(cmd->prev->end_status || 0);
        if(cmd->built_in){
            cmd->end_status=1;
            return 0;
        }

        exit(0);
    }

    if(cmd->nCO >0){//output redirection

        #ifdef DEBUG
        fprintf(stderr,BLUE"[%d][In execute function] redirect output, file to open :%s"WHITE"\n",getpid(),cmd->orarg);
        #endif // DEBUG

        mode_t flags=0;
        if(cmd->nCO==1){// ">"
            flags=O_WRONLY|O_CREAT|O_TRUNC;
        }
        else{ // ">>"
            flags=O_WRONLY|O_CREAT|O_APPEND;
        }
        int newOutput = open(cmd->orarg,flags,S_IRWXO);
        if(newOutput!=NULL){

            if(dup2(newOutput,STDOUT_FILENO)==-1) fprintf(stderr,"[%d]  In input redir, cannot dup2 ERRNO : %s\n",getpid(),strerror(errno));
            close(newOutput);
        }
        else{
            fprintf(stderr,"[%d][In execute function] Cannot open file : %s ERRNO : %s\n",getpid(),cmd->next->command[0],strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if(cmd->nCI == 1){//input redirection

        #ifdef DEBUG
        fprintf(stderr,BLUE"[%d][In execute function] redirect input, file to open :%s "WHITE"\n",getpid(),cmd->irarg);
        #endif // DEBUG

        mode_t flags;

        flags=O_RDONLY;

        int newInput = open(cmd->irarg,flags,S_IRWXO);

        if(newInput!=NULL){
            if(dup2(newInput,STDIN_FILENO)==-1) fprintf(stderr,"[%d]  In input redir, cannot dup2 ERRNO : %s\n",getpid(),strerror(errno));
            close(newInput);
        }
        else{
            fprintf(stderr,"[%d][In execute function] Cannot open file : %s ERRNO : %s\n",getpid(),cmd->next->command[0],strerror(errno));
            exit(EXIT_FAILURE);
        }
    }


    if(exec && cmd->piped){

        int fd[2];
        int status=0;

        if(pipe(fd) == -1) fprintf(stderr,"[%d]Cannot create pipe, ERRNO : %s\n",getpid(),strerror(errno));
        #ifdef DEBUG
        printf(RED"[In function execute] creating pipe"WHITE"\n");
        #endif // DEBUG


        pid_t pid=checkedFork();

        if(pid!=0){//in the first forked son we execute the second command

            if(dup2(fd[0],0)==-1) fprintf(stderr,"[%d] in pipe, cannot dup2 ERRNO : %s\n",getpid(),strerror(errno));
            close(fd[1]);close(fd[0]);
            #ifdef DEBUG
            printf(RED"[In function execute] redirection of input, waiting..."WHITE"\n");
            #endif

            cmd=cmd->next;
            if(cmd==NULL) return -1;
            #ifdef DEBUG
            printf(RED"[In function execute] child ended with status: %d, exec of : %s"WHITE"\n",cmd->end_status,cmd->command[0]);
            #endif

            pid_t second = checkedFork(); //second fork, needed to execute both commands at the same time
            if(!second){
                //recursive call to allow multiple pipes
                execute(cmd,hostname,current_directory);
                fprintf(stderr,"leak\n");
                exit(-1);
            }
            else{
                waitpid(pid,&status,0);

                if(WIFEXITED(status)){
                    if(!WEXITSTATUS(status)){
                        cmd->end_status=1;//useless
                        exit(0);
                    }
                }
                exit(-1);
            }

        }
        else{ //in the child of the first forked son we execute the first command
            #ifdef DEBUG
            printf(RED"[In function execute] redirection of output, exec of : %s"WHITE"\n",cmd->command[0]);
            #endif
            if(dup2(fd[1],1) ==-1) fprintf(stderr,"[%d] in pipe, cannot dup2 ERRNO : %s\n",getpid(),strerror(errno));//redirection output
            close(fd[0]);close(fd[1]);

            //execution of the first command
            free(hostname);free(current_directory);
            execvp(cmd->command[0],cmd->command);
            fprintf(stderr,"execvp failed ERRNO : %s\n",strerror(errno));
        }
    }
    else if(exec){

        /*Built in commands*/
        if(strcmp(cmd->command[0],"cd")==0){
            if(cd(cmd->command[1])==0){
                cmd->end_status=1;
                return 0;
            }
            return -1;
        }
        else if(strcmp(cmd->command[0],"exit")==0){

            free(hostname);
            free(current_directory);
            destroyCommande(cmd);

            exit(0);
        }
        else if(strcmp(cmd->command[0],"fg")==0){

            if(fg(atoi(cmd->command[1]))==0){
                cmd->end_status=1;
                return 0;
            }
            return -1;
        }
        /* Other commands */
        else{
            free(hostname);free(current_directory);
            execvp(cmd->command[0],cmd->command);
            fprintf(stderr,"execvp failed ERRNO : %s\n",strerror(errno));
            exit(-1);

        }
    }
    fprintf(stderr,"leak\n");
    exit(-1);
}
Commande* following(Commande* cmd)
{
    //if the command was piped, the following piped commands were also executed
    while(cmd->piped==1){

        cmd=cmd->next;
        if(cmd->next==NULL){
            #ifdef DEBUG
            fprintf(stderr,RED"[%d][In function following] Destroy!"WHITE"\n",getpid());
            #endif // DEBUG
            destroyCommande(cmd);

            return NULL;
        }
    }
    if(cmd->next==NULL){
        #ifdef DEBUG
        fprintf(stderr,RED"[%d][In function following] Destroy!"WHITE"\n",getpid());
        #endif // DEBUG
        destroyCommande(cmd);
        return NULL;
    }
    return cmd->next;
}

char* makeprompt(const char* user,const char* hostname,const char* current_directory)
{
    char* prompt=malloc(sizeof(char)*MAX_SIZE);

    int i=0;

    while(user[i]!=0){
        prompt[i]=user[i];
        ++i;
    }
    prompt[i]='@';
    ++i;

    int j=0;
    while(hostname[j]!=0){
        prompt[i+j]=hostname[j];
        ++j;
    }
    prompt[i+j]=':';
    ++j;

    int k=0;
    while(current_directory[k]!=0){
        prompt[i+j+k]=current_directory[k];
        ++k;
    }
    prompt[i+j+k]='$';
    prompt[i+j+k+1]=' ';
    prompt[i+j+k+2]=0;

    return prompt;

}





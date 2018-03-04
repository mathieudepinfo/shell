#include "Defines.h"
#include "CommandStruct.h"
#include "StringManagement.h"
#include "tesh.h"

int main(int argc,char* argv[]) {

    /*dynamic loading : libreadline.so*/
    void *handle=NULL;
    char* (*readline) (char*)=NULL;
    void (*add_history) (char*)=NULL;

    handle = dlopen("libreadline.so",RTLD_LAZY);
    if (!handle){fprintf(stderr, "%s\n", dlerror()) ; exit(EXIT_FAILURE);};

    *(void **) (&readline) = dlsym(handle, "readline");
    if (!handle){fprintf(stderr, "%s\n", dlerror()) ; exit(EXIT_FAILURE);};


    *(void **) (&add_history) = dlsym(handle, "add_history");
    if (!handle){fprintf(stderr, "%s\n", dlerror()) ; exit(EXIT_FAILURE);};

    //environment variables
    char* current_directory=malloc(sizeof(char)*MAX_SIZE);
    char* hostname=malloc(sizeof(char)*MAX_SIZE);
    char* user=NULL;

    //if -e is in argv, each failure in a cmd should exit the tesh
    int stop_on_error=0;
    if(argv!=NULL) stop_on_error=contains(argc-1,argv+1,"-e");

    FILE** scripts = malloc(MAX_SIZE*sizeof(FILE*));
    int current_script=0;
    int numberOfScripts=0;
    FILE* s=0;
    for(int i=1;i<argc;i++){

        if(argv[i][0]!='-'){
            s=fopen(argv[i],"r");
            if(s==NULL){
                fprintf(stderr,"cannot open %s\n ERRNO : %s\n",argv[i],strerror(errno));
            }
            else{
                scripts[numberOfScripts]=s;
                ++numberOfScripts;
            }
        }

    }

    //status is used in waitpid
    int status=0;

    #ifdef DEBUG
    printf(GREEN"stop on error : %d"WHITE"\n",stop_on_error);
    #endif

    //pid returned by the fork
    pid_t pid=0;

    //used to know if we should prompt or not (connected to term)
    int cTot= isatty(STDIN_FILENO);

    #ifdef DEBUG
    printf(GREEN"cTot : %d"WHITE"\n",cTot);
    #endif // DEBUG
    while(1){

        //update of the environment variables
        getcwd(current_directory,MAX_SIZE-1);
        gethostname(hostname,MAX_SIZE-1);
        user=getenv("USER");

        //wait for the input of a command
        char* command=NULL;
        #ifdef DEBUG
        fprintf(stderr,GREEN"[%d][In main loop] reading"WHITE"\n",getpid());
        #endif // DEBUG
        if(numberOfScripts){
            command=malloc(MAX_SIZE);
            if(current_script<numberOfScripts && fgets(command,MAX_SIZE,scripts[current_script])==NULL){
                command=NULL;
            }
            else{
                clean(command);
            }
        }
        else if(!cTot){
            command=malloc(MAX_SIZE);
            if(fgets(command,MAX_SIZE,stdin)==NULL){
                command=NULL;
            }
            else{
                clean(command);
            }
        }
        else{
            char* prompt=makeprompt(user,hostname,current_directory);
            //TODO utiliser fgets par défaut readline si -r est passé en argument
            command=readline(prompt);
            free(prompt);
        }

        #ifdef DEBUG
        fprintf(stderr,GREEN"[%d][In main loop] command==NULL :%d"WHITE"\n",getpid(),command==NULL);
        #endif // DEBUG

        //if EOF is reached exit the tesh, otherwise store the command in the history
        if(command!=NULL && strcmp(command,"")!=0 && cTot){
            add_history(command);
        }
        else if(command==NULL && !numberOfScripts){
            free(command);
            #ifdef DEBUG
            fprintf(stderr,RED"[%d][In main loop]About to exit"WHITE"\n",getpid());
            #endif // DEBUG
            exit(0);

        }
        else if(numberOfScripts && command==NULL){
            current_script++;
            if(current_script >= numberOfScripts){
                for(int i=0;i<numberOfScripts;i++){
                    fclose(scripts[i]);
                }
                free(command);exit(0);
            }
            continue;
        }

        //creation of the Commande structure
        Commande* cmd=createCommande();
        if(parsing(cmd,command)!=0) fprintf(stderr,"[%d][Warning] parsing failed\n",getpid());

        free(command);

        #ifdef DEBUG
        Commande* cmdtmp=cmd;
        while(cmd!=NULL){
            fprintf(stderr,BLUE"[%d][In main loop]command : %s"WHITE"\n",getpid(),cmd->command[0]);
            cmd=cmd->next;
        }
        cmd=cmdtmp;
        #endif // DEBUG

        while(cmd!=NULL){

            #ifdef DEBUG
            fprintf(stderr,GREEN"[%d][In execution loop] command : %s end status before exec :%d"WHITE"\n",getpid(),cmd->command[0],cmd->end_status);
            #endif

            //if cmd is built in, not forking
            if(cmd->built_in){

                execute(cmd,hostname,current_directory);
            }
            else{

                pid=checkedFork();

                if(pid==0){//in the child process we can execute the cmd

                    execute(cmd,hostname,current_directory);
                }
                else if(cmd->zombie==0){
                //in the father we wait for the child if it's not a zombie
                    waitpid(pid,&status,0);

                    if(WIFEXITED(status)){
                        if(!WEXITSTATUS(status)){
                            cmd->end_status=1;
                        }
                    }
                }
                else{//if it's a zombie we keep his ID in the list
                    fprintf(stdout,"[%d]\n",pid);
                    fflush(stdout);
                }

            }

            #ifdef DEBUG
            fprintf(stderr,GREEN"[%d][In execution loop] command : %s end status after exec :%d"WHITE"\n",getpid(),cmd->command[0],cmd->end_status);
            #endif

            if(cmd->end_status==0 && stop_on_error){

                //fprintf(stderr,"[%d] The command : %s failed, aborting",getpid(),cmd->command[0]);
                destroyCommande(cmd);
                free(hostname);
                free(current_directory);
                exit(-1);
            }

            cmd=following(cmd);

        }

    }

    return 0;

}

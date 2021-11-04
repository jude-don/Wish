#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>



#define finished 0
   char *file1;
    char *read_line(){
    size_t buffer_size = 64;
    char *value;
    value = (char*)malloc(buffer_size * sizeof(char*));
    if(value == NULL){
    perror("buffer allocation failed\n");
        exit(finished);
}
    getline(&value,&buffer_size,stdin); 
    printf("%c",*value);
    return value;
}
int redirect_var = 0;
char* file;

 char **input_splitter(char *item, const char *charac)
   { 
    char **instructs = malloc (64* sizeof(char*));
    char *my_commands;
    int index =0;
    my_commands = strtok(item, charac);

     while ( my_commands != NULL){
      
         instructs[index] = my_commands;
         index++;
         my_commands = strtok(NULL, charac);
     }
     instructs[index] = NULL;
     return instructs;
     }
char **parrallel_func(char *item){ 
  char **instructs = malloc (64* sizeof(char*));
    char *my_commands;
    int index =0;
    my_commands = strtok(item, "&");

     while ( my_commands != NULL){
         instructs[index] = my_commands;
         index++;
         my_commands = strtok(NULL,"&");
     }
     instructs[index] = NULL;
     return instructs;
     }

int execute(char **args){
    int status;
    pid_t pid = fork();
    if (pid < 0){
        fprintf( stderr,"the child process could not be created!\n");
        exit(finished);
    }
    else if (pid==0){ //creating child process
        if(redirect_var == 1) {
            int fp = open(file,  O_WRONLY | O_CREAT | O_TRUNC , 0600);
            dup2(fp , 1);
            dup2(fp , 2);
            execvp(args[0],args);
            exit(finished);
        }
        else{
            execvp(args[0],args);
            exit(finished);
        }
    }
    else{ //This is the parent process
        waitpid(pid,&status,WUNTRACED);// wait(2)
        return 1;
    }
}





void batch_mode(char *file){
    FILE *fp;
    fp = fopen(file,"r");
    char *output;
    output = (char*) malloc(50*sizeof(char));
    fscanf(fp,"%[^\n]",output);
    fclose(fp);
    char**my_commands;
    my_commands = input_splitter(output," ");
    execute(my_commands);
}

int redirectfunc(char *linee){
    redirect_var = 1;
    char * line;
    char ** args;
    char * second_line;
    char delim[] =" \n\t\r\a";
    char** commands = input_splitter(linee , ">");
    args = input_splitter(commands[0],delim);
    file = commands[1];
    free(commands);
    return execute(args);    
}


void wish(){
    char * line;
    char ** args;
    char * second_line;
    char * secondline;
    int  status;
   
    do{
        printf("%s","\nwish> ");

        line = read_line();
        second_line=line;
        char *my_commands;
        char delim[] =" \n\t\r\a";
        secondline = strdup(line);
        my_commands = strsep(&line,delim);
        if (my_commands == NULL){
            perror("\nAn empty command was keyed\n");
            free(line);
            exit(finished);
        }
        if(strchr(secondline, '&') != NULL ){
            char ** args_t = parrallel_func(secondline);
            int k=0;
            while(args_t[k]!=NULL){
                args= input_splitter(args_t[k]," \t\n\r\a");
                status = execute(args);
                k++;
                free(args);
            }
            printf("\nParallelism done\n");
            exit(1);
        }
         if (strchr(secondline, '>') != NULL){ 
            status = redirectfunc(secondline);
            exit(1);
        }
        if(strcmp(my_commands, "exit") == 0) {
                printf("\nExiting\n");
                //free(line);
                exit(finished);
             }
        if (strcmp(my_commands, "cd") == 0){
                my_commands= strsep(&line,delim);
                int errorcode = chdir(my_commands);
                if (errorcode < 0 ){
                perror(" directory not found\n");
                
                exit(finished);
                } 
                else {
                printf("\ndirectory found\n");
                char cwd[64];
                getcwd(cwd, sizeof(cwd));
                printf("\nCurrent directory: %s\n", cwd);
                }
                //free(line);
             }
        if (strcmp(my_commands, "path")==0){
                char *pathArgument = getenv("PATH");	
                while( (my_commands= strsep(&line,delim))!= NULL){
                    strcat(pathArgument, ":");
                    strcat(pathArgument, my_commands);
                }	
                setenv("PATH", pathArgument, 1);
                printf("PATH : %s\n", pathArgument);
                //free(line);	
	        }
                   

        else{
            args = input_splitter(second_line,delim);
            status = execute(args);  
            free(args); 
        }
    }
    while(status);


}

int main(int argc, char *argv[]){
    char *myfile;
    size_t size = 64;
    myfile=(char*)malloc(size*sizeof(char));
    setenv("PATH","/bin",1);
    if (argc>1){
        strcpy(myfile,argv[1]);
        batch_mode(myfile);
    }
    else{
        wish();
    }
    return 0;
}


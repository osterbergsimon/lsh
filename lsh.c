/* 
 * Main source code file for lsh shell program
 *
 * You are free to add functions to this file.
 * If you want to add functions in a separate file 
 * you will need to modify Makefile to compile
 * your additional functions.
 *
 * Add appropriate comments in your code to make it
 * easier for us while grading your assignment.
 *
 * Submit the entire lab1 folder as a tar archive (.tgz).
 * Command to create submission archive: 
      $> tar cvf lab1.tgz lab1/
 *
 * All the best 
 */


#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parse.h"
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>


/*
 * Function declarations
 */

void PrintCommand(int, Command *);
void PrintPgm(Pgm *);
void stripwhite(char *);
int  builtincmd(Command *);
void execute (Command *);

#define NORMAL 0
#define BACKGROUND 11
#define PIPE 22
#define OUT  33
#define IN   44
#define INOUT 55


/* When non-zero, this global means the user is done using this program. */
int done = 0;

/*
 * Name: sig_handler
 * Description: handles crtl-c input and kills zombie processes
 *
 */
void sig_handler(int signo)
{
  if (signo == SIGINT){
      pid_t pid;
      kill(pid,SIGTERM);
    }
   else if (signo == SIGCHLD) {
      int status;
      pid_t pid;
      pid = waitpid(-1, &status, WNOHANG);
   }
}


/*
 * Name: main
 *
 * Description: Gets the ball rolling...
 *
 */
int main(void)
{
  Command cmd;
  int n;
  char *arg_list[10];
  
  signal(SIGINT, sig_handler);
  signal(SIGCHLD, sig_handler);
  
  while (!done) {
    
    char *line;
    line = readline("> ");

    if (!line) {
      /* Encountered EOF at top level */
      done = 1;
    }
    else {
      /*
       * Remove leading and trailing whitespace from the line
       * Then, if there is anything left, add it to the history list
       * and execute it.
       */
      stripwhite(line);

      if(*line) {
        add_history(line);
        /* execute it */
        
        n = parse(line, &cmd);
        //PrintCommand(n, &cmd);
        
        if(builtincmd(&cmd) == 0) {
            execute(&cmd);
        }

      }
    }
    
    if(line) {
      free(line);
    }
  }
  return 0;
}

/*
 * Name: PrintCommand
 *
 * Description: Prints a Command structure as returned by parse on stdout.
 *
 */
void
PrintCommand (int n, Command *cmd)
{
  printf("Parse returned %d:\n", n);
  printf("   stdin : %s\n", cmd->rstdin  ? cmd->rstdin  : "<none>" );
  printf("   stdout: %s\n", cmd->rstdout ? cmd->rstdout : "<none>" );
  printf("   bg    : %s\n", cmd->bakground ? "yes" : "no");
  PrintPgm(cmd->pgm);
}

/*
 * Name: PrintPgm
 *
 * Description: Prints a list of Pgm:s
 *
 */
void
PrintPgm (Pgm *p)
{
  if (p == NULL) {
    return;
  }
  else {
    char **pl = p->pgmlist;

    /* The list is in reversed order so print
     * it reversed to get right
     */
    PrintPgm(p->next);
    printf("    [");
    while (*pl) {
      printf("%s ", *pl++);
    }
    printf("]\n");
  }
}

/*
 * Name: stripwhite
 *
 * Description: Strip whitespace from the start and end of STRING.
 */
void
stripwhite (char *string)
{
  register int i = 0;

  while (whitespace( string[i] )) {
    i++;
  }
  
  if (i) {
    strcpy (string, string + i);
  }

  i = strlen( string ) - 1;
  while (i> 0 && whitespace (string[i])) {
    i--;
  }

  string [++i] = '\0';
}

/*
 * Name: builtincmd
 *
 * Description: handles built in commands cd and exit
 */
int builtincmd(Command *cmd) {
  int r = 0;
  
  if (!strcmp(cmd->pgm->pgmlist[0],"exit")) {
    r = 1;
    printf("Bye!\n");
    exit(0);
  } 
  else if (!strcmp(cmd->pgm->pgmlist[0],"cd")) {
    if(chdir(cmd->pgm->pgmlist[1]) == -1) {
      perror("Error cd: ");
    }
    r = 1;
  }
  
  return r;
}


void execute(Command *cmd)
{
  int mode;
  pid_t pid, pid2;
  FILE *fd, *fd2;
  int pipeline[2];

  if(cmd->bakground){
    mode = BACKGROUND;
  }
  else if(cmd->pgm->next != NULL){
    mode = PIPE;
    if(pipe(pipeline)){
      printf("Pipe failure!\n");
      exit(-1);
    }
  }
  else if(!(cmd->rstdout == NULL)){
    if(!(cmd->rstdin == NULL)){
      mode = INOUT;
    }else{
      mode = OUT;
    }

  }
  else if(!(cmd->rstdin == NULL)){
    mode = IN;
  }

  else{
    mode = NORMAL;
  }

  pid = fork();
  if( pid < 0){
    perror("Error occured");
    exit(-1);
  }
  else if(pid==0)
  {
    switch(mode){
      case NORMAL:
        if(execvp(cmd->pgm->pgmlist[0],cmd->pgm->pgmlist)){
          perror("Error");
          exit(-1);
        }
        break;
  
      case PIPE:
        
        if(cmd->rstdin != NULL){
          fd = fopen(cmd->rstdin, "r");
          dup2(fileno(fd), 0);

        }
       /*) if(cmd->rstdout != NULL){
          fd2 = fopen(cmd->rstdout, "w+");
          dup2(fileno(fd2), 0);
        } */

        close(pipeline[0]);
        dup2(pipeline[1], fileno(stdout));
        close(pipeline[1]);
        
        if(execvp(cmd->pgm->next->pgmlist[0],cmd->pgm->next->pgmlist)){
          perror("Error");
          exit(-1);
        }
        break; 


      case IN:
        fd = fopen(cmd->rstdin, "r");
        dup2(fileno(fd), 0); 
        if(execvp(cmd->pgm->pgmlist[0],cmd->pgm->pgmlist)){
          perror("Error");
          exit(-1);
        }
        break;


      case OUT:
        fd = fopen(cmd->rstdout, "w+");
        dup2(fileno(fd),1);
        if(execvp(cmd->pgm->pgmlist[0],cmd->pgm->pgmlist)){
          perror("Error");
          exit(-1);
        }
        break;


      case INOUT:
        fd = fopen(cmd->rstdin, "r");
        dup2(fileno(fd), 0); 
        fd2 = fopen(cmd->rstdout, "w+");
        dup2(fileno(fd2),1);
        if(execvp(cmd->pgm->pgmlist[0],cmd->pgm->pgmlist)){
          perror("Error");
          exit(-1);
        }


      default :
        if(execvp(cmd->pgm->pgmlist[0],cmd->pgm->pgmlist)){
          perror("Error");
          exit(-1);
        }
        break;
    }
  }

  else{

    if(mode == BACKGROUND){
      ;
    }
    else if(mode == PIPE)
    {
      waitpid(pid,NULL,0);
      pid2 = fork();
      if( pid2 < 0)
      {
        perror("Error occured");
        exit(-1);
      }

      else if(pid2 == 0)
      {


        /*else if(cmd->rstdin != NULL){
          fd = fopen(cmd->rstdin, "r");
          dup2(fileno(fd), 0); 
        }*/

        close(pipeline[1]);   
        dup2(pipeline[0], fileno(stdin));
        close(pipeline[0]);
        if(cmd->rstdout != NULL){
          fd2 = fopen(cmd->rstdout, "w+");
          dup2(fileno(fd2), 1); 
        }
        if(execvp(cmd->pgm->pgmlist[0],cmd->pgm->pgmlist)){
          perror("Error: ");
          exit(-1);
        }
      }
      else{
        close(pipeline[0]);
        close(pipeline[1]);
        wait(NULL);
      }

    }
    else
    {
      wait(NULL);
    }

  }
}

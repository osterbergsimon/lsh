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

/*
 * Function declarations
 */

void PrintCommand(int, Command *);
void PrintPgm(Pgm *);
void stripwhite(char *);
int builtincmd(Command *);
void execute (Command *);

/* When non-zero, this global means the user is done using this program. */
int done = 0;

/*
 * Name: sig_handler
 * Description: handles crtl-c input
 *
 */
void sig_handler(int signo)
{
  if (signo == SIGINT){
      pid_t pid;
      pid = fork();
      kill(pid,SIGKILL);
      printf("received SIGINT\n");
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
  
  if (signal(SIGINT, sig_handler) == SIG_ERR)
  printf("\ncan't catch SIGINT\n");
      
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
        PrintCommand(n, &cmd);
        
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
    exit(0);
  } 
  else if (!strcmp(cmd->pgm->pgmlist[0],"cd")) {
    chdir(cmd->pgm->pgmlist[1]);
    r = 1;
  }
  
  return r;
}


void execute(Command *cmd)
{
  pid_t pid;
  pid = fork();
  if( pid < 0)
  {
    printf("Error occured");
    exit(-1);
  }
  else if(pid==0)
  {    
    execvp(cmd->pgm->pgmlist[0],cmd->pgm->pgmlist);
  }
  else if(cmd->bakground){
    ;
  }
  else 
  {
    wait(NULL);
  }
}

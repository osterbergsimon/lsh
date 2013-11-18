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

/*
 * Function declarations
 */

void PrintCommand(int, Command *);
void PrintPgm(Pgm *);
void stripwhite(char *);
void execute (Command *);


#define NORMAL        00
#define BACKGROUND    11
#define PIPE          22


/* When non-zero, this global means the user is done using this program. */
int done = 0;

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
        execute(&cmd);

      }
    
    if(line) {
      free(line);
    }
  }
  return 0;
}

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


void execute(Command *cmd)
{

  int mode;

  if(cmd->bakground){
    mode = BACKGROUND;
  }
  else if(cmd->pgm->next != NULL){
    mode = PIPE;
  }
  else{
    mode = NORMAL;
  }

  pid_t pid, pid2;
  FILE *fp;
  int pipe[2];

  pid = fork();
  if( pid < 0)
  {
    printf("Error occured");
    exit(-1);
  }
  else if(pid==0)
  {
    switch(mode){
      case PIPE:
        close(pipe[0]);
        dup2(pipe[1], fileno(stdout));
        close(pipe[1]);
        break;
      default:
        execvp(cmd->pgm->pgmlist[0],cmd->pgm->pgmlist);
        break;
    }
  }

  else{

    if(mode == BACKGROUND){
      ;
    }
    else if(mode == PIPE)
    {
      waitpid(pid, NULL, 0);
      pid2 = fork();
      if( pid2 < 0)
      {
        printf("Error occured");
        exit(-1);
      }

      else if(pid2 == 0)
      {
        close(pipe[1]);   
        dup2(pipe[0], fileno(stdin));
        close(pipe[0]);
        execvp(cmd->pgm->next->pgmlist[0], cmd->pgm->next->pgmlist);
      }
      else{
        close(pipe[0]);
        close(pipe[1]);
      }

    }
    else
    {
      wait(NULL);
    }

  }
}

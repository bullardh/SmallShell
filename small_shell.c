/* Heather Bullard
 * Originally a School Project
 * Project: Small Shell
 * Date Due: 2/19/2024
 * Description: See ReadME
 *
 *
 * Sources: 
 * ***Module 4 Lecture Skeleton Code
 * ***Linux Interface
 * ***Online Manpages
 * ***Prof Ben Brewster's YouTube "3.1 Processes" https://youtu.be/1R9h-H2UnLs?si=brQ84opD2juSHl_r
 * ***Prof Ben Brewster's YouTube "3.2 Process Management & Zombies" https://youtu.be/kx60fayG-qY?si=wErPAGj9Kdgjltog"
 * ***Prof Ben Brewster's YouTube "3.3 Signals" https://youtu.be/VwS3dx3uyiQ?si=sLIoor45wS5Q8ut6
 * */

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/wait.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>

#ifndef MAX_WORDS
#define MAX_WORDS 512
#endif
char *newPointers[MAX_WORDS];                            // there are nwords and the array needs a NULL pointer!!
int inRedIndex = 0;
int outRedIndex = 0;
int appRedIndex = 0;  

bool interactive = false;
bool bgOperBool = false;
bool inRedBool = false;
bool outRedBool = false;
bool appRedBool = false;
int exitStatus;
int childStatus = 0;
pid_t childPid = 0;
char *words[MAX_WORDS];
size_t wordsplit(char const *line);
char * expand(char const *word);
bool builtIn = false;

pid_t BGPID;
pid_t PID;
char *stuff = "";
char *stuff1 = "";
char *stuff2 = "";


void signHandler(int signum) {
  
}
/* TODO: move this to expand */


void execute (char** words) {                           // Video by Prof Ben Brewster on walkthrough on smallsh project
  //int Status = 1;
  //execvp(words[0], words);
  
  if (execvp(words[0], words) < 0) {
    execvp(words[1], words);
    fprintf(stderr, "%s\n", words[1]);
    exit(0);
  }
}

int main(int argc, char *argv[])
{
  FILE *input = stdin;
  char *input_fn = "(stdin)";
  if (argc == 2) {
    input_fn = argv[1];
    input = fopen(input_fn, "re");
    if (!input) err(1, "%s", input_fn);
  } else if (argc > 2) {
    errx(1, "too many arguments\n");
  }
  char *line = NULL;
  size_t n = 0;
  int count = 0; 
  for (;;) {
prompt:;
/* -----------Manage background processes---------
 * -----------This section from Module 4:---------
 * -------Monitoring Processes and man pages------ */
    if (bgOperBool == true) {
     while ((childPid = waitpid(0, &childStatus, WUNTRACED | WNOHANG)) > 0) {          //Mod 4. & Linux Interface p. 550-552
      if (WIFEXITED(childStatus)){
        fprintf(stderr, "Child process %jd done. Exit status %d.\n", (intmax_t) childPid, WEXITSTATUS(childStatus));      // Test 14 Prints this Don't change
        exitStatus = WEXITSTATUS(childStatus);
        //sprintf(words[1], "%d", exitStatus);
        break;
      }
      else if (WIFSIGNALED(childStatus)) {
        fprintf(stderr, "Child process %jd done. Signaled %d.\n", (intmax_t) childPid, WTERMSIG(childStatus));
        continue;
      }
      else if (WIFSTOPPED(childStatus)) {
        fprintf(stderr, "Child process %jd stopped. Continuing.\n", (intmax_t) childPid);
        kill(childPid, SIGCONT);
        BGPID = childPid;
        //exit(childPid);
        break;
       // printf("%jd", saveBgPid);
       // fflush(stdout);
      }
            //fprintf(stderr, "%jd\n", words[0]);

    };
    } struct sigaction SIGINT_action = {0}, IGNORE_action = {0}, oldActSIGINT = {0}, oldActSIGTSTP = {0};    // YouTube - Prof Ben Brewster
/* ---------------------PROMPT:--------------------
 * --------INTERACTIVE VS NON-INTERACTIVE---------- */
    //char *dollar = "$";                                               // default printed before each new command line PS1
    //char *root = "#";
    //if (stderr != NULL) fprintf(stderr, "%s", dollar);                                                     // print prompt 
    //if (stderr == NULL) fprintf(stderr, "%s", root);
    
    
    //if (input == stdin) {                                             // Needs sigaction for ctrl x and ctrl c
      SIGINT_action.sa_handler = signHandler;
      IGNORE_action.sa_handler = SIG_IGN;
      if (count == 0) {
      sigaction(SIGINT, &SIGINT_action, &oldActSIGINT);
      sigaction(SIGTSTP, &IGNORE_action, &oldActSIGTSTP);
      count++;
      }
      else {
      sigaction(SIGINT, &SIGINT_action, NULL);
      }


      char *prompt = getenv("PS1");

      if (prompt != NULL) {
        interactive = true;
        fprintf(stderr, "%s", prompt);                       //Interactive Mode = Test 18
        goto ps;
      }
      else {
        interactive = false;
        printf("%s", "");
        fflush(stdout);
      }
     
ps:;
/* ------------READING A LINE OF INPUT------------- */
      ssize_t line_len = getline(&line, &n, input);
      for (char *s = line; *s; ++s) {
        if (*s == '$') {
          ++s;
          
          if (*s == '{') {
            ++s;
            char *token = s;
            for(; *s != '}'; ++s) 
              ;
            char c = *s;
            *s = '\0';
            char const *val = getenv(token);
            if (val == NULL) val = token;
            printf("%s", val);
            fflush(stdout);
            *s = c;
          }
        } 
      }
      if (feof(input)) {                                              // hint on ED Discussions to add feof
        exitStatus = 0;
        exit(exitStatus);
      }
      if (line_len < 0) {
        clearerr(stdin);   
        fprintf(stderr, "\n");
      }
     
        
/* ----------WORDSPLIT creates space and-----------
 * ---------assigns a pointer to the array--------- */
    size_t nwords = wordsplit(line);
    for (size_t i = 1; i < nwords; ++i) {
     // fprintf(stderr, "%s\n", words[i]);

/* ----------EXPAND recognizes symbol and----------
 * ------------expands within each word------------ */
   //   if ((inRedBool == true) || (outRedBool == true) || (appRedBool == true)) {
   //     goto fork;
    //  }
      //if (words[0]) goto other;
//home:;
      char *exp_word = expand(words[i]);
      free(words[i]);
      words[i] = exp_word;
      
      //fprintf(stderr, "%s \n", words[i]);
    }
/* ----------Parsing needs to be below Fork--------- */    
/* ---------Check for the background operator------- */                                  
    
/*******************************EXECUTION****************************** 
 * If at this point no command word is present, smallsh shall silently
 * return to step 1 and print a new prompt message. This shall not be 
 * an error, and “$?” shall not be modified.                         
 *
 * Source: Man Pages, Modules 4 & 5, Video by Prof Ben Brewster        */

//other:;
    if (nwords == 0) goto prompt;
  
/*----------------------Built-in commands------------------------------
 * If the command to be executed is exit or cd the following built-in 
 * procedures shall be executed.Note: The redirection and background 
 * operators mentioned in Step 4 invoke undefined behavior when used 
 * with built-in commands (they may be treated as regular arguments, 
 * ignored, etc, and will not be tested in this way).
 *
 * Source: Video by Prof Ben Brewster
 * */
    char *exitParameter = "exit";
    char *cdParameter = "cd"; 
   
    if (nwords == 1) {                            // Built-in commands
/* -----------EXIT BUILT-IN ONLY----------- */ 
      if (strcmp(words[0], exitParameter) == 0) {     // YouTube Video by Prof. Ben Brewster on slide "Checking the Exit Status"
        exitStatus = 0;
        exit(exitStatus);
        break;
      }
/* ------------CD BUILT-IN ONLY------------ */
      if (strcmp(words[0], cdParameter) == 0) {

        int result = chdir(getenv("HOME"));
        if (result != 0) errx(1, "error CD\n");
        goto prompt;
      }
      if (strcmp(words[0], "pwd") == 0) chdir(getenv("PATH"));
    } 
        
/* -------EXIT BUILT-IN & 1 ARGUMENT------- */    
    else if (nwords == 2) {
      if (strcmp(words[0], exitParameter) == 0) {
        exitStatus = atoi(words[1]);
        exit(exitStatus);
        break; 
                                         // YouTube Video by Prof. Ben Brewster on slide "Checking the Exit Status"
      }
/* --------CD BUILT-IN & 1 ARGUMENT-------- */
      if (words[0] == cdParameter) {
        char *result = expand(words[1]);
        int path = chdir(result);
        if (path != 0) perror("chdir error\n");
        goto prompt;
        
              // YouTube Video by Prof. Ben Brewster on slide "Checking the Exit Status"
                              // video Posted to ED Discussions        
      }
/* -----------TOO MANY ARGUMENTS----------- */   
    } if (((nwords > 2) && (words[0] == exitParameter)) || ((nwords > 2) && (words[0] == cdParameter))) {
      errx(1, "Too many arguments");
      goto prompt;
    } else {
fork:;           
/*-----------Non Built-in Commands-----------*/
      int kidStatus = 0;
      char bgOper[2] = "&";  
      char inRed[2] = "<";
      char outRed[2] = ">";
      char appRed[3] = ">>";
           
      pid_t spawnpid = -5; 
     // exitStatus = -5;
      int openFD;
     // int newCount = 0;
                               //Linux interface page 586 signal treatment 
      SIGINT_action.sa_handler = SIG_DFL;
  
      spawnpid = fork();



      switch (spawnpid) {
        case -1: 
            perror("fork() failed!\n");
            exit(1);
            break;
            
        case 0:                      
            
          /* Module 5: Signal API breaks down what each part of signal means
           * provides structure and linux interface p. 1244-1245 •	All signals 
           * shall be reset to their original dispositions when smallsh was invoked.
           * If the command name does not include a “/”, the command shall be searched for in the system’s PATH environment 
           * variable (see EXECVP(3)). */
           
            sigaction(SIGINT, &oldActSIGINT, NULL);
            sigaction(SIGTSTP, &oldActSIGTSTP, NULL);
 
 /************************************PARSING***************************************
 * words are parsed syntactically into tokens If the last word is "&", 
 * it is interpreted as the "background operator". Any occurrence of the words 
 * ">", "<", or ">>" shall be interpreted as redirection operators (write, read, 
 * append). 
 * 
 * Source: Based on Online Manpages, Module 5: Processes and I/O
 *         Linux Interface, see below                                             */
           
            for (size_t i = 0; i < (nwords+1); ++i) {
              newPointers[i] = NULL;
            }
           
            for (int i = 0; i < (nwords); ++i) {                          // try to fix memory leak - linux interface p. 939        
              if ((strcmp(inRed, words[nwords-1]) == 0) || (strcmp(outRed, words[nwords-1]) == 0) || (strcmp(appRed, words[nwords-1]) == 0)) {
                errx(1, "%s", "No Path Included after direction: end of words\n");
                goto prompt;
              }
              if (strcmp(words[i], inRed) == 0) {
                i++;
                inRedIndex = i;
                openFD = open(words[inRedIndex], O_RDONLY | O_CLOEXEC);
                if (openFD == -1) err(1, "%s\n", "In Open () failed");
                int result = dup2(openFD, 0);
                if (result == -1) err(2, "%s\n", "Dup2 In failed");
              } 
              if (strcmp(words[i], outRed) == 0) { 
                i++;
                outRedIndex = i;
                openFD = open(words[outRedIndex], O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC, 0777);
                if (openFD == -1) err(1, "%s\n", "Out Open() failed");
                int result = dup2(openFD, 1);
                if (result == -1) err(2, "%s\n", "Dup2 Out failed");              
              }
              if (strcmp(words[i], appRed) == 0) { 
                i++;
                appRedIndex = i;
                openFD = open(words[appRedIndex], O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0777);
                if (openFD == -1) err(1, "%s\n", "Append Open() failed");
                int result = dup2(openFD, 1);
                if (result == -1) err(2, "%s\n", "Dup2 Append failed");
                
              } 
              else newPointers[i] = words[i];                                        // this list is used for parameters
            }
            execvp(words[0], newPointers);
            break;
        default:       
                                                                                    
            sigaction(SIGINT, &IGNORE_action, NULL);                                     
            sigaction(SIGTSTP, &oldActSIGTSTP, NULL);                                          // Module 5: Signals  

            if (strcmp(words[nwords-1], bgOper) == 0) {
              bgOperBool = true;
            }if (bgOperBool == true) {
              spawnpid = waitpid(spawnpid, &exitStatus, WNOHANG);                    // needs to keep separate, separate variables also
              if (spawnpid == -1) {                                                 // YouTube video by Prof Ben Brewster
                fprintf(stderr, "Child process %jd stopped. Continuing.\n", (intmax_t) spawnpid);      
                BGPID = spawnpid;
                exitStatus = WIFEXITED(exitStatus);
                exit(exitStatus);
              }
             // saveBgPid = BGspawnpid;                                                 // this should be a variable so $! updates instantly
            }
            else {

             spawnpid = waitpid(spawnpid, &kidStatus, WUNTRACED);                  // run again with WUNTRACED Linux Interface p. 549
              PID = spawnpid;
              if (WIFEXITED(kidStatus)) {
                BGPID = spawnpid;                                               // Test 2 Only the spawnpid and its wrong
          
              } 
                                                           // this should be a variable so $? updates instantly
              else if (WIFSIGNALED(kidStatus)) {
                exitStatus = WTERMSIG(kidStatus);
        
                BGPID = spawnpid;
          
              }else if (WIFSTOPPED(kidStatus)) {
                fprintf(stderr, "Child process %jd stopped. Continuing.\n", (intmax_t) spawnpid);         //Test 17 correct
                kill(spawnpid, SIGCONT);             // changed from SIGCONT TEST 17        
                BGPID = spawnpid;               // shows the last act of the FG where does it print
             
              }
            sigaction(SIGINT, &oldActSIGINT, NULL);                                   // YouTube video of Prof Ben Brewster
            sigaction(SIGTSTP, &oldActSIGTSTP, NULL);
 
            }
            break;
      }    // end of switch
        
   } 
    
  } 
}
char *words[MAX_WORDS] = {0};


/*******************************WORDSPLIT******************************
 * :Splits a string into words delimited by whitespace. Recognizes
 * comments as '#' at the beginning of a word, and backslash escapes.
 *
 * Returns number of words parsed, and updates the words[] array
 * with pointers to the words, each as an allocated string.
 *
 * Source: Provided as Skeleton Code by Prof Ryan Gambord
 */
size_t wordsplit(char const *line) {
  size_t wlen = 0;
  size_t wind = 0;

  char const *c = line;
  for (;*c && isspace(*c); ++c); /* discard leading space */

  for (; *c;) {
    if (wind == MAX_WORDS) break;
    /* read a word */
    if (*c == '#') break;
    if (*c == '>') {
      outRedBool = true;
      if (*c+1 == '>') {
        appRedBool = true;
        appRedIndex = wind;
      }
    }
      else outRedIndex = wind;
    if (*c == '<') {
      inRedBool = true;
      inRedIndex = wind;
     }
    if (*c == '&') bgOperBool = true;
    
    for (;*c && !isspace(*c); ++c) {
      if (*c == '\\') ++c;
      void *tmp = realloc(words[wind], sizeof **words * (wlen + 2)); 
      if (!tmp) err(1, "realloc");
      words[wind] = tmp;
      words[wind][wlen++] = *c; 
      words[wind][wlen] = '\0';
    }
    ++wind;
    wlen = 0;
    for (;*c && isspace(*c); ++c);
  }
  return wind;
}


/* ******************************PARAM_SCAN******************************
 * Find next instance of a parameter within a word. Sets
 * start and end pointers to the start and end of the parameter
 * token.
 *
 * Source: Provided as skeleton code by Prof Ryan Gambord
 */
char
param_scan(char const *word, char **start, char **end)
{
static char const *prev;
  if (!word) word = prev;
  
  char ret = 0;
  *start = 0;
  *end = 0;
  for (char const *s = word; *s && !ret; ++s) {
    char *s = strchr(s, '$');
    if (!s) break;
    switch (s[1]) {
    case '$':
    case '!':
    case '?':
      ret = s[1];
      *start = s;
      *end = s + 2;
      break;
    case '{':;
      char *e = strchr(s + 2, '}');
      if (e) {
        ret = s[1];
        *start = s;
        *end = e + 1;
      }
      break;
    }
  }
  prev = *end;
  return ret;
}

/* ******************************BUILD_STR******************************
 * Simple string-builder function. Builds up a base
 * string by appending supplied strings/character ranges
 * to it.
 *
 * Source: Provided as skeleton code by Prof Ryan Gambord
 */
char *
build_str(char const *start, char const *end)
{
  static size_t base_len = 0;
  static char *base = 0;
  
  if (!start) {
    /* Reset; new base string, return old one */
    char *ret = base;
    base = NULL;
    base_len = 0;
    return ret;
  }
  /* ----------Append [start, end) to base string---------- 
   * ----------If end is NULL, append whole start----------
   * ----------------string to base string.----------------
   * -----------Returns a newly allocated string-----------
   * --------------that the caller must free.--------------
   */
  size_t n = end ? end - start : strlen(start);
  size_t newsize = sizeof *base *(base_len + n + 1);
  void *tmp = realloc(base, newsize);
  if (!tmp) err(1, "realloc");
  base = tmp;
  memcpy(base + base_len, start, n);
  base_len += n;
  base[base_len] = '\0';

  return base;
}

/* ******************************EXPAND******************************
 * Expands all instances of $! $$ $? and ${param} in a string 
 * Returns a newly allocated string that the caller must free
 *
 * Source: Provided as skeleton code by Prof Ryan Gambord
 */
char *
expand(char const *word)
{
  char const *pos = word;
  char *start, *end;
  char c = param_scan(pos, &start, &end);
  build_str(NULL, NULL);
  build_str(pos, start);
  while (c) {
/* ----------Replaced with the Process ID of the----------
 * ------------Most Recent Background Process-------------*/    
    if (c == '!') {
      if (BGPID == 0) {
        build_str("", NULL);
      }
      else if (BGPID != 0) {
        asprintf(&stuff, "%d", BGPID);
        setenv(stuff, words[1], 1);
        build_str(stuff, NULL);
      }
    }
    else if (c == '$') {
      PID = getpid();
      if (PID == 0) build_str("<PID>", NULL);
      asprintf(&stuff, "%d", PID);
      setenv(stuff, "$$", 1);
      
      build_str(getenv("$$"), NULL);
      
    }
    else if (c == '?') {
      if (exitStatus > 0) {
        asprintf(&stuff, "%d", exitStatus);
        build_str(stuff, NULL);
      }
      else {
        exitStatus = 0;
        asprintf(&stuff, "%d", exitStatus);
        build_str(stuff, NULL);
      }
    }
    else if (c == '{') {
      build_str("Parameter: ", NULL);
      build_str(start + 2, end -1);
      build_str("", NULL);
    
    }
    pos = end;
    c = param_scan(pos, &start, &end);
    build_str(pos, start);  }
  return build_str(start, NULL);
}


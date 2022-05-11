/******************************************************************************
            < die version 1.1 is  Copyright (c) 1995 Jon N. Steiger > 
PROGRAM: 'die' kills off UNIX processes by name, thus eliminating the need
          to do 'ps -x', look up PID's, and use 'kill'.
SYNTAX: Compile: 'gcc -o die die.c'
        Execute: die [ -cfil ] [ process(es) ]
LICENSE: This code may be freely distributed and/or modified, provided it is 
         done free of charge, and not for financial gain.  I only ask that if
         you *do* change the code, that you indicate somewhere in the source
         as such, and give me credit for the original source.  I assume no 
         responsibility for any direct or indirect damages caused by a user's 
         use or misuse of this program.  die is freeware, and therefore, a 
         "use at your own risk" package.
AUTHOR: Jon N. Steiger / stei0302@cs.fredonia.edu
******************************************************************************/
#include <malloc.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#define DEBUG 0             /* used to deactivate actual system kill */
#define MAXSWITCH 5         /* maximum allowed number of switches */
#define MAXLINES 1000       /* maximum allowed number of processes from ps -x */
#define MAXLENGTH 250       /* maximum length of each job description (chars) */
#define MAXPID 10           /* maximum length of each PID */
#define TEMPFILE "/diefile" /* name of temporary data file which will be 
                              created in the user's root directory.  The
                              quotes and slash are required. */

void switchcheck(int argc, char *argv[]);
void ctrl_c_trap(void);
void formatstring(int *args2, char s1[], char *s2[]);
void itoa(int, char[]);
void kill_jobs(int argc, char *argv[], int last_index);
void kill_jobs_careful(int argc, char *argv[], int last_index);
void kill_jobs_interactive(int argc, char *argv[], int last_index);
void kill_jobs_prompt(int argc, char *argv[], int last_index);
int prepare();
void printhelp();
void pslist(int lastindex);
void strip_n_kill(char pslinesarr[]);
void useage();

FILE *fh1;
char *pslinesarr[MAXLINES];
char pathbuffer[MAXLENGTH]; 
char psheadbuffer[MAXLENGTH];
int Force = 0, Careful = 0, List = 0, Interactive = 0, Help = 0; 

main(int argc, char *argv[])
{
    int last_index;

    signal (SIGINT, ctrl_c_trap);     /* protect against messing up terminal */

    switchcheck(argc, argv);
    last_index = prepare();           /* load arrays */
 
    if (Help) 
        printhelp();
    else if (Interactive) 
        while(Interactive)
            kill_jobs_interactive(argc, argv, last_index);   /* interactive */
    else {
        if (List)
            pslist(last_index);                   /* print ps -x listing */
        if (Force)
            kill_jobs(argc, argv, last_index);    /* forcibly kill stuff */
        else if (Careful)
            kill_jobs_careful(argc, argv, last_index);   /* careful kill */ 
        else
            kill_jobs_prompt(argc, argv, last_index);     /* normal mode */
    }
}

/******************************************************************************
FUNCTION: 'switchcheck' checks the command line arguments, and checks their
           validity.  If they are valid, switchcheck will set the global 
           variables, (Careful, Force, List, Interactive) to indicate the 
           preferences of the user.
******************************************************************************/
void switchcheck(int argc, char *argv[])
{
    char argbuf[MAXSWITCH] = "\0";
    int x;

    if (argc == 1) {           /* If called with no arguments, */
        useage();              /* print useage message */ 
        exit(10);              /* and exit the program */
    }

    else if (argv[1][0] == '-') {                 /* if there are arguments */
        for (x = 1; x <= strlen(argv[1]); x++)    /* load the argument buffer */
            argbuf[x-1] = argv[1][x];
        argbuf[x] = '\0';
    } 
/*
 * Check to see that there are 3 or less arguments, that die was not called
 * with both the 'c' and 'f' options, and that die was not called with only
 * the 'f' or 'c' options.
 */ 

    if ((strlen(argbuf) > 3) ||
        (((strchr(argbuf, 'c') || (strchr(argbuf, 'C'))) && strchr(argbuf, 'f'))) ||
        ((strlen(argbuf) == 1) && 
         ((!strchr(argbuf, 'i')) && (!strchr(argbuf, 'I'))) && 
         ((!strchr(argbuf, 'l')) && (!strchr(argbuf, 'L')))  && 
         ((!strchr(argbuf, 'h')) && (!strchr(argbuf, 'H')))  && 
         (argc < 3))) {
        useage();
        exit(10);
    }

    else {                         /*  It was not, so evaluate the arguments */
        if (strchr(argbuf, 'c') || strchr(argbuf, 'C'))
            Careful = 1;
        if (strchr(argbuf, 'f') || strchr(argbuf, 'F'))
            Force = 1;
        if (strchr(argbuf, 'i') || strchr(argbuf, 'I'))
            Interactive = 1;
        if (strchr(argbuf, 'l') || strchr(argbuf, 'L'))
            List = 1;
        if (strchr(argbuf, 'h') || strchr(argbuf, 'H'))
            Help = 1;
    }
}

/******************************************************************************
FUNCTION: 'ctrl_c_trap' protects against breaking out of die with ^C, and 
           messing up the terminal settings.  This piece of code courtesy
           of Jacob Smith. (smit2204@cs.fredonia.edu)
******************************************************************************/
void ctrl_c_trap(void)
{
    system("stty echo -cbreak");
    exit(0);   /* exit after resetting terminal */
}

/******************************************************************************
FUNCTION: 'itoa' converts an integer to a string.  Needed since stdlib doesn't
          seem to have it on my machine.  :-P (This is K+R's Ansi C code, BTW.)
******************************************************************************/
void itoa(int n, char s[])
{
    void reverse(char s[]);
    int i, sign;

    if ((sign = n) < 0)                   /* record sign */
        n = -n;                           /* make n positive */
    i = 0;
    do {                                  /* generate digits in reverse order */
        s[i++] = n % 10 + '0';            /* get next digit */
    } while ((n /= 10) > 0);              /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}


/******************************************************************************
FUNCTION: 'formatstring' takes a string, and breaks it up into an array of
           pointers, which point to arrays of characters.  Each array of 
           characters contains an individual word.  Count is returned as 
           the number of individual words (and arrays).
******************************************************************************/
void formatstring(int *args2, char s1[], char *s2[])
{
    int x, y = 1, z = 0;
    
    for (x = 0; s1[x] != '\0'; x++)       /* go to end */
        ;
    for ( --x; s1[x] == ' '; x--)        /* unpad spaces */
        s1[x] = '\0';
    for (x = 0; s1[x] == ' '; x++)        /* unpad spaces */
        ;
    if (!s1[x])
        exit(10);
    z = x;
    s2[y] = (char *) malloc(MAXLENGTH * sizeof(char));
    if (!s2[y])  {
        puts("Not enough memory!!!");
        exit(10);
    }
    for ( ; s1[x] != '\0'; x++) {
        if (s1[x] == ' ') {
            s2[y][x-z] = '\0';
            y++;
            s2[y] = (char *) malloc(MAXLENGTH * sizeof(char));
                if (!s2[y])  {
                    puts("Not enough memory!!!");
                    exit(10);
                }
            for ( ; s1[x] == ' '; x++)        /* unpad spaces */
                ;
            z = x;
        }
        s2[y][x-z] = s1[x];


    }
    s2[y][x-z] = '\0';
    *args2 = y + 1;
}
/******************************************************************************
FUNCTION: 'kill_jobs' is fed main's argv array, which it uses to kill all jobs
           which have substrings the same as those in argv[].
******************************************************************************/

void kill_jobs(int argc, char *argv[], int last_index)
{

    int x, y, pid;
    char pidc[MAXPID];

    pid = getpid();
    itoa (pid, pidc);
    if (argv[1][0] == '-')
        x = 2;
    else
        x = 1;

    for ( ; x < argc; x++) {                      /* cycle thorough argv */
        for (y = 0; y <= last_index; y++){            /* cycle through ps -x */
            if ((strstr(pslinesarr[y], argv[x])) && 
                (!strstr(pslinesarr[y], pidc))) {     /* don't list yourself */
                strip_n_kill(pslinesarr[y]);
            }
           
        }
    }
}

/******************************************************************************
FUNCTION: 'kill_jobs_careful' is fed main's argv array, which it uses to kill,
           with 100% user permission, those jobs which have substrings the 
           same as those in argv[].
******************************************************************************/
void kill_jobs_careful(int argc, char *argv[], int last_index)
{
    int x, i, match_count = 0, pid;
    char ch, pidc[MAXPID], buf[30];
    
    pid = getpid();
    itoa (pid, pidc);
    if (argv[1][0] == '-')
        x = 2;
    else
        x = 1;

for ( ; x < argc; x++) 
    for (i = 0; i <= last_index; i++)
        if ((strstr(pslinesarr[i], argv[x])) &&
            (!strstr(pslinesarr[i], pidc))) {  /* don't list yourself */
            match_count++;
            printf("die: %s", pslinesarr[i]);
            system("stty -echo cbreak");         /* char by char on */
            printf("Waste match #%1d? (y/n) n\b", match_count);
            ch = getchar();
            putchar(ch);
            system("stty echo -cbreak");         /* char by char off */
            putchar('\n');
            if ((ch == 'y') || (ch == 'Y'))
                strip_n_kill(pslinesarr[i]);
        }                           
}                    

/******************************************************************************
FUNCTION: 'kill_jobs_interactive' is fed main's argv array, which it uses 
           to kill jobs with the options already set... If there are no such
           options, this function will prompt for the string to search for.
******************************************************************************/
void kill_jobs_interactive(int argc, char *argv[], int last_index)
{
    char inputstring[MAXLENGTH], *inputargv[MAXLENGTH], pidc[MAXPID], ch;
    int x, i, pid, args2; 

    List = 0;
    
    pid = getpid();
    itoa(pid, pidc);

    if (argc > 2)                     /* if process names were given */
        for (x = 2; x < argc; x++)
            if (Careful)
                kill_jobs_careful(argc, argv, last_index);
            else if (Force)
                kill_jobs(argc, argv, last_index);
            else
                kill_jobs_prompt(argc, argv, last_index);


/*
 * Any stray processes have been killed.  We are ready for the interactive
 * session.
 */

    pslist(last_index);                      /* print ps -x listing */
    printf("Kill what processes? (Enter = none): ");
    gets(inputstring);
    formatstring(&args2, inputstring, inputargv);/* format the string */
    printf("(Kill modes are f = force, c = careful, n = normal, "
             "Enter = same as before)\n");
    system("stty -echo cbreak");             /* char by char on */
    printf("Use what mode? ==> ");
    ch = getchar();
    putchar(ch);
    system("stty echo -cbreak");             /* char by char off */
    putchar('\n');
    if ((ch == 'f') || (ch == 'F')) {
        Force = 1;
        Careful = 0;
    }
    else if ((ch == 'c') || (ch == 'C')) {
        Careful = 1;
        Force = 0;
    }
    else if ((ch == 'n') || (ch == 'N')) {
        Careful = 0;
        Force = 0;
    }
    if (Force) {
        kill_jobs(args2,  inputargv, last_index);    /* forcibly kill stuff */
        last_index = prepare();  /* update arrays (for new processes) */
    }
    else if (Careful) {
        kill_jobs_careful(args2, inputargv, last_index);   /* careful kill */ 
        last_index = prepare();  /* update arrays (for new processes) */
    }
    else {
        kill_jobs_prompt(args2, inputargv, last_index);     /* normal mode */
        last_index = prepare();  /* update arrays (for new processes) */
    }
}
/******************************************************************************
FUNCTION: 'kill_jobs_prompt' is fed main's argv array, which it uses to kill, 
           with the permission of the user, (in the event of multiple matches)
           those jobs which have substrings the same as those in argv[].
******************************************************************************/
void kill_jobs_prompt(int argc, char *argv[], int last_index)
{
    int x = 1, i, occ_count = 0, occ_index, match_count = 0, pid;
    char ch, pidc[MAXPID], buf[30];
    
    pid = getpid();
    itoa (pid, pidc);

    if (List)                /* if set, start at argv[2] */
        x++;
    for ( ; x < argc; x++) {
        for (i = 0; i <= last_index; i++)
            if ((strstr(pslinesarr[i], argv[x])) &&  /* check for mult. match */
                (!strstr(pslinesarr[i], pidc))) { /* don't list yourself */
                occ_count++;        
                occ_index = i;
            }
        if (! --occ_count)  {                     /* if only one argument... */
            strip_n_kill(pslinesarr[occ_index]);
        }
        else                                    /* prompt mode */
            for (i = 0; i <= last_index; i++)
                if ((strstr(pslinesarr[i], argv[x])) &&
                    (!strstr(pslinesarr[i], pidc))) {  /* don't list yourself */
                    match_count++;
                    printf("die: %s", pslinesarr[i]);
                    system("stty -echo cbreak");         /* char by char on */
                    printf("Waste match #%1d? (y/n) n\b", match_count);
                    ch = getchar();
                    putchar(ch);
                    system("stty echo -cbreak");         /* char by char off */
                    putchar('\n');
                    if ((ch == 'y') || (ch == 'Y'))
                        strip_n_kill(pslinesarr[i]);
                }                           
    }                    
}

/******************************************************************************
FUNCTION: 'prepare' makes 2 UNIX system calls in order to get a copy of the 
           current 'ps -x' state, which will be used later.  It will save all
           lines of the 'ps -x' in memory.  Returns the index of the last elt.
******************************************************************************/
int prepare(char arg[MAXLENGTH])
{
    char rmbuffer[MAXLENGTH];
    char psbuffer[MAXLENGTH];
    char tempbuffer[MAXLENGTH];
    int continu = 1, x, pid, last_index;
    char pidc[MAXPID];


    strcpy(pathbuffer, (char *) getenv("HOME")); /* get user's root directory */
    strcat(pathbuffer, TEMPFILE);                /* temporary data file */
    strcpy(rmbuffer, "rm ");                     /* create string to delete */
    strcat(rmbuffer, pathbuffer);                /*  the temporary data file */
    strcpy(psbuffer, "ps -x > ");                /* create string to call */
    strcat(psbuffer, pathbuffer);                /*  ps -x */

    if (fh1 = fopen(pathbuffer, "r")) 
        system(rmbuffer);            /* delete the file if it already exists */

    system(psbuffer);                /* get ps -x */

    if (!(fh1 = fopen(pathbuffer, "r"))) {
        printf("Your system or user setup seems to be very weird.\n");
        printf("A support file required by 'die' could not be created.\n");
        exit(10);
    }
    
    fgets(psheadbuffer, MAXLENGTH, fh1);  /* save the ps -x column descriptor */
    for (x = 0; continu; x++) {         /* load arrays */
        pslinesarr[x] = (char *) malloc(MAXLENGTH * sizeof(char));
        if (!pslinesarr[x])  {
                    puts("Not enough memory!!!");
                    exit(10);
                }

        if (fgets (pslinesarr[x], MAXLENGTH, fh1))
            ;
        else 
            continu = 0;
    }

    fclose(fh1);
    system(rmbuffer);                   /* get rid of the temorary data file */
    
    last_index = x - 2;   /* last index */

/*
 * Now get rid of references to yourself in the ps -x listing.
 */
    pid = getpid();
    itoa (pid, pidc);

    for (x = 0; x <= last_index; x++)
        if ((strstr(pslinesarr[x], pidc)) || 
            strstr(pslinesarr[x], TEMPFILE) && 
            strstr(pslinesarr[x], "sh ") &&
            strstr(pslinesarr[x], "ps -x > "))
            strcpy(pslinesarr[x], "");
    
    return last_index;
}


/******************************************************************************
FUNCTION: 'printhelp' prints some more detailed help, availability info, etc.
******************************************************************************/
void printhelp() 
{
puts("\n"
"Program description: die is a UNIX utility which will allow you to kill a \n"
"                     job by process name, instead of number.  Instead of  \n"
"                     having to do a 'ps -x' to find the process you want, \n"
"                     look up the PID number, and do a 'kill <PID number>', \n"
"                     you can just do 'die <process>'.  If you are running \n"
"                     more than one process of the same name, die will \n"
"                     prompt you on wether or not to kill each one, unless \n"
"                     you used the -f switch, in which case die runs amok \n"
"                     through the process table, killing all your matching \n"
"                     processes, with sickening glee and enthusiasm.  \n"
"                   \n"
"                     You can call die with any number of commands (processes) \n"
"                     to kill.  If die does not find a match for an argument, \n"
"                     it will silently terminate.  If it does find a match, \n"
"                     it will just as silently kill the job.  You will probably\n"
"                     recieve a message from UNIX though, telling you that a \n"
"                     job has just been killed. \n"
"\n"
"Legal stuff:         die is freeware.  I retain all rights to the code, but \n"
"                     you are free to use or alter it in any way you like, as  \n"
"                     long as any program or application it is used in (or with) \n"
"                     is freeware as well, and I am given credit for my work.                         If for some unfathonable reason, you want to use this code \n"
"                     in a commercial software package, send me some mail or  \n"
"                     something and, assuming I haven't won the lottery or had \n"
"                     a rich uncle kick the bucket, I'll tailor something to  \n"
"                     your exact specifications.  For a fee, of course!  :-) \n"
"\n"
"Execution: die [-cfhil] [process(es)]  \n"
"           \n"
"           Running die with no arguments will give you the option list. \n"
" \n"
"           Options (switches): \n"
" \n"
"           c = Careful mode.  die will prompt you on wether or not to kill \n"
"               each and every matching process it finds. \n"
" \n"
"           i = Interactive mode.  die will give you a list of the processes \n"
"               which are running, ask you which ones to kill, and ask you \n"
"               which mode to kill them in.  (careful, force, normal, or \n"
"               in the same mode that you killed processes in last time; or \n"
"               if this is the first \"loop\", in the mode that you called \n"
"               die with.)  If you specify processes to kill when starting \n"
"               die in interactive mode, die will take care of them first, \n"
"               using the mode you specify at the command line. \n"
" \n"
"           f = Force mode.  Die will take the process names given at the \n"
"               command line and kill every occurence of them in the  \n"
"               process table, *without* asking permission.  In other  \n"
"               words, 'die -f sh' is a real quick way to log out.  ;-) \n"
" \n"
"           l = List mode.  die will give you a listing of the processes  \n"
"               currently running before killing anything off.  Calling die \n"
"               with -l and no processes will simply give you the process \n"
"               list. \n"
" \n"
"           h = Help mode.  Calling die with -h will bring up a bunch of  \n"
"               helpful info and some other stuff.  (More detailed help \n"
"               than can be obtained by just running die with no (or  \n"
"               invalid) arguments.) \n"
"\n"
"VERY, VERY IMPORTANT:  die cannot, and therefore does not kill your processes\n"
"                       by 'program name'.  It looks for substring matches in \n"
"                       the process table (ps -x).  This can be both good or\n"
"                       bad.  \n"
"\n"
"     The good:  Someone breaks into your account, and they are on ttyp0.  You\n"
"                type 'die -f ttyp0' and they're toast, along with anything\n"
"                they may have been running.\n"
"\n"
"     The bad:  You are editing the file, 'tin.txt'.  You are also in tin, \n"
"               reading alt.cows.moo.moo.moo.  Tin locks up, so you throw\n"
"               it in the background, and type 'die -f tin'.  Guess what?\n"
"               I hope you saved 'tin.txt' recently, because your revisions \n"
"               just got a one-way ticket to /dev/null.  Obviously, if you\n"
"               had just used 'die tin', it would have asked for a confirmation \n"
"               on both 'tin' and 'vi tin.txt', and you would have said, \n"
"               \"Wow, what a great program!  I think I'll put Jon in my will!\"\n"
"               Just keep your wits about you, and you will be fine.\n"
"\n"
"      The ugly:  You just leave that to me, ok?\n"
"\n"
"\n"
"For more information, see 'die.doc'.\n"
"\n"
"You can obtain the latest version of die via WWW at: \n"
"\n"
"   http://www.cs.fredonia.edu/~stei0302/PUB/SOURCE/MYSTUFF/ \n"
"\n"
"Author:  Jon N. Steiger / stei0302@cs.fredonia.edu\n"
"\n"
"Note:  If you use/like die, please take a moment to drop\n"
"       me a note.  I'd like to know wether or not I just\n"
"       wasted my time on this thing.  :-)\n"
"\n"
"\n");
}
/******************************************************************************
FUNCTION: 'pslist' gives a listing of the 'ps -x' state at the time of 
           execution.
******************************************************************************/
void pslist(int last_index)
{
    int i;

    printf(psheadbuffer);
    for (i = 0; i <= last_index; i++)
        printf("%s", pslinesarr[i]);
}
/******************************************************************************
FUNCTION: 'reverse' is a support function for 'itoa' which reverses a string. 
******************************************************************************/
void reverse(char s[MAXPID])
{
    char s2[MAXPID];
    int x, y = 0;

    strcpy(s2, s);                       /* copy string to buffer */
    for (x = 0; s2[x] != '\0'; x++)      /* put x at the end of the buffer */
        ;
    for (--x; x >= 0; x--) {             /* copy back to s, in reverse order */
        s[y] = s2[x];
        y++;
    }
}
        
/******************************************************************************
FUNCTION: 'strip_n_kill' is fed a line from the 'ps -x' array.  It strips the
           PID from the string, and uses a system call to kill the process.
******************************************************************************/
void strip_n_kill(char pslinesarr[MAXLENGTH])
{
    int x = 0, go = 1;    
    char killbuffer[MAXLENGTH] = " kill -9 ";
    
    while(go) {
        if (pslinesarr[x] != ' ') {              /* eat padded spaces */
            go = 0;
        }
        else x++;
    }
    go = 1; 
    for ( ; (x < MAXPID) && go; x++)                 /* read in PID as  */
        if (pslinesarr[x] == ' ') {
            pslinesarr[x] = '\0';
            go = 0;
        }
    strcat(killbuffer, pslinesarr);
#if DEBUG 
        puts(killbuffer);
#else  
       	system(killbuffer);        /* THIS IS THE CALL WITH IT ALL...  */
#endif
    strcpy(pslinesarr, "");

}
/******************************************************************************
FUNCTION: 'useage' prints a program useage message.
******************************************************************************/
void useage()
{
    puts("Useage: die [-cfhil|-CFHIL] [process(es)]");
    puts("Kill off processes by command name, rather than PID number:");
    puts("-c Careful mode.  Prompt for kill on all processes.");
    puts("-f forces die to kill off all listed processes, without asking questions.");
    puts("-i Interactive mode.  Interactively kill processes.");
    puts("-l List mode.  Gives a listing of all current processes");
    puts("-h Help mode.  Gives more detailed help and info. (pipe through more)");
    puts("(default mode is to confirm each kill only in the event of duplicate processes)"); 
    puts("Copyright (c) 1995 Jon N. Steiger / stei0302@cs.fredonia.edu"); 
    putchar('\n'); 
}


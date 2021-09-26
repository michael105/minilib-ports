
#define SMALL
#define TINY

#define SHELL

#define __printflike(x,y) 
#define __dead  __attribute__((__noreturn__))

//#define lint // several unknown definitions else
#define __RCSID(x)



#define setlocale(x,y)

#define PRIdMAX d


#include "alias.c"
#include "arith_token.c"
#include "arithmetic.c"
#include "builtins.c"
#include "cd.c"
#include "error.c"
#include "eval.c"
#include "exec.c"
#include "expand.c"
#include "histedit.c"
#include "init.c"
#include "input.c"
#include "jobs.c"
#include "kill.c"

#ifdef CHKMAIL
#include "mail.c"
#endif

#include "main.c"
#include "memalloc.c"
#include "miscbltin.c"
#include "mystring.c"
#include "nodes.c"
#include "options.c"
#include "output.c"
#include "parser.c"
#include "redir.c"
#include "show.c"
#include "syntax.c"
#include "test.c"
#include "trap.c"
#include "var.c"

// Not needed ( TINY defined )
//#include "printf.c"

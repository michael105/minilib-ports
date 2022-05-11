
#define SMALL
//#define TINY

//#define SHELL

#define __printflike(x,y) 
#define __dead  __attribute__((__noreturn__))

//#define lint // several unknown definitions else
// seems to me a bsd specific id. don't need it I hope
#define __RCSID(x)


// define builtins with TESTBULTIN, ECHOBUILTIN, 

// no locales, but is only unset anyways in ash - 
// dummy is enough
#define setlocale(x,y)

// int conversion for printf ("%d")
#define PRIdMAX "d"

// we know the name. 
#define getprogname() "ash"

#define DEFINE_OPTIONS 

#undef CEOF

//#define __predict_true(x) __builtin_expect(x,1)  
//#define __predict_false(x) __builtin_expect(x,0)  


#define kmalloc malloc
#define krealloc realloc
#define kcalloc calloc
#define kfree free
#include "kmalloc.c"



#include "alias.c"
#include "arith_token.c"
#include "arithmetic.c"
#include "builtins.c"
#include "cd.c"
#include "error.c"

#ifdef ECHOBUILTIN
#include "bltin/echo.c"
#endif

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
#ifdef TESTBUILTIN
#include "test.c"
#endif
#include "trap.c"
#include "var.c"

// Not needed ( TINY defined )
//#include "printf.c"

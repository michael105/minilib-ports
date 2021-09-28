/*	$NetBSD: main.c,v 1.85 2020/02/07 01:25:08 fox Exp $	*/

/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

//#include <sys/cdefs.h>
//#include <sys/cdefs.h>
#ifndef lint
//__COPYRIGHT("@(#) Copyright (c) 1991, 1993
// The Regents of the University of California.  All rights reserved.");
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)main.c	8.7 (Berkeley) 7/19/95";
#else
__RCSID("$NetBSD: main.c,v 1.85 2020/02/07 01:25:08 fox Exp $");
#endif
#endif /* not lint */

//#include <errno.h>
//#include <errno.h>
//#include <stdio.h>
//#include <stdio.h>
//#include <signal.h>
//#include <signal.h>
//#include <sys/stat.h>
//#include <sys/stat.h>
//#include <unistd.h>
//#include <unistd.h>
//#include <stdlib.h>
//#include <stdlib.h>
//#include <locale.h>
//#include <locale.h>
//#include <fcntl.h>
//#include <fcntl.h>


#include "shell.h"
#include "main.h"

#ifdef CHKMAIL
#include "mail.h"
#endif

#include "options.h"
#include "builtins.h"
#include "output.h"
#include "parser.h"
#include "nodes.h"
#include "expand.h"
#include "eval.h"
#include "jobs.h"
#include "input.h"
#include "trap.h"
#include "var.h"
#include "show.h"
#include "memalloc.h"
#include "error.h"
#include "init.h"
#include "mystring.h"
#include "exec.h"
#include "cd.h"
#include "redir.h"

#define PROFILE 0

int rootpid;
int rootshell;
struct jmploc main_handler;
int max_user_fd;
#if PROFILE
short profile_buf[16384];
extern int etext();
#endif

STATIC void read_profile(const char *);

/*
 * Main routine.  We initialize things, parse the arguments, execute
 * profiles if we're a login shell, and then call cmdloop to execute
 * commands.  The setjmp call sets up the location to jump to when an
 * exception occurs.  When an exception occurs the variable "state"
 * is used to figure out how far we had gotten.
 */

int
main(int argc, char **argv)
{
	struct stackmark smark;
	volatile int state;
	char *shinit;
	uid_t uid;
	gid_t gid;
	sigset_t mask;

	/*
	 * If we happen to be invoked with SIGCHLD ignored, we cannot
	 * successfully do almost anything.   Perhaps we should remember
	 * its state and pass it on ignored to children if it was ignored
	 * on entry, but that seems like just leaving the shit on the
	 * footpath for someone else to fall into...
	 */
	(void)signal(SIGCHLD, SIG_DFL);
	/*
	 * Similarly, SIGCHLD must not be blocked
	 */
	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	uid = getuid();
	gid = getgid();

	max_user_fd = 2;

	setlocale(LC_ALL, "");

	posix = getenv("POSIXLY_CORRECT") != NULL;
#if PROFILE
	monitor(4, etext, profile_buf, sizeof profile_buf, 50);
#endif
	state = 0;
	if (setjmp(main_handler.loc)) {
		/*
		 * When a shell procedure is executed, we raise the
		 * exception EXSHELLPROC to clean up before executing
		 * the shell procedure.
		 */
		switch (exception) {
		case EXSHELLPROC:
			rootpid = getpid();
			rootshell = 1;
			minusc = NULL;
			state = 3;
			break;

		case EXEXEC:
			exitstatus = exerrno;
			break;

		case EXERROR:
			exitstatus = 2;
			break;

		default:
			break;
		}

		if (exception != EXSHELLPROC) {
			if (state == 0 || iflag == 0 || ! rootshell ||
			    exception == EXEXIT)
				exitshell(exitstatus);
		}
		reset();
		if (exception == EXINT) {
			out2c('\n');
			flushout(&errout);
		}
		popstackmark(&smark);
		FORCEINTON;				/* enable interrupts */
		if (state == 1)
			goto state1;
		else if (state == 2)
			goto state2;
		else if (state == 3)
			goto state3;
		else
			goto state4;
	}
	handler = &main_handler;
#ifdef DEBUG
#if DEBUG >= 2
	debug = 1;	/* this may be reset by procargs() later */
#endif
	opentrace();
	trputs("Shell args:  ");  trargs(argv);
#if DEBUG >= 3
	set_debug(((DEBUG)==3 ? "_@" : "++"), 1);
#endif
#endif
	rootpid = getpid();
	rootshell = 1;
	init();
	initpwd();
	setstackmark(&smark);
	procargs(argc, argv);

	/*
	 * Limit bogus system(3) or popen(3) calls in setuid binaries,
	 * by requiring the -p flag
	 */
	if (!pflag && (uid != geteuid() || gid != getegid())) {
		setuid(uid);
		setgid(gid);
		/* PS1 might need to be changed accordingly. */
		choose_ps1();
	}

	if (argv[0] && argv[0][0] == '-') {
		state = 1;
		read_profile("/etc/profile");
 state1:
		state = 2;
		read_profile(".profile");
	}
 state2:
	state = 3;
	if ((iflag || !posix) &&
	    getuid() == geteuid() && getgid() == getegid()) {
		struct stackmark env_smark;

		setstackmark(&env_smark);
		if ((shinit = lookupvar("ENV")) != NULL && *shinit != '\0') {
			state = 3;
			read_profile(expandenv(shinit));
		}
		popstackmark(&env_smark);
	}
 state3:
	state = 4;
	line_number = 1;	/* undo anything from profile files */

	if (sflag == 0 || minusc) {
		static int sigs[] =  {
		    SIGINT, SIGQUIT, SIGHUP, 
#ifdef SIGTSTP
		    SIGTSTP,
#endif
		    SIGPIPE
		};
#define SIGSSIZE (sizeof(sigs)/sizeof(sigs[0]))
		size_t i;

		for (i = 0; i < SIGSSIZE; i++)
		    setsignal(sigs[i], 0);
	}

	if (minusc)
		evalstring(minusc, sflag ? 0 : EV_EXIT);

	if (sflag || minusc == NULL) {
 state4:	/* XXX ??? - why isn't this before the "if" statement */
		cmdloop(1);
		if (iflag) {
			out2str("\n");
			flushout(&errout);
		}
	}
#if PROFILE
	monitor(0);
#endif
	line_number = plinno;
	exitshell(exitstatus);
	/* NOTREACHED */
}


/*
 * Read and execute commands.  "Top" is nonzero for the top level command
 * loop; it turns on prompting if the shell is interactive.
 */

void
cmdloop(int top)
{
	union node *n;
	struct stackmark smark;
	int inter;
	int numeof = 0;
	enum skipstate skip;

	CTRACE(DBG_ALWAYS, ("cmdloop(%d) called\n", top));
	setstackmark(&smark);
	for (;;) {
		if (pendingsigs)
			dotrap();
		inter = 0;
		if (iflag == 1 && top) {
			inter = 1;
			showjobs(out2, SHOW_CHANGED);
#ifdef CHKMAIL
			chkmail(0);
#endif
			flushout(&errout);
			nflag = 0;
		}
		n = parsecmd(inter);
		VXTRACE(DBG_PARSE|DBG_EVAL|DBG_CMDS,("cmdloop: "),showtree(n));
		if (n == NEOF) {
			if (!top || numeof >= 50)
				break;
			if (nflag)
				break;
			if (!stoppedjobs()) {
				if (!iflag || !Iflag)
					break;
				out2str("\nUse \"exit\" to leave shell.\n");
			}
			numeof++;
		} else if (n != NULL && nflag == 0) {
			job_warning = (job_warning == 2) ? 1 : 0;
			numeof = 0;
			evaltree(n, 0);
		}
		rststackmark(&smark);

		/*
		 * Any SKIP* can occur here!  SKIP(FUNC|BREAK|CONT) occur when
		 * a dotcmd is in a loop or a function body and appropriate
		 * built-ins occurs in file scope in the sourced file.  Values
		 * other than SKIPFILE are reset by the appropriate eval*()
		 * that contained the dotcmd() call.
		 */
		skip = current_skipstate();
		if (skip != SKIPNONE) {
			if (skip == SKIPFILE)
				stop_skipping();
			break;
		}
	}
	popstackmark(&smark);
}



/*
 * Read /etc/profile or .profile.  Return on error.
 */

STATIC void
read_profile(const char *name)
{
	int fd;
	int xflag_set = 0;
	int vflag_set = 0;

	if (*name == '\0')
		return;

	INTOFF;
	if ((fd = open(name, O_RDONLY)) >= 0)
		setinputfd(fd, 1);
	INTON;
	if (fd < 0)
		return;
	/* -q turns off -x and -v just when executing init files */
	if (qflag)  {
	    if (xflag)
		    xflag = 0, xflag_set = 1;
	    if (vflag)
		    vflag = 0, vflag_set = 1;
	}
	(void)set_dot_funcnest(1);	/* allow profile to "return" */
	cmdloop(0);
	(void)set_dot_funcnest(0);
	if (qflag)  {
	    if (xflag_set)
		    xflag = 1;
	    if (vflag_set)
		    vflag = 1;
	}
	popfile();
}



/*
 * Read a file containing shell functions.
 */

void
readcmdfile(char *name)
{
	int fd;

	INTOFF;
	if ((fd = open(name, O_RDONLY)) >= 0)
		setinputfd(fd, 1);
	else
		error("Can't open %s", name);
	INTON;
	cmdloop(0);
	popfile();
}



int
exitcmd(int argc, char **argv)
{
	if (stoppedjobs())
		return 0;
	if (argc > 1)
		exitshell(number(argv[1]));
	else
		exitshell_savedstatus();
	/* NOTREACHED */
}

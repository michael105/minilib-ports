puts("
Program description: die is a UNIX utility which will allow you to kill a 
                     job by process name, instead of number.  Instead of  
                     having to do a 'ps -x' to find the process you want, 
                     look up the PID number, and do a 'kill <PID number>', 
                     you can just do 'die <process>'.  If you are running 
                     more than one process of the same name, die will 
                     prompt you on wether or not to kill each one, unless 
                     you used the -f switch, in which case die runs amok 
                     through the process table, killing all your matching 
                     processes, with sickening glee and enthusiasm.  
                   
                     You can call die with any number of commands (processes) 
                     to kill.  If die does not find a match for an argument, 
                     it will silently terminate.  If it does find a match, 
                     it will just as silently kill the job.  You will probably                       recieve a message from UNIX though, telling you that a 
                     job has just been killed. 

Legal stuff:         die is freeware.  I retain all rights to the code, but 
                     you are free to use or alter it in any way you like, as  
                     long as any program or application it is used in (or with) 
                     is freeware as well, and I am given credit for my work.                         If for some unfathonable reason, you want to use this code 
                     in a commercial software package, send me some mail or  
                     something and, assuming I haven't won the lottery or had 
                     a rich uncle kick the bucket, I'll tailor something to  
                     your exact specifications.  For a fee, of course!  :-) 

Execution: die [-cfhil] [process(es)]  
           
           Running die with no arguments will give you the option list. 
 
           Options (switches): 
 
           c = Careful mode.  die will prompt you on wether or not to kill 
               each and every matching process it finds. 
 
           i = Interactive mode.  die will give you a list of the processes 
               which are running, ask you which ones to kill, and ask you 
               which mode to kill them in.  (careful, force, normal, or 
               in the same mode that you killed processes in last time; or 
               if this is the first \"loop\", in the mode that you called 
               die with.)  If you specify processes to kill when starting 
               die in interactive mode, die will take care of them first, 
               using the mode you specify at the command line. 
 
           f = Force mode.  Die will take the process names given at the 
               command line and kill every occurence of them in the  
               process table, *without* asking permission.  In other  
               words, 'die -f sh' is a real quick way to log out.  ;-) 
 
           l = List mode.  die will give you a listing of the processes  
               currently running before killing anything off.  Calling die 
               with -l and no processes will simply give you the process 
               list. 
 
           h = Help mode.  Calling die with -h will bring up a bunch of  
               helpful info and some other stuff.  (More detailed help 
               than can be obtained by just running die with no (or  
               invalid) arguments.) 

VERY, VERY IMPORTANT:  die cannot, and therefore does not kill your processes
                       by 'program name'.  It looks for substring matches in 
                       the process table (ps -x).  This can be both good or
                       bad.  

     The good:  Someone breaks into your account, and they are on ttyp0.  You
                type 'die -f ttyp0' and they're toast, along with anything
                they may have been running.

     The bad:  You are editing the file, 'tin.txt'.  You are also in tin, 
               reading alt.cows.moo.moo.moo.  Tin locks up, so you throw
               it in the background, and type 'die -f tin'.  Guess what?
               I hope you saved 'tin.txt' recently, because your revisions 
               just got a one-way ticket to /dev/null.  Obviously, if you
               had just used 'die tin', it would have asked for a confirmation 
               on both 'tin' and 'vi tin.txt', and you would have said, 
               \"Wow, what a great program!  I think I'll put Jon in my will!\"
               Just keep your wits about you, and you will be fine.

      The ugly:  You just leave that to me, ok?


For more information, see 'die.doc'.

You can obtain the latest version of die via WWW at: 

   http://www.cs.fredonia.edu/~stei0302/PUB/SOURCE/MYSTUFF/ 

Author:  Jon N. Steiger / stei0302@cs.fredonia.edu

Note:  If you use/like die, please take a moment to drop
       me a note.  I'd like to know wether or not I just
       wasted my time on this thing.  :-)

");



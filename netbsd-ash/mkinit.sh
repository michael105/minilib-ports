#! /bin/sh
#	$NetBSD: mkinit.sh,v 1.10 2018/12/05 09:20:18 kre Exp $

# Copyright (c) 2003 The NetBSD Foundation, Inc.
# All rights reserved.
#
# This code is derived from software contributed to The NetBSD Foundation
# by David Laight.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
# ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

srcs="$*"

# use of echo in this script is broken

# some echo versions will expand \n in the args, which breaks C
# Note: this script is a HOST_PROG ... it must run in the
# build host's environment, with its shell.

# Fortunately, use of echo here is also trivially simplistic,
# we can easily replace all uses with ...

echo()
{
	printf '%s\n' "$1"
}

# CAUTION: for anyone modifying this script.... use printf
# rather than echo to output anything at all... then
# you will avoid being bitten by the simplicity of this function.
# This was done this way rather than wholesale replacement
# to avoid unnecessary code churn.


nl='
'
openparen='('

# shells have bugs (including older NetBSD sh) in how \ is
# used in pattern matching.   So work out what the shell
# running this script expects.   We could also just use a
# literal \ in the pattern, which would need to be quoted
# of course, but then we'd run into a whole host of potential
# other shell bugs (both with the quoting in the pattern, and
# with the matching that follows if that works as inended).
# Far easier, and more reliable, is to just work out what works,
# and then use it, which more or less mandates using a variable...
backslash='\\'
var='abc\'			# dummy test case.
if [ "$var" = "${var%$backslash}" ]
then
	# buggy sh, try the broken way
	backslash='\'
	if [ "$var" = "${var%$backslash}" ]
	then
		printf >&2 "$0: %s\n" 'No pattern match with \ (broken shell)'
		exit 1
	fi
fi
# We know we can detect the presence of a trailing \, which is all we need.
# Now to confirm we will not generate false matches.
var='abc'
if [ "$var" != "${var%$backslash}" ]
then
	printf >&2 "$0: %s\n" 'Bogus pattern match with \ (broken shell)'
	exit 1
fi
unset var

includes=' "shell.h" "mystring.h" "init.h" <stdint.h> '
defines=
decles=
event_init=
event_reset=
event_shellproc=

for src in $srcs; do
	exec <$src
	decnl="$nl"
	while IFS=; read -r line; do
		[ "$line" = x ]
		case "$line " in
		INIT["{ 	"]* ) event=init;;
		RESET["{ 	"]* ) event=reset;;
		SHELLPROC["{ 	"]* ) event=shellproc;;
		INCLUDE[\ \	]* )
			IFS=' 	'
			set -- $line
			# ignore duplicates
			[ "${includes}" != "${includes% $2 *}" ] && continue
			includes="$includes$2 "
			continue
			;;
		MKINIT\  )
			# struct declaration
			decles="$decles$nl"
			while
				read -r line
				decles="${decles}${line}${nl}"
				[ "$line" != "};" ]
			do
				:
			done
			decnl="$nl"
			continue
			;;
		MKINIT["{ 	"]* )
			# strip initialiser
			def=${line#MKINIT}
			comment="${def#*;}"
			def="${def%;$comment}"
			def="${def%%=*}"
			def="${def% }"
			decles="${decles}${decnl}extern${def};${comment}${nl}"
			decnl=
			continue
			;;
		\#define[\ \	]* )
			IFS=' 	'
			set -- $line
			# Ignore those with arguments
			[ "$2" = "${2##*$openparen}" ] || continue
			# and multiline definitions
			[ "$line" = "${line%$backslash}" ] || continue
			defines="${defines}#undef	$2${nl}${line}${nl}"
			continue
			;;
		* ) continue;;
		esac
		# code for events
		ev="${nl}	/* from $src: */${nl}	{${nl}"
		# Indent the text by an extra <tab>
		while
			read -r line
			[ "$line" != "}" ]
		do
			case "$line" in
			('')	;;
			('#'*)	;;
			(*)	line="	$line";;
			esac
			ev="${ev}${line}${nl}"
		done
		ev="${ev}	}${nl}"
		eval event_$event=\"\$event_$event\$ev\"
	done
done

exec >init.c.tmp

echo "/*"
echo " * This file was generated by the mkinit program."
echo " */"
echo

IFS=' '
for f in $includes; do
	echo "#include $f"
done

echo
echo
echo
echo "$defines"
echo
echo "$decles"
echo
echo
echo "/*"
echo " * Initialization code."
echo " */"
echo
echo "void"
echo "init(void)"
echo "{"
echo "${event_init}"
echo "}"
echo
echo
echo
echo "/*"
echo " * This routine is called when an error or an interrupt occurs in an"
echo " * interactive shell and control is returned to the main command loop."
echo " */"
echo
echo "void"
echo "reset(void)"
echo "{"
echo "${event_reset}"
echo "}"
echo
echo
echo
echo "/*"
echo " * This routine is called to initialize the shell to run a shell procedure."
echo " */"
echo
echo "void"
echo "initshellproc(void)"
echo "{"
echo "${event_shellproc}"
echo "}"

exec >&-
mv init.c.tmp init.c

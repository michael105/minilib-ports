ifndef SUBDIR
	SUBDIR:=.
endif


all: README.md up


up:
	git commit -a -m 'Update'
	git push github

bin_x64/README.md:
	SUBDIR=bin_x64 make README.md -B


.ONESHELL:
README.md: README.in
	cd $(SUBDIR)
	cp README.in README.md
	#ls -lho --time-style=long-iso | grep -oE '\s*(\S*\s*){5}$$' >> README.md
	#echo '```'>> README.md
	perl Makeindex.pl >> README.md
	echo '```'>> README.md


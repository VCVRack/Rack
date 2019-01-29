
all: doxygen

doxygen:
	doxygen Doxyfile

upload: doxygen
	rsync html/ vcvrack.com:vcvrack.com/docs/ -ruvz --delete

clean:
	rm -rfv html

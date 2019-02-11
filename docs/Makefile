
all: doxygen

doxygen:
	doxygen Doxyfile

run: doxygen
	http-server html

upload: doxygen
	rsync html/ vcvrack.com:vcvrack.com/docs/ -ruvz --delete

clean:
	rm -rfv html

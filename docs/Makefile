
all: doxygen

doxygen:
	doxygen Doxyfile

run: doxygen
	http-server html

upload: doxygen
	rsync html/ vcvrack.com:vcvrack.com/docs-v2/ -ruz --info=progress2 --delete

clean:
	rm -rfv html

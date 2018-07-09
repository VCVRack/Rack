# SUPER crude makefiles for my standalone tests.
# Basically use it as
# make run T=docsaw
# and that will build docsaw.cpp which depends on every header everywhere and voila.

HDRS = $(wildcard ../src/*.hpp) $(wildcard *.hpp)

CPPEXE = c++ -I../../../dep/include -L../../../dep/lib -lrtaudio -std=c++11 -stdlib=libc++  -Werror
RUN_PREFIX = DYLD_LIBRARY_PATH=../../../dep/lib

exe_%:	%.cpp $(HDRS)
	$(CPPEXE) -o $@ $<

run:	exe_$(T)
	$(RUN_PREFIX) ./$<

clean:
	rm -f exe*




CXX=g++
RM=rm -f
CD=cd
CPPFLAGS=-Wall -std=c++11 -O3
LDFLAGS=


SRCS=$(wildcard src/*.cpp)

OBJS=$(subst .cpp,.o,$(SRCS))

TEST_SRCS=$(wildcard tests/*.cpp)

TEST_OBJS=$(subst .cpp,.o,$(TEST_SRCS))

ifdef COV
  CPPFLAGS += -fprofile-arcs -ftest-coverage
  LDFLAGS += -fprofile-arcs -ftest-coverage
endif

all: test

test: $(OBJS) $(TEST_OBJS)
	$(CXX) $(LDFLAGS) -o testrunner $(OBJS) $(TEST_OBJS)

depend: .depend

.depend: $(SRCS) $(TEST_SRCS)
	$(RM) -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)
	$(RM) $(TEST_OBJS)

dist-clean: clean
	$(RM) *~ .depend

docs:
	doxygen

include .depend

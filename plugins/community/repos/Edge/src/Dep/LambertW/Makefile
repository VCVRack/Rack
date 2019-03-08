# Makefile for the Lambert W function related builds
#
# Copyright (C) 2011 Darko Veberic, darko.veberic@ijs.si
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


CPPFLAGS := -I.
#CXXFLAGS := -Wall -Wextra -ggdb3 -O0 -fno-inline -pipe
CXXFLAGS := -O2
#CXXFLAGS := -Wall -Wextra -ggdb3 -O2 -pipe
#CXXFLAGS := -Wall -Wextra -ggdb3 -march=native -Ofast -pipe
LDFLAGS := $(CXXFLAGS)

SUFFIXES := .o .cc .cxx

EXES := $(basename $(wildcard *.cxx))
OBJS := $(patsubst %.cc, %.o, $(wildcard *.cc))
DEPS := $(patsubst %.o, %.P, $(OBJS)) $(addsuffix .P, $(EXES))

define cxx_compile_with_dependency_creation
  $(COMPILE.cc) -MD -o $@ $<
  @sed -e 's|.*:|$*.o:|' <$*.d >$*.P
  @sed -e 's/.*://' -e 's/\\$$//' <$*.d | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >>$*.P
  @rm -f $*.d
endef

define cxx_link_rule
  $(CXX) $^ $(LDFLAGS) -o $@
endef

%.o: %.cc
	$(call cxx_compile_with_dependency_creation)

%.o: %.cxx
	$(call cxx_compile_with_dependency_creation)

%: %.o
	$(call cxx_link_rule)

.PHONY: all
all: $(EXES)

lambertw: lambertw.o $(OBJS)
test_accuracy: test_accuracy.o $(OBJS)

.PHONY: check
check: $(basename $(wildcard test_*.cxx))
	for t in $^ ; do echo $$t ; ./$$t || exit $$? ; done

.PHONY: clean
clean:
	- $(RM) $(OBJS) $(addsuffix .o, $(EXES)) $(EXES) *.P

-include $(DEPS)

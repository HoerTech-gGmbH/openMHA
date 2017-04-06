DOC_SRC = $(wildcard *.c) $(wildcard *.cpp)
DOC_OBJ = $(patsubst %,%.o,$(basename $(DOC_SRC)))
DOC_DEP = $(patsubst %,%.dep,$(basename $(DOC_SRC)))
CPPFLAGS = -Wall -I../../libmha/src
CFLAGS = -Wall -I../../libmha/src
CXXFLAGS = -Wall -I../../libmha/src

all: $(DOC_OBJ)

include $(DOC_DEP)

%.dep: %.c
	$(CPP) $(CPPFLAGS) -MM -MF temp_dep.mk $<
	cat temp_dep.mk | sed 's/\([^:]*\)\.o/\1.dep/g' > $@
	echo "" >> $@
	cat temp_dep.mk >> $@
	rm -f temp_dep.mk

%.dep: %.cpp
	$(CPP) $(CPPFLAGS) -MM -MF temp_dep.mk $<
	cat temp_dep.mk | sed 's/\([^:]*\)\.o/\1.dep/g' > $@
	echo "" >> $@
	cat temp_dep.mk >> $@
	rm -f temp_dep.mk

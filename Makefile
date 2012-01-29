SHELL=bash
CC=gcc
CPPFLAGS=
CFLAGS= -Wall -g
LIBS= -lGL

MODULES= svg path sprite gradient matrix
BINDIR= bin
EXES= t1

##---------------------------------------------------------------
OBJS= $(addsuffix .o, $(MODULES))
EXES:= $(addprefix $(BINDIR)/, $(EXES))

.PHONY: default
default: $(EXES)

## Executables
$(BINDIR)/%: test/%.o $(OBJS)
	mkdir -p $(dir $@)
	$(CC) -o $@  $^  $(LIBS)

## Auto-generate dependencies
## http://www.gnu.org/software/make/manual/html_node/Automatic-Prerequisites.html
##
%.dep: %.c
	$(SHELL) -ec '$(CC) -MM $(CPPFLAGS) $<  > $@'

-include $(OBJS:.o=.dep)


.PHONY: clean
clean:
	rm -f *.o *.dep $(EXES)

# Statistics
.PHONY: stats
stats:
	cloc . --exclude-dir=.git,test,bin

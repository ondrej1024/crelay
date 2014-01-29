#########################################
# Conrad USB 4-relay card utility
#
# Makefile
#
# gcc -Wall -o crelay crelay.c
#
#########################################

# Common defines
RM = \rm -f
PROG = crelay
BINPATH=/usr/local/bin

#DEBUG	= -O2
CC	= gcc
INCLUDE	= -I.
CFLAGS	= $(DEBUG) $(INCLUDE) -Wformat=2 -Wall -Winline  -pipe -fPIC 

# List of objects files for the dependency
OBJS_DEPEND=

# Compiler options
#OPTIONS = --verbose

all: target

target: Makefile
	@echo "--- Compile and Link all object files to create the binary file: $(PROG) ---"
	$(CC) $(PROG).c -o $(PROG) $(CFLAGS) $(OBJS_DEPEND) $(OPTIONS)
	@echo ""


clean :
	@echo "---- Cleaning all object files ----"
	$(RM) $(PROG)
	@echo "" 

install : target
	@echo "---- Install binaries ----"
	cp $(PROG) $(BINPATH)
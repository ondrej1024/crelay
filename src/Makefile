# ;
# Makefile:
###############################################################################
#
#
###############################################################################

DESTDIR=/usr
PREFIX=/local

BIN=crelay

#DEBUG	= -g -O0
DEBUG	= -O2
CC	= gcc
INCLUDE	= -I.
DEFS	= -D_GNU_SOURCE
CFLAGS	= $(DEBUG) $(DEFS) -Wformat=2 -Wall -Winline $(INCLUDE) -pipe -fPIC

LIBS    =

# Should not alter anything below this line
###############################################################################

SRC	=	crelay.c relay_drv.c relay_drv_conrad.c relay_drv_gpio.c

OBJ	=	$(SRC:.c=.o)

all:		$(BIN)

$(BIN):	$(OBJ)
	@echo "[Link]"
	@$(CC) -o crelay $(OBJ)

.c.o:
	@echo "[Compile]"
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONEY:	clean
clean:
	@echo "[Clean]"
	@rm -f $(OBJ) crelay

.PHONEY:	install
install:	$(BIN)
	@echo "[Install binary]"
	@install -m 0755 -d		$(DESTDIR)$(PREFIX)/bin
	@install -m 0755 $(BIN)		$(DESTDIR)$(PREFIX)/bin/$(BIN)


# DO NOT DELETE

#dht.o: dht.h
 

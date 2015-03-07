# ;
# Makefile:
###############################################################################
#
# crelay project makefile
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
LIBS	= -lftdi -lhidapi-libusb

# Main source files (don't change)
#SRC	= crelay.c relay_drv.c relay_drv_conrad.c relay_drv_gpio.c
SRC	= $(BIN).c
SRC	+= relay_drv.c

# Relay card specific driver source files (add new drivers here)
SRC	+= relay_drv_conrad.c
SRC	+= relay_drv_sainsmart.c
SRC	+= relay_drv_hidapi.c
SRC	+= relay_drv_gpio.c

OBJ	= $(SRC:.c=.o)

all:	$(BIN)

$(BIN):	$(OBJ)
	@echo "[Link $(BIN)]"
	@$(CC) -o $(BIN) $(OBJ) $(LIBS)

.c.o:
	@echo "[Compile $<]"
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONEY:	clean
clean:
	@echo "[Clean]"
	@rm -f $(OBJ) $(BIN)

.PHONEY:	install
install:	$(BIN)
	@echo "[Install binary]"
	@install -m 0755 -d		$(DESTDIR)$(PREFIX)/bin
	@install -m 0755 $(BIN)		$(DESTDIR)$(PREFIX)/bin/$(BIN)

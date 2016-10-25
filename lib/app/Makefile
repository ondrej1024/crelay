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

# By default enable all drivers
# To exclude a specific driver add DRV_<XYZ>=n to the make command
DRV_CONRAD	= y
DRV_SAINSMART	= y
DRV_SAINSMART16	= y
DRV_HIDAPI	= y

#DEBUG	= -g -O0
DEBUG	= -O2
CC	= gcc
INCLUDE	= -I.
DEFS	= -D_GNU_SOURCE
CFLAGS	= $(DEBUG) $(DEFS) -Wformat=2 -Wall -Winline $(INCLUDE) -pipe -fPIC

SRC	= crelay_cli.c
LIBS	= -lcrelay

# Include only needed drivers and libraries
ifeq ($(DRV_CONRAD), y)
LIBS	+= -lusb-1.0
OPTS	+= -DDRV_CONRAD
endif
ifeq ($(DRV_SAINSMART), y)
LIBS	+= -lftdi
OPTS	+= -DDRV_SAINSMART
endif
ifeq ($(DRV_SAINSMART16), y)
LIBS	+= -lhidapi-libusb
OPTS	+= -DDRV_SAINSMART16
endif
ifeq ($(DRV_HIDAPI), y)
LIBS	+= -lhidapi-libusb
OPTS	+= -DDRV_HIDAPI
endif

OBJ	= $(SRC:.c=.o)

all:	$(BIN)

$(BIN):	$(OBJ)
	@echo "[Link $(BIN)] with libs $(LIBS)"
	@$(CC) -o $(BIN) $(OBJ) $(LDFLAGS) $(LIBS)

.c.o:
	@echo "[Compile $<]"
	@$(CC) -c $(CFLAGS) $< -o $@  $(OPTS)

.PHONEY:	clean
clean:
	@echo "[Clean]"
	@rm -f $(OBJ) $(BIN)

.PHONEY:	install
install:	$(BIN)
	@echo "[Install binary]"
	@install -m 0755 -d		$(DESTDIR)$(PREFIX)/bin
	@install -m 0755 $(BIN)		$(DESTDIR)$(PREFIX)/bin/$(BIN)

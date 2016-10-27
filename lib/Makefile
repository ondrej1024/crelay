# ;
# Makefile:
###############################################################################
#
#  crelay library 
#
###############################################################################

DYN_VERS_MAJ=0
DYN_VERS_MIN=1

VERSION=$(DYN_VERS_MAJ).$(DYN_VERS_MIN)
DESTDIR=/usr
PREFIX=/local
SRCDIR=../src

NAME=libcrelay
STATIC=$(NAME).a
DYNAMIC=$(NAME).so.$(VERSION)

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

OPTS	= -DBUILD_LIB

# Main source files (don't change)
#########################################
SRC	= $(SRCDIR)/relay_drv.c

# Relay card specific driver source files
#########################################

#SRC	+= $(SRCDIR)/relay_drv_gpio.c

# Include only needed drivers and libraries
ifeq ($(DRV_CONRAD), y)
SRC	+= $(SRCDIR)/relay_drv_conrad.c
LIBS	+= -lusb-1.0
OPTS	+= -DDRV_CONRAD
endif
ifeq ($(DRV_SAINSMART), y)
SRC	+= $(SRCDIR)/relay_drv_sainsmart.c
LIBS	+= -lftdi
OPTS	+= -DDRV_SAINSMART
endif
ifeq ($(DRV_SAINSMART16), y)
SRC	+= $(SRCDIR)/relay_drv_sainsmart16.c
LIBS	+= -lhidapi-libusb
OPTS	+= -DDRV_SAINSMART16
endif
ifeq ($(DRV_HIDAPI), y)
SRC	+= $(SRCDIR)/relay_drv_hidapi.c
LIBS	+= -lhidapi-libusb
OPTS	+= -DDRV_HIDAPI
endif


OBJ	=	$(SRC:.c=.o)

all:		$(DYNAMIC)

static:		$(STATIC)

$(STATIC):	$(OBJ)
	@echo "[Link (Static)]"
	@ar rcs $(STATIC) $(OBJ)
	@ranlib $(STATIC)
#	@size   $(STATIC)

$(DYNAMIC):	$(OBJ)
	@echo "[Link (Dynamic)]"
	@$(CC) -shared -Wl,-soname,$(NAME).so -o $(NAME).so.$(VERSION) -lrt $(OBJ)

.c.o:
	@echo "[Compile $<]"
	@$(CC) -c $(CFLAGS) $< -o $@ $(OPTS)

.PHONEY:	clean
clean:
	@echo "[Clean]"
	@rm -f $(OBJ) $(OBJ_I2C) *~ core tags Makefile.bak $(NAME).*

.PHONEY:	tags
tags:	$(SRC)
	@echo [ctags]
	@ctags $(SRC)


.PHONEY:	install-headers
install-headers:
	@echo "[Install Headers]"
	@install -m 0755 -d		$(DESTDIR)$(PREFIX)/include
	@install -m 0644 $(SRCDIR)/relay_drv.h	$(DESTDIR)$(PREFIX)/include

.PHONEY:	install
install:	$(DYNAMIC) install-headers
	@echo "[Install Dynamic Lib]"
	@install -m 0755 -d					$(DESTDIR)$(PREFIX)/lib
	@install -m 0755 $(NAME).so.$(VERSION)			$(DESTDIR)$(PREFIX)/lib/$(NAME).so.$(VERSION)
	@ln -sf $(DESTDIR)$(PREFIX)/lib/$(NAME).so.$(VERSION)	$(DESTDIR)/lib/$(NAME).so
	@ldconfig

.PHONEY:	install-static
install-static:	$(STATIC) install-headers
	@echo "[Install Static Lib]"
	@install -m 0755 -d		$(DESTDIR)$(PREFIX)/lib
	@install -m 0755 $(NAME).a	$(DESTDIR)$(PREFIX)/lib

.PHONEY:	uninstall
uninstall:
	@echo "[UnInstall]"
	@rm -f $(DESTDIR)$(PREFIX)/include/relay_drv.h
	@rm -f $(DESTDIR)$(PREFIX)/lib/$(NAME).*
	@ldconfig


# DO NOT DELETE

relay_drv.o: relay_drv.h
 

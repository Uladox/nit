# See LICENSE file for copyright and license details.

include config.mk

SRC = hashmap.c maputils.c socket.c bimap.c
OBJ = $(SRC:.c=.o)

LIB = libnit.a
INC = list.h maputils.h hashmap.h socket.h bimap.h

all: $(LIB)

$(LIB): $(OBJ)
	@$(AR) -rcs $@ $(OBJ)

.c.o:
	@$(CC) $(CFLAGS) -c $<

install: $(LIB) $(INC)
	@echo @ install nitlib to $(DESTDIR)$(PREFIX)
	@mkdir -p $(DESTDIR)$(PREFIX)/lib
	@cp $(LIB) $(DESTDIR)$(PREFIX)/lib/$(LIB)
	@mkdir -p $(DESTDIR)$(PREFIX)/include/nitlib
	@cp $(INC) $(DESTDIR)$(PREFIX)/include/nitlib/

uninstall:
	@echo @ uninstall nitlib from $(DESTDIR)$(PREFIX)
	@rm -f $(DESTDIR)$(PREFIX)/lib/$(LIB)
	@rm -f $(DESTDIR)$(PREFIX)/include/nitlib/$(INC)

clean:
	rm -f $(LIB) $(OBJ)

# See LICENSE file for copyright and license details.

include config.mk

SRC = hashmap.c maputils.c socket.c bimap.c io.c
OBJ = $(SRC:.c=.o)

LIB = libnit.a
INC = list.h maputils.h hashmap.h socket.h bimap.h io.h palloc.h macros.h

all: $(LIB)

$(LIB): $(OBJ)
	@$(AR) -rcs $@ $(OBJ)

.c.o:
	@$(CC) $(CFLAGS) -c $<

install: $(LIB) $(INC)
	@echo @ install nitlib to $(DESTDIR)$(PREFIX)
	@mkdir -p $(DESTDIR)$(PREFIX)/lib
	@cp $(LIB) $(DESTDIR)$(PREFIX)/lib/$(LIB)
	@mkdir -p $(DESTDIR)$(PREFIX)/include/nit
	@cp $(INC) $(DESTDIR)$(PREFIX)/include/nit/

uninstall:
	@echo @ uninstall nit from $(DESTDIR)$(PREFIX)
	@rm -f $(DESTDIR)$(PREFIX)/lib/$(LIB)
	@rm -rf $(DESTDIR)$(PREFIX)/include/nit/

clean:
	rm -f $(LIB) $(OBJ)

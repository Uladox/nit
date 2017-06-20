# See LICENSE file for copyright and license details.

include config.mk

SRC = hset.c hmap.c
OBJ = $(SRC:.c=.o)

LIB = libnit.a
INC = macros.h palloc.h list.h hset.h hmap.h

all: $(LIB)

$(LIB): $(OBJ)
	@$(AR) -rcs $@ $(OBJ)

.c.o:
	@$(CC) $(CFLAGS) -c $<

install: $(LIB) $(INC)
	@echo @ install nit to $(DESTDIR)$(PREFIX)
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

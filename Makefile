# See LICENSE file for copyright and license details.

include config.mk

SRC = hmap.c socket.c io.c gap-buf.c gc.c hset.c radix.c
OBJ = $(SRC:.c=.o)

LIB = libnit.a
INC = list.h hmap.h socket.h io.h palloc.h macros.h \
      gap-buf.h gc.h hset.h radix.h

all: $(LIB)

$(LIB): $(OBJ)
	@$(AR) -rcs $@ $(OBJ)

.c.o:
	@$(CC) $(CFLAGS) -c $<

install: $(LIB) $(INC)
	@echo @ install libnit to $(DESTDIR)$(PREFIX)
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

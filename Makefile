CC = gcc
EFLAGS = -fsanitize=address 

SRCDIR = src
INCDIR = include
LIBDIR = lib
# please don't put '/' here otherwise you might wipe you system,if linux like
BUILDDIR = build
BINDIR = bin

_CFLAGS = -I$(INCDIR) -L$(LIBDIR) -ggdb3 -O0 -Wall -Wextra  -Wuninitialized
LIBARIES = ""
ifeq ($(OS),Windows_NT)
	CFLAGS = $(_CFLAGS)   -lws2_32 -DWIN32 
	LIBARIES = -l:civetwebwin.o -l:sqlite3.o
else
	CFLAGS = $(_CFLAGS)
	LIBARIES = -l:civetweb.o -l:sqlite3.o
endif

SOURCE = $(filter-out $(wildcard $(SRCDIR)/*thread*.c), $(wildcard $(SRCDIR)/*.c))
SOURCE_OBJ = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCE))

EXECUTABLE = peanut

# p: pbsql

r:all run

all:$(EXECUTABLE)

$(EXECUTABLE):$(SOURCE_OBJ)
	$(CC)  $(LIBARIES) $^ -o $@  $(CFLAGS)
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC)  $(LIBARIES) -c $< -o $@ $(CFLAGS)
run:
	./$(EXECUTABLE)
clean:
ifeq ($(OS),Windows_NT)
else
	rm -r $(BUILDDIR)
endif
	mkdir build

pbsql:
	$(CC) $(SRCDIR)/pbsql_parser.c -o a.out $(CFLAGS)
	./a.out

.PHONY: all clean run
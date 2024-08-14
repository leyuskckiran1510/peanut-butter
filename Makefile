CC = gcc
EFLAGS = -fsanitize=address 

SRCDIR = src
INCDIR = include
LIBDIR = lib
# please don't put '/' here otherwise you might wipe you system,if linux like
BUILDDIR = build
BINDIR = bin

_CFLAGS = -I$(INCDIR) -L$(LIBDIR) -ggdb3 -O3 -Wall -Wextra  -Wuninitialized
ifeq ($(OS),Windows_NT)
CFLAGS = $(_CFLAGS)   -lws2_32 -DWIN32 
CIVETWEB = civetwebwin.o
else
CFLAGS = $(_CFLAGS)
CIVETWEB = civetweb.o
endif

SOURCE = $(filter-out $(wildcard $(SRCDIR)/*thread*.c), $(wildcard $(SRCDIR)/*.c))
SOURCE_OBJ = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCE))

EXECUTABLE = peanut


r:clean all run

all:$(EXECUTABLE)

$(EXECUTABLE):$(SOURCE_OBJ)
	$(CC)  -l:$(CIVETWEB) $^ -o $@  $(CFLAGS)
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC)  -l:$(CIVETWEB) -c $< -o $@ $(CFLAGS)
run:
	./$(EXECUTABLE)
clean:
ifeq ($(OS),Windows_NT)
else
	rm -r $(BUILDDIR)
endif
	mkdir build

.PHONY: all clean run
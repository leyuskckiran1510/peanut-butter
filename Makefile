CC = gcc
EFLAGS = -Wall -Wextra -pedantic   -lpthread  -fsanitize=address

SRCDIR = src
INCDIR = include
LIBDIR = lib
BUILDDIR = build
BINDIR = bin

_CFLAGS = -I$(INCDIR) -L$(LIBDIR) -ggdb3 -O3 
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


r:all run

all:$(EXECUTABLE)

$(EXECUTABLE):$(SOURCE_OBJ)
	$(CC)  -l:$(CIVETWEB) $^ -o $@  $(CFLAGS)
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC)  -l:$(CIVETWEB) -c $< -o $@ $(CFLAGS)
run:
	./$(EXECUTABLE)
clean:
	rm  $(BUILDDIR)
	mkdir build
	

.PHONY: all clean run
CC = gcc
EFLAGS = -Wall -Wextra -pedantic   -lpthread  -fsanitize=address
SRCDIR = src
INCDIR = ./include
LIBDIR = ./lib
BUILDDIR = build
BINDIR = bin
CFLAGS = -I$(INCDIR) -L$(LIBDIR) -ggdb3 -O3
CIVETWEB = civetweb.o


SOURCE = $(filter-out $(wildcard $(SRCDIR)/*thread*.c), $(wildcard $(SRCDIR)/*.c))
SOURCE_OBJ = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCE))

EXECUTABLE = peanut


r:clean all run

all:$(EXECUTABLE)

$(EXECUTABLE):$(SOURCE_OBJ)
	$(CC) $(CFLAGS) -l:$(CIVETWEB) $^ -o $@ 

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -l:$(CIVETWEB) -c $< -o $@ $(CFLAGS)


run:
	./$(EXECUTABLE)


clean:
	@rm -f $(BUILDDIR)/*.o
	

.PHONY: all clean run
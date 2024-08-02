CC = gcc
EFLAGS = -ggdb3 -Wall -Wextra -pedantic   -lpthread -fsanitize=address
SRCDIR = src
INCDIR = ./include
LIBDIR = ./lib
BUILDDIR = build
BINDIR = bin
CFLAGS = -I$(INCDIR) -L$(LIBDIR)
CIVETWEB = civetweb.o

SOURCE = $(wildcard $(SRCDIR)/*.c)
SOURCE_OBJ = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCE))

EXECUTABLE = peanut

all:$(EXECUTABLE)

$(EXECUTABLE):$(SOURCE_OBJ)
	$(CC) $(CFLAGS) -l:$(CIVETWEB) $^ -o $@ 

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -l:$(CIVETWEB) -c $< -o $@ $(CFLAGS)

r:run

run:
	./$(EXECUTABLE)


clean:
	@rm -f $(BUILDDIR)/*.o
	

.PHONY: all clean run
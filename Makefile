CC = gcc
FLAGS = -ggdb3 -Wall -Wextra -pedantic -I./include  -lpthread -fsanitize=address

SRCDIR = src
INCDIR = include
BUILDDIR = build
BINDIR = bin

SOURCE = $(wildcard $(SRCDIR)/*.c)
SOURCE_OBJ = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCE))

EXECUTABLE = peanut

all:$(EXECUTABLE)

$(EXECUTABLE):$(SOURCE_OBJ)
	$(CC)  $^ -o $@ $(CFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC)  -c $< -o $@ $(CFLAGS)

r:run

run:
	./$(EXECUTABLE)


clean:
	@rm -f $(BUILDDIR)/*.o
	

.PHONY: all clean run
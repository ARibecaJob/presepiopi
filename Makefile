NAME = presepiopi
BIN = bin/
CC = gcc

CFLAGS_DEBUG = -Wall -Wextra -D_DEBUG -g -fsanitize=address -pthread -lpigpio -lrt -lm
CFLAGS_RELEASE = -Wall -Wextra -O3 -pthread -lpigpio -lrt -lm

all: release

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: $(BIN)$(NAME)

release: CFLAGS = $(CFLAGS_RELEASE)
release: $(BIN)$(NAME)

$(BIN)$(NAME): src/main.c
	mkdir -p $(BIN)
	$(CC) -o $(BIN)$(NAME) src/main.c $(CFLAGS)
	
clean:
	rm -f $(BIN)$(NAME)
	rm -f *.o
	
run:
	@echo "Running $(NAME)..."
	@sudo $(BIN)$(NAME)

NAME = button
CC = gcc

CFLAGS_DEBUG = -Wall -Wextra -D_DEBUG -g -fsanitize=address -pthread -lpigpio -lrt -lm
CFLAGS_RELEASE = -Wall -Wextra -O3 -pthread -lpigpio -lrt -lm

all: debug

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: $(NAME)

release: CFLAGS = $(CFLAGS_RELEASE)
release: $(NAME)

$(NAME): main.c
	$(CC) -o $(NAME) main.c $(CFLAGS)
	
clean:
	rm -f $(NAME)
	rm -f *.o
	
run:
	@echo "Running $(NAME)..."
	@sudo $(NAME)

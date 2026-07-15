NAME := ft_malcolm

SRCDIR := src
OBJDIR := build
INCDIR := include
LIBDIR := lib

SRC :=
vpath %.c $(SRCDIR)
SRC += main.c
SRC += signals.c
SRC += ip_utils.c
SRC += if_utils.c
SRC += mac_utils.c
SRC += arp_utils.c

OBJ := $(SRC:.c=.o)
OBJ := $(addprefix $(OBJDIR)/, $(OBJ))

DEPS := $(OBJ:.o=.d)
-include $(DEPS)

LIBFTDIR := $(LIBDIR)/libft
LIBFT := $(LIBFTDIR)/libft.a
LIBFT_INC := $(LIBFTDIR)/include

CC := cc
CFLAGS := -Wall -Wextra -Werror
CFLAGS += -pedantic -Wconversion -Wunreachable-code -Wshadow
CPPFLAGS := -I$(INCDIR) -I$(LIBFT_INC) -MD -MP
LDFLAGS := -L$(LIBFTDIR)
LDLIBS := -lft

debug: DEBUG := 1
ifeq ($(DEBUG), 1)
	CFLAGS += -ggdb3 -Og
	CPPFLAGS += -DDEBUG=1
	LDFLAGS += -ggdb3 -Og -DDEBUG=1
endif

MKDIR := mkdir -p
RM := rm -f

PLATFORM := linux/amd64
SHELL := /bin/bash

RED := \033[31m
GRN := \033[32m
MAG := \033[35m
MAB := \033[1;35m
CYA := \033[36m
CYB := \033[1;36m
NC  := \033[0m

.PHONY: all debug clean fclean re run help container shell
.DEFAULT_GOAL := all

all: $(NAME) # Compile all targets

$(NAME): $(LIBFT) $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS) $(LDLIBS)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJDIR):
	$(MKDIR) $@

$(LIBFT):
	$(MAKE) -C $(LIBFTDIR)

clean: # Clean intermediary files
	$(RM) $(OBJ)
	$(RM) -r $(OBJDIR)
	$(MAKE) -C $(LIBFTDIR) clean

fclean: clean # Clean all compiled files
	$(RM) $(NAME)
	$(MAKE) -C $(LIBFTDIR) fclean

re: fclean # Recompile all
	@$(MAKE) all

run: $(NAME) # Run the compiled program
	@./$(NAME) $(filter-out run,$(MAKECMDGOALS))

debug: fclean
	@DEBUG=1 $(MAKE) all

help:	# Show this helpful message
	@awk 'BEGIN { FS = ":.*#"; \
	printf "$(GRN)$(NAME)$(NC)\n"; \
	printf "Usage:\n\t$(CYB)make $(MAG)<target>$(NC)\n" } \
	/^[A-Za-z_0-9-]+:.*?#/ { printf "$(MAB)%-16s $(CYA)%s$(NC)\n", $$1, $$2}' Makefile

container: # Build a Docker container for the project
	@docker build --platform $(PLATFORM) -t $(NAME) .

shell: container # Run shell in the Docker container
	@docker run --rm --mount type=bind,src=${PWD},dst=/app --cap-add=NET_RAW --cap-add=NET_ADMIN --platform $(PLATFORM) -it $(NAME) /usr/bin/bash

# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jose-gon <jose-gon@student.42malaga.com>   +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/10/22 10:30:18 by jose-gon          #+#    #+#              #
#    Updated: 2026/05/26 22:02:02 by jose-gon         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv

CFLAGS = -Wall -Wextra -Werror -std=c++98

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = ./include

FILES = main.cpp Client.cpp Channel.cpp Aux.cpp
CMD_FILES = invite.cpp join.cpp kick.cpp mode.cpp nick.cpp part.cpp pass.cpp privmsg.cpp quit.cpp topic.cpp user.cpp
ALL_FILES = $(FILES) $(CMD_FILES)
OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(ALL_FILES:.cpp=.o))

all : $(NAME)

$(NAME): $(OBJ_FILES)
	c++ $(CFLAGS) -o $@ $^
	
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	c++ $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/commands/%.cpp
	@mkdir -p $(OBJ_DIR)
	c++ $(CFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	
fclean: clean 
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re

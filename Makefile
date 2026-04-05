# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: gekido <gekido@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/03/05 15:01:59 by egerin            #+#    #+#              #
#    Updated: 2026/03/17 05:28:16 by gekido           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS = main.cpp \
		Server.cpp \
		Client.cpp \
		Channel.cpp \
		Command.cpp \
		Nick.cpp \
		Pass.cpp \
		Join.cpp \
		User.cpp \
		Privmsg.cpp \
		Topic.cpp \
		Kick.cpp \
		Invite.cpp \
		Mode.cpp \
		Quit.cpp \
		Part.cpp
SRCS_PATH = ./src/

OBJS = ${patsubst %.cpp, ${OBJS_PATH}/%.o, ${SRCS}}
OBJS_PATH = ./objs/

CC = c++
INC = -I includes/
CFLAGS = -Wall -Wextra -Werror -std=c++98 -g3

vpath %.cpp ${SRCS_PATH}

NAME = ircserv
BOT_NAME = ircbot

all: ${NAME}

${OBJS}: ${OBJS_PATH}/%.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@ $(INC)

${NAME}: ${OBJS}
	@$(CC) $(CFLAGS) -o ${NAME} ${OBJS} $(INC)
	@echo "=== \033[0;32m${NAME} compiled successfully\033[0m ==="

clean:
	@rm -rf ${OBJS_PATH} ${BOT_OBJS}

fclean: clean
	@rm -f ${NAME} $(BOT_NAME)

re: fclean all

.PHONY: all clean fclean re
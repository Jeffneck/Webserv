NAME		= webserv

CC			= c++

CFLAGS		= -std=c++98 -g3 -Wall -Wextra -Werror -D_GLIBCXX_USE_CXX11_ABI=0

SRC_FILES 	=	src/main.cpp \
				src/HttpRequest.cpp \
				src/HttpResponse.cpp \
				src/RequestHandler.cpp \
				src/Server.cpp \
				src/Location.cpp \
				src/WebServer.cpp \
				src/ListeningSocket.cpp \
				src/ListeningSocketHandler.cpp \
				src/DataSocket.cpp \
				src/DataSocketHandler.cpp \
				src/Config.cpp \
				src/ConfigParser.cpp \
				src/Exceptions.cpp \
				src/Utils.cpp \
				src/CgiProcess.cpp \
				src/Error.cpp \
				


OBJ			= $(SRC_FILES:.cpp=.o)

INC 		=	includes/HttpRequest.hpp \
				includes/HttpResponse.hpp \
				includes/RequestHandler.hpp \
				includes/Server.hpp \
				includes/Location.hpp \
				includes/WebServer.hpp \
				includes/ListeningSocket.hpp \
				includes/ListeningSocketHandler.hpp \
				includes/DataSocket.hpp \
				includes/DataSocketHandler.hpp \
				includes/Config.hpp \
				includes/ConfigParser.hpp \
				includes/Exceptions.hpp \
				includes/Utils.hpp \
				includes/CgiProcess.hpp \
				includes/Error.hpp \
				

%.o   : %.cpp $(INC)
	${CC} ${CFLAGS} -c $< -o $@ -I./includes

all: $(NAME)

$(NAME): $(OBJ)
	@echo -n Compiling executable $(NAME)...
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJ)
	@echo Done.

clean:
	@echo -n Making clean...
	@rm -rf $(OBJ)
	@echo Done.

fclean: clean
	@echo -n Making fclean...
	@rm -f $(NAME)
	@echo Done.

test: re
	./$(NAME)
	@make fclean

re: fclean all

.PHONY : all clean fclean re test
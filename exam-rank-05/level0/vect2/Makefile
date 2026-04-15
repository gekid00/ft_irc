NAME	= vect2
CXX		= c++
CXXFLAGS= -Wall -Wextra -Werror -std=c++11

SRCS	= vect2.cpp main.cpp
OBJS	= $(SRCS:.cpp=.o)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp vect2.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: clean fclean re

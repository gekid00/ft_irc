NAME	= polyset
CXX		= c++
CXXFLAGS= -Wall -Wextra -Werror -std=c++11

SRCS	= array_bag.cpp \
		  tree_bag.cpp \
		  searchable_array_bag.cpp \
		  searchable_tree_bag.cpp \
		  set.cpp \
		  main.cpp

OBJS	= $(SRCS:.cpp=.o)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: clean fclean re

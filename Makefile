CXX = gcc
CXXFLAGS = -Wall -Werror -Wextra -O2 -I include -pedantic -std=c17 -g 

SRC = src/main.c
OBJ = $(SRC:.cc=.o)
EXEC = main

SRCACO = src/aco_tests.c
OBJACO = $(SRCACO:.cc=.o)
EXECACO = main

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LBLIBS)

aco: $(OBJACO)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJACO) $(LBLIBS)

clean:
	rm -rf main.exe
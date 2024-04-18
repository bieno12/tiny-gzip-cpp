# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++11
# Include directories
INCLUDES = -I.
# Source files
SRCS = tinygzip.cpp GZFile.cpp
# Object files
OBJS = $(SRCS:.cpp=.o)
# Library name
LIBRARY = ../libtinygzip.a

all: $(LIBRARY)

$(LIBRARY): $(OBJS)
	ar rcs $@ $(OBJS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(LIBRARY)

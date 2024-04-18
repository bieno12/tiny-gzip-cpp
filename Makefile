# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++11
# Include directories
INCLUDES = -I./Include/
# Source directory
SRC_DIR = ./src
# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
# Object files
OBJS = $(SRCS:.cpp=.o)
# Library directory
LIB_DIR = ./lib
# Library name
LIBRARY = $(LIB_DIR)/libtinygzip.a
# Executable name
EXECUTABLE = my_gzip_program

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS) $(LIBRARY)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(OBJS) $(LIBRARY)

$(LIBRARY):
	make -C lib/tinygzip

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXECUTABLE)
	make -C lib/tinygzip clean

run: $(EXECUTABLE)
	./$(EXECUTABLE)
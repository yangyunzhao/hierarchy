CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -pedantic -g
LDFLAGS =

# Source files
SRCS = HierarchyScopeTracker.cpp
OBJS = $(SRCS:.cpp=.o)

# Test files
TEST_SRCS = test_hierarchy.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_TARGET = test_hierarchy

# Library target
LIB_TARGET = libhierarchy.a

.PHONY: all clean test

all: $(LIB_TARGET) $(TEST_TARGET)

# Build static library
$(LIB_TARGET): $(OBJS)
	ar rcs $@ $^

# Build test executable
$(TEST_TARGET): $(TEST_OBJS) $(LIB_TARGET)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_OBJS) -L. -lhierarchy $(LDFLAGS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TEST_OBJS) $(LIB_TARGET) $(TEST_TARGET)

# Dependencies
HierarchyScopeTracker.o: HierarchyScopeTracker.cpp HierarchyScopeTracker.h
test_hierarchy.o: test_hierarchy.cpp HierarchyScopeTracker.h

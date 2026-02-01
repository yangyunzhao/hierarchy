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

# Example files
EXAMPLE_SRCS = example.cpp
EXAMPLE_OBJS = $(EXAMPLE_SRCS:.cpp=.o)
EXAMPLE_TARGET = example

# Library target
LIB_TARGET = libhierarchy.a

.PHONY: all clean test run-example

all: $(LIB_TARGET) $(TEST_TARGET) $(EXAMPLE_TARGET)

# Build static library
$(LIB_TARGET): $(OBJS)
	ar rcs $@ $^

# Build test executable
$(TEST_TARGET): $(TEST_OBJS) $(LIB_TARGET)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_OBJS) -L. -lhierarchy $(LDFLAGS)

# Build example executable
$(EXAMPLE_TARGET): $(EXAMPLE_OBJS) $(LIB_TARGET)
	$(CXX) $(CXXFLAGS) -o $@ $(EXAMPLE_OBJS) -L. -lhierarchy $(LDFLAGS)

# Compile source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Run example
run-example: $(EXAMPLE_TARGET)
	./$(EXAMPLE_TARGET)

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TEST_OBJS) $(EXAMPLE_OBJS) $(LIB_TARGET) $(TEST_TARGET) $(EXAMPLE_TARGET)

# Dependencies
HierarchyScopeTracker.o: HierarchyScopeTracker.cpp HierarchyScopeTracker.h
test_hierarchy.o: test_hierarchy.cpp HierarchyScopeTracker.h
example.o: example.cpp HierarchyScopeTracker.h

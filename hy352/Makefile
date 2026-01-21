# Makefile for Tekken Battle Simulation DSL
# HY352 - Software Technology

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

# Targets
TARGETS = test_battle example_simple example_advanced

.PHONY: all clean run_basic run_simple run_advanced help

all: $(TARGETS)

# Build individual programs
test_battle: test_battle.cpp Tekken.h
	$(CXX) $(CXXFLAGS) -o $@ $<

example_simple: example_simple.cpp Tekken.h
	$(CXX) $(CXXFLAGS) -o $@ $<

example_advanced: example_advanced.cpp Tekken.h
	$(CXX) $(CXXFLAGS) -o $@ $<

# Run targets
run_basic: test_battle
	@echo "=== Running Basic Example (Lee vs Jack-6) ==="
	@./test_battle

run_simple: example_simple
	@echo "=== Running Simple Example (4 fighters) ==="
	@./example_simple

run_advanced: example_advanced
	@echo "=== Running Advanced Example (Conditional abilities) ==="
	@./example_advanced

# Clean build artifacts
clean:
	rm -f $(TARGETS)
	@echo "Cleaned all build artifacts"

# Help target
help:
	@echo "Tekken Battle Simulation DSL - Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all              - Build all examples"
	@echo "  test_battle      - Build basic example from assignment"
	@echo "  example_simple   - Build simple demonstration"
	@echo "  example_advanced - Build advanced example"
	@echo ""
	@echo "  run_basic        - Build and run basic example"
	@echo "  run_simple       - Build and run simple example"
	@echo "  run_advanced     - Build and run advanced example"
	@echo ""
	@echo "  clean            - Remove all build artifacts"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make                  # Build all"
	@echo "  make run_simple       # Build and run simple example"
	@echo "  make clean all        # Clean and rebuild all"

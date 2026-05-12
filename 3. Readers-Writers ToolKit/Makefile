# ── Readers–Writers Toolkit ─ Makefile ────────────────────────────
#
# Targets:
#   make            Build the simulator (default)
#   make clean      Remove the binary
#   make run        Build and run with defaults (3 readers, 2 writers, reader-priority)
#   make run-wp     Build and run in writer-priority mode with logging
#   make help       Show usage
#

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -pthread
TARGET   := rw_simulator

.PHONY: all clean run run-wp help

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<
	@echo "  ✓ Built: ./$(TARGET)"

clean:
	rm -f $(TARGET)
	@echo "  ✓ Cleaned"

run: all
	./$(TARGET) -r 3 -w 2 -m reader -i 4

run-wp: all
	./$(TARGET) -r 4 -w 3 -m writer -i 3 -l run.log

help: all
	./$(TARGET) -h

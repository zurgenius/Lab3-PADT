CXX = g++
CPPFLAGS = -Iinclude -Igoogletest/googletest/include -Igoogletest/googletest
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic
LINT_FLAGS = -Werror

PROGRAM_SRCS = src/main.cpp src/menu.cpp src/bit_sequence.cpp
TEST_SRCS = tests/tests.cpp src/bit_sequence.cpp \
	googletest/googletest/src/gtest-all.cc \
	googletest/googletest/src/gtest_main.cc
TEST_BIN = tests_runner
FORMAT_FILES = $(shell find include src tests -type f \( -name '*.h' -o -name '*.cpp' -o -name '*.tpp' \))

.PHONY: all program tests lint format clean

all: program

program:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(PROGRAM_SRCS) -o program

tests:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(TEST_SRCS) -o $(TEST_BIN)

lint:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LINT_FLAGS) $(PROGRAM_SRCS) -o /tmp/lab2_program_lint
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LINT_FLAGS) $(TEST_SRCS) -o /tmp/lab2_tests_lint

format:
	@if command -v clang-format >/dev/null 2>&1; then \
		clang-format -i $(FORMAT_FILES); \
	else \
		echo "clang-format is not installed"; \
		exit 1; \
	fi

clean:
	rm -f program $(TEST_BIN) /tmp/lab2_program_lint /tmp/lab2_tests_lint

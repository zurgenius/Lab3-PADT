BUILD_DIR = build
LINT_BUILD_DIR = build-lint
FORMAT_FILES = $(shell find include src tests -type f \( -name '*.h' -o -name '*.cpp' -o -name '*.tpp' \))

.PHONY: all configure program run tests lint format clean

all: program

configure:
	cmake -S . -B $(BUILD_DIR)

program: configure
	cmake --build $(BUILD_DIR) --target lab2_program
	cp $(BUILD_DIR)/program program

run: program
	./program

tests: configure
	cmake --build $(BUILD_DIR) --target lab2_tests
	ctest --test-dir $(BUILD_DIR) --output-on-failure
	cp $(BUILD_DIR)/tests_runner tests_runner

lint:
	cmake -S . -B $(LINT_BUILD_DIR) -DLAB2_ENABLE_WERROR=ON
	cmake --build $(LINT_BUILD_DIR) --target lab2_program
	cmake --build $(LINT_BUILD_DIR) --target lab2_tests
	ctest --test-dir $(LINT_BUILD_DIR) --output-on-failure

format:
	@if command -v clang-format >/dev/null 2>&1; then \
		clang-format -i $(FORMAT_FILES); \
	else \
		echo "clang-format is not installed"; \
		exit 1; \
	fi

clean:
	rm -rf $(BUILD_DIR) $(LINT_BUILD_DIR) program tests_runner

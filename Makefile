# RouteGuard Makefile - ISO C11 Graph-Based Route Discovery Simulator
# Portable across Windows environments with mingw32-make or GNU Make

# Compiler discovery precedence:
# 1. RG_GCC_DIR if defined (directory containing gcc)
# 2. gcc discoverable on PATH
# 3. Actionable error if neither is available
ifdef RG_GCC_DIR
  CC := $(subst \,/,$(RG_GCC_DIR))/gcc
  export PATH := $(subst \,/,$(RG_GCC_DIR)):$(PATH)
else
  CC := gcc
endif

# Optimisation level configurable via RG_OPT (default: -O2)
ifdef RG_OPT
  OPT := $(RG_OPT)
else
  OPT ?= -O2
endif
CFLAGS = -std=c11 -Wall -Wextra -pedantic -Wshadow $(OPT) -Iinclude
STRICT_CFLAGS = -std=c11 -Wall -Wextra -pedantic -Wshadow -Werror $(OPT) -Iinclude

SRC = src/rg_alloc.c src/rg_graph.c src/rg_queue.c src/rg_stack.c \
      src/rg_traverse.c src/rg_analysis.c src/rg_fixture.c src/rg_svg.c

TEST_SRC = tests/test_main.c tests/test_graph.c tests/test_traverse.c \
           tests/test_analysis.c tests/test_fixture_ref.c

OBJ = $(patsubst src/%.c,obj/%.o,$(SRC))
TEST_OBJ = $(patsubst tests/%.c,obj/%.o,$(TEST_SRC))

TARGET = bin/routeguard.exe
TEST_TARGET = bin/rg_tests.exe
TARGET_CMD = bin\routeguard.exe
TEST_CMD = bin\rg_tests.exe

.PHONY: all strict test demo demo-check clean dirs compiler-check

all: dirs compiler-check $(TARGET) $(TEST_TARGET)

compiler-check:
	@$(CC) --version > nul 2>&1 || (echo ERROR: gcc not found. Set RG_GCC_DIR or add gcc to PATH. && exit 1)

dirs:
	@if not exist bin mkdir bin
	@if not exist obj mkdir obj
	@if not exist out mkdir out

$(TARGET): $(OBJ) obj/main.o
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_TARGET): $(OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.o: tests/%.c
	$(CC) $(CFLAGS) -c $< -o $@

strict:
	$(MAKE) clean
	$(MAKE) all CFLAGS="$(STRICT_CFLAGS)"

test: $(TEST_TARGET)
	$(TEST_CMD)

demo: $(TARGET)
	$(TARGET_CMD) --demo

demo-check: $(TARGET)
	@if not exist out mkdir out
	$(TARGET_CMD) --demo > out\demo_run1.txt
	$(TARGET_CMD) --demo > out\demo_run2.txt
	fc /B out\demo_run1.txt out\demo_run2.txt > nul
	@echo DEMO DETERMINISM VERIFIED: Both runs are byte-identical.

clean:
	@if exist obj rmdir /s /q obj
	@if exist bin rmdir /s /q bin
	@if exist out\demo_run1.txt del /q out\demo_run1.txt
	@if exist out\demo_run2.txt del /q out\demo_run2.txt
	@if exist out\*.svg del /q out\*.svg

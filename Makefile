# RouteGuard Makefile - ISO C11 Graph-Based Route Discovery Simulator
# Portable across Windows environments with mingw32-make or GNU Make

# Shell detection and OS-specific tool configuration:
ifeq ($(OS),Windows_NT)
  EXE_EXT := .exe
  ifdef COMSPEC
    SHELL := $(COMSPEC)
  else
    SHELL := cmd.exe
  endif
  .SHELLFLAGS := /c
  DEVNUL := nul
  TARGET_CMD := bin\routeguard.exe
  TEST_CMD := bin\rg_tests.exe
  OUT_RUN1 := out\demo_run1.txt
  OUT_RUN2 := out\demo_run2.txt
else
  EXE_EXT :=
  SHELL := /bin/sh
  DEVNUL := /dev/null
  TARGET_CMD := ./bin/routeguard
  TEST_CMD := ./bin/rg_tests
  OUT_RUN1 := out/demo_run1.txt
  OUT_RUN2 := out/demo_run2.txt
endif

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

# Optimisation knob: RG_OPT is authoritative, OPT is supported as an alias (default: -O2)
ifdef RG_OPT
  OPT_LEVEL := $(RG_OPT)
else ifdef OPT
  OPT_LEVEL := $(OPT)
else
  OPT_LEVEL := -O2
endif

CFLAGS = -std=c11 -Wall -Wextra -pedantic -Wshadow $(OPT_LEVEL) -Iinclude
STRICT_CFLAGS = -std=c11 -Wall -Wextra -pedantic -Wshadow -Werror $(OPT_LEVEL) -Iinclude

SRC = src/rg_alloc.c src/rg_graph.c src/rg_queue.c src/rg_stack.c \
      src/rg_traverse.c src/rg_analysis.c src/rg_fixture.c src/rg_svg.c

TEST_SRC = tests/test_main.c tests/test_graph.c tests/test_traverse.c \
           tests/test_analysis.c tests/test_fixture_ref.c tests/test_random.c

OBJ = $(patsubst src/%.c,obj/%.o,$(SRC))
TEST_OBJ = $(patsubst tests/%.c,obj/%.o,$(TEST_SRC))

TARGET = bin/routeguard$(EXE_EXT)
TEST_TARGET = bin/rg_tests$(EXE_EXT)

.PHONY: all strict test demo demo-check clean dirs compiler-check

all: dirs compiler-check $(TARGET) $(TEST_TARGET)

compiler-check:
	@$(CC) --version > $(DEVNUL) 2>&1 || (echo ERROR: gcc not found. Set RG_GCC_DIR or add gcc to PATH. && exit 1)

dirs:
ifeq ($(OS),Windows_NT)
	@if not exist bin mkdir bin
	@if not exist obj mkdir obj
	@if not exist out mkdir out
else
	@mkdir -p bin obj out
endif

$(TARGET): $(OBJ) obj/main.o | dirs
	$(CC) $(CFLAGS) -o $@ $(OBJ) obj/main.o

$(TEST_TARGET): $(OBJ) $(TEST_OBJ) | dirs
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(TEST_OBJ)

obj/main.o: src/main.c | dirs
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.o: src/%.c | dirs
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.o: tests/%.c | dirs
	$(CC) $(CFLAGS) -c $< -o $@

strict:
	$(MAKE) clean
	$(MAKE) all CFLAGS="$(STRICT_CFLAGS)"

test: $(TEST_TARGET)
	$(TEST_CMD)

demo: $(TARGET)
	$(TARGET_CMD) --demo

demo-check: $(TARGET) dirs
	$(TARGET_CMD) --demo > $(OUT_RUN1)
	$(TARGET_CMD) --demo > $(OUT_RUN2)
ifeq ($(OS),Windows_NT)
	fc /B $(OUT_RUN1) $(OUT_RUN2) > nul
else
	cmp -s $(OUT_RUN1) $(OUT_RUN2)
endif
	@echo DEMO DETERMINISM VERIFIED: Both runs are byte-identical.

clean:
ifeq ($(OS),Windows_NT)
	@if exist obj rmdir /s /q obj
	@if exist bin rmdir /s /q bin
	@if exist out\demo_run1.txt del /q out\demo_run1.txt
	@if exist out\demo_run2.txt del /q out\demo_run2.txt
	@if exist out\*.svg del /q out\*.svg
else
	@rm -rf obj bin
	@rm -f out/demo_run1.txt out/demo_run2.txt out/*.svg
endif


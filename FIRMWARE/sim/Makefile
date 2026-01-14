# ================================
# F1 Steering Wheel - Makefile
# ================================
# This Makefile automates the build process for the firmware.
# It can compile and link the source code for different targets (HALs)
# and configurations (Debug/Release).

# ---- Parameters ----
# These variables can be set from the command line to change the build type.
# The '?=' operator assigns a default value only if the variable is not already set.
# Example: 'make sim CONFIG=Release' will override the default 'Debug'.
# Target Hardware Abstraction Layer: 'host_pc' for PC simulation or 'target_s32k' for the hardware.
HAL    ?= host_pc
# Build Configuration: 'Debug' (with debug symbols) or 'Release' (optimized).
CONFIG ?= Debug

# The ':=' operator assigns the value after stripping any leading/trailing whitespace.
# This makes the variable assignments more robust.
HAL    := $(strip $(HAL))
CONFIG := $(strip $(CONFIG))

# ---- Toolchain ----
# Defines the commands for the build tools. These are the defaults for a native host build.
# The C compiler.
CC   := gcc
# The archiver, used for creating static libraries (.a files). Not currently used.
AR   := ar
# The size utility, used to report the memory usage of the final executable.
SIZE := size

# ---- Compilation Options ----
# Defines flags passed to the C compiler.
# Set the C language standard to C11.
CSTD := -std=c11
# Enable a comprehensive set of warnings to enforce high code quality.
WARN := -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wundef

# This block sets optimization, stripping, and preprocessor definitions based on the CONFIG variable.
ifeq ($(CONFIG),Release)
  # Settings for a 'Release' build.
  # Set high optimization level for performance and size.
  OPT     := -O3
  # Set STRIP to 1 to enable symbol stripping from the executable, reducing its size.
  STRIP   ?= 1
  # Define the NDEBUG macro, which is a standard way to disable asserts and other debug code.
  DEFS    := -DNDEBUG
else
  # Settings for a 'Debug' build (the default).
  # -O0 disables all optimizations, which makes debugging easier.
  # -g3 includes detailed debug information in the output files.
  OPT     := -O0 -g3
  # Set STRIP to 0 to disable symbol stripping, keeping debug symbols.
  STRIP   ?= 0
  # Define the DEBUG macro, which can be used with '#ifdef DEBUG' for conditional debug code.
  DEFS    := -DDEBUG
endif

# ---- Include Path ----
# Specifies the directories where the compiler should search for header files (.h).
# The '-I' flag is prepended to each path.
# The backslash '\' allows the variable definition to span multiple lines.
INCLUDES := \
  -I./app \
  -I./drivers \
  -I./hal \
  -I./hal/$(HAL) # This path is dynamic and changes based on the selected HAL.

# ---- Output Directories ----
# Defines the directory structure for the build artifacts (object files, executables, etc.).
# This "out-of-tree" build keeps the source directory clean.
# Base directory for the current build, e.g., 'build/host_pc/Debug'.
BUILDDIR := build/$(HAL)/$(CONFIG)
# Directory for compiled object files (.o).
OBJDIR   := $(BUILDDIR)/obj
# Directory for the final executable.
BINDIR   := $(BUILDDIR)/bin
# Directory for dependency files (.d).
DEPDIR   := $(BUILDDIR)/dep

# ---- Target ----
# Defines the name and full path of the final executable file.
# The name is dynamic based on the HAL, e.g., 'f1_steering_host_pc'.
TARGET   := $(BINDIR)/f1_steering_$(HAL)

# ---- Source File Detection (.c) ----
# Uses the 'wildcard' function to automatically find all .c files in the specified directories.
SRC_APP      := $(wildcard app/*.c)
SRC_DRIVERS  := $(wildcard drivers/*.c)
SRC_HAL      := $(wildcard hal/$(HAL)/*.c)
SRC_COMMON   := $(wildcard main.c)
SRC_TEST 	   := $(wildcard test/*.c)

# Create the final list of source files to be COMPILED AND LINKED.
# This list intentionally EXCLUDES the 'test/' directory for the main build.
SRCS := $(SRC_COMMON) $(SRC_APP) $(SRC_DRIVERS) $(SRC_HAL) $(SRC_TEST)
#SRCS := $(SRC_COMMON) $(filter-out hal/$(HAL)/hal_display_host.c, $(SRC_HAL)) $(filter-out drivers/display.c, $(SRC_DRIVERS)) $(filter-out test/display_demo.c, $(SRC_TEST))
# Create a list of ALL possible source files, including tests.
# This list is used ONLY to generate the dependency files correctly for incremental builds.
ALL_SRCS_FOR_DEPS := $(SRCS)  

# ---- Mapping .c -> .o and .d ----
# Uses 'patsubst' (pattern substitution) to generate the output file paths from the source file lists.
# List of object files that need to be linked to create the final executable.
OBJS     := $(patsubst %.c, $(OBJDIR)/%.o, $(SRCS))
# List of all possible object files that could be created.
ALL_OBJS_FOR_DEPS := $(patsubst %.c, $(OBJDIR)/%.o, $(ALL_SRCS_FOR_DEPS))
# List of all dependency files that will be generated.
DEPS     := $(patsubst $(OBJDIR)/%.o, $(DEPDIR)/%.d, $(ALL_OBJS_FOR_DEPS))

# ---- Common Flags ----
# Combines various flag variables into a single variable for the C compiler.
CFLAGS_COMMON := $(CSTD) $(WARN) $(OPT) $(DEFS) $(INCLUDES)
# Common flags for the linker (currently empty).
LDFLAGS_COMMON :=

# ---- HAL-Specific Configuration ----
# Sets specific compiler/linker flags based on the selected HAL.
ifeq ($(HAL),host_pc) # Configuration for Linux/WSL simulation build
  # Use the 'pkg-config' utility to query the system for the correct flags for the SDL2 library.
  # This makes the Makefile portable across different Linux systems.
  SDL_CFLAGS := $(shell pkg-config --cflags sdl2) # Gets compiler flags (e.g., -I/usr/include/SDL2).
  SDL_LIBS   := $(shell pkg-config --libs sdl2)   # Gets linker flags (e.g., -lSDL2).
  # Final compiler flags for the host build.
  CFLAGS := $(CFLAGS_COMMON) $(SDL_CFLAGS) -DH​AL_DISPLAY_HOST
  # Ensure Linker flags are empty for Linux/WSL to avoid Windows-specific options.
  LDFLAGS := $(LDFLAGS_COMMON)
  # Final libraries to link for the host build.
  LDLIBS  := $(SDL_LIBS)
else ifeq ($(HAL),target_s32k) # Placeholder configuration for the S32K target hardware
  # Cross-compilation settings would be defined here.
  CFLAGS := $(CFLAGS_COMMON)
  LDFLAGS := $(LDFLAGS_COMMON)
  LDLIBS  :=
else # If HAL is not 'host_pc' or 'target_s32k'
  # Stop the build with an error message using the 'error' function.
  $(error Unknown HAL: $(HAL). Use HAL=host_pc or HAL=target_s32k)
endif

# ================================
#           RULES
# ================================
# .PHONY declares targets that do not correspond to actual files. This tells 'make'
# to run the rule's recipe every time the target is invoked (e.g., 'make clean').
.PHONY: all sim hw run clean distclean print

# --- Top-Level Targets ---

# The 'all' target is the default goal if 'make' is called without arguments.
# Its prerequisite is the final executable file, $(TARGET).
all: $(TARGET)
 # This recipe runs *after* $(TARGET) is successfully built.
 # The '@' prefix suppresses echoing of the command itself.
	@echo "[OK] Build completed → $(TARGET)"
# If STRIP=1 (in Release builds), attempt to remove the symbol table from the executable.
ifeq ($(STRIP),1)
	@command -v strip >/dev/null 2>&1 && strip $(TARGET) || true
endif
# If building for the target hardware, run the 'size' utility to report memory usage.
ifeq ($(HAL),target_s32k)
	@$(SIZE) $(TARGET) 2>/dev/null || true
endif

# 'sim' is a convenience target to build the host PC simulation.
# It recursively calls 'make' itself ('$(MAKE)'), forcing HAL=host_pc.
sim:
	@$(MAKE) HAL=host_pc CONFIG=$(CONFIG) all

# 'hw' is a convenience target to build for the target hardware (S32K).
hw:
	@$(MAKE) HAL=target_s32k CONFIG=$(CONFIG) all

# 'run' is a convenience target to build AND run the host PC simulation.
# It depends on the 'sim' target to ensure the executable is built first.
run: sim
	@echo "[RUN] $(TARGET)"
	@$(TARGET)

# --- Linking Rule ---
# This rule defines how to create the final executable file $(TARGET).
# It depends on all the object files listed in $(OBJS) (main application objects only).
$(TARGET): $(OBJS)
  # Recipe commands must start with a tab character.
  # Create the output directory (e.g., build/host_pc/Debug/bin) if it doesn't exist.
	@mkdir -p $(dir $@)
	@echo "--- Linking Target: $(TARGET) ---"
	@echo "Using object files:"
  # '$^' is an automatic variable that holds the names of all prerequisites ($(OBJS) in this case).
	@echo "$^"
	@echo "--------------------------------"
  # The actual linker command. It uses the compiler $(CC) as the linker driver.
  # $^ expands to the list of all prerequisite object files.
  # -o $@ specifies the output file name, where '$@' is the target name ($(TARGET)).
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

# --- Generic Compilation Rule (.c -> .o) ---
# This is a pattern rule that tells 'make' how to compile *any* .c file into its
# corresponding .o file.
$(OBJDIR)/%.o: %.c
  # Create object and dependency subdirectories if they don't exist, maintaining the source tree structure.
	@mkdir -p $(dir $@) $(DEPDIR)/$(dir $*)
  # The compilation command.
  # -c: Compile only (create object file), do not link.
  # -MMD -MP: Instructs gcc to generate dependency information (a .d file).
  # -MF ...: Specifies the output file for the dependency info.
  # $<: The first prerequisite (the .c file).
  # -o $@: The output file (the .o file).
	$(CC) $(CFLAGS) -MMD -MP -MF $(DEPDIR)/$*.d -c $< -o $@

# --- Include Dependencies ---
# Attempt to include all generated dependency files (.d).
# The '-' prefix tells 'make' to ignore errors if the files don't exist (e.g., on the first build).
# These files enable correct incremental builds if header files are changed.
-include $(DEPS)

# --- Clean Rules ---
# 'clean': Removes build artifacts for the *current* HAL and CONFIG combination.
clean:
	@echo "[CLEAN] $(BUILDDIR)"
	@rm -rf "$(BUILDDIR)"

# 'distclean': Removes the *entire* 'build' directory for a complete reset.
distclean:
	@echo "[DISTCLEAN] build/"
	@rm -rf build

# --- Debug Rule ---
# 'print': A utility target to print the values of important Makefile variables for debugging the Makefile itself.
print:
	@echo "HAL       = $(HAL)"
	@echo "CONFIG    = $(CONFIG)"
	@echo "SRCS      = $(SRCS)"
	@echo "OBJS      = $(OBJS)"
	@echo "ALL_SRCS_FOR_DEPS = $(ALL_SRCS_FOR_DEPS)"
	@echo "ALL_OBJS_FOR_DEPS = $(ALL_OBJS_FOR_DEPS)"
	@echo "TARGET    = $(TARGET)"
	@echo "CFLAGS    = $(CFLAGS)"
	@echo "LDFLAGS   = $(LDFLAGS)"
	@echo "LDLIBS    = $(LDLIBS)"

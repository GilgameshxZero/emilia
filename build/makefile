SHELL := /bin/bash

# Directories relative paths.
ROOT_DIR := ../
INCLUDE_DIR := $(ROOT_DIR)include/
SRC_DIR := $(ROOT_DIR)src/
OBJ_DIR := $(ROOT_DIR)obj/
BIN_DIR := $(ROOT_DIR)bin/

# Rain library files
RAIN_INCLUDE_DIR := $(ROOT_DIR)rain/include/
RAIN_INCLUDES := $(shell find $(RAIN_INCLUDE_DIR) -name *.hpp)

# Groups of files and filenames.
VERSION_BUILD_INCLUDE := $(INCLUDE_DIR)build.hpp

# All of project's include files, excluding the build file.
PROJECT_INCLUDES := $(filter-out $(VERSION_BUILD_INCLUDE), \
	$(wildcard $(INCLUDE_DIR)*.hpp))

PROJECT_SRCS := $(wildcard $(SRC_DIR)*.cpp)
PROJECT_SRC_NAMES := $(basename $(notdir $(wildcard $(SRC_DIR)*.cpp)))
PROJECT_NAME := emilia
PROJECT_OBJS := $(addprefix $(OBJ_DIR)$(PROJECT_NAME)/, \
	$(addsuffix .o, $(PROJECT_SRC_NAMES)))
PROJECT_BIN := $(BIN_DIR)$(PROJECT_NAME)

# Flags for build. Debug by default.
CXX := g++
CXXFLAGS := -I$(INCLUDE_DIR) -I$(RAIN_INCLUDE_DIR) -std=c++17 -m64 -pthread \
	-Wall -Wextra -D_CONSOLE
CXXFLAGS_DEBUG := -D_DEBUG -g -O0
CXXFLAGS_RELEASE := -DNDEBUG -Ofast
CXXFLAGS_INSTRUMENT :=
LDLIBS := -pthread -lresolv
LDLIBS_INSTRUMENT := -fsanitize=address,undefined,leak

# Default to DEBUG builds. Check if RELEASE=1 is set.
ifeq ($(RELEASE), 1)
  CXXFLAGS := $(CXXFLAGS) $(CXXFLAGS_RELEASE)
else
  CXXFLAGS := $(CXXFLAGS) $(CXXFLAGS_DEBUG)
endif

ifeq ($(INSTRUMENT), 1)
  CXXFLAGS := $(CXXFLAGS) $(CXXFLAGS_INSTRUMENT)
	LDLIBS := $(LDLIBS) $(LDLIBS_INSTRUMENT)
endif

# -lstdc++fs is included by default on MacOS and cannot be specified on the \
	command line.
UNAME_S := $(shell uname -s)
ifneq ($(UNAME_S), Darwin)
	LDLIBS := $(LDLIBS) -lstdc++fs
endif

# Keep intermediate and output files.
.SECONDARY: $(PROJECT_OBJS) $(PROJECT_BIN)

# Mark .PHONY for rules to avoid compiling files named the same as rules.

# Build and run.
.PHONY: run
run: $(PROJECT_NAME)
	@cd $(BIN_DIR) && ./$(PROJECT_NAME)

# Build without running. Proxy for project name.
.PHONY: build
build: $(PROJECT_NAME)

# Delete intermediate files.
.PHONY: clean
clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

# Rule for BIN_NAME just redirects to the bin itself.
# Enable second expansion for some fancy pattern matching later.
.SECONDEXPANSION:
$(PROJECT_NAME): $(VERSION_BUILD_INCLUDE) $(BIN_DIR)$$@

# Static rules to nest one directory deep inside ../obj.
# Link rule.
$(PROJECT_BIN): $(BIN_DIR)% : $(PROJECT_OBJS)
	@mkdir -p $(@D)
	$(CXX) $^ $(LDLIBS) -o $@

# Compile rule. Stem % matches TEST_NAME/TEST_NAME, in which / is replaced \
	with a space and the last word is extracted for the cpp file name.
$(PROJECT_OBJS): $(OBJ_DIR)%.o : $(SRC_DIR)$$(lastword $$(subst /, ,$$*)).cpp \
	$(PROJECT_INCLUDES) $(RAIN_INCLUDES)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Increment build number.
$(VERSION_BUILD_INCLUDE): $(PROJECT_INCLUDES) $(RAIN_INCLUDES) $(PROJECT_SRCS)
	@read line < $(VERSION_BUILD_INCLUDE); \
	TOKENS=($$line); \
	RAIN_VERSION_BUILD=$$(($${TOKENS[2]} + 1)); \
	echo -e "$${TOKENS[0]} $${TOKENS[1]} $$RAIN_VERSION_BUILD\c" > \
		$(VERSION_BUILD_INCLUDE); \
	echo VERSION_BUILD: $$RAIN_VERSION_BUILD.

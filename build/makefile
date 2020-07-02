SHELL := /bin/bash

# Directories relative paths.
ROOT_DIR := ../
RAIN_INCLUDE_DIR := $(ROOT_DIR)../rain/include/
RAIN_SRC_DIR := $(ROOT_DIR)../rain/src/
INCLUDE_DIR := $(ROOT_DIR)include/
SRC_DIR := $(ROOT_DIR)src/
OBJ_DIR := $(ROOT_DIR)obj/
BIN_DIR := $(ROOT_DIR)bin/

# Groups of files and filenames.
RAIN_INCLUDE := $(wildcard $(RAIN_INCLUDE_DIR)*)
RAIN_NAME := $(basename $(notdir $(wildcard $(RAIN_SRC_DIR)*.cpp)))
RAIN_OBJ := $(addsuffix .o, $(addprefix $(OBJ_DIR)rain/, $(RAIN_NAME)))

EMILIA_API_NAME := $(basename $(notdir $(wildcard $(SRC_DIR)*.cpp)))
EMILIA_API_OBJ := $(addsuffix .o, $(addprefix $(OBJ_DIR)emilia-api/, $(EMILIA_API_NAME)))
EMILIA_API_BIN := $(BIN_DIR)emilia-api

VERSION_BUILD_INCLUDE := $(INCLUDE_DIR)build.hpp

# Flags for build. Release by default.
CXX := g++
CXXFLAG := -I$(INCLUDE_DIR) -I$(RAIN_INCLUDE_DIR) -std=c++17 -g -Wall -pthread
CXXFLAG_DEBUG := -O0
CXXFLAG_RELEASE := -O3 -march=native

# Check if DEBUG=1 is set.
ifeq ($(DEBUG), 1)
  CXXFLAG := $(CXXFLAG) $(CXXFLAG_DEBUG)
else
  CXXFLAG := $(CXXFLAG) $(CXXFLAG_RELEASE)
endif

# Object files should not be deleted to not recompile under no changes.
.SECONDARY: $(RAIN_OBJ) $(EMILIA_API_OBJ)

# Mark .PHONY for rules to avoid compiling files named the same as rules.

# Compile and run all tests.
.PHONY: emilia-api
emilia-api: increment-build $(EMILIA_API_BIN)
	./$(EMILIA_API_BIN)

# Rules for emilia-api binaries and objects.
$(BIN_DIR)%: $(OBJ_DIR)emilia-api/%.o $(RAIN_OBJ) $(RAIN_INCLUDE) $(EMILIA_API_OBJ)
	@mkdir -p $(@D)
	$(CXX) -o $@ $< $(RAIN_OBJ) $(CXXFLAG)
$(OBJ_DIR)emilia-api/%.o: $(SRC_DIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CXXFLAG)

# Delete compile-time files.
.PHONY: clean
clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

# Rules for rain library objects.
$(OBJ_DIR)rain/%.o: $(RAIN_SRC_DIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CXXFLAG)

# Increment build number.
.PHONY: increment-build
increment-build:
	@while read line || [ -n "$$line" ]; do \
		TOKENS=($$line); \
		RAIN_VERSION_BUILD=$$(($${TOKENS[2]} + 1)); \
		echo -e "$${TOKENS[0]} $${TOKENS[1]} $$RAIN_VERSION_BUILD\c" > \
			$(VERSION_BUILD_INCLUDE); \
	done < $(VERSION_BUILD_INCLUDE); \
	echo build $$RAIN_VERSION_BUILD

# GNU make (gmake) & nmake shared makefile.
# Conditional logic implemented with differing end-of-line comment syntax.
# To enable multithreaded compilation, nmake compiles all files together with
# 	/MP, while gmake compiles separately with -jN.
# Files are pre-compiled with pch.cpp.

# Variables:
# PROJ: a repository can have multiple *project*s, each in their own directory.
# BIN: name of the project; a project may have multiple.
# SRC: target filenames (can be multiple) to link, starting from the project
# 	directory.
# BUILD: 0 (default), 1, or 2, specifying the type of build (debug, release,
# 	instrument).
# ARCH: 64 (default) or 86, specifying the architecture to build for. The
# 	environment may need to be initialized accordingly (e.g. for cl.exe).
# PCH: 0 or 1 (default), whether or not to use precompiled pch.cpp. Always 0 on
# 	non-Windows.
# ARGS: command-line arguments to pass to the executable to be run.
# INCL: set from caller on Windows, enumerates header file directory wildcards.

# Targets:
# build (default): compile & link.
# run: compile, link, and run.
# clean: remove intermediates directory.

# Default target defined before any "fake" targets.
__default: _default

# ------------------------------------------------------------------------------
ifdef MAKEDIR: # gmake: false; nmake: unused target
!ifdef MAKEDIR # gmake: not seen; nmake: true
# ------------------------------------------------------------------------------

# nmake specific commands.
ROOT_DIR = $(MAKEDIR)\..^\
REPO_NAME = emilia
!IFNDEF PROJ
PROJ = src
!ENDIF
PROJ_NAME = $(PROJ)
!IFNDEF BIN
!IFNDEF SRC
BIN = $(REPO_NAME)
# This will include all *.cpp* directories as well. In this case, name
# directories as *.cpp.assets.
SRC = *.cpp
!ELSE
BIN = $(patsubsti %.cpp,%,$(SRC))
!ENDIF
!ELSE
!IFNDEF SRC
SRC = $(BIN).cpp
!ENDIF
!ENDIF
BIN_NAME = $(BIN)
SRC_NAME = $(SRC)
!IFNDEF BUILD
BUILD = 0
!ENDIF
!IF $(BUILD) == 2
BUILD_NAME = instrument
!ELSE IF $(BUILD) == 1
BUILD_NAME = release
!ELSE
BUILD_NAME = debug
!ENDIF
!IFNDEF ARCH
ARCH = 64
!ENDIF
!IFNDEF PCH
PCH = 1
!ENDIF

# The directory structure here must be consistent with macro substitution
# structure utilized later.
BIN_DIR = $(ROOT_DIR).bin^\
PROJ_BIN = $(BIN_DIR)$(PROJ_NAME)\$(BIN_NAME).$(BUILD_NAME).x$(ARCH).exe
PROJ_BIN_DIR = $(BIN_DIR)$(PROJ_NAME)^\
OBJ_DIR = $(ROOT_DIR).obj^\
PROJ_OBJ_DIR = $(OBJ_DIR)$(PROJ_NAME)^\
PROJ_SRC_DIR = $(ROOT_DIR)$(PROJ_NAME)^\
PROJ_SRC = $(patsubsti %,$(PROJ_SRC_DIR)\%,$(SRC_NAME))

COMPILER = cl /c
COMPILER_FLAGS_INCLUDES = /I$(ROOT_DIR)rain\include /I$(ROOT_DIR)include
COMPILER_FLAGS_COMMON = $(COMPILER_FLAGS_INCLUDES) /std:c++20 /D _CONSOLE\
	/Fp$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.pch\
	/fp:fast /MP /permissive-\
	/Zc:__cplusplus /Zc:wchar_t /Zc:forScope /Zc:inline /Zf\
	/GS /W3 /WX- /wd4250 /wd4068 /sdl /diagnostics:column /EHsc /Gm- /nologo
COMPILER_FLAGS_DEBUG = /D _DEBUG /MDd /Od /RTC1 /JMC /ZI
COMPILER_FLAGS_RELEASE = /D NDEBUG /MT /O2 /Oi /GL /Gy /Zi
COMPILER_FLAGS_INSTRUMENT = $(COMPILER_FLAGS_DEBUG) /Fa$(PROJ_OBJ_DIR)
COMPILER_FLAGS_64 =
COMPILER_FLAGS_86 =

LINKER = link
LINKER_FLAGS_LIBRARIES =
LINKER_FLAGS_COMMON = $(LINKER_FLAGS_LIBRARIES) /OUT:$(PROJ_BIN)\
	/PDB:$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).pdb\
	/ILK:$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).ilk\
	/MANIFESTUAC:"level='asInvoker' uiAccess='false'"\
	/MANIFESTFILE:$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).exe.intermediate.manifest\
	/LTCGOUT:$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).iobj\
	/SUBSYSTEM:CONSOLE /NOLOGO
LINKER_FLAGS_DEBUG = /DEBUG
LINKER_FLAGS_RELEASE = /INCREMENTAL:NO /OPT:ICF /OPT:REF /LTCG:incremental
LINKER_FLAGS_INSTRUMENT = $(LINKER_FLAGS_DEBUG)
LINKER_FLAGS_64 = /MACHINE:x64
LINKER_FLAGS_86 = /MACHINE:x86

!IF "$(BUILD_NAME)" == "instrument"
COMPILER_FLAGS = $(COMPILER_FLAGS_COMMON) $(COMPILER_FLAGS_INSTRUMENT)
LINKER_FLAGS = $(LINKER_FLAGS_COMMON) $(LINKER_FLAGS_INSTRUMENT)
!ELSE IF "$(BUILD_NAME)" == "release"
COMPILER_FLAGS = $(COMPILER_FLAGS_COMMON) $(COMPILER_FLAGS_RELEASE)
LINKER_FLAGS = $(LINKER_FLAGS_COMMON) $(LINKER_FLAGS_RELEASE)
!ELSE
COMPILER_FLAGS = $(COMPILER_FLAGS_COMMON) $(COMPILER_FLAGS_DEBUG)
LINKER_FLAGS = $(LINKER_FLAGS_COMMON) $(LINKER_FLAGS_DEBUG)
!ENDIF

!IF $(ARCH) == 86
COMPILER_FLAGS = $(COMPILER_FLAGS) $(COMPILER_FLAGS_86)
LINKER_FLAGS = $(LINKER_FLAGS) $(LINKER_FLAGS_86)
!ELSE
COMPILER_FLAGS = $(COMPILER_FLAGS) $(COMPILER_FLAGS_64)
LINKER_FLAGS = $(LINKER_FLAGS) $(LINKER_FLAGS_64)
!ENDIF

# Real targets.
# Default/build target only compiles and links, but does not run.
_default: build
# Ordering is important so that version is updated first.
build: version.build.txt $(PROJ_BIN)
# Build and run.
run: build
	@ECHO ---------------- [Run] $(BIN_NAME) ----------------
	@CD $(BIN_DIR)$(PROJ_NAME) &&\
		$(BIN_NAME).$(BUILD_NAME).x$(ARCH).exe $(ARGS)
# Clean target removes all intermediates, but remakes the in.txt file.
clean:
	@ECHO Cleaning...
	@IF EXIST $(BIN_DIR) RD /Q /S $(BIN_DIR)
	@IF EXIST $(OBJ_DIR) RD /Q /S $(OBJ_DIR)
# Updates version if any project dependency has changed.
# Mark files as up-to-date after.
# Updated version may not be propagated until clean.
# Order dependencies so that output is ordered; not super important.
version.build.txt: $(INCL) $(PROJ_SRC)
# @FOR %%I IN ($?) DO @ECHO Outdated: %%I.
	@version.bat $(uppercase $(REPO_NAME))
	@$(MAKE) /C /T version.build.txt
# Precompiled headers.
$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.pch: pch.cpp
	@ECHO Building $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.pch...
	@IF NOT EXIST $(PROJ_OBJ_DIR) MD $(PROJ_OBJ_DIR)
	@IF NOT EXIST $(PROJ_OBJ_DIR)$(BUILD_NAME) MD $(PROJ_OBJ_DIR)$(BUILD_NAME)
	@IF NOT EXIST $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\
		MD $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)
	@$(COMPILER) $(COMPILER_FLAGS) /Yc\
		/Fd$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.pdb\
		/Fo$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.obj pch.cpp
pch.cpp: $(INCL_PCH)

# Build target.
# First compile all outdated files (with /MP), then link everything together.
# Need additional variable to parse dependencies correctly.
$(PROJ_BIN): $(PROJ_SRC)
!IF $(PCH) == 1
	@$(MAKE) /C $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.pch PROJ=$(PROJ) BIN=$(BIN) SRC="$(SRC)" BUILD=$(BUILD) ARCH=$(ARCH) PCH=$(PCH)
!ENDIF
	@ECHO ---------------- [Build] $(BIN_NAME) ----------------
	@IF NOT EXIST $(PROJ_BIN_DIR) MD $(PROJ_BIN_DIR)
	@IF NOT EXIST $(PROJ_OBJ_DIR) MD $(PROJ_OBJ_DIR)
	@IF NOT EXIST $(PROJ_OBJ_DIR)$(BUILD_NAME) MD $(PROJ_OBJ_DIR)$(BUILD_NAME)
	@IF NOT EXIST $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\
		MD $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)
	@ECHO Building $(PROJ_BIN)...
!IF $(PCH) == 0
	@$(COMPILER) $(COMPILER_FLAGS)\
		/Fd$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).pdb\
		/Fo$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\ \
		$?
!ELSE
# PDB and IDB should be derived from the PCH versions.
	@IF NOT EXIST $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).pdb\
		CP $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.pdb\
			$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).pdb
	@IF NOT EXIST $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).idb\
		IF EXIST $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.idb\
			CP $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.idb\
				$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).idb
	@$(COMPILER) $(COMPILER_FLAGS)\
		/Fd$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).pdb\
		/Fo$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\ \
		/Yu"rain.hpp" $?
!ENDIF
	@ECHO Linking $(PROJ_BIN)...
# It's unclear why some of the wildcards need to be escaped, but it works.
	@$(LINKER) $(LINKER_FLAGS) $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.obj\
		$(patsubsti %.cpp,%.obj,\
		$(patsubsti $(PROJ_SRC_DIR)\%,$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\\%,$**))

# Mark source files as out of date if any headers have changed.
# An empty target here will correctly set the modified date.
$(PROJ_SRC): $(INCL) pch.cpp
	@EXIT

# ------------------------------------------------------------------------------
!else
else
# ------------------------------------------------------------------------------

# GNU make (gmake) specific commands.
# Second expansion for stem matching syntax.
.SECONDEXPANSION:
# For usage of bash commands like `read`.
SHELL := /bin/bash
# For recursive dependency listing.
rwildcard=$(foreach d,\
	$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

ROOT_DIR := ../
REPO_NAME := emilia
PROJ ?= src
PROJ_NAME := $(PROJ)
ifndef BIN
ifndef SRC
BIN := $(REPO_NAME)
SRC := *.cpp
else
BIN := $(patsubst .cpp,,$(SRC))
endif
else
SRC := $(BIN).cpp
endif
BIN_NAME := $(BIN)
SRC_NAME := $(SRC)
BUILD ?= 0
ifeq ($(BUILD), 2)
BUILD_NAME := instrument
else ifeq ($(BUILD), 1)
BUILD_NAME := release
else
BUILD_NAME := debug
endif
ARCH ?= 64
# PCH is not yet supported on non-Windows systems.
PCH := 0

INCL := $(call rwildcard,$(ROOT_DIR)rain/include,*.hpp) $(call rwildcard,$(ROOT_DIR)include,*.hpp)

BIN_DIR := $(ROOT_DIR).bin/
PROJ_BIN := $(BIN_DIR)$(PROJ_NAME)/$(BIN_NAME).$(BUILD_NAME).x$(ARCH)
PROJ_BIN_DIR := $(BIN_DIR)$(PROJ_NAME)/
OBJ_DIR := $(ROOT_DIR).obj/
PROJ_OBJ_DIR := $(OBJ_DIR)$(PROJ_NAME)/
PROJ_SRC_DIR := $(ROOT_DIR)$(PROJ_NAME)/
PROJ_SRC = $(patsubst %,$(PROJ_SRC_DIR)%,$(SRC_NAME))

COMPILER ?= clang++
COMPILER_FLAGS_INCLUDES := -I$(ROOT_DIR)rain/include -I$(ROOT_DIR)include
COMPILER_FLAGS_COMMON := $(COMPILER_FLAGS_INCLUDES) -std=c++20 -pthread\
	-Wall -Wextra -D_CONSOLE -masm=intel
COMPILER_FLAGS_DEBUG := -D _DEBUG -g -O0
COMPILER_FLAGS_RELEASE := -D NDEBUG -O3 -ffast-math
COMPILER_FLAGS_INSTRUMENT := $(COMPILER_FLAGS_DEBUG) /Fa$(PROJ_OBJ_DIR)
COMPILER_FLAGS_64 := -m64
COMPILER_FLAGS_86 := -m32

LINKER := $(COMPILER)
LINKER_FLAGS_LIBRARIES := -pthread -lresolv
LINKER_FLAGS_COMMON := $(LINKER_FLAGS_LIBRARIES)
LINKER_FLAGS_DEBUG :=
LINKER_FLAGS_RELEASE :=
LINKER_FLAGS_INSTRUMENT := $(LINKER_FLAGS_DEBUG)\
	-fsanitize=address,undefined,leak
LINKER_FLAGS_64 := -m64
LINKER_FLAGS_86 := -m32

# -lstdc++fs is neither available nor required on typical MacOS C++ compiler
# implementations.
ifneq ($(shell uname -s), Darwin)
	LINKER_FLAGS_LIBRARIES := $(LINKER_FLAGS_LIBRARIES) -lstdc++fs
endif

ifeq ($(BUILD_NAME), instrument)
	COMPILER_FLAGS := $(COMPILER_FLAGS_COMMON) $(COMPILER_FLAGS_INSTRUMENT)
	LINKER_FLAGS := $(LINKER_FLAGS_COMMON) $(LINKER_FLAGS_INSTRUMENT)
else ifeq ($(BUILD_NAME), release)
	COMPILER_FLAGS := $(COMPILER_FLAGS_COMMON) $(COMPILER_FLAGS_RELEASE)
	LINKER_FLAGS := $(LINKER_FLAGS_COMMON) $(LINKER_FLAGS_RELEASE)
else
	COMPILER_FLAGS := $(COMPILER_FLAGS_COMMON) $(COMPILER_FLAGS_DEBUG)
	LINKER_FLAGS := $(LINKER_FLAGS_COMMON) $(LINKER_FLAGS_DEBUG)
endif

ifeq ($(ARCH), 86)
	COMPILER_FLAGS := $(COMPILER_FLAGS) $(COMPILER_FLAGS_86)
	LINKER_FLAGS := $(LINKER_FLAGS) $(LINKER_FLAGS_86)
else
	COMPILER_FLAGS := $(COMPILER_FLAGS) $(COMPILER_FLAGS_64)
	LINKER_FLAGS := $(LINKER_FLAGS) $(LINKER_FLAGS_64)
endif

# Unlike `nmake`, `gmake` first compiles all the relevant obj files, then
# compiles the relevant binaries.
# Keep intermediate and output files.
# Introduce list of project object intermediates parsed from the source
# directory with SRC_NAME.
PROJ_OBJ := $(foreach s, $(SRC_NAME),\
	$(addprefix $(PROJ_OBJ_DIR)$(BUILD_NAME)/x$(ARCH)/, $(addsuffix .o,\
	$(basename $(notdir $(wildcard $(PROJ_SRC_DIR)$(s)))))))
.SECONDARY: $(PROJ_BIN) $(PROJ_OBJ)

# Ensure files with the same name as fake targets do not trigger.
.PHONY: __default _default build run clean tests
_default: build
build: version.build.txt $(PROJ_BIN)
run: build
	@echo ---------------- [Run] $(BIN_NAME) ----------------
	@cd $(PROJ_BIN_DIR) && ./$(BIN_NAME).$(BUILD_NAME).x$(ARCH) $(ARGS)
clean:
	@echo Cleaning...
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
# No MAKEFLAGS because passing -j8 causes warning.
tests: version.build.txt
	@for i in $(PROJ_SRC_DIR)*.cpp; do\
		echo ---------------- [Build] $$(basename $$i .cpp) ----------------;\
		$(MAKE) --no-print-directory -s build BIN=$$(basename $$i .cpp) BUILD=$(BUILD) ARCH=$(ARCH) PCH=$(PCH) || exit;\
	done
	@for i in $(PROJ_SRC_DIR)*.cpp; do\
		$(MAKE) --no-print-directory -s run BIN=$$(basename $$i .cpp) BUILD=$(BUILD) ARCH=$(ARCH) PCH=$(PCH) || exit;\
	done

version.build.txt: $(INCL) $(PROJ_SRC)
# @for i in $?; do echo Outdated: $$i.;\
	done
	@./version.sh $(shell echo $(REPO_NAME) | tr '[:lower:]' '[:upper:]')

# Compile rule. The first line includes all header dependencies.
# Stem % matches source file name.
$(PROJ_OBJ): $(INCL)
$(PROJ_OBJ): $(PROJ_OBJ_DIR)$(BUILD_NAME)/x$(ARCH)/%.o : $(PROJ_SRC_DIR)$$*.cpp
	@mkdir -p $(@D)
	@echo Building $@...
	@$(COMPILER) $(COMPILER_FLAGS) -c $< -o $@

# Link rule.
$(PROJ_BIN): $(BIN_DIR)% : $(PROJ_OBJ)
	@mkdir -p $(@D)
	@echo Linking $@...
	@$(LINKER) $(PROJ_OBJ) $(LINKER_FLAGS) -o $@

# ------------------------------------------------------------------------------
endif    # gmake: close condition; nmake: not seen
!endif : # gmake: unused target; nmake close conditional
# ------------------------------------------------------------------------------

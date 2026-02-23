# GNU make (`gmake`) & `nmake` shared makefile. Conditional
# logic implemented with differing end-of-line comment
# syntax. This makefile is designed to be implemented
# generically, without modification to its core, across all
# C++ projects.
#
# Instead of calling `nmake` directly on Windows, we prefer
# to call `make.bat` which calls `nmake` indirectly, while
# injecting a few resolved wildcard variables. Due to this
# split nature, the variables in `make.bat` must be adjusted
# manually when changing the INCL_PCH, INCL, or SRC
# variables here.
#
# There are subtle differences in which files `nmake` vs.
# `gmake` track timestamp changes for. As long as the file
# exists, and there is a rule with it as target, `nmake`
# will update its timestamp, regardless of whether the file
# was actually updated by the rule. `gmake` only updates
# the timestamp if the file was changed. This is annoying
# to deal with when the targets are source files, which are
# unchanged by makefiles.
#
# Multithreaded compilation is enabled in `gmake` with `-jN`
# by the caller, and in `nmake` with the `/MP` flag. Due to
# constraints on the `/Fo` flag for `cl.exe`, multithreaded
# compilation for `nmake` is only enabled on a per-directory
# basis for the source files.
#
# Both `nmake` and `gmake` support precompilation of source
# code found in `build/pch.cpp`.
#
# In general, specify directories with a trailing slash.
#
# Variables:
# PROJ: a repository can have multiple *project*s, each in
# 	their own directory. Defaults to `src`.
# INCL_PCH: space separated list of directories to include
# 	for the precompiled header. Defaults to empty.
# INCL: space separated list of directories to include for
# 	source files. Should exclude `INCL_PCH`. Defaults to
# 	`include/`.
# BIN: name of the executable to link for this project.
# 	Defaults to the name of repository. The final
# 	executable is linked as
# 	`.bin/$(BIN).$(BUILD_NAME).x$(ARCH)$(EXT)` with
# 	`$(EXT)$` as `.exe` on Windows and empty otherwise.
# SRC: target filenames (can be multiple) to link, starting
# 	from the project directory. Defaults to `*.cpp`.
# BUILD: 0 (default), 1, or 2, specifying the type of build
# 	(debug, release, instrument).
# ARCH: 64 (default) or 86, specifying the architecture to
# 	build for. The environment may need to be initialized
# 	accordingly (e.g. for cl.exe).
# PCH: 0 or 1 (default), whether or not to use precompiled
# 	pch.cpp.
# ARGS: command-line arguments to pass to the executable to
# 	be run. Defaults to empty.
#
# Additional variables are passed in from the called on
# Windows. These variables enable various batch and wildcard
# capabilities generally present on `gmake` but not on
# `nmake`. They are passed from `make.bat`.
# NMAKE_INCL_PCH: enumerates header file directory
# 	wildcards for the precompiled headers.
# NMAKE_INCL: enumerates header file directory wildcards.
# 	Should exclude `NMAKE_INCL_PCH`.
# NMAKE_PROJ_SRC: if we use wildcards in `$(SRC)`,
# 	`$(NMAKE_PROJ_SRC)` must be a preprocessed list given
# 	from `make.bat`.
#
# Additional variables are required by recursive `make`
# calls:
# NMAKE_COMPILE_TARGET: on Windows, batch multiprocess
# 	compilation proceeds on a per-directory basis. For each
# 	directory, `nmake` is called recursively, with this
# 	argument set to the directory (or file) to be compiled.
#
# Targets:
# build (default): compile & link.
# run: compile, link, and run.
# clean: remove intermediates directory.
# tests: builds all tests, then runs all tests. Tests must
# 	be single files in `test/`.
# info: print select makefile variables and exit.
#
# Repositories can also override defaults here without
# modifying the core `makefile`. These are not to be passed
# into `make` directly.
DEFAULT_PROJ=src
DEFAULT_INCL_PCH=rain/include/
DEFAULT_INCL=include/
DEFAULT_SRC=*.cpp
DEFAULT_BUILD=0
DEFAULT_ARCH=64
DEFAULT_PCH=1
DEFAULT_ARGS=
DEFAULT_VERSIONING=1

# `cl.exe` uses `/Yu"x.hpp"` to specify that the precompiled
# header ends after `#include <x.hpp>`. Thus we must specify
# NMAKE_PCH_YU for `nmake` builds.
DEFAULT_NMAKE_PCH_YU=rain.hpp

# Default target defined before any "fake" targets.
__default: _default

# ----------------------------------------------------------
ifdef MAKEDIR: # gmake: false; nmake: unused target
!ifdef MAKEDIR # gmake: not seen; nmake: true
# ----------------------------------------------------------
# nmake specific commands.

DEFAULT_SHOW_OUTDATED=0

# It is important we get the absolute path without `..` here
# for standardized path substitution techniques later. We
# use `$(ROOT_DIR_PAR)` to slice out the `$(REPO_NAME)`.
ROOT_DIR=$(abspath $(MAKEDIR)\..)^\
ROOT_DIR_PAR=$(abspath $(ROOT_DIR)..)^\
REPO_NAME=$(patsubst $(ROOT_DIR_PAR)\%\,%,$(ROOT_DIR))
!IFNDEF PROJ
PROJ=$(DEFAULT_PROJ)
!ENDIF
PROJ_NAME=$(PROJ)
!IFNDEF INCL_PCH
INCL_PCH=$(DEFAULT_INCL_PCH)
!ENDIF
!IFNDEF INCL
INCL=$(DEFAULT_INCL)
!ENDIF
!IFNDEF BIN
!IFNDEF SRC
BIN=$(REPO_NAME)
# This will include all *.cpp* directories as well. In this
# case, name directories as *.cpp.assets.
SRC=$(DEFAULT_SRC)
!ELSE
# patsubst wildcards need to be escaped iff they are part
# of a path.
BIN=$(patsubst %.cpp,%,$(SRC))
!ENDIF
!ELSE
!IFNDEF SRC
SRC=$(BIN).cpp
!ENDIF
!ENDIF
BIN_NAME=$(BIN)
SRC_NAME=$(SRC)
!IFNDEF BUILD
BUILD=$(DEFAULT_BUILD)
!ENDIF
!IF $(BUILD)==2
BUILD_NAME=instrument
!ELSE IF $(BUILD)==1
BUILD_NAME=release
!ELSE
BUILD_NAME=debug
!ENDIF
!IFNDEF ARCH
ARCH=$(DEFAULT_ARCH)
!ENDIF
!IFNDEF PCH
PCH=$(DEFAULT_PCH)
!ENDIF
!IFNDEF ARGS
ARGS=$(DEFAULT_ARGS)
!ENDIF
!IFNDEF VERSIONING
VERSIONING=$(DEFAULT_VERSIONING)
!ENDIF
!IFNDEF SHOW_OUTDATED
SHOW_OUTDATED=$(DEFAULT_SHOW_OUTDATED)
!ENDIF
!IFNDEF NMAKE_PCH_YU
NMAKE_PCH_YU=$(DEFAULT_NMAKE_PCH_YU)
!ENDIF

# The directory structure here must be consistent with
# macro substitution structure utilized later.
BIN_DIR=$(ROOT_DIR).bin^\
TEST_DIR=$(ROOT_DIR)test^\
PROJ_BIN=$(BIN_DIR)$(PROJ_NAME)\$(BIN_NAME).$(BUILD_NAME).x$(ARCH).exe
PROJ_BIN_DIR=$(BIN_DIR)$(PROJ_NAME)^\
OBJ_DIR=$(ROOT_DIR).obj^\
PROJ_OBJ_DIR=$(OBJ_DIR)$(PROJ_NAME)^\
PROJ_SRC_DIR=$(ROOT_DIR)$(PROJ_NAME)^\
!IF "$(SRC_NAME)"=="*.cpp"
PROJ_SRC=$(patsubst ..\\%,$(ROOT_DIR)\%,$(NMAKE_PROJ_SRC))
!ELSE
PROJ_SRC=$(patsubst %,$(PROJ_SRC_DIR)\%,$(SRC_NAME))
!ENDIF

COMPILER=cl /c
COMPILER_FLAGS_INCLUDES=\
	$(patsubst %,/I$(ROOT_DIR)\%,$(INCL_PCH))\
	$(patsubst %,/I$(ROOT_DIR)\%,$(INCL))
COMPILER_FLAGS_COMMON=$(COMPILER_FLAGS_INCLUDES)\
	/std:c++20 /D_CONSOLE\
	/Fp$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.pch\
	/fp:fast /MP /permissive-\
	/Zc:__cplusplus /Zc:wchar_t /Zc:forScope /Zc:inline /Zf\
	/GS /W3 /WX- /wd4250 /wd4068 /sdl\
	/diagnostics:column /EHsc /Gm- /nologo
COMPILER_FLAGS_DEBUG=/D_DEBUG /MDd /Od /RTC1 /JMC /ZI
COMPILER_FLAGS_RELEASE=/DNDEBUG /MT /O2 /Oi /GL /Gy /Zi
COMPILER_FLAGS_INSTRUMENT=$(COMPILER_FLAGS_DEBUG)\
	/Fa$(PROJ_OBJ_DIR)
COMPILER_FLAGS_64=
COMPILER_FLAGS_86=

LINKER=link
LINKER_FLAGS_LIBRARIES=
LINKER_FLAGS_COMMON=$(LINKER_FLAGS_LIBRARIES)\
	/OUT:$(PROJ_BIN)\
	/PDB:$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).pdb\
	/ILK:$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).ilk\
	/MANIFESTUAC:"level='asInvoker' uiAccess='false'"\
	/MANIFESTFILE:$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).exe.intermediate.manifest\
	/LTCGOUT:$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).iobj\
	/SUBSYSTEM:CONSOLE /NOLOGO
LINKER_FLAGS_DEBUG=/DEBUG
LINKER_FLAGS_RELEASE=/INCREMENTAL:NO\
	/OPT:ICF /OPT:REF /LTCG:incremental
LINKER_FLAGS_INSTRUMENT=$(LINKER_FLAGS_DEBUG)
LINKER_FLAGS_64=/MACHINE:x64
LINKER_FLAGS_86=/MACHINE:x86

!IF "$(BUILD_NAME)"=="instrument"
COMPILER_FLAGS=$(COMPILER_FLAGS_COMMON)\
	$(COMPILER_FLAGS_INSTRUMENT)
LINKER_FLAGS=$(LINKER_FLAGS_COMMON)\
	$(LINKER_FLAGS_INSTRUMENT)
!ELSE IF "$(BUILD_NAME)"=="release"
COMPILER_FLAGS=$(COMPILER_FLAGS_COMMON)\
	$(COMPILER_FLAGS_RELEASE)
LINKER_FLAGS=$(LINKER_FLAGS_COMMON) $(LINKER_FLAGS_RELEASE)
!ELSE
COMPILER_FLAGS=$(COMPILER_FLAGS_COMMON)\
	$(COMPILER_FLAGS_DEBUG)
LINKER_FLAGS=$(LINKER_FLAGS_COMMON) $(LINKER_FLAGS_DEBUG)
!ENDIF

!IF $(ARCH)==86
COMPILER_FLAGS=$(COMPILER_FLAGS) $(COMPILER_FLAGS_86)
LINKER_FLAGS=$(LINKER_FLAGS) $(LINKER_FLAGS_86)
!ELSE
COMPILER_FLAGS=$(COMPILER_FLAGS) $(COMPILER_FLAGS_64)
LINKER_FLAGS=$(LINKER_FLAGS) $(LINKER_FLAGS_64)
!ENDIF
# Used in the build step to save an if statement.
!IF $(PCH)==0
COMPILER_FLAGS_PCH=
LINKER_FLAGS_PCH=
!ELSE
COMPILER_FLAGS_PCH=/Yu"$(NMAKE_PCH_YU)"
LINKER_FLAGS_PCH=\
	$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.obj
!ENDIF

# Real targets.
# Default/build target only compiles and links, but does
# not run.
_default: build
# Ordering is important so that version is updated first.
build: version.build.txt $(PROJ_BIN)
# Build and run.
run: build
	@ECHO ---------------- [Run] $(BIN_NAME) ----------------
	@CD $(BIN_DIR)$(PROJ_NAME) &&\
		$(BIN_NAME).$(BUILD_NAME).x$(ARCH).exe $(ARGS)
# Clean target removes all intermediates, but remakes the
# in.txt file.
clean:
	@ECHO Cleaning...
	@IF EXIST $(BIN_DIR) RD /Q /S $(BIN_DIR)
	@IF EXIST $(OBJ_DIR) RD /Q /S $(OBJ_DIR)
tests:
	@FOR /F %%i IN ('DIR /B /A-D "$(TEST_DIR)*.cpp"') DO @(\
		@$(MAKE) /C build\
			NMAKE_INCL_PCH="$(NMAKE_INCL_PCH)"\
			NMAKE_INCL="$(NMAKE_INCL)"\
			NMAKE_PROJ_SRC="$(NMAKE_PROJ_SRC)"\
			NMAKE_PCH_YU="$(NMAKE_PCH_YU)"\
			PROJ=test INCL_PCH="$(INCL_PCH)" INCL="$(INCL)"\
			BIN="%~ni" SRC="%~ni.cpp"\
			BUILD=$(BUILD) ARCH=$(ARCH) PCH=$(PCH) ARGS="$(ARGS)"\
			VERSIONING=$(VERSIONING)\
			SHOW_OUTDATED=$(SHOW_OUTDATED) ||\
			EXIT /b\
	)
	@FOR /F %%i IN ('DIR /B /A-D "$(TEST_DIR)*.cpp"') DO @(\
		@$(MAKE) /C run\
			NMAKE_INCL_PCH="$(NMAKE_INCL_PCH)"\
			NMAKE_INCL="$(NMAKE_INCL)"\
			NMAKE_PROJ_SRC="$(NMAKE_PROJ_SRC)"\
			NMAKE_PCH_YU="$(NMAKE_PCH_YU)"\
			PROJ=test INCL_PCH="$(INCL_PCH)" INCL="$(INCL)"\
			BIN="%~ni" SRC="%~ni.cpp"\
			BUILD=$(BUILD) ARCH=$(ARCH) PCH=$(PCH) ARGS="$(ARGS)"\
			VERSIONING=$(VERSIONING)\
			SHOW_OUTDATED=$(SHOW_OUTDATED) ||\
			EXIT /b\
	)
info:
	@ECHO "NMAKE_INCL_PCH=$(NMAKE_INCL_PCH)"
	@ECHO "NMAKE_INCL=$(NMAKE_INCL)"
	@ECHO "NMAKE_PROJ_SRC=$(NMAKE_PROJ_SRC)"
	@ECHO "NMAKE_PCH_YU=$(NMAKE_PCH_YU)"

	@REM For some reason, some backslashes need to quoted to
	@REM display properly.
	@ECHO ROOT_DIR=$(ROOT_DIR)
	@ECHO "PROJ=$(PROJ)"
	@ECHO "INCL_PCH=$(INCL_PCH)"
	@ECHO "INCL=$(INCL)"
	@ECHO "BIN_NAME=$(BIN_NAME)"
	@ECHO "SRC_NAME=$(SRC_NAME)"
	@ECHO "BUILD_NAME=$(BUILD_NAME)"
	@ECHO "ARCH=$(ARCH)"
	@ECHO "PCH=$(PCH)"
	@ECHO "ARGS=$(ARGS)"
	@ECHO "VERSIONING=$(VERSIONING)"
	@ECHO "SHOW_OUTDATED=$(SHOW_OUTDATED)"
	@ECHO "PROJ_SRC=$(PROJ_SRC)"
	@ECHO "PROJ_OBJ=$(PROJ_OBJ)"
	@ECHO "COMPILER=$(COMPILER)"
	@ECHO "COMPILER_FLAGS=$(COMPILER_FLAGS)"
	@ECHO "LINKER_FLAGS=$(LINKER_FLAGS)"

# Updates version if any project dependency has changed.
# Mark files as up-to-date after.
# Updated version may not be propagated until clean.
# Order dependencies so that output is ordered; not super
# important.
version.build.txt:\
	$(NMAKE_INCL_PCH) $(NMAKE_INCL) $(PROJ_SRC)
!IF $(VERSIONING)==1
	@REM Command line limit is 8192. Use SHOW_OUTDATED=0 if
	@REM this fails.
!IF $(SHOW_OUTDATED)==1
	@FOR %%I IN ($?) DO @ECHO Outdated: %%I.
!ENDIF
	@version.bat $(uppercase $(REPO_NAME))
	@$(MAKE) /C /T $@
!ENDIF
$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.pch: pch.cpp
	@ECHO Building\
		$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.pch...
	@IF NOT EXIST $(PROJ_OBJ_DIR) MD $(PROJ_OBJ_DIR)
	@IF NOT EXIST $(PROJ_OBJ_DIR)$(BUILD_NAME)\
		MD $(PROJ_OBJ_DIR)$(BUILD_NAME)
	@IF NOT EXIST $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\
		MD $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)
	@$(COMPILER) $(COMPILER_FLAGS) /Yc\
		/Fd$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.pdb\
		/Fo$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.obj pch.cpp
# Within `nmake`, tracking timestamps of source files is
# required (why?).
pch.cpp: $(NMAKE_INCL_PCH)

# Binary build target.
# First compile all outdated files (with /MP), then link
# everything together.
# Need additional variable to parse dependencies correctly.
$(PROJ_BIN): $(PROJ_SRC)
!IF $(PCH)==1
	@$(MAKE) /C\
		"$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.pch"\
		NMAKE_INCL_PCH="$(NMAKE_INCL_PCH)"\
		NMAKE_INCL="$(NMAKE_INCL)"\
		NMAKE_PROJ_SRC="$(NMAKE_PROJ_SRC)"\
		NMAKE_PCH_YU="$(NMAKE_PCH_YU)"\
		PROJ="$(PROJ)" INCL_PCH="$(INCL_PCH)" INCL="$(INCL)"\
		BIN="$(BIN)" SRC="$(SRC)"\
		BUILD=$(BUILD) ARCH=$(ARCH) PCH=$(PCH) ARGS="$(ARGS)"\
		VERSIONING=$(VERSIONING)\
		SHOW_OUTDATED=$(SHOW_OUTDATED)
!ENDIF
	@ECHO --------------- [Build] $(BIN_NAME) ---------------
	@IF NOT EXIST $(PROJ_BIN_DIR) MD $(PROJ_BIN_DIR)
	@IF NOT EXIST $(PROJ_OBJ_DIR) MD $(PROJ_OBJ_DIR)
	@IF NOT EXIST $(PROJ_OBJ_DIR)$(BUILD_NAME)\
		MD $(PROJ_OBJ_DIR)$(BUILD_NAME)
	@IF NOT EXIST $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\
		MD $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)
!IF $(PCH)==1
	@REM PDB and IDB should be derived from the PCH versions.
	@IF NOT EXIST\
		$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).pdb\
		CP $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.pdb\
			$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).pdb
	@IF NOT EXIST $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).idb\
		IF EXIST $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.idb\
			CP $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\pch.idb\
				$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).idb
!ENDIF
	@REM We use an `nmake` inline file here to track all
	@REM arguments as it is easy to exceed the 8192 limit.
	@<<.nmake.tmp.txt
NMAKE_INCL_PCH="$(NMAKE_INCL_PCH)"
NMAKE_INCL="$(NMAKE_INCL)"
NMAKE_PROJ_SRC="$(NMAKE_PROJ_SRC)"
NMAKE_PCH_YU="$(NMAKE_PCH_YU)"
PROJ="$(PROJ)"
INCL_PCH="$(INCL_PCH)" INCL="$(INCL)"
BIN="$(BIN)" SRC="$(SRC)"
BUILD=$(BUILD) ARCH=$(ARCH) PCH=$(PCH) ARGS="$(ARGS)"
VERSIONING=$(VERSIONING)
SHOW_OUTDATED=$(SHOW_OUTDATED)
<<NOKEEP
	@REM Compile all subdirectories (and files) separately so
	@REM as to place object files in the correct relative
	@REM subdirectory.
	@REM Each subdirectory uses its own recursive call and
	@REM dependency timestamp evaluation.
	@FOR %%I IN ($(patsubst %*.obj,"%",\
		$(patsubst %.cpp,%.obj,\
			$(patsubst $(PROJ_SRC_DIR)\%,%,$(PROJ_SRC))))) DO\
		@IF "%%~xI" NEQ ".obj" (\
			$(MAKE) /C $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\%%~I.sentinel.tmp\
				NMAKE_COMPILE_TARGET="%%~I"\
				@.nmake.tmp.txt\
		) ELSE (\
			$(MAKE) /C $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\%%~I\
				NMAKE_COMPILE_TARGET="%%~I"\
				@.nmake.tmp.txt\
		)
	@ECHO Linking $(PROJ_BIN)...
	@$(LINKER) $(LINKER_FLAGS) $(LINKER_FLAGS_PCH)\
		$(patsubst %.cpp,%.obj,\
		$(patsubst $(PROJ_SRC_DIR)\%,\
			$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\\%,\
			$**))

# Secondary build target for subdirectories. We are able to
# use nmake macros again via the recursive call, and also
# avoid rebuilding unchanged source files.
# Use a sentinel in the .obj directory to mark the directory
# as up-to-date. Without the sentinel and using a fake
# target instead, all dependents are considered out of date.
$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(NMAKE_COMPILE_TARGET).sentinel.tmp:\
	$(PROJ_SRC_DIR)$(NMAKE_COMPILE_TARGET)*.cpp
	@IF NOT EXIST "$@\..\" MD "$@\..\"
	@ECHO Building directory $(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(NMAKE_COMPILE_TARGET)*.obj...
	@$(COMPILER) $(COMPILER_FLAGS) $(COMPILER_FLAGS_PCH)\
		/Fd$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).pdb\
		/Fo$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(NMAKE_COMPILE_TARGET)\
		$?
	@TYPE NUL>$@
# Secondary build target for files. Need ths proxy variable\
# or the dependency line results in a syntax error.
WINDOWS_INNER_BUILD_FILE_DEP=$(PROJ_SRC_DIR)$(patsubst \
	%.obj,%.cpp,$(NMAKE_COMPILE_TARGET))
$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(NMAKE_COMPILE_TARGET):\
	$(WINDOWS_INNER_BUILD_FILE_DEP)
	@IF NOT EXIST "$@\..\" MD "$@\..\"
	@ECHO Building file $@...
	@$(COMPILER) $(COMPILER_FLAGS) $(COMPILER_FLAGS_PCH)\
		/Fd$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(BIN_NAME).pdb\
		/Fo$(PROJ_OBJ_DIR)$(BUILD_NAME)\x$(ARCH)\$(NMAKE_COMPILE_TARGET)\..\ \
		$?

# Mark source files as out of date if any headers have
# changed.
# An empty target here will correctly set the modified date.
$(PROJ_SRC): $(NMAKE_INCL_PCH) $(NMAKE_INCL) pch.cpp
	@EXIT

# ----------------------------------------------------------
!else
else
# ----------------------------------------------------------
# GNU make (gmake) specific commands.

DEFAULT_SHOW_OUTDATED=1

# Second expansion for stem matching syntax.
.SECONDEXPANSION:
# For usage of bash commands like `read`.
SHELL:=/bin/bash
# For recursive dependency listing. Directories must end
# with /.
rwildcard=$(foreach d,\
	$(wildcard $(1:%=%*)),\
	$(filter $(subst *,%,$2),$d) $(call rwildcard,$d/,$2))

ROOT_DIR:=../
REPO_NAME:=$(shell basename $(abspath $(CURDIR)/../))
PROJ?=$(DEFAULT_PROJ)
PROJ_NAME:=$(PROJ)
INCL_PCH?=$(DEFAULT_INCL_PCH)
INCL?=$(DEFAULT_INCL)
ifndef BIN
	ifndef SRC
		BIN:=$(REPO_NAME)
		SRC:=$(DEFAULT_SRC)
	else
		BIN:=$(patsubst .cpp,,$(SRC))
	endif
else
	SRC:=$(BIN).cpp
endif
BIN_NAME:=$(BIN)
SRC_NAME:=$(SRC)
BUILD?=$(DEFAULT_BUILD)
ifeq ($(BUILD),2)
	BUILD_NAME:=instrument
else ifeq ($(BUILD),1)
	BUILD_NAME:=release
else
	BUILD_NAME:=debug
endif
ARCH?=$(DEFAULT_ARCH)
PCH?=$(DEFAULT_PCH)
ARGS?=$(DEFAULT_ARGS)
VERSIONING?=$(DEFAULT_VERSIONING)
SHOW_OUTDATED?=$(DEFAULT_SHOW_OUTDATED)

BIN_DIR:=$(ROOT_DIR).bin/
TEST_DIR:=$(ROOT_DIR)test/
PROJ_BIN:=\
	$(BIN_DIR)$(PROJ_NAME)/$(BIN_NAME).$(BUILD_NAME).x$(ARCH)
PROJ_BIN_DIR:=$(BIN_DIR)$(PROJ_NAME)/
OBJ_DIR:=$(ROOT_DIR).obj/
PROJ_OBJ_DIR:=$(OBJ_DIR)$(PROJ_NAME)/
PROJ_SRC_DIR:=$(ROOT_DIR)$(PROJ_NAME)/
ifeq ($(SRC_NAME),*.cpp)
	PROJ_SRC:=$(call rwildcard,$(PROJ_SRC_DIR),*.cpp)
else
	PROJ_SRC:=$(patsubst %,$(PROJ_SRC_DIR)%,$(SRC_NAME))
endif

COMPILER?=clang++
COMPILER_FLAGS_INCLUDES:=\
	$(patsubst %,-I$(ROOT_DIR)%,$(INCL_PCH))\
	$(patsubst %,-I$(ROOT_DIR)%,$(INCL))
COMPILER_FLAGS_COMMON:=$(COMPILER_FLAGS_INCLUDES)\
	-std=c++20 -pthread\
	-Wall -Wextra -D_CONSOLE -masm=intel
COMPILER_FLAGS_DEBUG:=-D_DEBUG -g -O0
COMPILER_FLAGS_RELEASE:=-DNDEBUG -O3 -ffast-math
COMPILER_FLAGS_INSTRUMENT:=$(COMPILER_FLAGS_DEBUG)\
	/Fa$(PROJ_OBJ_DIR)
COMPILER_FLAGS_64:=-m64
COMPILER_FLAGS_86:=-m32

LINKER:=$(COMPILER)
LINKER_FLAGS_LIBRARIES:=-pthread -lresolv
LINKER_FLAGS_COMMON:=$(LINKER_FLAGS_LIBRARIES)
LINKER_FLAGS_DEBUG:=
LINKER_FLAGS_RELEASE:=
LINKER_FLAGS_INSTRUMENT:=$(LINKER_FLAGS_DEBUG)\
	-fsanitize=address,undefined,leak
LINKER_FLAGS_64:=-m64
LINKER_FLAGS_86:=-m32

# -lstdc++fs is neither available nor required on typical
# MacOS C++ compiler implementations.
ifneq ($(OS),Darwin)
	LINKER_FLAGS_LIBRARIES:=$(LINKER_FLAGS_LIBRARIES)\
		-lstdc++fs
endif

ifeq ($(BUILD_NAME),instrument)
	COMPILER_FLAGS:=$(COMPILER_FLAGS_COMMON)\
		$(COMPILER_FLAGS_INSTRUMENT)
	LINKER_FLAGS:=$(LINKER_FLAGS_COMMON)\
		$(LINKER_FLAGS_INSTRUMENT)
else ifeq ($(BUILD_NAME),release)
	COMPILER_FLAGS:=$(COMPILER_FLAGS_COMMON)\
		$(COMPILER_FLAGS_RELEASE)
	LINKER_FLAGS:=$(LINKER_FLAGS_COMMON)\
		$(LINKER_FLAGS_RELEASE)
else
	COMPILER_FLAGS:=$(COMPILER_FLAGS_COMMON)\
		$(COMPILER_FLAGS_DEBUG)
	LINKER_FLAGS:=$(LINKER_FLAGS_COMMON) $(LINKER_FLAGS_DEBUG)
endif

ifeq ($(ARCH),86)
	COMPILER_FLAGS:=$(COMPILER_FLAGS) $(COMPILER_FLAGS_86)
	LINKER_FLAGS:=$(LINKER_FLAGS) $(LINKER_FLAGS_86)
else
	COMPILER_FLAGS:=$(COMPILER_FLAGS) $(COMPILER_FLAGS_64)
	LINKER_FLAGS:=$(LINKER_FLAGS) $(LINKER_FLAGS_64)
endif
ifeq ($(PCH),1)
	PCH_PCH=$(PROJ_OBJ_DIR)$(BUILD_NAME)/x$(ARCH)/pch.pch
	COMPILER_FLAGS_PCH=-include-pch\
		$(PROJ_OBJ_DIR)$(BUILD_NAME)/x$(ARCH)/pch.pch
else
	PCH_PCH=
	COMPILER_FLAGS_PCH=
endif

# Explicitly wildcard individual files for dependencies.
INCL_PCH_HPP:=$(call rwildcard,\
	$(patsubst %,$(ROOT_DIR)%,$(INCL_PCH)),*.hpp)
INCL_HPP:=$(call rwildcard,\
	$(patsubst %,$(ROOT_DIR)%,$(INCL)),*.hpp)

# Unlike `nmake`, `gmake` first compiles all the relevant
# obj files, then compiles the relevant binaries.
# Keep intermediate and output files.
# Introduce list of project object intermediates parsed
# from the source directory with SRC_NAME.
PROJ_OBJ:=$(foreach s,$(PROJ_SRC),\
	$(addprefix $(PROJ_OBJ_DIR)$(BUILD_NAME)/x$(ARCH)/,\
		$(addsuffix .o,\
			$(s:$(PROJ_SRC_DIR)%.cpp=%))))
.SECONDARY:$(PROJ_BIN) $(PROJ_OBJ)

# Ensure files with the same name as fake targets do not
# trigger.
.PHONY: __default _default build run clean tests info
_default: build
build: version.build.txt $(PROJ_BIN)
run: build
	@echo ---------------- [Run] $(BIN_NAME) ----------------
	@cd $(PROJ_BIN_DIR) &&\
		./$(BIN_NAME).$(BUILD_NAME).x$(ARCH) $(ARGS)
clean:
	@echo Cleaning...
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
# We cannot easily print `[Build]` before building each
# test, so we do it here instead.
tests:
	@for i in $(TEST_DIR)*.cpp; do\
		echo --------------- [Build] $$(basename $$i .cpp) ---------------;\
		$(MAKE) --no-print-directory -s build\
			PROJ=test INCL_PCH="$(INCL_PCH)" INCL="$(INCL)"\
			BIN="$$(basename $$i .cpp)" SRC="$$(basename $$i)"\
			BUILD=$(BUILD) ARCH=$(ARCH) PCH=$(PCH) ARGS="$(ARGS)"\
			VERSIONING=$(VERSIONING)\
			SHOW_OUTDATED=$(SHOW_OUTDATED) ||\
			exit;\
	done
	@for i in $(TEST_DIR)*.cpp; do\
		$(MAKE) --no-print-directory -s run\
			PROJ=test INCL_PCH="$(INCL_PCH)" INCL="$(INCL)"\
			BIN="$$(basename $$i .cpp)" SRC="$$(basename $$i)"\
			BUILD=$(BUILD) ARCH=$(ARCH) PCH=$(PCH) ARGS="$(ARGS)"\
			VERSIONING=$(VERSIONING)\
			SHOW_OUTDATED=$(SHOW_OUTDATED) ||\
			exit;\
	done
info:
	@echo INCL_PCH_HPP=$(INCL_PCH_HPP)
	@echo INCL_HPP=$(INCL_HPP)
	@echo PCH_PCH=$(PCH_PCH)

	@echo ROOT_DIR=$(ROOT_DIR)
	@echo PROJ=$(PROJ)
	@echo INCL_PCH=$(INCL_PCH)
	@echo INCL=$(INCL)
	@echo BIN_NAME=$(BIN_NAME)
	@echo SRC_NAME=$(SRC_NAME)
	@echo BUILD_NAME=$(BUILD_NAME)
	@echo ARCH=$(ARCH)
	@echo PCH=$(PCH)
	@echo ARGS=$(ARGS)
	@echo VERSIONING=$(VERSIONING)
	@echo SHOW_OUTDATED=$(SHOW_OUTDATED)
	@echo PROJ_SRC=$(PROJ_SRC)
	@echo PROJ_OBJ=$(PROJ_OBJ)
	@echo COMPILER=$(COMPILER)
	@echo COMPILER_FLAGS=$(COMPILER_FLAGS)
	@echo LINKER_FLAGS=$(LINKER_FLAGS)

version.build.txt: $(INCL_PCH_HPP) $(INCL_HPP) $(PROJ_SRC)
ifeq ($(VERSIONING),1)
ifeq ($(SHOW_OUTDATED),1)
	@for i in $?; do echo Outdated: $$i.; done
endif
	@./version.sh $(shell \
		echo $(REPO_NAME) | tr '[:lower:]' '[:upper:]')
endif
$(PROJ_OBJ_DIR)$(BUILD_NAME)/x$(ARCH)/pch.pch:\
	$(INCL_PCH_HPP) pch.cpp
	@echo Building $@...
	@mkdir -p $(PROJ_OBJ_DIR)$(BUILD_NAME)/x$(ARCH)
	@$(COMPILER) $(COMPILER_FLAGS) -xc++-header pch.cpp -o $@

# Compile rule. The first line includes all header
# dependencies.
# Static pattern rule: stem % matches source file name.
$(PROJ_OBJ): $(INCL_PCH_HPP) $(INCL_HPP) $(PCH_PCH)
$(PROJ_OBJ): $(PROJ_OBJ_DIR)$(BUILD_NAME)/x$(ARCH)/%.o :\
	$(PROJ_SRC_DIR)$$*.cpp
	@mkdir -p $(@D)
	@echo Building $@...
	@$(COMPILER) $(COMPILER_FLAGS) $(COMPILER_FLAGS_PCH)\
		-c $< -o $@

# Link rule.
$(PROJ_BIN): $(BIN_DIR)% : $(PROJ_OBJ)
	@mkdir -p $(@D)
	@echo Linking $@...
	@$(LINKER) $(PROJ_OBJ) $(LINKER_FLAGS) -o $@

# ----------------------------------------------------------
endif    # gmake: close condition; nmake: not seen
!endif : # gmake: unused target; nmake close conditional
# ----------------------------------------------------------

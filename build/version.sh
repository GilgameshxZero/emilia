# Bash parallel of `version.bat`.
ROOT_DIR=../
REPO_NAME=$1

read VERSION_MAJOR<version.major.txt
read VERSION_MINOR<version.minor.txt
read VERSION_REVISION<version.revision.txt
read VERSION_BUILD<version.build.txt
VERSION_BUILD=$(($VERSION_BUILD+1))
echo $VERSION_BUILD>version.build.txt

# Create header. ${} avoids needing a space after the variable.
echo \#pragma once>version.hpp
echo>>version.hpp
echo \#define ${REPO_NAME}_VERSION_MAJOR $VERSION_MAJOR>>version.hpp
echo \#define ${REPO_NAME}_VERSION_MINOR $VERSION_MINOR>>version.hpp
echo \#define ${REPO_NAME}_VERSION_REVISION $VERSION_REVISION>>version.hpp
echo \#define ${REPO_NAME}_VERSION_BUILD $VERSION_BUILD>>version.hpp

echo Version $VERSION_MAJOR.$VERSION_MINOR.$VERSION_REVISION.$VERSION_BUILD.

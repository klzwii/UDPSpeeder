# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /usr/lib/crc32c

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /usr/lib/crc32c/build

# Include any dependencies generated for this target.
include CMakeFiles/crc32c_arm64.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/crc32c_arm64.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/crc32c_arm64.dir/flags.make

CMakeFiles/crc32c_arm64.dir/src/crc32c_arm64.cc.o: CMakeFiles/crc32c_arm64.dir/flags.make
CMakeFiles/crc32c_arm64.dir/src/crc32c_arm64.cc.o: ../src/crc32c_arm64.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/lib/crc32c/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/crc32c_arm64.dir/src/crc32c_arm64.cc.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/crc32c_arm64.dir/src/crc32c_arm64.cc.o -c /usr/lib/crc32c/src/crc32c_arm64.cc

CMakeFiles/crc32c_arm64.dir/src/crc32c_arm64.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/crc32c_arm64.dir/src/crc32c_arm64.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/lib/crc32c/src/crc32c_arm64.cc > CMakeFiles/crc32c_arm64.dir/src/crc32c_arm64.cc.i

CMakeFiles/crc32c_arm64.dir/src/crc32c_arm64.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/crc32c_arm64.dir/src/crc32c_arm64.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/lib/crc32c/src/crc32c_arm64.cc -o CMakeFiles/crc32c_arm64.dir/src/crc32c_arm64.cc.s

crc32c_arm64: CMakeFiles/crc32c_arm64.dir/src/crc32c_arm64.cc.o
crc32c_arm64: CMakeFiles/crc32c_arm64.dir/build.make

.PHONY : crc32c_arm64

# Rule to build all files generated by this target.
CMakeFiles/crc32c_arm64.dir/build: crc32c_arm64

.PHONY : CMakeFiles/crc32c_arm64.dir/build

CMakeFiles/crc32c_arm64.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/crc32c_arm64.dir/cmake_clean.cmake
.PHONY : CMakeFiles/crc32c_arm64.dir/clean

CMakeFiles/crc32c_arm64.dir/depend:
	cd /usr/lib/crc32c/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /usr/lib/crc32c /usr/lib/crc32c /usr/lib/crc32c/build /usr/lib/crc32c/build /usr/lib/crc32c/build/CMakeFiles/crc32c_arm64.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/crc32c_arm64.dir/depend


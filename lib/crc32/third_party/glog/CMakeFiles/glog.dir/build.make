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
include third_party/glog/CMakeFiles/glog.dir/depend.make

# Include the progress variables for this target.
include third_party/glog/CMakeFiles/glog.dir/progress.make

# Include the compile flags for this target's objects.
include third_party/glog/CMakeFiles/glog.dir/flags.make

third_party/glog/CMakeFiles/glog.dir/src/demangle.cc.o: third_party/glog/CMakeFiles/glog.dir/flags.make
third_party/glog/CMakeFiles/glog.dir/src/demangle.cc.o: ../third_party/glog/src/demangle.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/lib/crc32c/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object third_party/glog/CMakeFiles/glog.dir/src/demangle.cc.o"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/glog.dir/src/demangle.cc.o -c /usr/lib/crc32c/third_party/glog/src/demangle.cc

third_party/glog/CMakeFiles/glog.dir/src/demangle.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/glog.dir/src/demangle.cc.i"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/lib/crc32c/third_party/glog/src/demangle.cc > CMakeFiles/glog.dir/src/demangle.cc.i

third_party/glog/CMakeFiles/glog.dir/src/demangle.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/glog.dir/src/demangle.cc.s"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/lib/crc32c/third_party/glog/src/demangle.cc -o CMakeFiles/glog.dir/src/demangle.cc.s

third_party/glog/CMakeFiles/glog.dir/src/logging.cc.o: third_party/glog/CMakeFiles/glog.dir/flags.make
third_party/glog/CMakeFiles/glog.dir/src/logging.cc.o: ../third_party/glog/src/logging.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/lib/crc32c/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object third_party/glog/CMakeFiles/glog.dir/src/logging.cc.o"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/glog.dir/src/logging.cc.o -c /usr/lib/crc32c/third_party/glog/src/logging.cc

third_party/glog/CMakeFiles/glog.dir/src/logging.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/glog.dir/src/logging.cc.i"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/lib/crc32c/third_party/glog/src/logging.cc > CMakeFiles/glog.dir/src/logging.cc.i

third_party/glog/CMakeFiles/glog.dir/src/logging.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/glog.dir/src/logging.cc.s"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/lib/crc32c/third_party/glog/src/logging.cc -o CMakeFiles/glog.dir/src/logging.cc.s

third_party/glog/CMakeFiles/glog.dir/src/raw_logging.cc.o: third_party/glog/CMakeFiles/glog.dir/flags.make
third_party/glog/CMakeFiles/glog.dir/src/raw_logging.cc.o: ../third_party/glog/src/raw_logging.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/lib/crc32c/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object third_party/glog/CMakeFiles/glog.dir/src/raw_logging.cc.o"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/glog.dir/src/raw_logging.cc.o -c /usr/lib/crc32c/third_party/glog/src/raw_logging.cc

third_party/glog/CMakeFiles/glog.dir/src/raw_logging.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/glog.dir/src/raw_logging.cc.i"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/lib/crc32c/third_party/glog/src/raw_logging.cc > CMakeFiles/glog.dir/src/raw_logging.cc.i

third_party/glog/CMakeFiles/glog.dir/src/raw_logging.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/glog.dir/src/raw_logging.cc.s"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/lib/crc32c/third_party/glog/src/raw_logging.cc -o CMakeFiles/glog.dir/src/raw_logging.cc.s

third_party/glog/CMakeFiles/glog.dir/src/symbolize.cc.o: third_party/glog/CMakeFiles/glog.dir/flags.make
third_party/glog/CMakeFiles/glog.dir/src/symbolize.cc.o: ../third_party/glog/src/symbolize.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/lib/crc32c/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object third_party/glog/CMakeFiles/glog.dir/src/symbolize.cc.o"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/glog.dir/src/symbolize.cc.o -c /usr/lib/crc32c/third_party/glog/src/symbolize.cc

third_party/glog/CMakeFiles/glog.dir/src/symbolize.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/glog.dir/src/symbolize.cc.i"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/lib/crc32c/third_party/glog/src/symbolize.cc > CMakeFiles/glog.dir/src/symbolize.cc.i

third_party/glog/CMakeFiles/glog.dir/src/symbolize.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/glog.dir/src/symbolize.cc.s"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/lib/crc32c/third_party/glog/src/symbolize.cc -o CMakeFiles/glog.dir/src/symbolize.cc.s

third_party/glog/CMakeFiles/glog.dir/src/utilities.cc.o: third_party/glog/CMakeFiles/glog.dir/flags.make
third_party/glog/CMakeFiles/glog.dir/src/utilities.cc.o: ../third_party/glog/src/utilities.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/lib/crc32c/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object third_party/glog/CMakeFiles/glog.dir/src/utilities.cc.o"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/glog.dir/src/utilities.cc.o -c /usr/lib/crc32c/third_party/glog/src/utilities.cc

third_party/glog/CMakeFiles/glog.dir/src/utilities.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/glog.dir/src/utilities.cc.i"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/lib/crc32c/third_party/glog/src/utilities.cc > CMakeFiles/glog.dir/src/utilities.cc.i

third_party/glog/CMakeFiles/glog.dir/src/utilities.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/glog.dir/src/utilities.cc.s"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/lib/crc32c/third_party/glog/src/utilities.cc -o CMakeFiles/glog.dir/src/utilities.cc.s

third_party/glog/CMakeFiles/glog.dir/src/vlog_is_on.cc.o: third_party/glog/CMakeFiles/glog.dir/flags.make
third_party/glog/CMakeFiles/glog.dir/src/vlog_is_on.cc.o: ../third_party/glog/src/vlog_is_on.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/lib/crc32c/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object third_party/glog/CMakeFiles/glog.dir/src/vlog_is_on.cc.o"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/glog.dir/src/vlog_is_on.cc.o -c /usr/lib/crc32c/third_party/glog/src/vlog_is_on.cc

third_party/glog/CMakeFiles/glog.dir/src/vlog_is_on.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/glog.dir/src/vlog_is_on.cc.i"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/lib/crc32c/third_party/glog/src/vlog_is_on.cc > CMakeFiles/glog.dir/src/vlog_is_on.cc.i

third_party/glog/CMakeFiles/glog.dir/src/vlog_is_on.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/glog.dir/src/vlog_is_on.cc.s"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/lib/crc32c/third_party/glog/src/vlog_is_on.cc -o CMakeFiles/glog.dir/src/vlog_is_on.cc.s

third_party/glog/CMakeFiles/glog.dir/src/signalhandler.cc.o: third_party/glog/CMakeFiles/glog.dir/flags.make
third_party/glog/CMakeFiles/glog.dir/src/signalhandler.cc.o: ../third_party/glog/src/signalhandler.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/lib/crc32c/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object third_party/glog/CMakeFiles/glog.dir/src/signalhandler.cc.o"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/glog.dir/src/signalhandler.cc.o -c /usr/lib/crc32c/third_party/glog/src/signalhandler.cc

third_party/glog/CMakeFiles/glog.dir/src/signalhandler.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/glog.dir/src/signalhandler.cc.i"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/lib/crc32c/third_party/glog/src/signalhandler.cc > CMakeFiles/glog.dir/src/signalhandler.cc.i

third_party/glog/CMakeFiles/glog.dir/src/signalhandler.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/glog.dir/src/signalhandler.cc.s"
	cd /usr/lib/crc32c/build/third_party/glog && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/lib/crc32c/third_party/glog/src/signalhandler.cc -o CMakeFiles/glog.dir/src/signalhandler.cc.s

# Object files for target glog
glog_OBJECTS = \
"CMakeFiles/glog.dir/src/demangle.cc.o" \
"CMakeFiles/glog.dir/src/logging.cc.o" \
"CMakeFiles/glog.dir/src/raw_logging.cc.o" \
"CMakeFiles/glog.dir/src/symbolize.cc.o" \
"CMakeFiles/glog.dir/src/utilities.cc.o" \
"CMakeFiles/glog.dir/src/vlog_is_on.cc.o" \
"CMakeFiles/glog.dir/src/signalhandler.cc.o"

# External object files for target glog
glog_EXTERNAL_OBJECTS =

third_party/glog/libglog.a: third_party/glog/CMakeFiles/glog.dir/src/demangle.cc.o
third_party/glog/libglog.a: third_party/glog/CMakeFiles/glog.dir/src/logging.cc.o
third_party/glog/libglog.a: third_party/glog/CMakeFiles/glog.dir/src/raw_logging.cc.o
third_party/glog/libglog.a: third_party/glog/CMakeFiles/glog.dir/src/symbolize.cc.o
third_party/glog/libglog.a: third_party/glog/CMakeFiles/glog.dir/src/utilities.cc.o
third_party/glog/libglog.a: third_party/glog/CMakeFiles/glog.dir/src/vlog_is_on.cc.o
third_party/glog/libglog.a: third_party/glog/CMakeFiles/glog.dir/src/signalhandler.cc.o
third_party/glog/libglog.a: third_party/glog/CMakeFiles/glog.dir/build.make
third_party/glog/libglog.a: third_party/glog/CMakeFiles/glog.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/usr/lib/crc32c/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Linking CXX static library libglog.a"
	cd /usr/lib/crc32c/build/third_party/glog && $(CMAKE_COMMAND) -P CMakeFiles/glog.dir/cmake_clean_target.cmake
	cd /usr/lib/crc32c/build/third_party/glog && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/glog.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
third_party/glog/CMakeFiles/glog.dir/build: third_party/glog/libglog.a

.PHONY : third_party/glog/CMakeFiles/glog.dir/build

third_party/glog/CMakeFiles/glog.dir/clean:
	cd /usr/lib/crc32c/build/third_party/glog && $(CMAKE_COMMAND) -P CMakeFiles/glog.dir/cmake_clean.cmake
.PHONY : third_party/glog/CMakeFiles/glog.dir/clean

third_party/glog/CMakeFiles/glog.dir/depend:
	cd /usr/lib/crc32c/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /usr/lib/crc32c /usr/lib/crc32c/third_party/glog /usr/lib/crc32c/build /usr/lib/crc32c/build/third_party/glog /usr/lib/crc32c/build/third_party/glog/CMakeFiles/glog.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : third_party/glog/CMakeFiles/glog.dir/depend


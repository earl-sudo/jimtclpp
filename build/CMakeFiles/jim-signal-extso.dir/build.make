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


# Produce verbose output by default.
VERBOSE = 1

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
CMAKE_SOURCE_DIR = /mnt/hgfs/jimtcl_share/jimtclpp/jimtclpp

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/hgfs/jimtcl_share/jimtclpp/jimtclpp/build

# Include any dependencies generated for this target.
include CMakeFiles/jim-signal-extso.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/jim-signal-extso.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/jim-signal-extso.dir/flags.make

CMakeFiles/jim-signal-extso.dir/binary_ext/jim-signal-ext.cpp.o: CMakeFiles/jim-signal-extso.dir/flags.make
CMakeFiles/jim-signal-extso.dir/binary_ext/jim-signal-ext.cpp.o: ../binary_ext/jim-signal-ext.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/jimtcl_share/jimtclpp/jimtclpp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/jim-signal-extso.dir/binary_ext/jim-signal-ext.cpp.o"
	g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/jim-signal-extso.dir/binary_ext/jim-signal-ext.cpp.o -c /mnt/hgfs/jimtcl_share/jimtclpp/jimtclpp/binary_ext/jim-signal-ext.cpp

CMakeFiles/jim-signal-extso.dir/binary_ext/jim-signal-ext.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/jim-signal-extso.dir/binary_ext/jim-signal-ext.cpp.i"
	g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/jimtcl_share/jimtclpp/jimtclpp/binary_ext/jim-signal-ext.cpp > CMakeFiles/jim-signal-extso.dir/binary_ext/jim-signal-ext.cpp.i

CMakeFiles/jim-signal-extso.dir/binary_ext/jim-signal-ext.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/jim-signal-extso.dir/binary_ext/jim-signal-ext.cpp.s"
	g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/jimtcl_share/jimtclpp/jimtclpp/binary_ext/jim-signal-ext.cpp -o CMakeFiles/jim-signal-extso.dir/binary_ext/jim-signal-ext.cpp.s

# Object files for target jim-signal-extso
jim__signal__extso_OBJECTS = \
"CMakeFiles/jim-signal-extso.dir/binary_ext/jim-signal-ext.cpp.o"

# External object files for target jim-signal-extso
jim__signal__extso_EXTERNAL_OBJECTS =

../bin/libjim-signal-extso.so: CMakeFiles/jim-signal-extso.dir/binary_ext/jim-signal-ext.cpp.o
../bin/libjim-signal-extso.so: CMakeFiles/jim-signal-extso.dir/build.make
../bin/libjim-signal-extso.so: CMakeFiles/jim-signal-extso.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/hgfs/jimtcl_share/jimtclpp/jimtclpp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library ../bin/libjim-signal-extso.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/jim-signal-extso.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/jim-signal-extso.dir/build: ../bin/libjim-signal-extso.so

.PHONY : CMakeFiles/jim-signal-extso.dir/build

CMakeFiles/jim-signal-extso.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/jim-signal-extso.dir/cmake_clean.cmake
.PHONY : CMakeFiles/jim-signal-extso.dir/clean

CMakeFiles/jim-signal-extso.dir/depend:
	cd /mnt/hgfs/jimtcl_share/jimtclpp/jimtclpp/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/hgfs/jimtcl_share/jimtclpp/jimtclpp /mnt/hgfs/jimtcl_share/jimtclpp/jimtclpp /mnt/hgfs/jimtcl_share/jimtclpp/jimtclpp/build /mnt/hgfs/jimtcl_share/jimtclpp/jimtclpp/build /mnt/hgfs/jimtcl_share/jimtclpp/jimtclpp/build/CMakeFiles/jim-signal-extso.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/jim-signal-extso.dir/depend


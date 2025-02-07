# This file is a 'toolchain description file' for CMake.
# It teaches CMake about the Emscripten compiler, so that CMake can generate makefiles
# from CMakeLists.txt that invoke emcc.

# To use this toolchain file with CMake, invoke CMake with the following command line parameters
# cmake -DCMAKE_TOOLCHAIN_FILE=<EmscriptenRoot>/cmake/Platform/Emscripten.cmake
#       -DCMAKE_BUILD_TYPE=<Debug|RelWithDebInfo|Release|MinSizeRel>
#       -G "Unix Makefiles" (Linux and OSX)
#       -G "MinGW Makefiles" (Windows)
#       <path/to/CMakeLists.txt> # Note, pass in here ONLY the path to the file, not the filename 'CMakeLists.txt' itself.

# After that, build the generated Makefile with the command 'make'. On Windows, you may download and use 'mingw32-make' instead.

# The following variable describes the target OS we are building to.
# Ideally, this could be 'Emscripten', but as Emscripten mimics the Linux platform, setting this to Linux will allow more of existing software to build.
# Be sure to run Emscripten test_openjpeg if planning to change this.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_CROSSCOMPILING TRUE)

# Do a no-op access on the CMAKE_TOOLCHAIN_FILE variable so that CMake will not issue a warning on it being unused.
if (CMAKE_TOOLCHAIN_FILE)
endif()

# Locate where the Emscripten compiler resides in relative to this toolchain file.
if ("${EMSCRIPTEN_ROOT_PATH}" STREQUAL "")
	get_filename_component(GUESS_EMSCRIPTEN_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)
	if (EXISTS "${GUESS_EMSCRIPTEN_ROOT_PATH}/emranlib")
		set(EMSCRIPTEN_ROOT_PATH "${GUESS_EMSCRIPTEN_ROOT_PATH}")
	endif()
endif()

# If not found by above search, locate using the EMSCRIPTEN environment variable.
if ("${EMSCRIPTEN_ROOT_PATH}" STREQUAL "")
	set(EMSCRIPTEN_ROOT_PATH "$ENV{EMSCRIPTEN}")
endif()

# Abort if not found. 
if ("${EMSCRIPTEN_ROOT_PATH}" STREQUAL "")
	message(FATAL_ERROR "Could not locate the Emscripten compiler toolchain directory! Either set the EMSCRIPTEN environment variable, or pass -DEMSCRIPTEN_ROOT_PATH=xxx to CMake to explicitly specify the location of the compiler!")
endif()

# Normalize, convert Windows backslashes to forward slashes or CMake will crash.
get_filename_component(EMSCRIPTEN_ROOT_PATH "${EMSCRIPTEN_ROOT_PATH}" ABSOLUTE)

if (NOT CMAKE_MODULE_PATH)
	set(CMAKE_MODULE_PATH "")
endif()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${EMSCRIPTEN_ROOT_PATH}/cmake/Modules")

set(CMAKE_FIND_ROOT_PATH "${EMSCRIPTEN_ROOT_PATH}/system")

if (CMAKE_HOST_WIN32)
	set(EMCC_SUFFIX ".bat")
else()
	set(EMCC_SUFFIX "")
endif()

# Specify the compilers to use for C and C++
if ("${CMAKE_C_COMPILER}" STREQUAL "")
	set(CMAKE_C_COMPILER "${EMSCRIPTEN_ROOT_PATH}/emcc${EMCC_SUFFIX}")
endif()
if ("${CMAKE_CXX_COMPILER}" STREQUAL "")
	set(CMAKE_CXX_COMPILER "${EMSCRIPTEN_ROOT_PATH}/em++${EMCC_SUFFIX}")
endif()

if ("${CMAKE_AR}" STREQUAL "")
	set(CMAKE_AR "${EMSCRIPTEN_ROOT_PATH}/emar${EMCC_SUFFIX}" CACHE FILEPATH "Emscripten ar")
endif()

if ("${CMAKE_RANLIB}" STREQUAL "")
	set(CMAKE_RANLIB "${EMSCRIPTEN_ROOT_PATH}/emranlib${EMCC_SUFFIX}" CACHE FILEPATH "Emscripten ranlib")
endif()

# Don't do compiler autodetection, since we are cross-compiling.
include(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER("${CMAKE_C_COMPILER}" Clang)
CMAKE_FORCE_CXX_COMPILER("${CMAKE_CXX_COMPILER}" Clang)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_SYSTEM_INCLUDE_PATH "${EMSCRIPTEN_ROOT_PATH}/system/include")

# We would prefer to specify a standard set of Clang+Emscripten-friendly common convention for suffix files, especially for CMake executable files,
# but if these are adjusted, ${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake will fail, since it depends on being able to compile output files with predefined names.
#SET(CMAKE_LINK_LIBRARY_SUFFIX "")
#SET(CMAKE_STATIC_LIBRARY_PREFIX "")
#SET(CMAKE_STATIC_LIBRARY_SUFFIX ".bc")
#SET(CMAKE_SHARED_LIBRARY_PREFIX "")
#SET(CMAKE_SHARED_LIBRARY_SUFFIX ".bc")
#IF (NOT CMAKE_EXECUTABLE_SUFFIX)
#	SET(CMAKE_EXECUTABLE_SUFFIX ".js")
#endif()
#SET(CMAKE_FIND_LIBRARY_PREFIXES "")
#SET(CMAKE_FIND_LIBRARY_SUFFIXES ".bc")

SET(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES 1)
SET(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES 1)
SET(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS 1)
SET(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS 1)
SET(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES 1)
SET(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES 1)

set(CMAKE_C_RESPONSE_FILE_LINK_FLAG "@")
set(CMAKE_CXX_RESPONSE_FILE_LINK_FLAG "@")

# Specify the program to use when building static libraries. Force Emscripten-related command line options to clang.
set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_AR> rc <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_CXX_CREATE_STATIC_LIBRARY "<CMAKE_AR> rc <TARGET> <LINK_FLAGS> <OBJECTS>")

# Set a global EMSCRIPTEN variable that can be used in client CMakeLists.txt to detect when building using Emscripten.
set(EMSCRIPTEN 1 CACHE BOOL "If true, we are targeting Emscripten output.")

# We are cross-compiling, so unset the common CMake variables that represent the target platform. Leave UNIX define enabled, since Emscripten
# mimics a Linux environment.
SET(WIN32)
SET(APPLE)

set(CMAKE_C_SIZEOF_DATA_PTR 4)
set(CMAKE_CXX_SIZEOF_DATA_PTR 4)

set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O2" CACHE STRING "Emscripten-overridden CMAKE_C_FLAGS_RELEASE")
set(CMAKE_C_FLAGS_MINSIZEREL "-DNDEBUG -Os" CACHE STRING "Emscripten-overridden CMAKE_C_FLAGS_MINSIZEREL")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2" CACHE STRING "Emscripten-overridden CMAKE_C_FLAGS_RELWITHDEBINFO")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O2" CACHE STRING "Emscripten-overridden CMAKE_CXX_FLAGS_RELEASE")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-DNDEBUG -Os" CACHE STRING "Emscripten-overridden CMAKE_CXX_FLAGS_MINSIZEREL")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2" CACHE STRING "Emscripten-overridden CMAKE_CXX_FLAGS_RELWITHDEBINFO")

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-O2" CACHE STRING "Emscripten-overridden CMAKE_EXE_LINKER_FLAGS_RELEASE")
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "-Os" CACHE STRING "Emscripten-overridden CMAKE_EXE_LINKER_FLAGS_MINSIZEREL")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-O2 -g" CACHE STRING "Emscripten-overridden CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-O2" CACHE STRING "Emscripten-overridden CMAKE_SHARED_LINKER_FLAGS_RELEASE")
set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "-Os" CACHE STRING "Emscripten-overridden CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL")
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "-O2 -g" CACHE STRING "Emscripten-overridden CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO")
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "-O2" CACHE STRING "Emscripten-overridden CMAKE_MODULE_LINKER_FLAGS_RELEASE")
set(CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL "-Os" CACHE STRING "Emscripten-overridden CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL")
set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "-O2 -g" CACHE STRING "Emscripten-overridden CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO")

function(em_validate_asmjs_after_build target)
	add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E echo Validating build output for asm.js... COMMAND "python" ARGS "${EMSCRIPTEN_ROOT_PATH}/tools/validate_asmjs.py" "$<TARGET_FILE:${target}>")
endfunction()

# A global counter to guarantee unique names for js library files.
set(link_js_counter 1)

# Internal function: Do not call from user CMakeLists.txt files. Use one of em_link_js_library()/em_link_pre_js()/em_link_post_js() instead.
function(em_add_tracked_link_flag target flagname)
	get_target_property(props ${target} LINK_FLAGS)
	if(NOT props)
	    set(props "")
	endif()

	# User can input list of JS files either as a single list, or as variable arguments to this function, so iterate over varargs, and treat each
	# item in varargs as a list itself, to support both syntax forms.
	foreach(jsFileList ${ARGN})
		foreach(jsfile ${jsFileList})
			# Add link command to the given JS file.
			set(props "${props} ${flagname} \"${jsfile}\"")
			
			# If the user edits the JS file, we want to relink the emscripten application, but unfortunately it is not possible to make a link step
			# depend directly on a source file. Instead, we must make a dummy no-op build target on that source file, and make the project depend on
			# that target.
			
			# Sanitate the source .js filename to a good symbol name to use as a dummy filename.
			get_filename_component(jsname "${jsfile}" NAME)
			string(REGEX REPLACE "[/:\\\\.\ ]" "_" dummy_js_target ${jsname})
			set(dummy_lib_name ${target}_${link_js_counter}_${dummy_js_target})
			set(dummy_c_name "${CMAKE_BINARY_DIR}/${dummy_js_target}_tracker.c")

			# Create a new static library target that with a single dummy .c file.
			add_library(${dummy_lib_name} STATIC ${dummy_c_name})
			# Make the dummy .c file depend on the .js file we are linking, so that if the .js file is edited, the dummy .c file, and hence the static library will be rebuild (no-op). This causes the main application to be relinked, which is what we want.
			# This approach was recommended by http://www.cmake.org/pipermail/cmake/2010-May/037206.html
			add_custom_command(OUTPUT ${dummy_c_name} COMMAND ${CMAKE_COMMAND} -E touch ${dummy_c_name} DEPENDS ${jsfile})
			target_link_libraries(${target} ${dummy_lib_name})

			math(EXPR link_js_counter "${link_js_counter} + 1")
		endforeach()
	endforeach()
	set_target_properties(${target} PROPERTIES LINK_FLAGS "${props}")
endfunction()

# This function links a (list of ) .js library file(s) to the given CMake project.
# Example: em_link_js_library(my_executable "lib1.js" "lib2.js")
#    will result in emcc passing --js-library lib1.js --js-library lib2.js to the emscripten linker, as well as
#    tracking the modification timestamp between the linked .js files and the main project, so that editing the .js file
#    will cause the target project to be relinked.
function(em_link_js_library target)
	em_add_tracked_link_flag(${target} "--js-library" ${ARGN})
endfunction()

# This function is identical to em_link_js_library(), except the .js files will be added with '--pre-js file.js' command line flag,
# which is generally used to add some preamble .js code to a generated output file.
function(em_link_pre_js target)
	em_add_tracked_link_flag(${target} "--pre-js" ${ARGN})
endfunction()

# This function is identical to em_link_js_library(), except the .js files will be added with '--post-js file.js' command line flag,
# which is generally used to add some postamble .js code to a generated output file.
function(em_link_post_js target)
	em_add_tracked_link_flag(${target} "--post-js" ${ARGN})
endfunction()

# Experimental support for targeting generation of Visual Studio project files (vs-tool) of Emscripten projects for Windows.
# To use this, pass the combination -G "Visual Studio 10" -DCMAKE_TOOLCHAIN_FILE=Emscripten.cmake
if ("${CMAKE_GENERATOR}" MATCHES "^Visual Studio.*")
	# By default, CMake generates VS project files with a <GenerateManifest>true</GenerateManifest> directive.
	# This causes VS to attempt to invoke rc.exe during the build, which will fail since app manifests are meaningless for Emscripten.
	# To disable this, add the following linker flag. This flag will not go to emcc, since the Visual Studio CMake generator will swallow it.
	set(EMSCRIPTEN_VS_LINKER_FLAGS "/MANIFEST:NO")
	# CMake is hardcoded to write a ClCompile directive <ObjectFileName>$(IntDir)</ObjectFileName> in all VS project files it generates.
	# This makes VS pass emcc a -o param that points to a directory instead of a file, which causes emcc autogenerate the output filename.
	# CMake is hardcoded to assume all object files have the suffix .obj, so adjust the emcc-autogenerated default suffix name to match.
	set(EMSCRIPTEN_VS_LINKER_FLAGS "${EMSCRIPTEN_VS_LINKER_FLAGS} --default-obj-ext .obj")
	# Also hint CMake that it should not hardcode <ObjectFileName> generation. Requires a custom CMake build for this to work (ignored on others)
	# See http://www.cmake.org/Bug/view.php?id=14673 and https://github.com/juj/CMake
	set(CMAKE_VS_NO_DEFAULT_OBJECTFILENAME 1)

	# Apply and cache Emscripten Visual Studio IDE-specific linker flags.
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EMSCRIPTEN_VS_LINKER_FLAGS}" CACHE STRING "")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${EMSCRIPTEN_VS_LINKER_FLAGS}" CACHE STRING "")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${EMSCRIPTEN_VS_LINKER_FLAGS}" CACHE STRING "")
endif()

SET(OS "qnx")
#SET(NO_PTHREAD_CANCEL TRUE)
SET(RENDER_EMBEDDED TRUE CACHE BOOL "OpenGL ES")
SET(RENDER_OPENGL FALSE CACHE BOOL "OGL")
SET(RENDER_GL2 TRUE CACHE BOOL "GL2")
SET(DISABLE_SOUND FALSE CACHE BOOL "Sound")

SET(OPENAL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/Adventure/AdvEngine/Engine/openal/include)
SET(OPENAL_LIBRARY ${PROJECT_SOURCE_DIR}/Adventure/AdvEngine/Engine/openal/lib/android/libopenal.so)
SET(EMSCRIPTEN_FRONTEND TRUE CACHE BOOL "emscripten frontend")
SET(DISABLE_XF86VMODE TRUE CACHE BOOL "x11 xf86")
ADD_DEFINITIONS(-Wno-warn-absolute-paths)

# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)

# Declares and names the project.

project("oggdemo2")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.


set(VORBIS_PUBLIC_HEADERS
        vorbis/include/vorbis/codec.h
        vorbis/include/vorbis/vorbisenc.h
        vorbis/include/vorbis/vorbisfile.h
        )

set(VORBIS_HEADERS
        vorbis/lib/envelope.h
        vorbis/lib/lpc.h
        vorbis/lib/lsp.h
        vorbis/lib/codebook.h
        vorbis/lib/misc.h
        vorbis/lib/psy.h
        vorbis/lib/masking.h
        vorbis/lib/os.h
        vorbis/lib/mdct.h
        vorbis/lib/smallft.h
        vorbis/lib/highlevel.h
        vorbis/lib/registry.h
        vorbis/lib/scales.h
        vorbis/lib/window.h
        vorbis/lib/lookup.h
        vorbis/lib/lookup_data.h
        vorbis/lib/codec_internal.h
        vorbis/lib/backends.h
        vorbis/lib/bitrate.h
        )

set(VORBIS_SOURCES
        vorbis/lib/mdct.c
        vorbis/lib/smallft.c
        vorbis/lib/block.c
        vorbis/lib/envelope.c
        vorbis/lib/window.c
        vorbis/lib/lsp.c
        vorbis/lib/lpc.c
        vorbis/lib/analysis.c
        vorbis/lib/synthesis.c
        vorbis/lib/psy.c
        vorbis/lib/info.c
        vorbis/lib/floor1.c
        vorbis/lib/floor0.c
        vorbis/lib/res0.c
        vorbis/lib/mapping0.c
        vorbis/lib/registry.c
        vorbis/lib/codebook.c
        vorbis/lib/sharedbook.c
        vorbis/lib/lookup.c
        vorbis/lib/bitrate.c
        )

set(VORBISFILE_SOURCES
        vorbis/lib/vorbisfile.c
        )

set(VORBISENC_SOURCES
        vorbis/lib/vorbisenc.c
        )

add_library( # Sets the name of the library.
             ogg-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             ogg-lib.cpp
             vorbis/libogg/framing.c
             vorbis/libogg/bitwise.c
             ${VORBIS_PUBLIC_HEADERS} ${VORBIS_HEADERS} ${VORBIS_SOURCES} ${VORBISFILE_SOURCES} ${VORBISENC_SOURCES})

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       ogg-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )
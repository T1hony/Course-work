# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Yandex.Disk/Course work/build/_deps/sdl3_ttf-src")
  file(MAKE_DIRECTORY "C:/Yandex.Disk/Course work/build/_deps/sdl3_ttf-src")
endif()
file(MAKE_DIRECTORY
  "C:/Yandex.Disk/Course work/build/_deps/sdl3_ttf-build"
  "C:/Yandex.Disk/Course work/build/_deps/sdl3_ttf-subbuild/sdl3_ttf-populate-prefix"
  "C:/Yandex.Disk/Course work/build/_deps/sdl3_ttf-subbuild/sdl3_ttf-populate-prefix/tmp"
  "C:/Yandex.Disk/Course work/build/_deps/sdl3_ttf-subbuild/sdl3_ttf-populate-prefix/src/sdl3_ttf-populate-stamp"
  "C:/Yandex.Disk/Course work/build/_deps/sdl3_ttf-subbuild/sdl3_ttf-populate-prefix/src"
  "C:/Yandex.Disk/Course work/build/_deps/sdl3_ttf-subbuild/sdl3_ttf-populate-prefix/src/sdl3_ttf-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Yandex.Disk/Course work/build/_deps/sdl3_ttf-subbuild/sdl3_ttf-populate-prefix/src/sdl3_ttf-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Yandex.Disk/Course work/build/_deps/sdl3_ttf-subbuild/sdl3_ttf-populate-prefix/src/sdl3_ttf-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()

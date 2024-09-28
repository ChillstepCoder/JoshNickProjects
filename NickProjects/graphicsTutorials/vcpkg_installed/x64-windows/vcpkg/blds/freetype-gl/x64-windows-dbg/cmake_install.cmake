# Install script for directory: C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/pkgs/freetype-gl_x64-windows/debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "OFF")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/x64-windows-dbg/freetype-gl.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/freetype-gl" TYPE FILE PERMISSIONS OWNER_READ GROUP_READ WORLD_READ FILES
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/distance-field.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/edtaa3func.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/font-manager.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/freetype-gl.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/markup.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/opengl.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/platform.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/text-buffer.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/texture-atlas.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/texture-font.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/utf8-utils.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/ftgl-utils.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/vec234.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/vector.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/vertex-attribute.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/vertex-buffer.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/src/2a4a3a1f5d-a92423a07b.clean/freetype-gl-errdef.h"
    "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/x64-windows-dbg/config.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/freetype-gl/freetype-glConfig.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/freetype-gl/freetype-glConfig.cmake"
         "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/x64-windows-dbg/CMakeFiles/Export/2d2e3410392ce4ff3814a668caf8cd1a/freetype-glConfig.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/freetype-gl/freetype-glConfig-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/freetype-gl/freetype-glConfig.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/freetype-gl" TYPE FILE FILES "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/x64-windows-dbg/CMakeFiles/Export/2d2e3410392ce4ff3814a668caf8cd1a/freetype-glConfig.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/freetype-gl" TYPE FILE FILES "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/x64-windows-dbg/CMakeFiles/Export/2d2e3410392ce4ff3814a668caf8cd1a/freetype-glConfig-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Users/Nicholas/Documents/GitHub/JoshNickProjects2/NickProjects/graphicsTutorials/vcpkg_installed/x64-windows/vcpkg/blds/freetype-gl/x64-windows-dbg/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")

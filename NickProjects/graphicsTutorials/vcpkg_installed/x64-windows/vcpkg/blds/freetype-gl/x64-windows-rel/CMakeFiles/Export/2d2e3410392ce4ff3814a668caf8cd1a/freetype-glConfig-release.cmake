#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "freetype-gl" for configuration "Release"
set_property(TARGET freetype-gl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(freetype-gl PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/freetype-gl.lib"
  )

list(APPEND _cmake_import_check_targets freetype-gl )
list(APPEND _cmake_import_check_files_for_freetype-gl "${_IMPORT_PREFIX}/lib/freetype-gl.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)

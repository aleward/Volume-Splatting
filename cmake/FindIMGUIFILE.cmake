#
# Find IMGUIFILE
#
# Try to find IMGUI and ImGuiFile Dialog from aiekick's github
# This module defines 
# - IMGUIFILE_INCLUDE_DIRS
# - IMGUIFILE_FOUND
#
# The following variables can be set as arguments for the module.
# - IMGUIFILE_ROOT_DIR : Root library directory of ImGuiFileDialog
#
# References:
# - https://github.com/aiekick/ImGuiFileDialog
#

# Additional modules
include(FindPackageHandleStandardArgs)

if (WIN32)
	# Find include files
	find_path(
		IMGUIFILE_INCLUDE_DIR
		NAMES ImGuiFileDialog/CustomImGuiFileDialogConfig.h
		PATHS
		$ENV{PROGRAMFILES}/include
		${IMGUIFILE_ROOT_DIR}/include
		DOC "The directory where ImGuiFileDialog/CustomImGuiFileDialogConfig.h resides")
else()
	# Find include files
	find_path(
		IMGUIFILE_INCLUDE_DIR
		NAMES ImGuiFileDialog/CustomImGuiFileDialogConfig.h
		PATHS
		/usr/include
		/usr/local/include
		/sw/include
		/opt/local/include
		${IMGUIFILE_ROOT_DIR}/include
		DOC "The directory where ImGuiFileDialog/CustomImGuiFileDialogConfig.h resides")
endif()

# Handle REQUIRD argument, define *_FOUND variable
find_package_handle_standard_args(IMGUIFILE DEFAULT_MSG IMGUIFILE_INCLUDE_DIR)

# Define IMGUIFILE_INCLUDE_DIRS
if (IMGUIFILE_FOUND)
	set(IMGUIFILE_INCLUDE_DIRS ${IMGUIFILE_INCLUDE_DIR})
endif()

# Hide some variables
mark_as_advanced(IMGUIFILE_INCLUDE_DIR)

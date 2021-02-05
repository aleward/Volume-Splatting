set(ITKEigen3_LOADED 1)
set(ITKEigen3_ENABLE_SHARED "0")
set(ITKEigen3_DEPENDS "")
set(ITKEigen3_PUBLIC_DEPENDS "")
set(ITKEigen3_TRANSITIVE_DEPENDS "")
set(ITKEigen3_PRIVATE_DEPENDS "")
set(ITKEigen3_LIBRARIES "ITKInternalEigen3::Eigen")
set(ITKEigen3_INCLUDE_DIRS "${ITK_INSTALL_PREFIX}/include/ITK-5.1")
set(ITKEigen3_LIBRARY_DIRS "")
set(ITKEigen3_RUNTIME_LIBRARY_DIRS "${ITK_INSTALL_PREFIX}/bin")
set(ITKEigen3_TARGETS_FILE "")
set(ITKEigen3_FACTORY_NAMES "")

set(ITK_USE_SYSTEM_EIGEN "OFF")
set(ITKInternalEigen3_DIR "${ITKDEBUG_MODULES_DIR}")
find_package(ITKInternalEigen3 3.3 REQUIRED CONFIG)


set(ITKIOMeshVTK_LOADED 1)
set(ITKIOMeshVTK_ENABLE_SHARED "1")
set(ITKIOMeshVTK_DEPENDS "ITKCommon;ITKDoubleConversion;ITKIOMeshBase;ITKMesh")
set(ITKIOMeshVTK_PUBLIC_DEPENDS "ITKCommon;ITKIOMeshBase")
set(ITKIOMeshVTK_TRANSITIVE_DEPENDS "ITKCommon;ITKIOMeshBase;ITKMesh")
set(ITKIOMeshVTK_PRIVATE_DEPENDS "ITKDoubleConversion")
set(ITKIOMeshVTK_LIBRARIES "ITKIOMeshVTK")
set(ITKIOMeshVTK_INCLUDE_DIRS "${ITK_INSTALL_PREFIX}/include/ITK-5.1")
set(ITKIOMeshVTK_LIBRARY_DIRS "")
set(ITKIOMeshVTK_RUNTIME_LIBRARY_DIRS "${ITK_INSTALL_PREFIX}/bin")
set(ITKIOMeshVTK_TARGETS_FILE "")
set(ITKIOMeshVTK_FACTORY_NAMES "MeshIO::VTKPolyData")


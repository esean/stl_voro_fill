vtk_module(vtkIOImage
  GROUPS
    StandAlone
  TEST_DEPENDS
    vtkTestingCore
    vtkImagingSources
    vtkImagingMath
    vtkInteractionImage
    vtkInteractionStyle
    vtkRenderingContext${VTK_RENDERING_BACKEND}
    vtkTestingCore
    vtkTestingRendering
    vtkIOLegacy
  KIT
    vtkIO
  DEPENDS
    vtkCommonCore
    vtkCommonExecutionModel
  PRIVATE_DEPENDS
    vtkCommonDataModel
    vtkCommonMath
    vtkCommonMisc
    vtkCommonSystem
    vtkCommonTransforms
    vtkDICOMParser
    vtkMetaIO
    vtkjpeg
    vtkpng
    vtksys
    vtktiff
    vtkzlib
  )
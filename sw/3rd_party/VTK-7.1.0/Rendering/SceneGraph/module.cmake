vtk_module(vtkRenderingSceneGraph
  TEST_DEPENDS
    vtkTestingCore
    vtkRendering${VTK_RENDERING_BACKEND}
    vtkRenderingVolume
  KIT
    vtkRendering
  DEPENDS
    vtkCommonCore
  PRIVATE_DEPENDS
    vtkCommonDataModel
    vtkCommonMath
    vtkRenderingCore
  )
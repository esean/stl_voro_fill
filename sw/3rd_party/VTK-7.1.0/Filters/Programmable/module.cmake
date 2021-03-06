vtk_module(vtkFiltersProgrammable
  GROUPS
    StandAlone
  TEST_DEPENDS
    vtkRendering${VTK_RENDERING_BACKEND}
    vtkTestingRendering
    vtkInteractionStyle
  KIT
    vtkFilters
  DEPENDS
    vtkCommonExecutionModel
  PRIVATE_DEPENDS
    vtkCommonCore
    vtkCommonDataModel
    vtkCommonTransforms
  )
vtk_module(vtkWebCore
  GROUPS
    Web
  TEST_DEPENDS
    vtkImagingSources
    vtkTestingCore
  TEST_LABELS
    VTKWEB
  EXCLUDE_FROM_ALL
  DEPENDS
    vtkCommonCore
  PRIVATE_DEPENDS
    vtkCommonDataModel
    vtkCommonSystem
    vtkFiltersGeneral
    vtkIOCore
    vtkIOImage
    vtkParallelCore
    vtkPython
    vtkRenderingCore
    vtkWebGLExporter
    vtksys
)
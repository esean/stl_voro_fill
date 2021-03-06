/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestPolyDataConnectivityFilter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkPointData.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkSmartPointer.h>

namespace
{
void InitializePolyData(vtkPolyData *polyData, int dataType)
{
  vtkSmartPointer<vtkMinimalStandardRandomSequence> randomSequence
    = vtkSmartPointer<vtkMinimalStandardRandomSequence>::New();
  randomSequence->SetSeed(1);

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> verts = vtkSmartPointer<vtkCellArray>::New();
  verts->InsertNextCell(4);
  vtkSmartPointer<vtkFloatArray> scalars = vtkSmartPointer<vtkFloatArray>::New();

  if(dataType == VTK_DOUBLE)
  {
    points->SetDataType(VTK_DOUBLE);
    for(unsigned int i = 0; i < 4; ++i)
    {
      randomSequence->Next();
      scalars->InsertNextValue(randomSequence->GetValue());
      double point[3];
      for(unsigned int j = 0; j < 3; ++j)
      {
        randomSequence->Next();
        point[j] = randomSequence->GetValue();
      }
      verts->InsertCellPoint(points->InsertNextPoint(point));
    }
  }
  else
  {
    points->SetDataType(VTK_FLOAT);
    for(unsigned int i = 0; i < 4; ++i)
    {
      randomSequence->Next();
      scalars->InsertNextValue(randomSequence->GetValue());
      float point[3];
      for(unsigned int j = 0; j < 3; ++j)
      {
        randomSequence->Next();
        point[j] = static_cast<float>(randomSequence->GetValue());
      }
      verts->InsertCellPoint(points->InsertNextPoint(point));
    }
  }

  scalars->Squeeze();
  polyData->GetPointData()->SetScalars(scalars);
  points->Squeeze();
  polyData->SetPoints(points);
  verts->Squeeze();
  polyData->SetVerts(verts);
}

int FilterPolyDataConnectivity(int dataType, int outputPointsPrecision)
{
  vtkSmartPointer<vtkPolyData> inputPolyData
    = vtkSmartPointer<vtkPolyData>::New();
  InitializePolyData(inputPolyData, dataType);

  vtkSmartPointer<vtkPolyDataConnectivityFilter> polyDataConnectivityFilter
    = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
  polyDataConnectivityFilter->SetOutputPointsPrecision(outputPointsPrecision);
  polyDataConnectivityFilter->ScalarConnectivityOn();
  polyDataConnectivityFilter->SetScalarRange(0.25, 0.75);
  polyDataConnectivityFilter->SetInputData(inputPolyData);

  polyDataConnectivityFilter->Update();

  vtkSmartPointer<vtkPolyData> outputPolyData = polyDataConnectivityFilter->GetOutput();
  vtkSmartPointer<vtkPoints> points = outputPolyData->GetPoints();

  return points->GetDataType();
}
}

int TestPolyDataConnectivityFilter(int vtkNotUsed(argc), char *vtkNotUsed(argv)[])
{
  int dataType = FilterPolyDataConnectivity(VTK_FLOAT, vtkAlgorithm::DEFAULT_PRECISION);

  if(dataType != VTK_FLOAT)
  {
    return EXIT_FAILURE;
  }

  dataType = FilterPolyDataConnectivity(VTK_DOUBLE, vtkAlgorithm::DEFAULT_PRECISION);

  if(dataType != VTK_DOUBLE)
  {
    return EXIT_FAILURE;
  }

  dataType = FilterPolyDataConnectivity(VTK_FLOAT, vtkAlgorithm::SINGLE_PRECISION);

  if(dataType != VTK_FLOAT)
  {
    return EXIT_FAILURE;
  }

  dataType = FilterPolyDataConnectivity(VTK_DOUBLE, vtkAlgorithm::SINGLE_PRECISION);

  if(dataType != VTK_FLOAT)
  {
    return EXIT_FAILURE;
  }

  dataType = FilterPolyDataConnectivity(VTK_FLOAT, vtkAlgorithm::DOUBLE_PRECISION);

  if(dataType != VTK_DOUBLE)
  {
    return EXIT_FAILURE;
  }

  dataType = FilterPolyDataConnectivity(VTK_DOUBLE, vtkAlgorithm::DOUBLE_PRECISION);

  if(dataType != VTK_DOUBLE)
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

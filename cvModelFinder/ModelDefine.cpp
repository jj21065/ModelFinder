#include "pch.h"
#include "ModelDefine.h"
using namespace Cpp;
ModelDefine::ModelDefine() {
	degreeStart = -1; degreeEnd = 1;
	rotationResolution = 0.5;
	searshScore = 0.7;
	noOfCordinates = 0;
	modelDefined = false;
}
ModelDefine::~ModelDefine()
{
	Release();
}

void ModelDefine::Release()
{
	if (cordinates != nullptr)
	{
		delete[] cordinates;
	}
	if (edgeMagnitude != nullptr)
	{
		delete[] edgeMagnitude;
	}
	if (edgeDerivativeX != nullptr)
	{
		delete[] edgeDerivativeX;
	}
	if (edgeDerivativeY != nullptr)
	{
		delete[] edgeDerivativeY;
	}

	if (cordinatesRotate != nullptr)
	{
		for (int i = 0; i < totalDegree; i++)
		{
			delete[] cordinatesRotate[i];
		}
		delete[] cordinatesRotate;
	}
	if (edgeDerivativeXRotate != nullptr)
	{
		for (int i = 0; i < totalDegree; i++)
		{
			delete[] edgeDerivativeXRotate[i];
		}
		delete[] edgeDerivativeXRotate;
	}
	if (edgeDerivativeYRotate != nullptr)
	{
		for (int i = 0; i < totalDegree; i++)
		{
			delete[] edgeDerivativeYRotate[i];
		}
		delete[] edgeDerivativeYRotate;
	}
}
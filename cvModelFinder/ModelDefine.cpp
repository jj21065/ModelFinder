#include "pch.h"
#include "ModelDefine.h"
using namespace Cpp;
ModelDefine::ModelDefine() {
	degreeStart = -1; degreeEnd = 1;
	rotationResolution = 0.5;
	searshScore = 0.7;
	noOfCordinates = 0;
	modelDefined = false;

	cordinates = nullptr;
	edgeDerivativeX = nullptr;
	edgeDerivativeY = nullptr;
	edgeMagnitude = nullptr;
	cordinatesRotate = nullptr;
	edgeDerivativeXRotate = nullptr;
	edgeDerivativeYRotate = nullptr;

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
		cordinates = nullptr;
	}
	if (edgeMagnitude != nullptr)
	{
		delete[] edgeMagnitude;
		edgeMagnitude = nullptr;
	}
	if (edgeDerivativeX != nullptr)
	{
		delete[] edgeDerivativeX;
		edgeDerivativeX = nullptr;
	}
	if (edgeDerivativeY != nullptr)
	{
		delete[] edgeDerivativeY;
		edgeDerivativeY = nullptr;
	}
	ReleaseMatrix();

}
void ModelDefine::ReleaseMatrix()
{
	if (cordinatesRotate != nullptr)
	{
		for (int i = 0; i < totalDegree; i++)
		{
			delete[] cordinatesRotate[i];
		}
		delete[] cordinatesRotate;
		cordinatesRotate = nullptr;
	}
	if (edgeDerivativeXRotate != nullptr)
	{
		for (int i = 0; i < totalDegree; i++)
		{
			delete[] edgeDerivativeXRotate[i];
		}
		delete[] edgeDerivativeXRotate;
		edgeDerivativeXRotate = nullptr;
	}
	if (edgeDerivativeYRotate != nullptr)
	{
		for (int i = 0; i < totalDegree; i++)
		{
			delete[] edgeDerivativeYRotate[i];
		}
		delete[] edgeDerivativeYRotate;
		edgeDerivativeYRotate = nullptr;
	}
	modelDefined = false;
}
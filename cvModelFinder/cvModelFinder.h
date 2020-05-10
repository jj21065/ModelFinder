#pragma once
#include "NCC_ModelFinder.h"
using namespace System;
using namespace System::Drawing;
namespace CLI {
	public ref class SearchResult
	{
	public:
		double X;
		double Y;
		double Rotation;
		double scale = 1;
		double Score = 0;

	};
	public ref class SearchROI
	{
	public :
		PointF point;
		double width;
		double height;
	};
	public ref class ModelFinder
	{
		// TODO: 請在此新增此類別的方法。
	private :
		Cpp::NCC_ModelFinder* modelFinder = nullptr;
		
	public :
		SearchResult^ searchResult;
		SearchROI^ roi;
		ModelFinder();
		~ModelFinder();
		void SetROI(SearchROI ^_roi)
		{
			roi->point.X = _roi->point.X;
			roi->point.Y = _roi->point.Y;
			roi->width = _roi->width;
			roi->height = _roi->height;
			
			modelFinder->SetROI(roi->point.X, roi->point.Y, roi->width, roi->height);
		}
		void SetROI(double x, double y, double width, double height)
		{
			roi->point.X = x;
			roi->point.Y = y;
			roi->width = width;
			roi->height = height;
			modelFinder->SetROI(roi->point.X, roi->point.Y, roi->width, roi->height);
		}
		void DisableROI();
		void CreateModelFromImage(Bitmap^ _image);
		void ModelFind(Bitmap^ _image);
		void SetModelPara(double r1, double r2, double resolution,double score);
		void SetSobelThreshold(int high, int low);
		bool showCvImage = true;
	};

	
}


#pragma once
#include "opencv2413/include/opencv2/imgproc/imgproc_c.h"
#include "opencv2413/include/opencv2/core/core.hpp"
#include "ModelDefine.h"
#pragma comment(lib, "opencv2413/x86/vc14/lib/opencv_core2413d.lib")
#pragma comment(lib, "opencv2413/x86/vc14/lib/opencv_highgui2413d.lib")
#pragma comment(lib, "opencv2413/x86/vc14/lib/opencv_imgproc2413d.lib")
using namespace cv;
namespace Cpp {
	class NCC_ModelFinder
	{
	public:

		bool showCvImage = true;
		SearchROI roi;
		SearchResult result;
		

	private:
		
		
		
		void CreateDoubleMatrix(double**& matrix, CvSize size);
		void ReleaseDoubleMatrix(double**& matrix, int size);
	public:
		
		ModelDefine m_modelDefine;
		ModelDefine m_modelDefineLow;

		NCC_ModelFinder();
		~NCC_ModelFinder();

		void SetSobelThreshold(int high,int low);

		void SetROI(double, double, double, double);

		void DisableROI();

		void SetModelPara(double, double, double,double);

		void CreatModel(cv::Mat);

		void ModelEraser(int x, int y, int eraserWidth);
		
		void CreateRotateModel();

		void ModelFind(cv::Mat);
		
		void PyramidTestFlow(cv::Mat templateImage, cv::Mat srcImage);

		int CreateGeoMatchModel(const void* templateArr, double maxContrast, double minContrast);

		double FindGeoMatchModelRotateParallel(const void* srcarr, double minScore, double greediness, double& rotation);

		double FindGeoMatchModelRotateParallelSSE(const void* srcarr, double minScore, double greediness, double& rotation);

		void DrawContours(IplImage* pImage, CvPoint COG, CvScalar, int);

		void DrawContours(IplImage* pImage, CvScalar, int);

		void DrawContours(IplImage* source, CvPoint COG, CvScalar color, int lineWidth, int rotation);
	private:

	};

	
	
}
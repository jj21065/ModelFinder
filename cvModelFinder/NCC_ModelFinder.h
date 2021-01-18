
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
		SearchROI roi2;
		SearchResult result;
		int PrymidSize = 3;

	private:
		
		
		
		void CreateDoubleMatrix(double**& matrix, CvSize size);
		void ReleaseDoubleMatrix(double**& matrix, int size);
	public:
		
		ModelDefine m_modelDefine[5];
		//ModelDefine m_modelDefineLow;

		NCC_ModelFinder();
		~NCC_ModelFinder();

		void SetSobelThreshold(int high,int low);

		void SetROI(double, double, double, double);

		void DisableROI();

		void SetModelPara(double, double, double,double);

		void CreatModel(cv::Mat);

		void ModelEraser(int x, int y, int eraserWidth);

		void ModelEraserMask(int x, int y, int eraserWidth);
		
		void CreateRotateModel();

		void CreateRotateModel(Cpp::ModelDefine& model);
		
		void ModelFind(cv::Mat);
	
		int CreateGeoMatchModel(const void* templateArr, Cpp::ModelDefine& model, double maxContrast, double minContrast);

		double FindGeoMatchModelRotateParallelSSE(const void* srcarr, Cpp::ModelDefine& model, SearchROI tmpRoi, int x, int y, int xend, int yend, int rotateStart, int rotateEnd, double minScore, double greediness, double& rotation);

		void DrawContours(IplImage* pImage, CvPoint COG, CvScalar, int);

		void DrawContours(IplImage* pImage, CvScalar, int);

		void DrawContours(IplImage* source, CvPoint COG, CvScalar color, int lineWidth, int rotation);

		void DrawContours(IplImage* source, ModelDefine &model, CvScalar color, int lineWidth);
	private:

	};

	
	
}
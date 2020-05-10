#pragma once
#include "C:/opencv/build/include/opencv2/imgproc/imgproc_c.h"
#include "C:/opencv/build/include/opencv2/core/core.hpp"

#pragma comment(lib, "opencv2413/x64/vc14/lib/opencv_core2413d.lib")
#pragma comment(lib, "opencv2413/x64/vc14/lib/opencv_highgui2413d.lib")
#pragma comment(lib, "opencv2413/x64/vc14/lib/opencv_imgproc2413d.lib")
using namespace cv;
namespace Cpp {
	class ModelDefine
	{
	public:
		double searshScore;
		int				noOfCordinates;		//Number of elements in coordinate array
		CvPoint* cordinates;		//Coordinates array to store model points	
		int				modelHeight;		//Template height
		int				modelWidth;			//Template width
		double* edgeMagnitude = nullptr;		//gradient magnitude
		double* edgeDerivativeX = nullptr;	//gradient in X direction
		double* edgeDerivativeY = nullptr;	//radient in Y direction	

		CvPoint** cordinatesRotate = nullptr;		//Coordinates array to store model points	
		double** edgeDerivativeXRotate = nullptr;	//gradient in X direction
		double** edgeDerivativeYRotate = nullptr;	//radient in Y direction	

		int totalDegree = 0;
		CvPoint			centerOfGravity;	//Center of gravity of template 

		bool			modelDefined;
		double degreeStart, degreeEnd;
		double rotationResolution;

		int SobelLow = 100;
		int SobelHigh = 100;
		ModelDefine();
		~ModelDefine();

		void Release();
		
	
	};

	class SearchResult
	{
	public:
		CvPoint location;
		double rotation;
		double score;
		bool isFind;

	};

	class SearchROI
	{
	public :
		CvRect rect;
		bool isEnable = false;
	};
}
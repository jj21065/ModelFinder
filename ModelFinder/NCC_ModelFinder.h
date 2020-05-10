#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/core/core.hpp"
using namespace cv;
#pragma comment(lib, "opencv2413/x64/vc14/lib/opencv_core2413.lib")
#pragma comment(lib, "opencv2413/x64/vc14/lib/opencv_highgui2413.lib")
#pragma comment(lib, "opencv2413/x64/vc14/lib/opencv_imgproc2413.lib")

#pragma once
class NCC_ModelFinder
{
public :


private:
	int				noOfCordinates;		//Number of elements in coordinate array
	CvPoint* cordinates;		//Coordinates array to store model points	
	int				modelHeight;		//Template height
	int				modelWidth;			//Template width
	double* edgeMagnitude;		//gradient magnitude
	double* edgeDerivativeX;	//gradient in X direction
	double* edgeDerivativeY;	//radient in Y direction	

	CvPoint** cordinatesRotate;		//Coordinates array to store model points	
	double** edgeDerivativeXRotate;	//gradient in X direction
	double** edgeDerivativeYRotate;	//radient in Y direction	

	CvPoint** cordinatesRotate2nd;		//Coordinates array to store model points	
	double** edgeDerivativeXRotate2nd;	//gradient in X direction
	double** edgeDerivativeYRotate2nd;	//radient in Y direction	

	CvPoint** cordinatesRotate3rd;		//Coordinates array to store model points	
	double** edgeDerivativeXRotate3rd;	//gradient in X direction
	double** edgeDerivativeYRotate3rd;	//radient in Y direction	

	int totalDegree = 0;
	CvPoint			centerOfGravity;	//Center of gravity of template 

	bool			modelDefined;

	void CreateDoubleMatrix(double**& matrix, CvSize size);
	void ReleaseDoubleMatrix(double**& matrix, int size);
public:
	NCC_ModelFinder();
	~NCC_ModelFinder();
	NCC_ModelFinder(const void* templateArr);
	

	int CreateGeoMatchModel(const void* templateArr, double, double);
	int CreateGeoMatchModel(const void* templateArr, double maxContrast, double minContrast, double r1, double r2);
	
	int CreateGeoMatchModelPyramid(const void* templateArr, double maxContrast, double minContrast, double r1, double r2, CvPoint** point, double** edgeX, double** edgeY);
	int CreateModelsPyramid(const void* templateArr, double maxContrast, double minContrast, double r1, double r2);
	
	double FindGeoMatchModel(const void* srcarr, double minScore, double greediness, CvPoint* resultPoint);
	double FindGeoMatchModelRotate(const void* srcarr, double minScore, double greediness, CvPoint* resultPoint,double& rotation);
	double FindGeoMatchModelRotateParallel(const void* srcarr, double minScore, double greediness, CvPoint* resultPoint, double& rotation);
	
	double FindGeoMatchModelRotatePyramid(const void* srcarr, double minScore, double greediness, int searchx, int searchy,CvPoint* resultPoint, double& rotation);
	double FindGeoMatchModelRotatePyramid(const void* srcarr, double minScore, double greediness, int searchx, int searchy, CvPoint** Tpoint, double** TedgeX, double** TedgeY,CvPoint* resultPoint, double& rotation);

	void DrawContours(IplImage* pImage, CvPoint COG, CvScalar, int);
	void DrawContours(IplImage* pImage, CvScalar, int);
	void DrawContours(IplImage* source, CvPoint COG, CvScalar color, int lineWidth, int rotation);
private :

};
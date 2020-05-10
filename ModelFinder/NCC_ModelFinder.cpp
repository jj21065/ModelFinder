
#include "NCC_ModelFinder.h"
#include "C:/opencv/build/include/opencv2/highgui/highgui_c.h"
#include <windows.h>
#include <ppl.h>
#include <iostream>

using namespace concurrency;
using namespace std;

NCC_ModelFinder::NCC_ModelFinder(void)
{
	noOfCordinates = 0;  // Initilize  no of cppodinates in model points
	modelDefined = false;
}



int NCC_ModelFinder::CreateGeoMatchModel(const void* templateArr, double maxContrast, double minContrast)
{

	CvMat* gx = 0;		//Matrix to store X derivative
	CvMat* gy = 0;		//Matrix to store Y derivative
	CvMat* nmsEdges = 0;		//Matrix to store temp restult
	CvSize Ssize;

	// Convert IplImage to Matrix for integer operations
	CvMat srcstub, * src = (CvMat*)templateArr;
	src = cvGetMat(src, &srcstub);
	if (CV_MAT_TYPE(src->type) != CV_8UC1)
	{
		return 0;
	}

	// set width and height
	Ssize.width = src->width;
	Ssize.height = src->height;
	modelHeight = src->height;		//Save Template height
	modelWidth = src->width;			//Save Template width

	noOfCordinates = 0;											//initialize	
	cordinates = new CvPoint[modelWidth * modelHeight];		//Allocate memory for coorinates of selected points in template image

	edgeMagnitude = new double[modelWidth * modelHeight];		//Allocate memory for edge magnitude for selected points
	edgeDerivativeX = new double[modelWidth * modelHeight];			//Allocate memory for edge X derivative for selected points
	edgeDerivativeY = new double[modelWidth * modelHeight];			////Allocate memory for edge Y derivative for selected points


	// Calculate gradient of Template
	gx = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1);		//create Matrix to store X derivative
	gy = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1);		//create Matrix to store Y derivative
	cvSobel(src, gx, 1, 0, 3);		//gradient in X direction			
	cvSobel(src, gy, 0, 1, 3);	//gradient in Y direction

	nmsEdges = cvCreateMat(Ssize.height, Ssize.width, CV_32F);		//create Matrix to store Final nmsEdges
	const short* _sdx;
	const short* _sdy;
	double fdx, fdy;
	double MagG, DirG;
	double MaxGradient = -99999.99;
	double direction;
	int* orients = new int[Ssize.height * Ssize.width];
	int count = 0, i, j; // count variable;

	double** magMat;
	CreateDoubleMatrix(magMat, Ssize);

	for (i = 1; i < Ssize.height - 1; i++)
	{
		for (j = 1; j < Ssize.width - 1; j++)
		{
			_sdx = (short*)(gx->data.ptr + gx->step * i);
			_sdy = (short*)(gy->data.ptr + gy->step * i);
			fdx = _sdx[j]; fdy = _sdy[j];        // read x, y derivatives

			MagG = sqrt((float)(fdx * fdx) + (float)(fdy * fdy)); //Magnitude = Sqrt(gx^2 +gy^2)
			direction = cvFastArctan((float)fdy, (float)fdx);	 //Direction = invtan (Gy / Gx)
			magMat[i][j] = MagG;

			if (MagG > MaxGradient)
				MaxGradient = MagG; // get maximum gradient value for normalizing.


				// get closest angle from 0, 45, 90, 135 set
			if ((direction > 0 && direction < 22.5) || (direction > 157.5 && direction < 202.5) || (direction > 337.5 && direction < 360))
				direction = 0;
			else if ((direction > 22.5 && direction < 67.5) || (direction > 202.5 && direction < 247.5))
				direction = 45;
			else if ((direction > 67.5 && direction < 112.5) || (direction > 247.5 && direction < 292.5))
				direction = 90;
			else if ((direction > 112.5 && direction < 157.5) || (direction > 292.5 && direction < 337.5))
				direction = 135;
			else
				direction = 0;

			orients[count] = (int)direction;
			count++;
		}
	}

	count = 0; // init count
	// non maximum suppression
	double leftPixel, rightPixel;

	for (i = 1; i < Ssize.height - 1; i++)
	{
		for (j = 1; j < Ssize.width - 1; j++)
		{
			switch (orients[count])
			{
			case 0:
				leftPixel = magMat[i][j - 1];
				rightPixel = magMat[i][j + 1];
				break;
			case 45:
				leftPixel = magMat[i - 1][j + 1];
				rightPixel = magMat[i + 1][j - 1];
				break;
			case 90:
				leftPixel = magMat[i - 1][j];
				rightPixel = magMat[i + 1][j];
				break;
			case 135:
				leftPixel = magMat[i - 1][j - 1];
				rightPixel = magMat[i + 1][j + 1];
				break;
			}
			// compare current pixels value with adjacent pixels
			if ((magMat[i][j] < leftPixel) || (magMat[i][j] < rightPixel))
				(nmsEdges->data.ptr + nmsEdges->step * i)[j] = 0;
			else
				(nmsEdges->data.ptr + nmsEdges->step * i)[j] = (uchar)(magMat[i][j] / MaxGradient * 255);

			count++;
		}
	}


	int RSum = 0, CSum = 0;
	int curX, curY;
	int flag = 1;

	//Hysterisis threshold
	for (i = 1; i < Ssize.height - 1; i++)
	{
		for (j = 1; j < Ssize.width; j++)
		{
			_sdx = (short*)(gx->data.ptr + gx->step * i);
			_sdy = (short*)(gy->data.ptr + gy->step * i);
			fdx = _sdx[j]; fdy = _sdy[j];

			MagG = sqrt(fdx * fdx + fdy * fdy); //Magnitude = Sqrt(gx^2 +gy^2)
			DirG = cvFastArctan((float)fdy, (float)fdx);	 //Direction = tan(y/x)

			////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]= MagG;
			flag = 1;
			// 用Threshold 篩選上下界線
			if (((double)((nmsEdges->data.ptr + nmsEdges->step * i))[j]) < maxContrast)
			{
				if (((double)((nmsEdges->data.ptr + nmsEdges->step * i))[j]) < minContrast)
				{

					(nmsEdges->data.ptr + nmsEdges->step * i)[j] = 0;
					flag = 0; // remove from edge
					////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]=0;
				}
				else
				{   // if any of 8 neighboring pixel is not greater than max contraxt remove from edge
					if ((((double)((nmsEdges->data.ptr + nmsEdges->step * (i - 1)))[j - 1]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * (i - 1)))[j]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * (i - 1)))[j + 1]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * i))[j - 1]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * i))[j + 1]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * (i + 1)))[j - 1]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * (i + 1)))[j]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * (i + 1)))[j + 1]) < maxContrast))
					{
						(nmsEdges->data.ptr + nmsEdges->step * i)[j] = 0;
						flag = 0;
						////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]=0;
					}
				}

			}

			// save selected edge information
			curX = i;	curY = j;
			if (flag != 0)
			{
				if (fdx != 0 || fdy != 0)
				{
					RSum = RSum + curX;	CSum = CSum + curY; // Row sum and column sum for center of gravity

					cordinates[noOfCordinates].x = curX;
					cordinates[noOfCordinates].y = curY;
					edgeDerivativeX[noOfCordinates] = fdx;
					edgeDerivativeY[noOfCordinates] = fdy;

					//handle divide by zero
					if (MagG != 0)
						edgeMagnitude[noOfCordinates] = 1 / MagG;  // gradient magnitude 
					else
						edgeMagnitude[noOfCordinates] = 0;

					noOfCordinates++;
				}
			}
		}
	}

	centerOfGravity.x = RSum / noOfCordinates; // center of gravity
	centerOfGravity.y = CSum / noOfCordinates;	// center of gravity

	// change coordinates to reflect center of gravity
	for (int m = 0; m < noOfCordinates; m++)
	{
		int temp;

		temp = cordinates[m].x;
		cordinates[m].x = temp - centerOfGravity.x;
		temp = cordinates[m].y;
		cordinates[m].y = temp - centerOfGravity.y;
	}

	////cvSaveImage("Edges.bmp",imgGDir);

	// free alocated memories
	delete[] orients;
	////cvReleaseImage(&imgGDir);
	cvReleaseMat(&gx);
	cvReleaseMat(&gy);
	cvReleaseMat(&nmsEdges);

	ReleaseDoubleMatrix(magMat, Ssize.height);

	modelDefined = true;
	return 1;
}

int NCC_ModelFinder::CreateGeoMatchModel(const void* templateArr, double maxContrast, double minContrast, double r1, double r2)
{
	double resolution = 0.5;

	CvMat* gx = 0;		//Matrix to store X derivative
	CvMat* gy = 0;		//Matrix to store Y derivative
	CvMat* nmsEdges = 0;		//Matrix to store temp restult
	CvSize Ssize;

	// Convert IplImage to Matrix for integer operations
	CvMat srcstub, * src = (CvMat*)templateArr;
	src = cvGetMat(src, &srcstub);
	if (CV_MAT_TYPE(src->type) != CV_8UC1)
	{
		return 0;
	}

	// set width and height
	Ssize.width = src->width;
	Ssize.height = src->height;
	modelHeight = src->height;		//Save Template height
	modelWidth = src->width;			//Save Template width

	noOfCordinates = 0;											//initialize	
	cordinates = new CvPoint[modelWidth * modelHeight];		//Allocate memory for coorinates of selected points in template image

	edgeMagnitude = new double[modelWidth * modelHeight];		//Allocate memory for edge magnitude for selected points
	edgeDerivativeX = new double[modelWidth * modelHeight];			//Allocate memory for edge X derivative for selected points
	edgeDerivativeY = new double[modelWidth * modelHeight];			////Allocate memory for edge Y derivative for selected points


	// Calculate gradient of Template
	gx = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1);		//create Matrix to store X derivative
	gy = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1);		//create Matrix to store Y derivative
	cvSobel(src, gx, 1, 0, 3);		//gradient in X direction			
	cvSobel(src, gy, 0, 1, 3);	//gradient in Y direction

	nmsEdges = cvCreateMat(Ssize.height, Ssize.width, CV_32F);		//create Matrix to store Final nmsEdges
	const short* _sdx;
	const short* _sdy;
	double fdx, fdy;
	double MagG, DirG;
	double MaxGradient = -99999.99;
	double direction;
	int* orients = new int[Ssize.height * Ssize.width];
	int count = 0, i, j; // count variable;

	double** magMat;
	CreateDoubleMatrix(magMat, Ssize);

	for (i = 1; i < Ssize.height - 1; i++)
	{
		for (j = 1; j < Ssize.width - 1; j++)
		{
			_sdx = (short*)(gx->data.ptr + gx->step * i);
			_sdy = (short*)(gy->data.ptr + gy->step * i);
			fdx = _sdx[j]; fdy = _sdy[j];        // read x, y derivatives

			MagG = sqrt((float)(fdx * fdx) + (float)(fdy * fdy)); //Magnitude = Sqrt(gx^2 +gy^2)
			direction = cvFastArctan((float)fdy, (float)fdx);	 //Direction = invtan (Gy / Gx)
			magMat[i][j] = MagG;

			if (MagG > MaxGradient)
				MaxGradient = MagG; // get maximum gradient value for normalizing.


				// get closest angle from 0, 45, 90, 135 set
			if ((direction > 0 && direction < 22.5) || (direction > 157.5 && direction < 202.5) || (direction > 337.5 && direction < 360))
				direction = 0;
			else if ((direction > 22.5 && direction < 67.5) || (direction > 202.5 && direction < 247.5))
				direction = 45;
			else if ((direction > 67.5 && direction < 112.5) || (direction > 247.5 && direction < 292.5))
				direction = 90;
			else if ((direction > 112.5 && direction < 157.5) || (direction > 292.5 && direction < 337.5))
				direction = 135;
			else
				direction = 0;

			orients[count] = (int)direction;
			count++;
		}
	}

	count = 0; // init count
	// non maximum suppression
	double leftPixel, rightPixel;

	for (i = 1; i < Ssize.height - 1; i++)
	{
		for (j = 1; j < Ssize.width - 1; j++)
		{
			switch (orients[count])
			{
			case 0:
				leftPixel = magMat[i][j - 1];
				rightPixel = magMat[i][j + 1];
				break;
			case 45:
				leftPixel = magMat[i - 1][j + 1];
				rightPixel = magMat[i + 1][j - 1];
				break;
			case 90:
				leftPixel = magMat[i - 1][j];
				rightPixel = magMat[i + 1][j];
				break;
			case 135:
				leftPixel = magMat[i - 1][j - 1];
				rightPixel = magMat[i + 1][j + 1];
				break;
			}
			// compare current pixels value with adjacent pixels
			if ((magMat[i][j] < leftPixel) || (magMat[i][j] < rightPixel))
				(nmsEdges->data.ptr + nmsEdges->step * i)[j] = 0;
			else
				(nmsEdges->data.ptr + nmsEdges->step * i)[j] = (uchar)(magMat[i][j] / MaxGradient * 255);

			count++;
		}
	}


	int RSum = 0, CSum = 0;
	int curX, curY;
	int flag = 1;

	//Hysterisis threshold
	for (i = 1; i < Ssize.height - 1; i++)
	{
		for (j = 1; j < Ssize.width; j++)
		{
			_sdx = (short*)(gx->data.ptr + gx->step * i);
			_sdy = (short*)(gy->data.ptr + gy->step * i);
			fdx = _sdx[j]; fdy = _sdy[j];

			MagG = sqrt(fdx * fdx + fdy * fdy); //Magnitude = Sqrt(gx^2 +gy^2)
			DirG = cvFastArctan((float)fdy, (float)fdx);	 //Direction = tan(y/x)

			////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]= MagG;
			flag = 1;
			// 用Threshold 篩選上下界線
			if (((double)((nmsEdges->data.ptr + nmsEdges->step * i))[j]) < maxContrast)
			{
				if (((double)((nmsEdges->data.ptr + nmsEdges->step * i))[j]) < minContrast)
				{

					(nmsEdges->data.ptr + nmsEdges->step * i)[j] = 0;
					flag = 0; // remove from edge
					////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]=0;
				}
				else
				{   // if any of 8 neighboring pixel is not greater than max contraxt remove from edge
					if ((((double)((nmsEdges->data.ptr + nmsEdges->step * (i - 1)))[j - 1]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * (i - 1)))[j]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * (i - 1)))[j + 1]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * i))[j - 1]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * i))[j + 1]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * (i + 1)))[j - 1]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * (i + 1)))[j]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * (i + 1)))[j + 1]) < maxContrast))
					{
						(nmsEdges->data.ptr + nmsEdges->step * i)[j] = 0;
						flag = 0;
						////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]=0;
					}
				}

			}

			// save selected edge information
			curX = i;	curY = j;
			if (flag != 0)
			{
				if (fdx != 0 || fdy != 0)
				{
					RSum = RSum + curX;	CSum = CSum + curY; // Row sum and column sum for center of gravity

					cordinates[noOfCordinates].x = curX;
					cordinates[noOfCordinates].y = curY;
					edgeDerivativeX[noOfCordinates] = fdx;
					edgeDerivativeY[noOfCordinates] = fdy;

					//handle divide by zero
					if (MagG != 0)
						edgeMagnitude[noOfCordinates] = 1 / MagG;  // gradient magnitude 
					else
						edgeMagnitude[noOfCordinates] = 0;

					noOfCordinates++;
				}
			}
		}
	}

	centerOfGravity.x = RSum / noOfCordinates; // center of gravity
	centerOfGravity.y = CSum / noOfCordinates;	// center of gravity

	// change coordinates to reflect center of gravity
	for (int m = 0; m < noOfCordinates; m++)
	{
		int temp;

		temp = cordinates[m].x;
		cordinates[m].x = temp - centerOfGravity.x;
		temp = cordinates[m].y;
		cordinates[m].y = temp - centerOfGravity.y;
	}

	totalDegree = (r2 - r1) / resolution + 1;
	count = 0;
	cordinatesRotate = new CvPoint * [totalDegree];		//Coordinates array to store model points	
	edgeDerivativeXRotate = new double* [totalDegree];	//gradient in X direction
	edgeDerivativeYRotate = new double* [totalDegree]; 	//radient in Y direction	
	for (int i = 0; i < totalDegree; i++)
	{
		cordinatesRotate[i] = new CvPoint[noOfCordinates];
		edgeDerivativeXRotate[i] = new double[noOfCordinates];
		edgeDerivativeYRotate[i] = new double[noOfCordinates];
	}
	for (float degree = r1; degree < r2; degree += resolution)
	{
		for (int i = 0; i < noOfCordinates; i++)
		{
			float thida = degree * CV_PI / 180.0;
			cordinatesRotate[count][i].x = cordinates[i].x * cos(thida) - cordinates[i].y * sin(thida);
			cordinatesRotate[count][i].y = cordinates[i].x * sin(thida) + cordinates[i].y * cos(thida);
			edgeDerivativeXRotate[count][i] = edgeDerivativeX[i] * cos(thida) - edgeDerivativeY[i] * sin(thida);
			edgeDerivativeYRotate[count][i] = edgeDerivativeX[i] * sin(thida) + edgeDerivativeY[i] * cos(thida);

		}
		count++;
	}
	////cvSaveImage("Edges.bmp",imgGDir);

	// free alocated memories
	delete[] orients;
	////cvReleaseImage(&imgGDir);
	cvReleaseMat(&gx);
	cvReleaseMat(&gy);
	cvReleaseMat(&nmsEdges);

	ReleaseDoubleMatrix(magMat, Ssize.height);

	modelDefined = true;
	return 1;
}

int NCC_ModelFinder::CreateGeoMatchModelPyramid(const void* templateArr, double maxContrast, double minContrast, double r1, double r2,CvPoint** point,double** edgeX,double **edgeY)
{
	double resolution = 0.5;

	CvMat* gx = 0;		//Matrix to store X derivative
	CvMat* gy = 0;		//Matrix to store Y derivative
	CvMat* nmsEdges = 0;		//Matrix to store temp restult
	CvSize Ssize;

	// Convert IplImage to Matrix for integer operations
	CvMat srcstub, * src = (CvMat*)templateArr;
	src = cvGetMat(src, &srcstub);
	if (CV_MAT_TYPE(src->type) != CV_8UC1)
	{
		return 0;
	}

	// set width and height
	Ssize.width = src->width;
	Ssize.height = src->height;
	modelHeight = src->height;		//Save Template height
	modelWidth = src->width;			//Save Template width

	noOfCordinates = 0;											//initialize	
	cordinates = new CvPoint[modelWidth * modelHeight];		//Allocate memory for coorinates of selected points in template image

	edgeMagnitude = new double[modelWidth * modelHeight];		//Allocate memory for edge magnitude for selected points
	edgeDerivativeX = new double[modelWidth * modelHeight];			//Allocate memory for edge X derivative for selected points
	edgeDerivativeY = new double[modelWidth * modelHeight];			////Allocate memory for edge Y derivative for selected points


	// Calculate gradient of Template
	gx = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1);		//create Matrix to store X derivative
	gy = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1);		//create Matrix to store Y derivative
	cvSobel(src, gx, 1, 0, 3);		//gradient in X direction			
	cvSobel(src, gy, 0, 1, 3);	//gradient in Y direction

	nmsEdges = cvCreateMat(Ssize.height, Ssize.width, CV_32F);		//create Matrix to store Final nmsEdges
	const short* _sdx;
	const short* _sdy;
	double fdx, fdy;
	double MagG, DirG;
	double MaxGradient = -99999.99;
	double direction;
	int* orients = new int[Ssize.height * Ssize.width];
	int count = 0, i, j; // count variable;

	double** magMat;
	CreateDoubleMatrix(magMat, Ssize);

	for (i = 1; i < Ssize.height - 1; i++)
	{
		for (j = 1; j < Ssize.width - 1; j++)
		{
			_sdx = (short*)(gx->data.ptr + gx->step * i);
			_sdy = (short*)(gy->data.ptr + gy->step * i);
			fdx = _sdx[j]; fdy = _sdy[j];        // read x, y derivatives

			MagG = sqrt((float)(fdx * fdx) + (float)(fdy * fdy)); //Magnitude = Sqrt(gx^2 +gy^2)
			direction = cvFastArctan((float)fdy, (float)fdx);	 //Direction = invtan (Gy / Gx)
			magMat[i][j] = MagG;

			if (MagG > MaxGradient)
				MaxGradient = MagG; // get maximum gradient value for normalizing.


				// get closest angle from 0, 45, 90, 135 set
			if ((direction > 0 && direction < 22.5) || (direction > 157.5 && direction < 202.5) || (direction > 337.5 && direction < 360))
				direction = 0;
			else if ((direction > 22.5 && direction < 67.5) || (direction > 202.5 && direction < 247.5))
				direction = 45;
			else if ((direction > 67.5 && direction < 112.5) || (direction > 247.5 && direction < 292.5))
				direction = 90;
			else if ((direction > 112.5 && direction < 157.5) || (direction > 292.5 && direction < 337.5))
				direction = 135;
			else
				direction = 0;

			orients[count] = (int)direction;
			count++;
		}
	}

	count = 0; // init count
	// non maximum suppression
	double leftPixel, rightPixel;

	for (i = 1; i < Ssize.height - 1; i++)
	{
		for (j = 1; j < Ssize.width - 1; j++)
		{
			switch (orients[count])
			{
			case 0:
				leftPixel = magMat[i][j - 1];
				rightPixel = magMat[i][j + 1];
				break;
			case 45:
				leftPixel = magMat[i - 1][j + 1];
				rightPixel = magMat[i + 1][j - 1];
				break;
			case 90:
				leftPixel = magMat[i - 1][j];
				rightPixel = magMat[i + 1][j];
				break;
			case 135:
				leftPixel = magMat[i - 1][j - 1];
				rightPixel = magMat[i + 1][j + 1];
				break;
			}
			// compare current pixels value with adjacent pixels
			if ((magMat[i][j] < leftPixel) || (magMat[i][j] < rightPixel))
				(nmsEdges->data.ptr + nmsEdges->step * i)[j] = 0;
			else
				(nmsEdges->data.ptr + nmsEdges->step * i)[j] = (uchar)(magMat[i][j] / MaxGradient * 255);

			count++;
		}
	}


	int RSum = 0, CSum = 0;
	int curX, curY;
	int flag = 1;

	//Hysterisis threshold
	for (i = 1; i < Ssize.height - 1; i++)
	{
		for (j = 1; j < Ssize.width; j++)
		{
			_sdx = (short*)(gx->data.ptr + gx->step * i);
			_sdy = (short*)(gy->data.ptr + gy->step * i);
			fdx = _sdx[j]; fdy = _sdy[j];

			MagG = sqrt(fdx * fdx + fdy * fdy); //Magnitude = Sqrt(gx^2 +gy^2)
			DirG = cvFastArctan((float)fdy, (float)fdx);	 //Direction = tan(y/x)

			////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]= MagG;
			flag = 1;
			// 用Threshold 篩選上下界線
			if (((double)((nmsEdges->data.ptr + nmsEdges->step * i))[j]) < maxContrast)
			{
				if (((double)((nmsEdges->data.ptr + nmsEdges->step * i))[j]) < minContrast)
				{

					(nmsEdges->data.ptr + nmsEdges->step * i)[j] = 0;
					flag = 0; // remove from edge
					////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]=0;
				}
				else
				{   // if any of 8 neighboring pixel is not greater than max contraxt remove from edge
					if ((((double)((nmsEdges->data.ptr + nmsEdges->step * (i - 1)))[j - 1]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * (i - 1)))[j]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * (i - 1)))[j + 1]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * i))[j - 1]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * i))[j + 1]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * (i + 1)))[j - 1]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * (i + 1)))[j]) < maxContrast) &&
						(((double)((nmsEdges->data.ptr + nmsEdges->step * (i + 1)))[j + 1]) < maxContrast))
					{
						(nmsEdges->data.ptr + nmsEdges->step * i)[j] = 0;
						flag = 0;
						////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]=0;
					}
				}

			}

			// save selected edge information
			curX = i;	curY = j;
			if (flag != 0)
			{
				if (fdx != 0 || fdy != 0)
				{
					RSum = RSum + curX;	CSum = CSum + curY; // Row sum and column sum for center of gravity

					cordinates[noOfCordinates].x = curX;
					cordinates[noOfCordinates].y = curY;
					edgeDerivativeX[noOfCordinates] = fdx;
					edgeDerivativeY[noOfCordinates] = fdy;

					//handle divide by zero
					if (MagG != 0)
						edgeMagnitude[noOfCordinates] = 1 / MagG;  // gradient magnitude 
					else
						edgeMagnitude[noOfCordinates] = 0;

					noOfCordinates++;
				}
			}
		}
	}

	centerOfGravity.x = RSum / noOfCordinates; // center of gravity
	centerOfGravity.y = CSum / noOfCordinates;	// center of gravity

	// change coordinates to reflect center of gravity
	for (int m = 0; m < noOfCordinates; m++)
	{
		int temp;

		temp = cordinates[m].x;
		cordinates[m].x = temp - centerOfGravity.x;
		temp = cordinates[m].y;
		cordinates[m].y = temp - centerOfGravity.y;
	}

	totalDegree = (r2 - r1) / resolution + 1;
	count = 0;
	point = new CvPoint * [totalDegree];		//Coordinates array to store model points	
	edgeX = new double* [totalDegree];	//gradient in X direction
	edgeY = new double* [totalDegree]; 	//radient in Y direction	
	for (int i = 0; i < totalDegree; i++)
	{
		point[i] = new CvPoint[noOfCordinates];
		edgeX[i] = new double[noOfCordinates];
		edgeY[i] = new double[noOfCordinates];
	}
	for (float degree = r1; degree < r2; degree += resolution)
	{
		for (int i = 0; i < noOfCordinates; i++)
		{
			float thida = degree * CV_PI / 180.0;
			point[count][i].x = cordinates[i].x * cos(thida) - cordinates[i].y * sin(thida);
			point[count][i].y = cordinates[i].x * sin(thida) + cordinates[i].y * cos(thida);
			edgeX[count][i] = edgeDerivativeX[i] * cos(thida) - edgeDerivativeY[i] * sin(thida);
			edgeY[count][i] = edgeDerivativeX[i] * sin(thida) + edgeDerivativeY[i] * cos(thida);

		}
		count++;
	}
	////cvSaveImage("Edges.bmp",imgGDir);

	// free alocated memories
	delete[] orients;
	////cvReleaseImage(&imgGDir);
	cvReleaseMat(&gx);
	cvReleaseMat(&gy);
	cvReleaseMat(&nmsEdges);

	ReleaseDoubleMatrix(magMat, Ssize.height);

	delete []edgeMagnitude ;		//Allocate memory for edge magnitude for selected points
	delete[]edgeDerivativeX;
	delete[]edgeDerivativeY;

	modelDefined = true;
	return 1;
}

int NCC_ModelFinder::CreateModelsPyramid(const void* templateArr, double maxContrast, double minContrast, double r1, double r2)
{
	CvMat srcstub, * src = (CvMat*)templateArr;
	src = cvGetMat(src, &srcstub);
	if (CV_MAT_TYPE(src->type) != CV_8UC1)
	{
		return 0;
	}

	CreateGeoMatchModelPyramid(src, maxContrast, minContrast, r1, r2, cordinatesRotate,edgeDerivativeXRotate,edgeDerivativeYRotate);
	
	CvMat *dst1 = cvCreateMat(src->width/2,src->height/2,CV_8UC1);
	cvResize(src, dst1);
	CreateGeoMatchModelPyramid(src, maxContrast, minContrast, r1, r2, cordinatesRotate2nd, edgeDerivativeXRotate2nd, edgeDerivativeYRotate2nd);
	
	CvMat* dst2 = cvCreateMat(src->width / 2, src->height / 2, CV_8UC1);
	cvResize(src, dst2);
	CreateGeoMatchModelPyramid(src, maxContrast, minContrast, r1, r2, cordinatesRotate3rd, edgeDerivativeXRotate3rd, edgeDerivativeYRotate3rd);
	cvReleaseMat(&dst1);
	cvReleaseMat(&dst2);
	return 1;
}

double NCC_ModelFinder::FindGeoMatchModel(const void* srcarr, double minScore, double greediness, CvPoint* resultPoint)
{
	CvMat* Sdx = 0, * Sdy = 0;

	double resultScore = 0;
	double partialSum = 0;
	double sumOfCoords = 0;
	double partialScore;
	const short* _Sdx;
	const short* _Sdy;
	int i, j, m;			// count variables
	double iTx, iTy, iSx, iSy;
	double gradMag;
	int curX, curY;

	double** matGradMag;  //Gradient magnitude matrix

	CvMat srcstub, * src = (CvMat*)srcarr;
	src = cvGetMat(src, &srcstub);
	if (CV_MAT_TYPE(src->type) != CV_8UC1 || !modelDefined)
	{
		return 0;
	}

	// source image size
	CvSize Ssize;
	Ssize.width = src->width;
	Ssize.height = src->height;

	CreateDoubleMatrix(matGradMag, Ssize); // create image to save gradient magnitude  values

	Sdx = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1); // X derivatives
	Sdy = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1); // y derivatives

	cvSobel(src, Sdx, 1, 0, 3);  // find X derivatives
	cvSobel(src, Sdy, 0, 1, 3); // find Y derivatives

	// stoping criterias to search for model
	double normMinScore = minScore / noOfCordinates; // precompute minumum score 
	double normGreediness = ((1 - greediness * minScore) / (1 - greediness)) / noOfCordinates; // precompute greedniness 

	for (i = 0; i < Ssize.height; i++)
	{
		_Sdx = (short*)(Sdx->data.ptr + Sdx->step * (i));
		_Sdy = (short*)(Sdy->data.ptr + Sdy->step * (i));

		for (j = 0; j < Ssize.width; j++)
		{
			iSx = _Sdx[j];  // X derivative of Source image
			iSy = _Sdy[j];  // Y derivative of Source image

			gradMag = sqrt((iSx * iSx) + (iSy * iSy)); //Magnitude = Sqrt(dx^2 +dy^2)

			if (gradMag != 0) // hande divide by zero
				matGradMag[i][j] = 1 / gradMag;   // 1/Sqrt(dx^2 +dy^2)
			else
				matGradMag[i][j] = 0;

		}
	}
	for (i = 0; i < Ssize.height; i++)
	{
		for (j = 0; j < Ssize.width; j++)
		{

			partialSum = 0; // initilize partialSum measure
			for (m = 0; m < noOfCordinates; m++)
			{

				curX = i + cordinates[m].x;	// template X coordinate
				curY = j + cordinates[m].y; // template Y coordinate
				iTx = edgeDerivativeX[m];	// template X derivative
				iTy = edgeDerivativeY[m];    // template Y derivative


				if (curX<0 || curY<0 || curX>Ssize.height - 1 || curY>Ssize.width - 1)
					continue;

				_Sdx = (short*)(Sdx->data.ptr + Sdx->step * (curX));
				_Sdy = (short*)(Sdy->data.ptr + Sdy->step * (curX));

				iSx = _Sdx[curY]; // get curresponding  X derivative from source image
				iSy = _Sdy[curY];// get curresponding  Y derivative from source image

				if ((iSx != 0 || iSy != 0) && (iTx != 0 || iTy != 0))
				{
					//partial Sum  = Sum of(((Source X derivative* Template X drivative) + Source Y derivative * Template Y derivative)) / Edge magnitude of(Template)* edge magnitude of(Source))
					partialSum = partialSum + ((iSx * iTx) + (iSy * iTy)) * (edgeMagnitude[m] * matGradMag[curX][curY]);

				}

				sumOfCoords = m + 1;
				partialScore = partialSum / sumOfCoords;
				// check termination criteria
				// if partial score score is less than the score than needed to make the required score at that position
				// break serching at that coordinate.
				if (partialScore < (MIN((minScore - 1) + normGreediness * sumOfCoords, normMinScore * sumOfCoords)))
					break;

			}
			if (partialScore > resultScore)
			{
				resultScore = partialScore; //  Match score
				resultPoint->x = i;			// result coordinate X		
				resultPoint->y = j;			// result coordinate Y
			}

		}
	}

	// free used resources and return score
	ReleaseDoubleMatrix(matGradMag, Ssize.height);
	cvReleaseMat(&Sdx);
	cvReleaseMat(&Sdy);

	return resultScore;
}

double NCC_ModelFinder::FindGeoMatchModelRotate(const void* srcarr, double minScore, double greediness, CvPoint* resultPoint, double& rotation)
{
	CvMat* Sdx = 0, * Sdy = 0;

	double resultScore = 0;
	double partialSum = 0;
	double sumOfCoords = 0;
	double partialScore;
	const short* _Sdx;
	const short* _Sdy;
	int i, j, m;			// count variables
	double iTx, iTy, iSx, iSy;
	double gradMag;
	int curX, curY;

	double** matGradMag;  //Gradient magnitude matrix

	CvMat srcstub, * src = (CvMat*)srcarr;
	src = cvGetMat(src, &srcstub);
	if (CV_MAT_TYPE(src->type) != CV_8UC1 || !modelDefined)
	{
		return 0;
	}

	// source image size
	CvSize Ssize;
	Ssize.width = src->width;
	Ssize.height = src->height;

	CreateDoubleMatrix(matGradMag, Ssize); // create image to save gradient magnitude  values

	Sdx = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1); // X derivatives
	Sdy = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1); // y derivatives

	cvSobel(src, Sdx, 1, 0, 3);  // find X derivatives
	cvSobel(src, Sdy, 0, 1, 3); // find Y derivatives

	// stoping criterias to search for model
	double normMinScore = minScore / noOfCordinates; // precompute minumum score 
	double normGreediness = ((1 - greediness * minScore) / (1 - greediness)) / noOfCordinates; // precompute greedniness 

	for (i = 0; i < Ssize.height; i++)
	{
		_Sdx = (short*)(Sdx->data.ptr + Sdx->step * (i));
		_Sdy = (short*)(Sdy->data.ptr + Sdy->step * (i));

		for (j = 0; j < Ssize.width; j++)
		{
			iSx = _Sdx[j];  // X derivative of Source image
			iSy = _Sdy[j];  // Y derivative of Source image

			gradMag = sqrt((iSx * iSx) + (iSy * iSy)); //Magnitude = Sqrt(dx^2 +dy^2)

			if (gradMag != 0) // hande divide by zero
				matGradMag[i][j] = 1 / gradMag;   // 1/Sqrt(dx^2 +dy^2)
			else
				matGradMag[i][j] = 0;

		}
	}
	for (i = 0; i < Ssize.height; i++)
	{
		for (j = 0; j < Ssize.width; j++)
		{

			//#pragma omp parallel for
			for (int degree = 0; degree < totalDegree; degree++)
			{
				partialSum = 0; // initilize partialSum measure

				for (m = 0; m < noOfCordinates; m++)
				{

					curX = i + cordinatesRotate[degree][m].x;	// template X coordinate
					curY = j + cordinatesRotate[degree][m].y; // template Y coordinate
					iTx = edgeDerivativeXRotate[degree][m];	// template X derivative
					iTy = edgeDerivativeYRotate[degree][m];    // template Y derivative


					if (curX<0 || curY<0 || curX>Ssize.height - 1 || curY>Ssize.width - 1)
						continue;

					_Sdx = (short*)(Sdx->data.ptr + Sdx->step * (curX));
					_Sdy = (short*)(Sdy->data.ptr + Sdy->step * (curX));

					iSx = _Sdx[curY]; // get curresponding  X derivative from source image
					iSy = _Sdy[curY];// get curresponding  Y derivative from source image

					if ((iSx != 0 || iSy != 0) && (iTx != 0 || iTy != 0))
					{
						//partial Sum  = Sum of(((Source X derivative* Template X drivative) + Source Y derivative * Template Y derivative)) / Edge magnitude of(Template)* edge magnitude of(Source))
						partialSum = partialSum + ((iSx * iTx) + (iSy * iTy)) * (edgeMagnitude[m] * matGradMag[curX][curY]);

					}

					sumOfCoords = m + 1;
					partialScore = partialSum / sumOfCoords;
					// check termination criteria
					// if partial score score is less than the score than needed to make the required score at that position
					// break serching at that coordinate.
					if (partialScore < (MIN((minScore - 1) + normGreediness * sumOfCoords, normMinScore * sumOfCoords)))
						break;

				}
				if (partialScore > resultScore)
				{
					resultScore = partialScore; //  Match score
					resultPoint->x = i;			// result coordinate X		
					resultPoint->y = j;			// result coordinate Y
					rotation = degree;

				}
			}
		}
	}

	// free used resources and return score
	ReleaseDoubleMatrix(matGradMag, Ssize.height);
	cvReleaseMat(&Sdx);
	cvReleaseMat(&Sdy);

	return resultScore;
}

double NCC_ModelFinder::FindGeoMatchModelRotateParallel(const void* srcarr, double minScore, double greediness, CvPoint* resultPoint, double& rotation)
{
	CvMat* Sdx = 0, * Sdy = 0;

	double* resultScore = new double[totalDegree];
	CvPoint* tmpPoint = new CvPoint[totalDegree];

	for (int i = 0; i < totalDegree; i++)
	{
		resultScore[i] = 0;
		tmpPoint[i].x = 0;
		tmpPoint[i].y = 0;
	}
	//double partialSum = 0;
	
	//double partialScore;
	//const short* _Sdx;
	//const short* _Sdy;
	int i, j;			// count variables
	//double iTx, iTy, iSx, iSy;
	
	//int curX, curY;

	double** matGradMag;  //Gradient magnitude matrix

	CvMat srcstub, * src = (CvMat*)srcarr;
	src = cvGetMat(src, &srcstub);
	if (CV_MAT_TYPE(src->type) != CV_8UC1 || !modelDefined)
	{
		return 0;
	}

	// source image size
	CvSize Ssize;
	Ssize.width = src->width;
	Ssize.height = src->height;

	CreateDoubleMatrix(matGradMag, Ssize); // create image to save gradient magnitude  values

	Sdx = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1); // X derivatives
	Sdy = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1); // y derivatives

	cvSobel(src, Sdx, 1, 0, 3);  // find X derivatives
	cvSobel(src, Sdy, 0, 1, 3); // find Y derivatives

	// stoping criterias to search for model
	double normMinScore = minScore / noOfCordinates; // precompute minumum score 
	double normGreediness = ((1 - greediness * minScore) / (1 - greediness)) / noOfCordinates; // precompute greedniness 


	for (i = 0; i < Ssize.height; i++)
	{
		const short* _Sdx;
		const short* _Sdy;
		//double iSx, iSy;

		_Sdx = (short*)(Sdx->data.ptr + Sdx->step * (i));
		_Sdy = (short*)(Sdy->data.ptr + Sdy->step * (i));

		parallel_for(size_t(0), size_t(Ssize.width), [&](size_t j)
			//for (j = 0; j < Ssize.width; j++)
			{
				double iSx, iSy;
				iSx = _Sdx[j];  // X derivative of Source image
				iSy = _Sdy[j];  // Y derivative of Source image
				double gradMag;
				gradMag = sqrt((iSx * iSx) + (iSy * iSy)); //Magnitude = Sqrt(dx^2 +dy^2)

				if (gradMag != 0) // hande divide by zero
					matGradMag[i][j] = 1 / gradMag;   // 1/Sqrt(dx^2 +dy^2)
				else
					matGradMag[i][j] = 0;

			}
		);
	}

	//parallel_for(size_t(0), size_t(Ssize.height), [&](size_t i)
		for (i = 0; i < Ssize.height; i++)
		{
			parallel_for(size_t(0), size_t(Ssize.width), [&](size_t j)
				//for (j = 0; j < Ssize.width; j++)
				{

					parallel_for(size_t(0), size_t(totalDegree), [&](size_t degree)
						//for (int degree = 0; degree < totalDegree; degree++)
						{
							double sumOfCoords = 0;
							double partialScore = 0;
							double partialSum = 0; // initilize partialSum measure
							const short* _Sdx;
							const short* _Sdy;

							double iTx, iTy, iSx, iSy;
							double gradMag;
							int curX, curY;
							for (int m = 0; m < noOfCordinates; m++)
							{

								curX = i + cordinatesRotate[degree][m].x;	// template X coordinate
								curY = j + cordinatesRotate[degree][m].y; // template Y coordinate
								iTx = edgeDerivativeXRotate[degree][m];	// template X derivative
								iTy = edgeDerivativeYRotate[degree][m];    // template Y derivative


								if (curX<0 || curY<0 || curX>Ssize.height - 1 || curY>Ssize.width - 1)
									continue;

								_Sdx = (short*)(Sdx->data.ptr + Sdx->step * (curX));
								_Sdy = (short*)(Sdy->data.ptr + Sdy->step * (curX));

								iSx = _Sdx[curY]; // get curresponding  X derivative from source image
								iSy = _Sdy[curY];// get curresponding  Y derivative from source image

								if ((iSx != 0 || iSy != 0) && (iTx != 0 || iTy != 0))
								{
									//partial Sum  = Sum of(((Source X derivative* Template X drivative) + Source Y derivative * Template Y derivative)) / Edge magnitude of(Template)* edge magnitude of(Source))
									partialSum = partialSum + ((iSx * iTx) + (iSy * iTy)) * (edgeMagnitude[m] * matGradMag[curX][curY]);

								}

								sumOfCoords = m + 1;
								partialScore = partialSum / sumOfCoords;
								// check termination criteria
								// if partial score score is less than the score than needed to make the required score at that position
								// break serching at that coordinate.
								if (partialScore < (MIN((minScore - 1) + normGreediness * sumOfCoords, normMinScore * sumOfCoords)))
									break;

							}
							if (partialScore > resultScore[degree])
							{
								resultScore[degree] = partialScore; //  Match score
								tmpPoint[degree].x = i;
								tmpPoint[degree].y = j;
								//cout << partialScore <<", "<<degree<< endl;
								//resultPoint->x = i;			// result coordinate X		
								//resultPoint->y = j;			// result coordinate Y
								//rotation = degree;

							}
						}
					);
				}
			);
		}
	//);
	int tmpindex = 0;
	double tmpScore = 0;
	for (int i = 0; i < totalDegree; i++)
	{
		cout << resultScore[i] << ", " << i<< endl;
		if(resultScore[i] > 0 && resultScore[i] <=1)
		if (resultScore[i] > tmpScore)
		{
			tmpScore = resultScore[i];
			tmpindex = i;
		}
	}
	resultPoint->x = tmpPoint[tmpindex].x;			// result coordinate X		
	resultPoint->y = tmpPoint[tmpindex].y;			// result coordinate Y
	rotation = tmpindex;
	// free used resources and return score
	ReleaseDoubleMatrix(matGradMag, Ssize.height);
	cvReleaseMat(&Sdx);
	cvReleaseMat(&Sdy);

	return resultScore[tmpindex];
}

double NCC_ModelFinder::FindGeoMatchModelRotatePyramid(const void* srcarr, double minScore, double greediness,int searchx,int searchy, CvPoint* resultPoint, double& rotation)
{
	CvMat* Sdx = 0, * Sdy = 0;

	double* resultScore = new double[totalDegree];
	CvPoint* tmpPoint = new CvPoint[totalDegree];

	for (int i = 0; i < totalDegree; i++)
	{
		resultScore[i] = 0;
		tmpPoint[i].x = 0;
		tmpPoint[i].y = 0;
	}
	//double partialSum = 0;

	//double partialScore;
	//const short* _Sdx;
	//const short* _Sdy;
	int i, j;			// count variables
	//double iTx, iTy, iSx, iSy;

	//int curX, curY;

	double** matGradMag;  //Gradient magnitude matrix

	CvMat srcstub, * src = (CvMat*)srcarr;
	src = cvGetMat(src, &srcstub);
	if (CV_MAT_TYPE(src->type) != CV_8UC1 || !modelDefined)
	{
		return 0;
	}

	// source image size
	CvSize Ssize;
	Ssize.width = src->width;
	Ssize.height = src->height;

	CreateDoubleMatrix(matGradMag, Ssize); // create image to save gradient magnitude  values

	Sdx = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1); // X derivatives
	Sdy = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1); // y derivatives

	cvSobel(src, Sdx, 1, 0, 3);  // find X derivatives
	cvSobel(src, Sdy, 0, 1, 3); // find Y derivatives

	// stoping criterias to search for model
	double normMinScore = minScore / noOfCordinates; // precompute minumum score 
	double normGreediness = ((1 - greediness * minScore) / (1 - greediness)) / noOfCordinates; // precompute greedniness 


	for (i = 0; i < Ssize.height; i++)
	{
		const short* _Sdx;
		const short* _Sdy;
		//double iSx, iSy;

		_Sdx = (short*)(Sdx->data.ptr + Sdx->step * (i));
		_Sdy = (short*)(Sdy->data.ptr + Sdy->step * (i));

		parallel_for(size_t(0), size_t(Ssize.width), [&](size_t j)
			//for (j = 0; j < Ssize.width; j++)
			{
				double iSx, iSy;
				iSx = _Sdx[j];  // X derivative of Source image
				iSy = _Sdy[j];  // Y derivative of Source image
				double gradMag;
				gradMag = sqrt((iSx * iSx) + (iSy * iSy)); //Magnitude = Sqrt(dx^2 +dy^2)

				if (gradMag != 0) // hande divide by zero
					matGradMag[i][j] = 1 / gradMag;   // 1/Sqrt(dx^2 +dy^2)
				else
					matGradMag[i][j] = 0;

			}
		);
	}

	//parallel_for(size_t(0), size_t(Ssize.height), [&](size_t i)
	for (i = 0; i < Ssize.height; i++)
	{
		parallel_for(size_t(0), size_t(Ssize.width), [&](size_t j)
			//for (j = 0; j < Ssize.width; j++)
			{

				parallel_for(size_t(0), size_t(totalDegree), [&](size_t degree)
					//for (int degree = 0; degree < totalDegree; degree++)
					{
						double sumOfCoords = 0;
						double partialScore = 0;
						double partialSum = 0; // initilize partialSum measure
						const short* _Sdx;
						const short* _Sdy;

						double iTx, iTy, iSx, iSy;
						double gradMag;
						int curX, curY;
						for (int m = 0; m < noOfCordinates; m++)
						{

							curX = i + cordinatesRotate[degree][m].x;	// template X coordinate
							curY = j + cordinatesRotate[degree][m].y; // template Y coordinate
							iTx = edgeDerivativeXRotate[degree][m];	// template X derivative
							iTy = edgeDerivativeYRotate[degree][m];    // template Y derivative


							if (curX<0 || curY<0 || curX>Ssize.height - 1 || curY>Ssize.width - 1)
								continue;

							_Sdx = (short*)(Sdx->data.ptr + Sdx->step * (curX));
							_Sdy = (short*)(Sdy->data.ptr + Sdy->step * (curX));

							iSx = _Sdx[curY]; // get curresponding  X derivative from source image
							iSy = _Sdy[curY];// get curresponding  Y derivative from source image

							if ((iSx != 0 || iSy != 0) && (iTx != 0 || iTy != 0))
							{
								//partial Sum  = Sum of(((Source X derivative* Template X drivative) + Source Y derivative * Template Y derivative)) / Edge magnitude of(Template)* edge magnitude of(Source))
								partialSum = partialSum + ((iSx * iTx) + (iSy * iTy)) * (edgeMagnitude[m] * matGradMag[curX][curY]);

							}

							sumOfCoords = m + 1;
							partialScore = partialSum / sumOfCoords;
							// check termination criteria
							// if partial score score is less than the score than needed to make the required score at that position
							// break serching at that coordinate.
							if (partialScore < (MIN((minScore - 1) + normGreediness * sumOfCoords, normMinScore * sumOfCoords)))
								break;

						}
						if (partialScore > resultScore[degree])
						{
							resultScore[degree] = partialScore; //  Match score
							tmpPoint[degree].x = i;
							tmpPoint[degree].y = j;
							//cout << partialScore <<", "<<degree<< endl;
							//resultPoint->x = i;			// result coordinate X		
							//resultPoint->y = j;			// result coordinate Y
							//rotation = degree;

						}
					}
				);
			}
		);
	}
	//);
	int tmpindex = 0;
	double tmpScore = 0;
	for (int i = 0; i < totalDegree; i++)
	{
		cout << resultScore[i] << ", " << i << endl;
		if (resultScore[i] > 0 && resultScore[i] <= 1)
			if (resultScore[i] > tmpScore)
			{
				tmpScore = resultScore[i];
				tmpindex = i;
			}
	}
	resultPoint->x = tmpPoint[tmpindex].x;			// result coordinate X		
	resultPoint->y = tmpPoint[tmpindex].y;			// result coordinate Y
	rotation = tmpindex;
	// free used resources and return score
	ReleaseDoubleMatrix(matGradMag, Ssize.height);
	cvReleaseMat(&Sdx);
	cvReleaseMat(&Sdy);

	return resultScore[tmpindex];
}

double NCC_ModelFinder::FindGeoMatchModelRotatePyramid(const void* srcarr, double minScore, double greediness, int searchx, int searchy, CvPoint** Tpoint, double** TedgeX, double** TedgeY, CvPoint* resultPoint, double& rotation)
{
	CvMat* Sdx = 0, * Sdy = 0;

	double* resultScore = new double[totalDegree];
	CvPoint* tmpPoint = new CvPoint[totalDegree];

	for (int i = 0; i < totalDegree; i++)
	{
		resultScore[i] = 0;
		tmpPoint[i].x = 0;
		tmpPoint[i].y = 0;
	}
	//double partialSum = 0;

	//double partialScore;
	//const short* _Sdx;
	//const short* _Sdy;
	int i, j;			// count variables
	//double iTx, iTy, iSx, iSy;

	//int curX, curY;

	double** matGradMag;  //Gradient magnitude matrix

	CvMat srcstub, * src = (CvMat*)srcarr;
	src = cvGetMat(src, &srcstub);
	if (CV_MAT_TYPE(src->type) != CV_8UC1 || !modelDefined)
	{
		return 0;
	}

	// source image size
	CvSize Ssize;
	Ssize.width = src->width;
	Ssize.height = src->height;

	CreateDoubleMatrix(matGradMag, Ssize); // create image to save gradient magnitude  values

	Sdx = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1); // X derivatives
	Sdy = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1); // y derivatives

	cvSobel(src, Sdx, 1, 0, 3);  // find X derivatives
	cvSobel(src, Sdy, 0, 1, 3); // find Y derivatives

	// stoping criterias to search for model
	double normMinScore = minScore / noOfCordinates; // precompute minumum score 
	double normGreediness = ((1 - greediness * minScore) / (1 - greediness)) / noOfCordinates; // precompute greedniness 


	for (i = 0; i < Ssize.height; i++)
	{
		const short* _Sdx;
		const short* _Sdy;
		//double iSx, iSy;

		_Sdx = (short*)(Sdx->data.ptr + Sdx->step * (i));
		_Sdy = (short*)(Sdy->data.ptr + Sdy->step * (i));

		parallel_for(size_t(0), size_t(Ssize.width), [&](size_t j)
			//for (j = 0; j < Ssize.width; j++)
			{
				double iSx, iSy;
				iSx = _Sdx[j];  // X derivative of Source image
				iSy = _Sdy[j];  // Y derivative of Source image
				double gradMag;
				gradMag = sqrt((iSx * iSx) + (iSy * iSy)); //Magnitude = Sqrt(dx^2 +dy^2)

				if (gradMag != 0) // hande divide by zero
					matGradMag[i][j] = 1 / gradMag;   // 1/Sqrt(dx^2 +dy^2)
				else
					matGradMag[i][j] = 0;

			}
		);
	}

	//parallel_for(size_t(0), size_t(Ssize.height), [&](size_t i)
	for (i = 0; i < Ssize.height; i++)
	{
		parallel_for(size_t(0), size_t(Ssize.width), [&](size_t j)
			//for (j = 0; j < Ssize.width; j++)
			{

				parallel_for(size_t(0), size_t(totalDegree), [&](size_t degree)
					//for (int degree = 0; degree < totalDegree; degree++)
					{
						double sumOfCoords = 0;
						double partialScore = 0;
						double partialSum = 0; // initilize partialSum measure
						const short* _Sdx;
						const short* _Sdy;

						double iTx, iTy, iSx, iSy;
						double gradMag;
						int curX, curY;
						for (int m = 0; m < noOfCordinates; m++)
						{

							curX = i + cordinatesRotate[degree][m].x;	// template X coordinate
							curY = j + cordinatesRotate[degree][m].y; // template Y coordinate
							iTx = edgeDerivativeXRotate[degree][m];	// template X derivative
							iTy = edgeDerivativeYRotate[degree][m];    // template Y derivative


							if (curX<0 || curY<0 || curX>Ssize.height - 1 || curY>Ssize.width - 1)
								continue;

							_Sdx = (short*)(Sdx->data.ptr + Sdx->step * (curX));
							_Sdy = (short*)(Sdy->data.ptr + Sdy->step * (curX));

							iSx = _Sdx[curY]; // get curresponding  X derivative from source image
							iSy = _Sdy[curY];// get curresponding  Y derivative from source image

							if ((iSx != 0 || iSy != 0) && (iTx != 0 || iTy != 0))
							{
								//partial Sum  = Sum of(((Source X derivative* Template X drivative) + Source Y derivative * Template Y derivative)) / Edge magnitude of(Template)* edge magnitude of(Source))
								partialSum = partialSum + ((iSx * iTx) + (iSy * iTy)) * (edgeMagnitude[m] * matGradMag[curX][curY]);

							}

							sumOfCoords = m + 1;
							partialScore = partialSum / sumOfCoords;
							// check termination criteria
							// if partial score score is less than the score than needed to make the required score at that position
							// break serching at that coordinate.
							if (partialScore < (MIN((minScore - 1) + normGreediness * sumOfCoords, normMinScore * sumOfCoords)))
								break;

						}
						if (partialScore > resultScore[degree])
						{
							resultScore[degree] = partialScore; //  Match score
							tmpPoint[degree].x = i;
							tmpPoint[degree].y = j;
							//cout << partialScore <<", "<<degree<< endl;
							//resultPoint->x = i;			// result coordinate X		
							//resultPoint->y = j;			// result coordinate Y
							//rotation = degree;

						}
					}
				);
			}
		);
	}
	//);
	int tmpindex = 0;
	double tmpScore = 0;
	for (int i = 0; i < totalDegree; i++)
	{
		cout << resultScore[i] << ", " << i << endl;
		if (resultScore[i] > 0 && resultScore[i] <= 1)
			if (resultScore[i] > tmpScore)
			{
				tmpScore = resultScore[i];
				tmpindex = i;
			}
	}
	resultPoint->x = tmpPoint[tmpindex].x;			// result coordinate X		
	resultPoint->y = tmpPoint[tmpindex].y;			// result coordinate Y
	rotation = tmpindex;
	// free used resources and return score
	ReleaseDoubleMatrix(matGradMag, Ssize.height);
	cvReleaseMat(&Sdx);
	cvReleaseMat(&Sdy);

	return resultScore[tmpindex];
}

// destructor
NCC_ModelFinder::~NCC_ModelFinder(void)
{
	delete[] cordinates;
	delete[] edgeMagnitude;
	delete[] edgeDerivativeX;
	delete[] edgeDerivativeY;

	for (int i = 0; i < totalDegree; i++)
	{
		delete[] cordinatesRotate[i];
		delete[] edgeDerivativeXRotate[i];
		delete[] edgeDerivativeYRotate[i];
	}
	delete[] cordinatesRotate;
	delete[] edgeDerivativeXRotate;
	delete[] edgeDerivativeYRotate;
}

//allocate memory for doubel matrix
void NCC_ModelFinder::CreateDoubleMatrix(double**& matrix, CvSize size)
{
	matrix = new double* [size.height];
	for (int iInd = 0; iInd < size.height; iInd++)
		matrix[iInd] = new double[size.width];
}
// release memory
void NCC_ModelFinder::ReleaseDoubleMatrix(double**& matrix, int size)
{
	for (int iInd = 0; iInd < size; iInd++)
		delete[] matrix[iInd];
}


// draw contours around result image
void NCC_ModelFinder::DrawContours(IplImage* source, CvPoint COG, CvScalar color, int lineWidth)
{
	CvPoint point;
	point.y = COG.x;
	point.x = COG.y;
	for (int i = 0; i < noOfCordinates; i++)
	{
		point.y = cordinates[i].x + COG.x;
		point.x = cordinates[i].y + COG.y;
		cvLine(source, point, point, color, lineWidth);
	}
}

void NCC_ModelFinder::DrawContours(IplImage* source, CvPoint COG, CvScalar color, int lineWidth, int rotation)
{
	CvPoint point;
	point.y = COG.x;
	point.x = COG.y;
	for (int i = 0; i < noOfCordinates; i++)
	{
		point.y = cordinatesRotate[rotation][i].x + COG.x;
		point.x = cordinatesRotate[rotation][i].y + COG.y;
		cvLine(source, point, point, color, lineWidth);
	}
}

// draw contour at template image
void NCC_ModelFinder::DrawContours(IplImage* source, CvScalar color, int lineWidth)
{
	CvPoint point;
	for (int i = 0; i < noOfCordinates; i++)
	{
		point.y = cordinates[i].x + centerOfGravity.x;
		point.x = cordinates[i].y + centerOfGravity.y;
		cvLine(source, point, point, color, lineWidth);
	}
}


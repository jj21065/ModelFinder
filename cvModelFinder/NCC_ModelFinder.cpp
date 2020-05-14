#include "pch.h"
#include "NCC_ModelFinder.h"
#include "C:/opencv/build/include/opencv2/highgui/highgui_c.h"
#include <windows.h>
#include <stdio.h> 
#include <time.h>
#include <iostream>
#include <intrin.h> 
#include <vector>
#include <set>
using namespace std;
using namespace Cpp;
NCC_ModelFinder::NCC_ModelFinder(void)
{
}

void NCC_ModelFinder::SetModelPara(double r1, double r2, double resolution, double score)
{
	this->m_modelDefine.degreeStart = r1;
	this->m_modelDefine.degreeEnd = r2;
	this->m_modelDefine.rotationResolution = resolution;
	this->m_modelDefine.searshScore = score;

}

void NCC_ModelFinder::SetSobelThreshold(int high, int low)
{
	m_modelDefine.SobelHigh = high;
	m_modelDefine.SobelLow = low;
}

void NCC_ModelFinder::SetROI(double x, double y, double width, double height)
{
	this->roi.rect.x = x;
	this->roi.rect.y = y;
	this->roi.rect.width = width;
	this->roi.rect.height = height;
	this->roi.isEnable = true;
}

void NCC_ModelFinder::DisableROI()
{
	this->roi.isEnable = false;
}

void NCC_ModelFinder::CreatModel(cv::Mat mat)
{

	int lowThreshold = m_modelDefine.SobelLow;		//deafult value
	int highThreashold = m_modelDefine.SobelHigh;	//deafult value

	IplImage* templateimage = new IplImage(mat);

	if (templateimage == NULL)
	{
		cout << "\nERROR: Could not load Template Image.\n" << endl;
		return;
	}
	CvSize templateSize = cvSize(templateimage->width, templateimage->height);
	IplImage* grayTemplateImg = cvCreateImage(templateSize, IPL_DEPTH_8U, 1);
	if (templateimage->nChannels == 3)
	{
		cvCvtColor(templateimage, grayTemplateImg, CV_RGB2GRAY);
	}
	else
	{
		cvCopy(templateimage, grayTemplateImg);
	}

	if (!CreateGeoMatchModel(grayTemplateImg, lowThreshold, highThreashold))
	{
		cout << "ERROR: could not create model...";
		return;
	}
	DrawContours(templateimage, CV_RGB(255, 0, 0), 1);
	cout << " Shape model created.." << "with  Low Threshold = " << lowThreshold << " High Threshold = " << highThreashold << endl;


	//Display result
	if (showCvImage) {
		cvNamedWindow("Template", CV_WINDOW_AUTOSIZE);
		cvShowImage("Template", templateimage);
	}

	//// wait for both windows to be closed before releasing images
	/*cvWaitKey(0);*/
	////
	//cvDestroyWindow("Template");

	cvReleaseImage(&grayTemplateImg);
	delete templateimage;
}

void NCC_ModelFinder::ModelEraser(int x, int y, int eraserWidth)
{
	if (m_modelDefine.modelDefined)
	{
		int i = 0;
		int no = 0;
		int size = m_modelDefine.noOfCordinates;
		int* xx = new int[m_modelDefine.noOfCordinates]{ 0 };
		int* yy = new int[m_modelDefine.noOfCordinates]{ 0 };
		double* ex = new double[m_modelDefine.noOfCordinates]{ 0 };
		double* ey = new double[m_modelDefine.noOfCordinates]{ 0 };
		double* m = new double[m_modelDefine.noOfCordinates]{ 0 };
		for (i = 0; i < size; i++)
		{

			if (abs(m_modelDefine.cordinates[i].x + m_modelDefine.centerOfGravity.x - x) < eraserWidth && abs(m_modelDefine.cordinates[i].y + m_modelDefine.centerOfGravity.y - y) < eraserWidth) {
			
			}
			else
			{
				++no;
				xx[no] = m_modelDefine.cordinates[i].x;
				yy[no] = m_modelDefine.cordinates[i].y;
				ex[no] = m_modelDefine.edgeDerivativeX[i];
				ey[no] = m_modelDefine.edgeDerivativeY[i];
				m[no] = m_modelDefine.edgeMagnitude[i];
			}

		}

		for (i = 0; i < m_modelDefine.noOfCordinates; ++i)
		{
			
			m_modelDefine.cordinates[i].x = xx[i];
			 m_modelDefine.cordinates[i].y = yy[i];
			 m_modelDefine.edgeDerivativeX[i] = ex[i];
			m_modelDefine.edgeDerivativeY[i] = ey[i];
			 m_modelDefine.edgeMagnitude[i] = m[i];
		}
		m_modelDefine.noOfCordinates = no;
		//std::cout << '\n';
		//m_modelDefine.Release();

		//m_modelDefine.cordinates = new CvPoint[m_modelDefine.modelWidth * m_modelDefine.modelHeight];		//Allocate memory for coorinates of selected points in template image
		//m_modelDefine.edgeMagnitude = new double[m_modelDefine.modelWidth * m_modelDefine.modelHeight];		//Allocate memory for edge magnitude for selected points
		//m_modelDefine.edgeDerivativeX = new double[m_modelDefine.modelWidth * m_modelDefine.modelHeight];			//Allocate memory for edge X derivative for selected points
		//m_modelDefine.edgeDerivativeY = new double[m_modelDefine.modelWidth * m_modelDefine.modelHeight];			////Allocate memory for edge Y derivative for selected points



		//std::set<int>::iterator iterx = xx.begin();
		//std::set<int>::iterator itery = yy.begin();
		//std::set<double>::iterator iterex = edgex.begin();
		//std::set<double>::iterator iterey = edgey.begin();
		//std::set<double>::iterator itermag = mag.begin();
		//
		//for (i = 0; i < m_modelDefine.noOfCordinates; ++i,++iterx,++itery,++iterex,++iterey,++itermag)
		//{
		//	{
		//		m_modelDefine.cordinates[i].x = (*iterx);
		//		m_modelDefine.cordinates[i].y = (*itery);
		//		m_modelDefine.edgeDerivativeX[i] = (*iterex);
		//		m_modelDefine.edgeDerivativeY[i] = (*iterey);
		//		m_modelDefine.edgeMagnitude[i] = (*itermag);
		//	}
		//}
		m_modelDefine.ReleaseMatrix();
		float r1 = m_modelDefine.degreeStart;
		float r2 = m_modelDefine.degreeEnd;
		float resolution = m_modelDefine.rotationResolution;
		m_modelDefine.totalDegree = (r2 - r1) / m_modelDefine.rotationResolution + 1;
		int count = 0;
		m_modelDefine.cordinatesRotate = new CvPoint * [m_modelDefine.totalDegree];		//Coordinates array to store model points	
		m_modelDefine.edgeDerivativeXRotate = new double* [m_modelDefine.totalDegree];	//gradient in X direction
		m_modelDefine.edgeDerivativeYRotate = new double* [m_modelDefine.totalDegree]; 	//radient in Y direction	
		for (int i = 0; i < m_modelDefine.totalDegree; i++)
		{
			m_modelDefine.cordinatesRotate[i] = new CvPoint[m_modelDefine.noOfCordinates];
			m_modelDefine.edgeDerivativeXRotate[i] = new double[m_modelDefine.noOfCordinates];
			m_modelDefine.edgeDerivativeYRotate[i] = new double[m_modelDefine.noOfCordinates];
		}
		for (float degree = r1; degree < r2; degree += resolution)
		{
			for (int i = 0; i < m_modelDefine.noOfCordinates; i++)
			{
				float thida = degree * CV_PI / 180.0;
				m_modelDefine.cordinatesRotate[count][i].x = (m_modelDefine.cordinates[i].x) * cos(thida) - (m_modelDefine.cordinates[i].y) * sin(thida);
				m_modelDefine.cordinatesRotate[count][i].y = (m_modelDefine.cordinates[i].x) * sin(thida) + (m_modelDefine.cordinates[i].y) * cos(thida);
				m_modelDefine.edgeDerivativeXRotate[count][i] = m_modelDefine.edgeDerivativeX[i] * cos(thida) - m_modelDefine.edgeDerivativeY[i] * sin(thida);
				m_modelDefine.edgeDerivativeYRotate[count][i] = m_modelDefine.edgeDerivativeX[i] * sin(thida) + m_modelDefine.edgeDerivativeY[i] * cos(thida);

			}
			count++;
		}
	}
}

void NCC_ModelFinder::ModelFind(cv::Mat mat)
{

	int lowThreshold = 100;		//deafult value
	int highThreashold = 150;	//deafult value

	double minScore = m_modelDefine.searshScore;		//deafult value
	double greediness = 0.9;		//deafult value

	double total_time = 0;
	double score = 0;
	clock_t start_time1 = clock();

	IplImage* searchImage = new IplImage(mat);
	if (roi.isEnable)
		cvSetImageROI(searchImage, roi.rect);

	if (searchImage == NULL)
	{
		cout << "\nERROR: Could not load src Image.\n" << endl;
		return;
	}

	CvSize searchSize;
	if (roi.isEnable)
	{
		if (roi.rect.x < 0)
			roi.rect.x = 0;
		if (roi.rect.y < 0)
			roi.rect.y;
		if (roi.rect.x + roi.rect.width > searchImage->width - 1)
			roi.rect.width = searchImage->width - 1 - roi.rect.x;
		if (roi.rect.x + roi.rect.height > searchImage->height - 1)
			roi.rect.height = searchImage->height - 1 - roi.rect.y;
		searchSize = cvSize(roi.rect.width, roi.rect.height);
	}
	else
	{
		searchSize = cvSize(searchImage->width, searchImage->height);
	}
	IplImage* graySearchImg = cvCreateImage(searchSize, IPL_DEPTH_8U, 1);

	// Convert color image to gray image. 
	if (searchImage->nChannels == 3)
		cvCvtColor(searchImage, graySearchImg, CV_RGB2GRAY);
	else
	{
		cvCopy(searchImage, graySearchImg);
	}
	cout << " Finding Shape Model.." << " Minumum Score = " << minScore << " Greediness = " << greediness << "\n\n";
	cout << " ------------------------------------\n";

	double rotation = 0;


	//score = GM.FindGeoMatchModel(graySearchImg, minScore, greediness, &result);
	score = FindGeoMatchModelRotateParallel(graySearchImg, minScore, greediness, rotation);
	//score = GM.FindGeoMatchModelRotate(graySearchImg, minScore, greediness, &result, rotation);
	clock_t finish_time1 = clock();
	total_time = (double)(finish_time1 - start_time1) / CLOCKS_PER_SEC;

	if (score > minScore) // if score is atleast 0.4
	{
		result.isFind = true;
		cout << " Found at [" << result.location.x << ", " << result.location.y << ", Rotation " << result.rotation << "]\n Score = " << score << "\n Searching Time = " << total_time * 1000 << "ms";

		if (showCvImage)
		{
			cvResetImageROI(searchImage);
			DrawContours(searchImage, result.location, CV_RGB(0, 255, 0), 1, (int)rotation);
			cvNamedWindow("Search Image", CV_WINDOW_AUTOSIZE);
			cvShowImage("Search Image", searchImage);
		}
	}
	else
	{
		result.isFind = false;
		cout << " Object Not found";
		if (showCvImage)
		{
			cvNamedWindow("Search Image", CV_WINDOW_AUTOSIZE);
			cvShowImage("Search Image", searchImage);
		}
	}

	cout << "\n ------------------------------------\n\n";
	cout << "\n Press any key to exit!";


	/*cvWaitKey(0);
	cvDestroyWindow("Search Image");*/

	cvReleaseImage(&graySearchImg);
	delete searchImage;
}

int NCC_ModelFinder::CreateGeoMatchModel(const void* templateArr, double maxContrast, double minContrast)
{
	if (m_modelDefine.modelDefined)
		m_modelDefine.Release();
	double resolution = m_modelDefine.rotationResolution;
	double r1 = m_modelDefine.degreeStart;
	double r2 = m_modelDefine.degreeEnd;

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
	m_modelDefine.modelHeight = src->height;		//Save Template height
	m_modelDefine.modelWidth = src->width;			//Save Template width

	m_modelDefine.noOfCordinates = 0;											//initialize	
	m_modelDefine.cordinates = new CvPoint[m_modelDefine.modelWidth * m_modelDefine.modelHeight];		//Allocate memory for coorinates of selected points in template image
	m_modelDefine.edgeMagnitude = new double[m_modelDefine.modelWidth * m_modelDefine.modelHeight];		//Allocate memory for edge magnitude for selected points
	m_modelDefine.edgeDerivativeX = new double[m_modelDefine.modelWidth * m_modelDefine.modelHeight];			//Allocate memory for edge X derivative for selected points
	m_modelDefine.edgeDerivativeY = new double[m_modelDefine.modelWidth * m_modelDefine.modelHeight];			////Allocate memory for edge Y derivative for selected points


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

					m_modelDefine.cordinates[m_modelDefine.noOfCordinates].x = curX;
					m_modelDefine.cordinates[m_modelDefine.noOfCordinates].y = curY;
					m_modelDefine.edgeDerivativeX[m_modelDefine.noOfCordinates] = fdx;
					m_modelDefine.edgeDerivativeY[m_modelDefine.noOfCordinates] = fdy;

					//handle divide by zero
					if (MagG != 0)
						m_modelDefine.edgeMagnitude[m_modelDefine.noOfCordinates] = 1 / MagG;  // gradient magnitude 
					else
						m_modelDefine.edgeMagnitude[m_modelDefine.noOfCordinates] = 0;

					m_modelDefine.noOfCordinates++;
				}
			}
		}
	}

	m_modelDefine.centerOfGravity.x = RSum / m_modelDefine.noOfCordinates; // center of gravity
	m_modelDefine.centerOfGravity.y = CSum / m_modelDefine.noOfCordinates;	// center of gravity

	// change coordinates to reflect center of gravity
	for (int m = 0; m < m_modelDefine.noOfCordinates; m++)
	{
		int temp;

		temp = m_modelDefine.cordinates[m].x;
		m_modelDefine.cordinates[m].x = temp - m_modelDefine.centerOfGravity.x;
		temp = m_modelDefine.cordinates[m].y;
		m_modelDefine.cordinates[m].y = temp - m_modelDefine.centerOfGravity.y;
	}

	m_modelDefine.totalDegree = (r2 - r1) / resolution + 1;
	count = 0;
	m_modelDefine.cordinatesRotate = new CvPoint * [m_modelDefine.totalDegree];		//Coordinates array to store model points	
	m_modelDefine.edgeDerivativeXRotate = new double* [m_modelDefine.totalDegree];	//gradient in X direction
	m_modelDefine.edgeDerivativeYRotate = new double* [m_modelDefine.totalDegree]; 	//radient in Y direction	
	for (int i = 0; i < m_modelDefine.totalDegree; i++)
	{
		m_modelDefine.cordinatesRotate[i] = new CvPoint[m_modelDefine.noOfCordinates];
		m_modelDefine.edgeDerivativeXRotate[i] = new double[m_modelDefine.noOfCordinates];
		m_modelDefine.edgeDerivativeYRotate[i] = new double[m_modelDefine.noOfCordinates];
	}
	for (float degree = r1; degree < r2; degree += resolution)
	{
		for (int i = 0; i < m_modelDefine.noOfCordinates; i++)
		{
			float thida = degree * CV_PI / 180.0;
			m_modelDefine.cordinatesRotate[count][i].x = (m_modelDefine.cordinates[i].x) * cos(thida) - (m_modelDefine.cordinates[i].y) * sin(thida);
			m_modelDefine.cordinatesRotate[count][i].y = (m_modelDefine.cordinates[i].x) * sin(thida) + (m_modelDefine.cordinates[i].y) * cos(thida);
			m_modelDefine.edgeDerivativeXRotate[count][i] = m_modelDefine.edgeDerivativeX[i] * cos(thida) - m_modelDefine.edgeDerivativeY[i] * sin(thida);
			m_modelDefine.edgeDerivativeYRotate[count][i] = m_modelDefine.edgeDerivativeX[i] * sin(thida) + m_modelDefine.edgeDerivativeY[i] * cos(thida);

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

	m_modelDefine.modelDefined = true;
	return 1;
}

double NCC_ModelFinder::FindGeoMatchModelRotateParallel(const void* srcarr, double minScore, double greediness, double& rotation)
{
	CvMat* Sdx = 0, * Sdy = 0;

	int i, j;			// count variables

	double** matGradMag;  //Gradient magnitude matrix

	CvMat srcstub, * src = (CvMat*)srcarr;
	src = cvGetMat(src, &srcstub);
	if (CV_MAT_TYPE(src->type) != CV_8UC1 || !m_modelDefine.modelDefined)
	{
		return 0;
	}
	double* resultScore = new double[m_modelDefine.totalDegree];
	CvPoint* tmpPoint = new CvPoint[m_modelDefine.totalDegree];

	for (int i = 0; i < m_modelDefine.totalDegree; i++)
	{
		resultScore[i] = 0;
		tmpPoint[i].x = 0;
		tmpPoint[i].y = 0;
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
	double normMinScore = minScore / m_modelDefine.noOfCordinates; // precompute minumum score 
	double normGreediness = ((1 - greediness * minScore) / (1 - greediness)) / m_modelDefine.noOfCordinates; // precompute greedniness 

#pragma omp parallel for
	for (i = 0; i < Ssize.height; i++)
	{
		const short* _Sdx;
		const short* _Sdy;
		//double iSx, iSy;

		_Sdx = (short*)(Sdx->data.ptr + Sdx->step * (i));
		_Sdy = (short*)(Sdy->data.ptr + Sdy->step * (i));

		//	parallel_for(size_t(0), size_t(Ssize.width), [&](size_t j)
#pragma omp parallel for
		for (j = 0; j < Ssize.width; j++)
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
	}

#pragma omp parallel for
	for (i = 0; i < Ssize.height; i++)
	{
#pragma omp parallel for

		for (j = 0; j < Ssize.width; j++)
		{
#pragma omp parallel for
			for (int degree = 0; degree < m_modelDefine.totalDegree; degree++)
			{
				double sumOfCoords = 0;
				double partialScore = 0;
				double partialSum = 0; // initilize partialSum measure
				const short* _Sdx;
				const short* _Sdy;

				double iTx, iTy, iSx, iSy;
				double gradMag;
				int curX, curY;
				for (int m = 0; m < m_modelDefine.noOfCordinates; m++)
				{
					/*		__m128 m1, m2, m3, m4;

							__m128* pSrc1 = (__m128*) pArray1;
							__m128* pSrc2 = (__m128*) pArray2;
							__m128* pDest = (__m128*) pResult;*/

					curX = i + m_modelDefine.cordinatesRotate[degree][m].x;	// template X coordinate
					curY = j + m_modelDefine.cordinatesRotate[degree][m].y; // template Y coordinate
					iTx = m_modelDefine.edgeDerivativeXRotate[degree][m];	// template X derivative
					iTy = m_modelDefine.edgeDerivativeYRotate[degree][m];    // template Y derivative


					if (curX<0 || curY<0 || curX>Ssize.height - 1 || curY>Ssize.width - 1)
						continue;

					_Sdx = (short*)(Sdx->data.ptr + Sdx->step * (curX));
					_Sdy = (short*)(Sdy->data.ptr + Sdy->step * (curX));

					iSx = _Sdx[curY]; // get curresponding  X derivative from source image
					iSy = _Sdy[curY];// get curresponding  Y derivative from source image

					if ((iSx != 0 || iSy != 0) && (iTx != 0 || iTy != 0))
					{
						//partial Sum  = Sum of(((Source X derivative* Template X drivative) + Source Y derivative * Template Y derivative)) / Edge magnitude of(Template)* edge magnitude of(Source))
						partialSum = partialSum + ((iSx * iTx) + (iSy * iTy)) * (m_modelDefine.edgeMagnitude[m] * matGradMag[curX][curY]);

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

				}
			}

		}

	}
	//);
	int tmpindex = 0;
	double tmpScore = 0;
	for (int i = 0; i < m_modelDefine.totalDegree; i++)
	{
		/*cout << resultScore[i] << ", " << i << endl;*/
		if (resultScore[i] > 0 && resultScore[i] <= 1)
			if (resultScore[i] > tmpScore)
			{
				tmpScore = resultScore[i];
				tmpindex = i;
			}
	}

	rotation = tmpindex;

	double score = resultScore[tmpindex];

	result.location.x = tmpPoint[tmpindex].x;			// result coordinate X		
	result.location.y = tmpPoint[tmpindex].y;			// result coordinate Y
	if (roi.isEnable)
	{
		result.location.x += roi.rect.y;
		result.location.y += roi.rect.x;
	}
	result.score = score;
	result.rotation = m_modelDefine.degreeStart + m_modelDefine.rotationResolution * rotation;

	// free used resources and return score
	ReleaseDoubleMatrix(matGradMag, Ssize.height);
	cvReleaseMat(&Sdx);
	cvReleaseMat(&Sdy);
	delete[] tmpPoint;
	delete[]  resultScore;
	return score;
}

double NCC_ModelFinder::FindGeoMatchModelRotateParallelSSE(const void* srcarr, double minScore, double greediness, double& rotation)
{
	CvMat* Sdx = 0, * Sdy = 0;

	int i, j;			// count variables

	double** matGradMag;  //Gradient magnitude matrix

	CvMat srcstub, * src = (CvMat*)srcarr;
	src = cvGetMat(src, &srcstub);
	if (CV_MAT_TYPE(src->type) != CV_8UC1 || !m_modelDefine.modelDefined)
	{
		return 0;
	}
	double* resultScore = new double[m_modelDefine.totalDegree];
	CvPoint* tmpPoint = new CvPoint[m_modelDefine.totalDegree];

	for (int i = 0; i < m_modelDefine.totalDegree; i++)
	{
		resultScore[i] = 0;
		tmpPoint[i].x = 0;
		tmpPoint[i].y = 0;
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
	double normMinScore = minScore / m_modelDefine.noOfCordinates; // precompute minumum score 
	double normGreediness = ((1 - greediness * minScore) / (1 - greediness)) / m_modelDefine.noOfCordinates; // precompute greedniness 

#pragma omp parallel for
	for (i = 0; i < Ssize.height; i++)
	{
		const short* _Sdx;
		const short* _Sdy;
		//double iSx, iSy;

		_Sdx = (short*)(Sdx->data.ptr + Sdx->step * (i));
		_Sdy = (short*)(Sdy->data.ptr + Sdy->step * (i));

		//	parallel_for(size_t(0), size_t(Ssize.width), [&](size_t j)
#pragma omp parallel for
		for (j = 0; j < Ssize.width; j++)
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
	}

#pragma omp parallel for
	for (i = 0; i < Ssize.height; i++)
	{
#pragma omp parallel for

		for (j = 0; j < Ssize.width; j++)
		{
#pragma omp parallel for
			for (int degree = 0; degree < m_modelDefine.totalDegree; degree++)
			{
				double sumOfCoords = 0;
				double partialScore = 0;
				double partialSum = 0; // initilize partialSum measure
				const short* _Sdx;
				const short* _Sdy;

				double iTx, iTy, iSx, iSy;
				double gradMag;
				int curX, curY;

				for (int m = 0; m < m_modelDefine.noOfCordinates; m++)
				{
					/*		__m128 m1, m2, m3, m4;

							__m128* pSrc1 = (__m128*) pArray1;
							__m128* pSrc2 = (__m128*) pArray2;
							__m128* pDest = (__m128*) pResult;*/

					curX = i + m_modelDefine.cordinatesRotate[degree][m].x;	// template X coordinate
					curY = j + m_modelDefine.cordinatesRotate[degree][m].y; // template Y coordinate
					iTx = m_modelDefine.edgeDerivativeXRotate[degree][m];	// template X derivative
					iTy = m_modelDefine.edgeDerivativeYRotate[degree][m];    // template Y derivative


					if (curX<0 || curY<0 || curX>Ssize.height - 1 || curY>Ssize.width - 1)
						continue;

					_Sdx = (short*)(Sdx->data.ptr + Sdx->step * (curX));
					_Sdy = (short*)(Sdy->data.ptr + Sdy->step * (curX));

					iSx = _Sdx[curY]; // get curresponding  X derivative from source image
					iSy = _Sdy[curY];// get curresponding  Y derivative from source image

					if ((iSx != 0 || iSy != 0) && (iTx != 0 || iTy != 0))
					{
						//partial Sum  = Sum of(((Source X derivative* Template X drivative) + Source Y derivative * Template Y derivative)) / Edge magnitude of(Template)* edge magnitude of(Source))
						partialSum = partialSum + ((iSx * iTx) + (iSy * iTy)) * (m_modelDefine.edgeMagnitude[m] * matGradMag[curX][curY]);

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

				}
			}

		}

	}
	//);
	int tmpindex = 0;
	double tmpScore = 0;
	for (int i = 0; i < m_modelDefine.totalDegree; i++)
	{
		/*cout << resultScore[i] << ", " << i << endl;*/
		if (resultScore[i] > 0 && resultScore[i] <= 1)
			if (resultScore[i] > tmpScore)
			{
				tmpScore = resultScore[i];
				tmpindex = i;
			}
	}

	rotation = tmpindex;

	double score = resultScore[tmpindex];

	result.location.x = tmpPoint[tmpindex].x;			// result coordinate X		
	result.location.y = tmpPoint[tmpindex].y;			// result coordinate Y
	if (roi.isEnable)
	{
		result.location.x += roi.rect.y;
		result.location.y += roi.rect.x;
	}
	result.score = score;
	result.rotation = m_modelDefine.degreeStart + m_modelDefine.rotationResolution * rotation;

	// free used resources and return score
	ReleaseDoubleMatrix(matGradMag, Ssize.height);
	cvReleaseMat(&Sdx);
	cvReleaseMat(&Sdy);
	delete[] tmpPoint;
	delete[]  resultScore;
	return score;
}

void NCC_ModelFinder::PyramidTestFlow(cv::Mat templateImage, cv::Mat srcImage)
{

}

// destructor
NCC_ModelFinder::~NCC_ModelFinder(void)
{

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
	for (int i = 0; i < m_modelDefine.noOfCordinates; i++)
	{
		point.y = m_modelDefine.cordinates[i].x + COG.x;
		point.x = m_modelDefine.cordinates[i].y + COG.y;
		cvLine(source, point, point, color, lineWidth);
	}
}

void NCC_ModelFinder::DrawContours(IplImage* source, CvPoint COG, CvScalar color, int lineWidth, int rotation)
{
	CvPoint point;
	point.y = COG.x;
	point.x = COG.y;
	for (int i = 0; i < m_modelDefine.noOfCordinates; i++)
	{
		point.y = m_modelDefine.cordinatesRotate[rotation][i].x + COG.x;
		point.x = m_modelDefine.cordinatesRotate[rotation][i].y + COG.y;
		cvLine(source, point, point, color, lineWidth);
	}
}

// draw contour at template image
void NCC_ModelFinder::DrawContours(IplImage* source, CvScalar color, int lineWidth)
{
	CvPoint point;
	for (int i = 0; i < m_modelDefine.noOfCordinates; i++)
	{
		point.y = m_modelDefine.cordinates[i].x + m_modelDefine.centerOfGravity.x;
		point.x = m_modelDefine.cordinates[i].y + m_modelDefine.centerOfGravity.y;
		cvLine(source, point, point, color, lineWidth);
	}
}

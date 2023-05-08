#include "pch.h"
#include "NCC_ModelFinder.h"
#include "opencv2413/include/opencv2/highgui/highgui_c.h"
#include <windows.h>
#include <stdio.h> 
#include <time.h>
#include <iostream>
#include <intrin.h> 
#include <vector>
using namespace std;
using namespace Cpp;
NCC_ModelFinder::NCC_ModelFinder(void)
{
}

void NCC_ModelFinder::SetModelPara(double r1, double r2, double resolution, double score)
{
	for (int i = 0; i < PrymidSize; i++) {
		this->m_modelDefine[i].degreeStart = r1;
		this->m_modelDefine[i].degreeEnd = r2;
		this->m_modelDefine[i].rotationResolution = resolution;
		this->m_modelDefine[i].searshScore = score;
	}

}

void NCC_ModelFinder::SetSobelThreshold(int high, int low)
{
	m_modelDefine[0].SobelHigh = high;
	m_modelDefine[0].SobelLow = low;


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

	int lowThreshold = m_modelDefine[0].SobelLow;		//deafult value
	int highThreashold = m_modelDefine[0].SobelHigh;	//deafult value

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
	IplImage* dst = 0;          //目標影像指標

	CvSize dst_cvsize;          //目標影像尺寸
	
	cout << " Shape model created.." << "with  Low Threshold = " << lowThreshold << " High Threshold = " << highThreashold << endl;
	
	for (int i = 0; i < PrymidSize; i++) {
		cout << "Pyramid " << i << endl;
		//CreatePyramidModel(i, m_modelDefine[0], m_modelDefine[i]);
		dst = 0;          //目標影像指標
		dst_cvsize.width = grayTemplateImg->width/pow(2,i);       //目標影像的寬為源影像寬的scale倍
		dst_cvsize.height = grayTemplateImg->height / pow(2, i); //
		dst = cvCreateImage(dst_cvsize, grayTemplateImg->depth, grayTemplateImg->nChannels);
		cvResize(grayTemplateImg, dst, CV_INTER_LINEAR);    //

		cout << "Pyramid imageDone " << i << "Done" << endl;
		if (i > 0)
			m_modelDefine[i].searshScore = m_modelDefine[i-1].searshScore*0.8;
		m_modelDefine[i].PyramidIdx = i;
		if (!CreateGeoMatchModel(dst, m_modelDefine[i], lowThreshold, highThreashold))
		{
			cout << "ERROR: could not create model...";
			return;
		}

		cout << "Pyramid " << i << "Done" <<endl;
		/*DrawContours(dst,m_modelDefine[i], CV_RGB(0, 255, 5), 1);
		string str = to_string(i);
		cvNamedWindow(str.c_str(), CV_WINDOW_AUTOSIZE);
		cvShowImage(str.c_str(), dst);
	*/
		cvReleaseImage(&dst);
	}



	//DrawContours(templateimage, m_modelDefine[0], CV_RGB(255, 0, 0), 1);
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
	for (int prymidIdx = 0; prymidIdx < this->PrymidSize; prymidIdx++) {
		if (m_modelDefine[prymidIdx].modelDefined)
		{
			try {
				x = x / pow(2, prymidIdx);
				y = y / pow(2, prymidIdx);
				eraserWidth = eraserWidth / pow(2, prymidIdx);
				int i = 0;
				int no = 0;
				int size = m_modelDefine[prymidIdx].noOfCordinates;
				int* xx = new int[m_modelDefine[prymidIdx].noOfCordinates];
				int* yy = new int[m_modelDefine[prymidIdx].noOfCordinates];
				double* ex = new double[m_modelDefine[prymidIdx].noOfCordinates];
				double* ey = new double[m_modelDefine[prymidIdx].noOfCordinates];
				double* m = new double[m_modelDefine[prymidIdx].noOfCordinates];

				fill(xx, xx + size, 0);
				fill(yy, yy + size, 0);
				fill(ex, ex + size, 0);
				fill(ey, ey + size, 0);
				fill(m, m + size, 0);



				for (i = 0; i < size; i++)
				{

					if (abs(m_modelDefine[prymidIdx].cordinates[i].x + m_modelDefine[prymidIdx].centerOfGravity.x - x) <= eraserWidth && abs(m_modelDefine[prymidIdx].cordinates[i].y + m_modelDefine[prymidIdx].centerOfGravity.y - y) <= eraserWidth) {

					}
					else
					{

						xx[no] = m_modelDefine[prymidIdx].cordinates[i].x;
						yy[no] = m_modelDefine[prymidIdx].cordinates[i].y;
						ex[no] = m_modelDefine[prymidIdx].edgeDerivativeX[i];
						ey[no] = m_modelDefine[prymidIdx].edgeDerivativeY[i];
						m[no] = m_modelDefine[prymidIdx].edgeMagnitude[i];
						++no;
					}

				}
				std::cout << "check roi" << std::endl;
				for (i = 0; i < no; ++i)
				{

					m_modelDefine[prymidIdx].cordinates[i].x = xx[i];
					m_modelDefine[prymidIdx].cordinates[i].y = yy[i];
					m_modelDefine[prymidIdx].edgeDerivativeX[i] = ex[i];
					m_modelDefine[prymidIdx].edgeDerivativeY[i] = ey[i];
					m_modelDefine[prymidIdx].edgeMagnitude[i] = m[i];
				}
				m_modelDefine[prymidIdx].noOfCordinates = no;
				std::cout << "no = " << no << std::endl;
		
				delete[] xx;
				delete[] yy;
				delete[] ex;
				delete[] ey;
				delete[] m;
				if (showCvImage) {
					CvSize dst_cvsize;          //目標影像尺寸
					IplImage* dst = 0;
					dst_cvsize.width = m_modelDefine[prymidIdx].modelWidth;       //目標影像的寬為源影像寬的scale倍
					dst_cvsize.height = m_modelDefine[prymidIdx].modelHeight; //
					dst = cvCreateImage(dst_cvsize, IPL_DEPTH_8U, 3);
					cvZero(dst);
					DrawContours(dst, m_modelDefine[prymidIdx], CV_RGB(0, 255, 5), 1);
					string str = to_string(prymidIdx);
					cvNamedWindow(str.c_str(), CV_WINDOW_AUTOSIZE);
					cvShowImage(str.c_str(), dst);
					cvReleaseImage(&dst);
				}

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
				CreateRotateModel(m_modelDefine[prymidIdx]);
			}

			catch (std::exception & ex)
			{
				std::cout << ex.what() << std::endl;
				getchar();
			}
		}
	}
}

void NCC_ModelFinder::ModelEraserMask(int x, int y, int eraserWidth)
{

}

void NCC_ModelFinder::CreateRotateModel()
{
	//m_modelDefine.ReleaseMatrix();
	//float r1 = m_modelDefine.degreeStart;
	//float r2 = m_modelDefine.degreeEnd;
	//float resolution = m_modelDefine.rotationResolution;
	//m_modelDefine.totalDegree = (r2 - r1) / m_modelDefine.rotationResolution + 1;
	//int count = 0;
	//m_modelDefine.cordinatesRotate = new CvPoint *[m_modelDefine.totalDegree];		//Coordinates array to store model points	
	//m_modelDefine.edgeDerivativeXRotate = new double*[m_modelDefine.totalDegree];	//gradient in X direction
	//m_modelDefine.edgeDerivativeYRotate = new double*[m_modelDefine.totalDegree]; 	//radient in Y direction	
	//for (int i = 0; i < m_modelDefine.totalDegree; i++)
	//{
	//	m_modelDefine.cordinatesRotate[i] = new CvPoint[m_modelDefine.noOfCordinates];
	//	m_modelDefine.edgeDerivativeXRotate[i] = new double[m_modelDefine.noOfCordinates];
	//	m_modelDefine.edgeDerivativeYRotate[i] = new double[m_modelDefine.noOfCordinates];
	//}
	//for (float degree = r1; degree < r2; degree += resolution)
	//{
	//	for (int i = 0; i < m_modelDefine.noOfCordinates; i++)
	//	{
	//		float thida = degree * CV_PI / 180.0;
	//		m_modelDefine.cordinatesRotate[count][i].x = (m_modelDefine.cordinates[i].x) * cos(thida) - (m_modelDefine.cordinates[i].y) * sin(thida);
	//		m_modelDefine.cordinatesRotate[count][i].y = (m_modelDefine.cordinates[i].x) * sin(thida) + (m_modelDefine.cordinates[i].y) * cos(thida);
	//		m_modelDefine.edgeDerivativeXRotate[count][i] = m_modelDefine.edgeDerivativeX[i] * cos(thida) - m_modelDefine.edgeDerivativeY[i] * sin(thida);
	//		m_modelDefine.edgeDerivativeYRotate[count][i] = m_modelDefine.edgeDerivativeX[i] * sin(thida) + m_modelDefine.edgeDerivativeY[i] * cos(thida);

	//	}
	//	count++;
	//}
	//m_modelDefine.modelDefined = true;
}

void NCC_ModelFinder::CreateRotateModel(Cpp::ModelDefine& model)
{
	model.ReleaseMatrix();
	float r1 = model.degreeStart;
	float r2 = model.degreeEnd;
	float l = sqrt(model.modelWidth * model.modelWidth + model.modelHeight * model.modelHeight);
	float resolution = acos(1 - 2 / (l * l));
	model.rotationResolution = resolution;
	model.totalDegree = (r2 - r1) / model.rotationResolution + 1;
	int count = 0;
	model.cordinatesRotate = new CvPoint * [model.totalDegree];		//Coordinates array to store model points	
	model.edgeDerivativeXRotate = new double* [model.totalDegree];	//gradient in X direction
	model.edgeDerivativeYRotate = new double* [model.totalDegree]; 	//radient in Y direction	
	for (int i = 0; i < model.totalDegree; i++)
	{
		model.cordinatesRotate[i] = new CvPoint[model.noOfCordinates];
		model.edgeDerivativeXRotate[i] = new double[model.noOfCordinates];
		model.edgeDerivativeYRotate[i] = new double[model.noOfCordinates];
	}
	for (int degree = 0; degree < model.totalDegree ; degree++)
	{
		for (int i = 0; i < model.noOfCordinates; i++)
		{
			//float thida = degree * CV_PI / 180.0;
			float thida = (r1 + count * resolution) * CV_PI / 180.0;
			model.cordinatesRotate[count][i].x = (model.cordinates[i].x) * cos(thida) - (model.cordinates[i].y) * sin(thida);
			model.cordinatesRotate[count][i].y = (model.cordinates[i].x) * sin(thida) + (model.cordinates[i].y) * cos(thida);
			model.edgeDerivativeXRotate[count][i] = model.edgeDerivativeX[i] * cos(thida) - model.edgeDerivativeY[i] * sin(thida);
			model.edgeDerivativeYRotate[count][i] = model.edgeDerivativeX[i] * sin(thida) + model.edgeDerivativeY[i] * cos(thida);

		}
		count++;
	}
	model.modelDefined = true;
}


void NCC_ModelFinder::ModelFind(cv::Mat mat)
{

	int lowThreshold = m_modelDefine[0].SobelLow;		//deafult value
	int highThreashold = m_modelDefine[0].SobelHigh;	//deafult value

	double minScore = m_modelDefine[0].searshScore;		//deafult value
	double greediness = 0.9;		//deafult value

	double total_time = 0;
	double score = 0;
	clock_t start_time1 = clock();

	IplImage* searchImage = new IplImage(mat);


	if (searchImage == NULL)
	{
		cout << "\nERROR: Could not load src Image.\n" << endl;
		return;
	}

	CvSize searchSize;
	searchSize = cvSize(searchImage->width, searchImage->height);
	IplImage* graySearchImg = cvCreateImage(searchSize, IPL_DEPTH_8U, 1);

	// Convert color image to gray image. 
	if (searchImage->nChannels == 3)
		cvCvtColor(searchImage, graySearchImg, CV_RGB2GRAY);
	else
	{
		cvCopy(searchImage, graySearchImg);
	}
	/*if (roi.isEnable)
	{
		if (roi.rect.x < 0)
			roi.rect.x = 0;
		if (roi.rect.y < 0)
			roi.rect.y;
		if (roi.rect.x + roi.rect.width > searchImage->width)
			roi.rect.width = searchImage->width - roi.rect.x;
		if (roi.rect.y + roi.rect.height > searchImage->height)
			roi.rect.height = searchImage->height - roi.rect.y;


		cvSetImageROI(graySearchImg, roi.rect);
	}
	else
	{
		cvResetImageROI(graySearchImg);

	}*/
	cout << " Finding Shape Model.." << " Minumum Score = " << minScore << " Greediness = " << greediness << "\n\n";
	cout << " ------------------------------------\n";

	double rotation = 0;
	int rotateStart = 0;
	double tempX = 0;
	double tempY = 0;

	int sizeOfPrymid = PrymidSize;
	if (roi.isEnable)
	{

		int rotateEnd = 0;

		double tempXEnd = 0;
		double tempYEnd = 0;


		for (int prymidIdx = sizeOfPrymid - 1; prymidIdx >= 0; prymidIdx--) {

			SearchROI tmproi = SearchROI();
			tmproi.isEnable = true;
			tmproi.rect.x = roi.rect.x / pow(2, prymidIdx);
			tmproi.rect.y = roi.rect.y / pow(2, prymidIdx);
			tmproi.rect.width = roi.rect.width / pow(2, prymidIdx);
			tmproi.rect.height = roi.rect.height / pow(2, prymidIdx);

			CvSize dst_cvsize;          //目標影像尺寸
			IplImage* dst = 0;          //目標影像指標
			dst_cvsize.width = graySearchImg->width / pow(2, prymidIdx);       //目標影像的寬為源影像寬的scale倍
			dst_cvsize.height = graySearchImg->height / pow(2, prymidIdx); //
			dst = cvCreateImage(dst_cvsize, graySearchImg->depth, graySearchImg->nChannels);

			if (prymidIdx == sizeOfPrymid - 1) {
				rotateEnd = (int)(abs(m_modelDefine[prymidIdx].degreeEnd - (m_modelDefine[prymidIdx].degreeStart)) / (m_modelDefine[prymidIdx].rotationResolution));
				tempXEnd = tmproi.rect.height;
				tempYEnd = tmproi.rect.width;
			}

			cvResize(graySearchImg, dst, CV_INTER_LINEAR);    //
		/*	cvNamedWindow("Search dst", CV_WINDOW_AUTOSIZE);
			cvShowImage("Search dst", dst);*/
			cvSetImageROI(dst, tmproi.rect);
			/*cvNamedWindow("Search roi dst", CV_WINDOW_AUTOSIZE);
			cvShowImage("Search roi dst", dst);*/

			cout << "total degree " << m_modelDefine[prymidIdx].totalDegree << endl;;
			score = FindGeoMatchModelRotateParallelSSE(dst, m_modelDefine[prymidIdx], tmproi, tempX, tempY, tempXEnd, tempYEnd, rotateStart, rotateEnd, minScore, greediness, rotation);
			//if (score > minScore)
			{
				clock_t finish_time1 = clock();
				total_time = (double)(finish_time1 - start_time1) / CLOCKS_PER_SEC;
				cout << " Found little image at [" << result.location.x << ", " << result.location.y << ", Rotation " << result.rotation << "]\n Score = " << score << "\n Searching Time = " << total_time * 1000 << "ms" << endl;
				if (roi.isEnable)
				{
					if (roi.rect.x < 0)
						roi.rect.x = 0;
					if (roi.rect.y < 0)
						roi.rect.y;
					if (roi.rect.x + roi.rect.width > searchImage->width)
						roi.rect.width = searchImage->width - roi.rect.x;
					if (roi.rect.y + roi.rect.height > searchImage->height)
						roi.rect.height = searchImage->height - roi.rect.y;


					//cvSetImageROI(graySearchImg, roi.rect);
				}
				else
				{
					cvResetImageROI(graySearchImg);

				}
				if (prymidIdx > 0) {
				
					int RangeStart = rotation - ((prymidIdx+1) / m_modelDefine[prymidIdx].rotationResolution);
					RangeStart = (RangeStart < 0) ? 0 : RangeStart;
						
					int RangeEnd = rotation + ((prymidIdx +1)/ m_modelDefine[prymidIdx].rotationResolution);
					RangeEnd = (RangeEnd > m_modelDefine[prymidIdx].totalDegree-1) ? m_modelDefine[prymidIdx].totalDegree-1 : RangeEnd;

					rotateStart = (int)(m_modelDefine[prymidIdx].rotationResolution * (RangeStart) / m_modelDefine[prymidIdx - 1].rotationResolution);
					rotateEnd = (int)(m_modelDefine[prymidIdx].rotationResolution * (RangeEnd) / m_modelDefine[prymidIdx - 1].rotationResolution);

					//cout << "r1 " << RangeStart << "," << RangeEnd << " rotation " << rotation << " roate new " << rotateStart << " rotate new end " << rotateEnd << endl;
				/*	tempX = (result.location.x) * 2 - 20;
					tempY = (result.location.y) * 2 -20;
					tempXEnd = tempX + 40;
					tempYEnd = tempY + 40;
					cout << "roix " << tempX << endl;
					cout << "roiy " << tempY << endl;
					cout << "result x " << tempXEnd << endl;
					cout << "result y " << tempYEnd << endl;*/
					tempX = (result.location.x - tmproi.rect.y) * 2 -15;
					tempY = (result.location.y - tmproi.rect.x) * 2 - 15;
					tempYEnd = tempY + 30;
					tempXEnd = tempX + 30;;
				}
			
			}
			/*else
			{
				break;
			}*/
			cvReleaseImage(&dst);
		}
	}
	else
	{
		cvResetImageROI(graySearchImg);
	}

	//score = FindGeoMatchModelRotateParallel(graySearchImg, minScore, greediness, rotation);

	clock_t finish_time1 = clock();
	total_time = (double)(finish_time1 - start_time1) / CLOCKS_PER_SEC;

	if (score > minScore) // if score is atleast 0.4
	{
		result.isFind = true;
		cout << " Found at [" << result.location.x << ", " << result.location.y << ", Rotation " << result.rotation << "]\n Score = " << score << "\n Searching Time = " << total_time * 1000 << "ms";

		DrawContours(searchImage, result.location, CV_RGB(0, 255, 0), 1, (int)rotation);
		cvResetImageROI(graySearchImg);
		if (showCvImage)
		{

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


int NCC_ModelFinder::CreateGeoMatchModel(const void* templateArr, Cpp::ModelDefine& model, double maxContrast, double minContrast)
{
	if (model.modelDefined)
		model.Release();
	//double resolution = model.rotationResolution;

	
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
	model.modelHeight = src->height;		//Save Template height
	model.modelWidth = src->width;			//Save Template width

	model.noOfCordinates = 0;											//initialize	
	model.cordinates = new CvPoint[model.modelWidth * model.modelHeight];		//Allocate memory for coorinates of selected points in template image
	model.edgeMagnitude = new double[model.modelWidth * model.modelHeight];		//Allocate memory for edge magnitude for selected points
	model.edgeDerivativeX = new double[model.modelWidth * model.modelHeight];			//Allocate memory for edge X derivative for selected points
	model.edgeDerivativeY = new double[model.modelWidth * model.modelHeight];			////Allocate memory for edge Y derivative for selected points

	float l = sqrt(src->width * src->width + src->height * src->height);
	float resolution = acos(1 - 2 / (l * l));
	model.rotationResolution = resolution;
	double r1 = model.degreeStart;
	double r2 = model.degreeEnd;

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
		for (j = 1; j < Ssize.width - 1; j++)
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

					model.cordinates[model.noOfCordinates].x = curX;
					model.cordinates[model.noOfCordinates].y = curY;
					model.edgeDerivativeX[model.noOfCordinates] = fdx;
					model.edgeDerivativeY[model.noOfCordinates] = fdy;

					//handle divide by zero
					if (MagG != 0)
						model.edgeMagnitude[model.noOfCordinates] = 1 / MagG;  // gradient magnitude 
					else
						model.edgeMagnitude[model.noOfCordinates] = 0;

					model.noOfCordinates++;
				}
			}
		}
	}
	model.centerOfGravity.x = RSum / model.noOfCordinates; // center of gravity
	model.centerOfGravity.y = CSum / model.noOfCordinates;	// center of gravity
	// change coordinates to reflect center of gravity
	for (int m = 0; m < model.noOfCordinates; m++)
	{
		int temp;

		temp = model.cordinates[m].x;
		model.cordinates[m].x = temp - model.centerOfGravity.x;
		temp = model.cordinates[m].y;
		model.cordinates[m].y = temp - model.centerOfGravity.y;
	}

	model.totalDegree = (r2 - r1) / resolution + 1;
	count = 0;
	model.cordinatesRotate = new CvPoint * [model.totalDegree];		//Coordinates array to store model points	
	model.edgeDerivativeXRotate = new double* [model.totalDegree];	//gradient in X direction
	model.edgeDerivativeYRotate = new double* [model.totalDegree]; 	//radient in Y direction	
	for (int i = 0; i < model.totalDegree; i++)
	{
		model.cordinatesRotate[i] = new CvPoint[model.noOfCordinates];
		model.edgeDerivativeXRotate[i] = new double[model.noOfCordinates];
		model.edgeDerivativeYRotate[i] = new double[model.noOfCordinates];
	}
	for (int degree = 0; degree < model.totalDegree-1; degree ++)
	{
		for (int i = 0; i < model.noOfCordinates; i++)
		{
			//float thida = degree * CV_PI / 180.0;
			float thida = (r1 + count * resolution) * CV_PI / 180.0;
			model.cordinatesRotate[count][i].x = (model.cordinates[i].x) * cos(thida) - (model.cordinates[i].y) * sin(thida);
			model.cordinatesRotate[count][i].y = (model.cordinates[i].x) * sin(thida) + (model.cordinates[i].y) * cos(thida);
			model.edgeDerivativeXRotate[count][i] = model.edgeDerivativeX[i] * cos(thida) - model.edgeDerivativeY[i] * sin(thida);
			model.edgeDerivativeYRotate[count][i] = model.edgeDerivativeX[i] * sin(thida) + model.edgeDerivativeY[i] * cos(thida);

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
	model.modelDefined = true;
	return 1;
}

double NCC_ModelFinder::FindGeoMatchModelRotateParallelSSE(const void* srcarr, Cpp::ModelDefine& model, SearchROI tmpRoi, int x, int y, int xend, int yend, int rotateStart, int rotateEnd, double minScore, double greediness, double& rotation)
{
	//CvMat* Sdx = 0, * Sdy = 0;
	CvMat** Sd = new CvMat * [2];
	int i, j;			// count variables

	double** matGradMag;  //Gradient magnitude matrix

	CvMat srcstub, * src = (CvMat*)srcarr;
	src = cvGetMat(src, &srcstub);
	if (CV_MAT_TYPE(src->type) != CV_8UC1 || !model.modelDefined)
	{
		return 0;
	}
	double* resultScore = new double[model.totalDegree];
	CvPoint* tmpPoint = new CvPoint[model.totalDegree];

	for (int i = 0; i < model.totalDegree; i++)
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

	//Sdx = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1); // X derivatives
	//Sdy = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1); // y derivatives

	//cvSobel(src, Sdx, 1, 0, 3);  // find X derivatives
	//cvSobel(src, Sdy, 0, 1, 3); // find Y derivatives

#pragma omp parallel for
	for (i = 0; i < 2; i++) {
		Sd[i] = cvCreateMat(Ssize.height, Ssize.width, CV_16SC1);
		if (i == 0)
		{
			cvSobel(src, Sd[i], 1, 0, 3);  // find X derivatives
		}
		else
		{
			cvSobel(src, Sd[i], 0, 1, 3); // find Y derivatives
		}
	}

#pragma omp barrier
	// stoping criterias to search for model
	double normMinScore = minScore / model.noOfCordinates; // precompute minumum score 
	double normGreediness = ((1 - greediness * minScore) / (1 - greediness)) / model.noOfCordinates; // precompute greedniness 
	bool isFound = false;

#pragma omp parallel for
	for (i = 0; i < Ssize.height; i++)
	{
		const short* _Sdx;
		const short* _Sdy;
		//double iSx, iSy;

		_Sdx = (short*)(Sd[0]->data.ptr + Sd[0]->step * (i));
		_Sdy = (short*)(Sd[1]->data.ptr + Sd[1]->step * (i));

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
	/*int height = Ssize.height;
	int width = Ssize.width;*/

	int height = xend;
	int width = yend;
	int degreeStart = rotateStart;
	int degreeEnd = rotateEnd;

	// cout << "iterateion X = " << (height - x) << " Y =" << (width - y) << " R =" << (degreeEnd - degreeStart);
#pragma omp parallel for
	for (i = x; i < height; i++)
	{
		if (isFound)
			continue;
#pragma omp parallel for

		for (j = y; j < width; j++)
		{
			if (isFound)
				continue;
#pragma omp parallel for
			for (int degree = degreeStart; degree < degreeEnd; degree++)
			{
				/*if (isFound)
					continue;*/
				double sumOfCoords = 0;
				double partialScore = 0;
				double partialSum = 0; // initilize partialSum measure
				const short* _Sdx;
				const short* _Sdy;

				double iTx, iTy, iSx, iSy;
				double gradMag;
				int curX, curY;
				uchar* sd0Data = Sd[0]->data.ptr;
				uchar* sd1Data = Sd[1]->data.ptr;
				int sd0Step = Sd[0]->step;
				int sd1Step = Sd[1]->step;
				for (int m = 0; m < model.noOfCordinates; m++)
				{
					/*		__m128 m1, m2, m3, m4;

							__m128* pSrc1 = (__m128*) pArray1;
							__m128* pSrc2 = (__m128*) pArray2;
							__m128* pDest = (__m128*) pResult;*/
							//	std::cout << degree << " " << m << endl;
					curX = i + model.cordinatesRotate[degree][m].x;	// template X coordinate
					curY = j + model.cordinatesRotate[degree][m].y; // template Y coordinate
					iTx = model.edgeDerivativeXRotate[degree][m];	// template X derivative
					iTy = model.edgeDerivativeYRotate[degree][m];    // template Y derivative


					if (curX<0 || curY<0 || curX>Ssize.height - 1 || curY>Ssize.width - 1)
						continue;

					_Sdx = (short*)(sd0Data + sd0Step * (curX));
					_Sdy = (short*)(sd1Data + sd1Step * (curX));

					iSx = _Sdx[curY]; // get curresponding  X derivative from source image
					iSy = _Sdy[curY];// get curresponding  Y derivative from source image

					if ((iSx != 0 || iSy != 0) && (iTx != 0 || iTy != 0))
					{
						//partial Sum  = Sum of(((Source X derivative* Template X drivative) + Source Y derivative * Template Y derivative)) / Edge magnitude of(Template)* edge magnitude of(Source))
						partialSum = partialSum + ((iSx * iTx) + (iSy * iTy)) * (model.edgeMagnitude[m] * matGradMag[curX][curY]);

					}

					sumOfCoords = m + 1;
					partialScore = partialSum / sumOfCoords;
					// check termination criteria
					// if partial score score is less than the score than needed to make the required score at that position
					// break serching at that coordinate.
					if (partialScore < (MIN((minScore - 1) + normGreediness * sumOfCoords, normMinScore * sumOfCoords)))
						break;

				}
				//if (model.PyramidIdx != 0)
				//{
					if (partialScore > resultScore[degree])
					{
						resultScore[degree] = partialScore; //  Match score
						tmpPoint[degree].x = i;
						tmpPoint[degree].y = j;
					}
				//}
				//else {
				//	if (partialScore > minScore)
				//	{
				//		resultScore[degree] = partialScore; //  Match score
				//		tmpPoint[degree].x = i;
				//		tmpPoint[degree].y = j;
				//		isFound = true;
				//	}
				//}
			}
		}
	}
	//);
	int tmpindex = 0;
	double tmpScore = 0;
	for (int i = 0; i < model.totalDegree; i++)
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
	if (tmpRoi.isEnable)
	{
		result.location.x += tmpRoi.rect.y;
		result.location.y += tmpRoi.rect.x;
	}
	result.score = score;
	result.rotation = model.degreeStart + model.rotationResolution * rotation;

	// free used resources and return score
	ReleaseDoubleMatrix(matGradMag, Ssize.height);
	cvReleaseMat(&Sd[0]);
	cvReleaseMat(&Sd[1]);
	delete []Sd;
	delete[] tmpPoint;
	delete[]  resultScore;
	return score;
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
	delete[] matrix;
}


// draw contours around result image
void NCC_ModelFinder::DrawContours(IplImage* source, CvPoint COG, CvScalar color, int lineWidth)
{
	CvPoint point;
	point.y = COG.x;
	point.x = COG.y;
	for (int i = 0; i < m_modelDefine[0].noOfCordinates; i++)
	{
		point.y = m_modelDefine[0].cordinates[i].x + COG.x;
		point.x = m_modelDefine[0].cordinates[i].y + COG.y;
		cvLine(source, point, point, color, lineWidth);
	}
}

void NCC_ModelFinder::DrawContours(IplImage* source, CvPoint COG, CvScalar color, int lineWidth, int rotation)
{
	CvPoint point;
	point.y = COG.x;
	point.x = COG.y;
	for (int i = 0; i < m_modelDefine[0].noOfCordinates; i++)
	{
		point.y = m_modelDefine[0].cordinatesRotate[rotation][i].x + COG.x;
		point.x = m_modelDefine[0].cordinatesRotate[rotation][i].y + COG.y;
		cvLine(source, point, point, color, lineWidth);
	}
}

// draw contour at template image
void NCC_ModelFinder::DrawContours(IplImage* source, CvScalar color, int lineWidth)
{
	CvPoint point;
	for (int i = 0; i < m_modelDefine[0].noOfCordinates; i++)
	{
		point.y = m_modelDefine[0].cordinates[i].x + m_modelDefine[0].centerOfGravity.x;
		point.x = m_modelDefine[0].cordinates[i].y + m_modelDefine[0].centerOfGravity.y;
		cvLine(source, point, point, color, lineWidth);
	}
}

void NCC_ModelFinder::DrawContours(IplImage* source, ModelDefine& model, CvScalar color, int lineWidth)
{
	CvPoint point;
	for (int i = 0; i < model.noOfCordinates; i++)
	{
		point.y = model.cordinates[i].x + model.centerOfGravity.x;
		point.x = model.cordinates[i].y + model.centerOfGravity.y;
		cvLine(source, point, point, color, lineWidth);
	}
}

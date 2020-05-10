// ModelFinder.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include <iostream>
#include <cstdio>
//#include "opencv2/opencv.hpp"
#include "opencv2\opencv.hpp"
#include <vector>
#include <ctime>
#include "NCC_ModelFinder.h"
using namespace cv;
using namespace std;
void onMoue(int Event, int x, int y, int flags, void* param);
void InitModel();
int main()
{

	InitModel();
	waitKey();

	return 0;
}
void onMouse(int Event, int x, int y, int flags, void* param) {
	if (Event == CV_EVENT_LBUTTONDOWN) {
		/*	VertexLeftTop.x = x;
			VertexLeftTop.y = y;*/
		cout << "x=" << x << endl;
		cout << "y=" << y << endl;
	}
	if (Event == CV_EVENT_LBUTTONUP) {
		/*VertexRightDown.x = x;
		VertexRightDown.y = y;*/
	}
}

void InitModel()
{
	NCC_ModelFinder GM;				// object to implent geometric matching	
	int lowThreshold = 100;		//deafult value
	int highThreashold = 150;	//deafult value

	double minScore = 0.7;		//deafult value
	double greediness = 0.8;		//deafult value

	double total_time = 0;
	double score = 0;
	CvPoint result;
	
<<<<<<< HEAD
	IplImage* templateimage = cvLoadImage("template1.jpg", -1);
	IplImage* searchImage = cvLoadImage("image4.jpg", -1);
=======
	IplImage* templateimage = cvLoadImage("template.jpg", -1);
	IplImage* searchImage = cvLoadImage("test.jpg", -1);
>>>>>>> 880293493ef1cf486fa1caa68036363057718032
	if (searchImage == NULL)
	{
		cout << "\nERROR: Could not load src Image.\n" << endl;
		return;
	}
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

	if (!GM.CreateGeoMatchModel(grayTemplateImg, lowThreshold, highThreashold,-5,5))
	{
		cout << "ERROR: could not create model...";
		return;
	}
	GM.DrawContours(templateimage, CV_RGB(255, 0, 0), 1);
	cout << " Shape model created.." << "with  Low Threshold = " << lowThreshold << " High Threshold = " << highThreashold << endl;
	CvSize searchSize = cvSize(searchImage->width, searchImage->height);
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
	clock_t start_time1 = clock();
	
	//score = GM.FindGeoMatchModel(graySearchImg, minScore, greediness, &result);
	score = GM.FindGeoMatchModelRotateParallel(graySearchImg, minScore, greediness, &result, rotation);
	//score = GM.FindGeoMatchModelRotate(graySearchImg, minScore, greediness, &result, rotation);
	clock_t finish_time1 = clock();
	total_time = (double)(finish_time1 - start_time1) / CLOCKS_PER_SEC;

	if (score > minScore) // if score is atleast 0.4
	{
		cout << " Found at [" << result.x << ", " << result.y << "]\n Score = " << score << "\n Searching Time = " << total_time * 1000 << "ms";
		GM.DrawContours(searchImage, result, CV_RGB(0, 255, 0), 1, (int)rotation);
	}
	else
		cout << " Object Not found";

	cout << "\n ------------------------------------\n\n";
	cout << "\n Press any key to exit!";

	//Display result
	cvNamedWindow("Template", CV_WINDOW_AUTOSIZE);
	cvShowImage("Template", templateimage);
	cvNamedWindow("Search Image", CV_WINDOW_AUTOSIZE);
	cvShowImage("Search Image", searchImage);
	// wait for both windows to be closed before releasing images
	cvWaitKey(0);
	cvDestroyWindow("Search Image");
	cvDestroyWindow("Template");
	cvReleaseImage(&searchImage);
	cvReleaseImage(&graySearchImg);
	cvReleaseImage(&templateimage);
	cvReleaseImage(&grayTemplateImg);
}
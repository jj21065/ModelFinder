// ModelFinder.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include <iostream>
#include <cstdio>
//#include "opencv2/opencv.hpp"
#include "opencv2\opencv.hpp"
#include <vector>
#include "KdTree.h"
#include "ICP.h"
#include "fftm.hpp"
#include "NCC_ModelFinder.h"
using namespace cv;
using namespace std;
void onMoue(int Event, int x, int y, int flags, void* param);
void InitModel();
int main()
{

	//std::cout << "Hello World!\n";
	//Mat src = imread("canny_1.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	//GaussianBlur(src, src, Size(3, 3), 0, 0);
	//Mat dst1, dst2;
	//Canny(src, dst1, 50, 150, 3);

	//int width1 = 100;
	//int height1 = 70;
	//int x1 = 100;
	//int y1 = 70;
	//int width2 = 0;
	//int height2 = 0;
	//int x2 = 170;
	//int y2 = 140;


	//Rect rect(100, 70, 100, 70);
	//Mat image_roi;
	//dst1(rect).copyTo(image_roi);

	//Rect rect2(0, 0, 200, 200);
	//Mat Find_ROI;
	//


	//Mat displayImg = src.clone();
	//Mat result;
	//result.create(dst1.rows - image_roi.rows + 1, dst1.cols - image_roi.cols + 1, CV_32FC1);


	//double angle =-5;  //button increments its click by 1 per click
	//Mat transform_m = getRotationMatrix2D(Point(dst1.cols / 2, dst1.rows / 2), angle, 1);  //Creating rotation matrix
	//Mat current_frame;
	//warpAffine(dst1, current_frame, transform_m, Size(dst1.cols, dst1.rows));
	//transform(dst1, current_frame, transform_m);  //Transforming captured image into a new one
	//current_frame(rect2).copyTo(Find_ROI);
	//current_frame.copyTo(displayImg);  //Copy resized to original

	//double minVal;
	//double maxVal;
	//Point minLoc;
	//Point maxLoc;
	//double tryAngle =0;
	//Mat tmpMat;
	//int count = 0;
	//float min = 1e8;
	//while (1)
	//{
	//	
	//	Mat search_m = getRotationMatrix2D(Point(image_roi.cols / 2, image_roi.rows / 2), tryAngle, 1);
	//
	//	warpAffine(image_roi, tmpMat, search_m, Size(image_roi.cols, image_roi.rows));
	//	matchTemplate(Find_ROI, tmpMat, result, CV_TM_SQDIFF_NORMED);
	//	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
	//	minMaxLoc(result, &minVal, 0, &minLoc, 0);
	//	
	//	cout << "minVal " << minVal << endl;
	//	if (minVal < min)
	//		min = minVal;
	//	else if (minVal > min && minVal < 0.17)
	//	{
	//
	//		break;
	//	}
	//	
	//		
	//		tryAngle=tryAngle + 0.1;
	//	
	//	if (count >= 200)
	//		break;
	//	count++;

	//}
	//cout << "degree " << tryAngle << endl;
	//cout << "minVal " << minVal << endl;
	//for (int i = 0; i < tmpMat.cols; i++)
	//{
	//	uchar* data = image_roi.ptr<uchar>(i);
	//	for (int j = 0; j < tmpMat.rows; j++)
	//	{
	//		if ((int)tmpMat.at<uchar>(j, i) > 0)
	//		{
	//			current_frame.at<uchar>(j + minLoc.y, i + minLoc.x) = 255;//tmpMat.at<uchar>(j, i);
	//		}
	//	}

	//}


	//rectangle(displayImg, minLoc, Point(minLoc.x + image_roi.cols, minLoc.y + image_roi.rows), Scalar::all(0), 3);
	///*imshow("origin", src);*/
	//imshow("roi", image_roi);
	//imshow("tmp", tmpMat);
	//imshow("result2", current_frame);
	//waitKey(0);


	//vector<gs::Point*> b;
	//vector<gs::Point*> target;


	//float err = 0;
	//gs::icp(b, target, err);
	//for (int i = 0; i < b.size(); i++)
	//{
	//	/*if (b[i]->pos[1] < 70 && b[i]->pos[0] < 100)*/
	//	dst1.at<uchar>((-b[i]->pos[1]) / 10, (b[i]->pos[0]) / 10) = 150;


	//}
	//imshow("Find", Find_ROI);


	//cout << "error = " << err << endl;
	//threshold(dst1, dst2, 128, 255, THRESH_BINARY_INV);  //反轉影像，讓邊緣呈現黑線
	//imshow("origin", src);
	//imshow("Canny_1", dst1);
	//setMouseCallback("Canny_1", onMouse, NULL);
	///*imshow("Canny_2", dst2);*/
	//waitKey(0);

	//for (int i = 0; i < b.size(); i++)
	//{
	//	delete b[i];

	//}
	//for (int i = 0; i < target.size(); i++)
	//{
	//	delete target[i];
	//}
	//



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
	
	IplImage* templateimage = cvLoadImage("template.jpg", -1);
	IplImage* searchImage = cvLoadImage("test.jpg", -1);
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
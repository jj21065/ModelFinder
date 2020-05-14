#include "pch.h"
#include <iostream>
#include "cvModelFinder.h"
#include "C:/opencv/build/include/opencv2/highgui/highgui_c.h"

using namespace CLI;
using namespace System;
using namespace System::Drawing;
int main()
{
	
	ModelFinder^ modelFinder = gcnew ModelFinder();
	Bitmap^ image = safe_cast<Bitmap^>(Image::FromFile(".//template1.jpg"));
	Bitmap^ image2 = safe_cast<Bitmap^>(Image::FromFile(".//image3.jpg"));

	modelFinder->SetModelPara(0, 15, 0.05, 0.5);
	modelFinder->CreateModelFromImage(image);

	int high = 140; 
	int low = 100;
	//modelFinder->SetROI(1230, 1400, 200, 300);
	modelFinder->SetSobelThreshold(170, 210);
	modelFinder->SetROI(120, 120, 300, 300);

	while(1)
	{
		char c = cvWaitKey(4);
		if (c == 's')
		{
			Bitmap^ image2 = safe_cast<Bitmap^>(Image::FromFile(".//image3.jpg"));
			std::cout << "S" << std::endl;
			modelFinder->showCvImage = true;

			modelFinder->ModelFind(image2);
			std::cout <<std::endl << "ModelFind at (" << modelFinder->searchResult->X << "< " << modelFinder->searchResult->Y << ")"<<std::endl;
		}
		if (c == '+')
		{
			Bitmap^ image = safe_cast<Bitmap^>(Image::FromFile(".//template1.jpg"));
			
			high += 10;
			low += 10;
			std::cout << std::endl << "high" << std::endl;
			modelFinder->SetSobelThreshold(high, low);
			modelFinder->CreateModelFromImage(image);
			
		}
		if (c == '-')
		{
			Bitmap^ image = safe_cast<Bitmap^>(Image::FromFile(".//template1.jpg"));
			high-= 10;
			low -= 10;
		
			std::cout << std::endl << "low" << std::endl;
			modelFinder->SetSobelThreshold(high, low);
			modelFinder->CreateModelFromImage(image);
		
		}
		if (c == '1')
		{
			Bitmap^ image = safe_cast<Bitmap^>(Image::FromFile(".//template1.jpg"));
			modelFinder->EraseModel(43, 89, 55);
			modelFinder->DrawModel(image);
		}
	}

}

ModelFinder::ModelFinder()
{
	modelFinder = new Cpp::NCC_ModelFinder();
	searchResult = gcnew SearchResult();
	roi = gcnew SearchROI() ;
	roi->point.X = 0;
	roi->point.Y = 0;
	

}

ModelFinder::~ModelFinder()
{
	if (modelFinder!=nullptr)
		delete modelFinder;
}

void ModelFinder::SetModelPara(double r1, double r2, double resolution,double score)
{
	modelFinder->SetModelPara(r1, r2, resolution, score);
}

void ModelFinder::SetSobelThreshold(int high, int low)
{
	modelFinder->SetSobelThreshold(high, low);
}
void ModelFinder::DisableROI()
{
	modelFinder->DisableROI();
}

void ModelFinder::CreateModelFromImage(Bitmap^ _image)
{
	modelFinder->showCvImage = this->showCvImage;
	//System::Drawing::Rectangle blank = System::Drawing::Rectangle(0, 0, _image->Width, _image->Height);
	//System::Drawing::Imaging::BitmapData^ bmpdata = _image->LockBits(blank, System::Drawing::Imaging::ImageLockMode::ReadWrite, System::Drawing::Imaging::PixelFormat::Format32bppArgb);
	//
	////cv::Mat thisimage(cv::Size(_image->Width, _image->Height), CV_8UC3, bmpdata->Scan0.ToPointer(), cv::Mat::AUTO_STEP);
	//cv::Mat thisimage = cv::Mat(cv::Size(_image->Width, _image->Height), CV_8UC3, (char*)bmpdata->Scan0.ToPointer());
	//_image->UnlockBits(bmpdata);

	//鎖定Bitmap數據
	cv::Mat thisimage;
	System::Drawing::Imaging::BitmapData^ bmpData = _image->LockBits(
		System::Drawing::Rectangle(0, 0, _image->Width, _image->Height),
		System::Drawing::Imaging::ImageLockMode::ReadWrite, _image->PixelFormat);
	int wb = ((_image->Width * 24 + 31) / 32) * 4;
		//將 bmpImg 的數據指針復制到 cvImg 中，不拷貝數據
	if (_image->PixelFormat == System::Drawing::Imaging::PixelFormat::Format8bppIndexed)	// 灰度圖像
	{
		thisimage = cv::Mat(_image->Height, _image->Width, CV_8UC1, (char*)bmpData->Scan0.ToPointer(), wb);
	}
	else if (_image->PixelFormat == System::Drawing::Imaging::PixelFormat::Format24bppRgb)	// 彩色圖像
	{
		thisimage = cv::Mat(_image->Height, _image->Width, CV_8UC3, (char*)bmpData->Scan0.ToPointer(), wb);
	}

	//解鎖Bitmap數據
	_image->UnlockBits(bmpData);

	delete bmpData;
	
	modelFinder->CreatModel(thisimage);
}

void ModelFinder::ModelFind(Bitmap^ _image)
{
	modelFinder->showCvImage = this->showCvImage;
	cv::Mat thisimage;
	System::Drawing::Imaging::BitmapData^ bmpData = _image->LockBits(
		System::Drawing::Rectangle(0, 0, _image->Width, _image->Height),
		System::Drawing::Imaging::ImageLockMode::ReadWrite, _image->PixelFormat);
	int wb = ((_image->Width * 24 + 31) / 32) * 4;
	//將 bmpImg 的數據指針復制到 cvImg 中，不拷貝數據
	if (_image->PixelFormat == System::Drawing::Imaging::PixelFormat::Format8bppIndexed)	// 灰度圖像
	{
		thisimage = cv::Mat(_image->Height, _image->Width, CV_8UC1, (char*)bmpData->Scan0.ToPointer(), wb);
	}
	else if (_image->PixelFormat == System::Drawing::Imaging::PixelFormat::Format24bppRgb)	// 彩色圖像
	{
		thisimage = cv::Mat(_image->Height, _image->Width, CV_8UC3, (char*)bmpData->Scan0.ToPointer(), wb);
	}
	_image->UnlockBits(bmpData);
	delete bmpData;

	modelFinder->ModelFind(thisimage);
	if (modelFinder->result.isFind)
	{
		searchResult->Y = modelFinder->result.location.x;
		searchResult->X = modelFinder->result.location.y;
		searchResult->Score = modelFinder->result.score;
		searchResult->Rotation = modelFinder->result.rotation;
	}
}

void ModelFinder::EraseModel(int x, int y, int width)
{
	modelFinder->ModelEraser(x, y, width);
}
void ModelFinder::DrawModel(Bitmap^ _image)
{
	modelFinder->showCvImage = this->showCvImage;
	cv::Mat thisimage;
	System::Drawing::Imaging::BitmapData^ bmpData = _image->LockBits(
		System::Drawing::Rectangle(0, 0, _image->Width, _image->Height),
		System::Drawing::Imaging::ImageLockMode::ReadWrite, _image->PixelFormat);
	int wb = ((_image->Width * 24 + 31) / 32) * 4;
	//將 bmpImg 的數據指針復制到 cvImg 中，不拷貝數據
	if (_image->PixelFormat == System::Drawing::Imaging::PixelFormat::Format8bppIndexed)	// 灰度圖像
	{
		thisimage = cv::Mat(_image->Height, _image->Width, CV_8UC1, (char*)bmpData->Scan0.ToPointer(), wb);
	}
	else if (_image->PixelFormat == System::Drawing::Imaging::PixelFormat::Format24bppRgb)	// 彩色圖像
	{
		thisimage = cv::Mat(_image->Height, _image->Width, CV_8UC3, (char*)bmpData->Scan0.ToPointer(), wb);
	}
	_image->UnlockBits(bmpData);
	delete bmpData;

	IplImage* model = new IplImage(thisimage);
	modelFinder->DrawContours(model, CV_RGB(0, 255, 0), 1);
	cvNamedWindow("Search Image", CV_WINDOW_AUTOSIZE);
	cvShowImage("Search Image", model);
	
	delete model;
}
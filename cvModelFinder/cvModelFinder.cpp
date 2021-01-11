#include "pch.h"
#include <iostream>
#include "cvModelFinder.h"
#include <opencv2/highgui/highgui_c.h>

using namespace CLI;
using namespace System;
using namespace System::Drawing;
int main()
{
	
	ModelFinder^ modelFinder = gcnew ModelFinder();
	Bitmap^ image = safe_cast<Bitmap^>(Image::FromFile(".//template1.jpg"));
	Bitmap^ image2 = safe_cast<Bitmap^>(Image::FromFile(".//Test_Golden.tif"));

	modelFinder->SetModelPara(-5, 5, 0.051, 0.5);
	modelFinder->CreateModelFromImage(image);

	/*modelFinder->SerializeModel("template.mod");
	modelFinder->DeSerializeModel("template.mod");*/
	int high = 140; 
	int low = 100;
	modelFinder->SetROI(1100, 850, 700, 700);
	modelFinder->SetSobelThreshold(170, 210);
//	modelFinder->SetROI(100, 100, 400, 300);
	modelFinder->showCvImage = true;

	modelFinder->ModelFind(image2);
	while(1)
	{
		char c = cvWaitKey(4);
		if (c == 's')
		{
			std::cout << "S" << std::endl;
			modelFinder->showCvImage = false;
			Bitmap^ image2 = safe_cast<Bitmap^>(Image::FromFile(".//Test_Golden.tif"));
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
	}

}

ModelFinder::ModelFinder()
{
	modelFinder = new Cpp::NCC_ModelFinder();
	searchResult = gcnew SearchResult();
	roi = gcnew SearchROI();
	roi->point.X = 0;
	roi->point.Y = 0;


}

ModelFinder::~ModelFinder()
{
	if (modelFinder != nullptr)
		delete modelFinder;
}

void ModelFinder::SetModelPara(double r1, double r2, double resolution, double score)
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
		thisimage = cv::Mat(_image->Height, _image->Width, CV_8UC1, (char*)bmpData->Scan0.ToPointer(), bmpData->Stride);
	}
	else if (_image->PixelFormat == System::Drawing::Imaging::PixelFormat::Format24bppRgb)	// 彩色圖像
	{
		thisimage = cv::Mat(_image->Height, _image->Width, CV_8UC3, (char*)bmpData->Scan0.ToPointer(), bmpData->Stride);
	}

	//解鎖Bitmap數據
	_image->UnlockBits(bmpData);

	delete bmpData;

	modelFinder->CreatModel(thisimage);
}

void ModelFinder::CreateRotateModel()
{
	modelFinder->CreateRotateModel();
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
		thisimage = cv::Mat(_image->Height, _image->Width, CV_8UC1, (char*)bmpData->Scan0.ToPointer(), bmpData->Stride);
	}
	else if (_image->PixelFormat == System::Drawing::Imaging::PixelFormat::Format24bppRgb)	// 彩色圖像
	{
		thisimage = cv::Mat(_image->Height, _image->Width, CV_8UC3, (char*)bmpData->Scan0.ToPointer(), bmpData->Stride);
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
		searchResult->Result = true;
	}
	else
	{
		searchResult->Y = 0;
		searchResult->X = 0;
		searchResult->Score = 0;
		searchResult->Rotation = 0;
		searchResult->Result = false;
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
		thisimage = cv::Mat(_image->Height, _image->Width, CV_8UC1, (char*)bmpData->Scan0.ToPointer(), bmpData->Stride);
	}
	else if (_image->PixelFormat == System::Drawing::Imaging::PixelFormat::Format24bppRgb)	// 彩色圖像
	{
		thisimage = cv::Mat(_image->Height, _image->Width, CV_8UC3, (char*)bmpData->Scan0.ToPointer(), bmpData->Stride);
	}
	_image->UnlockBits(bmpData);
	delete bmpData;

	IplImage* model = new IplImage(thisimage);
	modelFinder->DrawContours(model, CV_RGB(0, 255, 0), 1);
	if (modelFinder->showCvImage)
	{
		cvNamedWindow("Search Image", CV_WINDOW_AUTOSIZE);
		cvShowImage("Search Image", model);
	}

	delete model;
}

void ModelFinder::SerializeModel(System::String^ filename)
{
	modelDefine = gcnew ModelDefineDotNet();
	/*modelDefine->searshScore = modelFinder->m_modelDefine.searshScore;
	modelDefine->noOfCordinates = modelFinder->m_modelDefine.noOfCordinates;
	modelDefine->modelHeight = modelFinder->m_modelDefine.modelHeight;
	modelDefine->modelWidth = modelFinder->m_modelDefine.modelWidth;
	modelDefine->totalDegree = modelFinder->m_modelDefine.totalDegree;
	modelDefine->modelDefined = modelFinder->m_modelDefine.modelDefined;
	modelDefine->degreeStart = modelFinder->m_modelDefine.degreeStart;
	modelDefine->degreeEnd = modelFinder->m_modelDefine.degreeEnd;
	modelDefine->rotationResolution = modelFinder->m_modelDefine.rotationResolution;
	modelDefine->SobelLow = modelFinder->m_modelDefine.SobelLow;
	modelDefine->SobelHigh = modelFinder->m_modelDefine.SobelHigh;

	modelDefine->centerOfGravityX = modelFinder->m_modelDefine.centerOfGravity.x;
	modelDefine->centerOfGravityY = modelFinder->m_modelDefine.centerOfGravity.y;

	modelDefine->cordinatesX = gcnew array<int>(modelDefine->modelWidth*modelDefine->modelHeight);
	modelDefine->cordinatesY = gcnew array<int>(modelDefine->modelWidth*modelDefine->modelHeight);
	modelDefine->edgeMagnitude = gcnew array<double>(modelDefine->modelWidth*modelDefine->modelHeight);
	modelDefine->edgeDerivativeX = gcnew array<double>(modelDefine->modelWidth*modelDefine->modelHeight);
	modelDefine->edgeDerivativeY = gcnew array<double>(modelDefine->modelWidth*modelDefine->modelHeight);

	for (int i = 0; i < modelDefine->noOfCordinates; i++)
	{
		modelDefine->cordinatesX[i] = modelFinder->m_modelDefine.cordinates[i].x;
		modelDefine->cordinatesY[i] = modelFinder->m_modelDefine.cordinates[i].y;
		modelDefine->edgeMagnitude[i] = modelFinder->m_modelDefine.edgeMagnitude[i];
		modelDefine->edgeDerivativeX[i] = modelFinder->m_modelDefine.edgeDerivativeX[i];
		modelDefine->edgeDerivativeY[i] = modelFinder->m_modelDefine.edgeDerivativeY[i];
	}*/

	SerializeProject::SaveSerialize(modelDefine, filename);
}

void ModelFinder::DeSerializeModel(System::String^ filename)
{
	modelDefine = SerializeProject::LoadDeserialize(filename);

	/*modelFinder->m_modelDefine.searshScore = modelDefine->searshScore;
	modelFinder->m_modelDefine.noOfCordinates = modelDefine->noOfCordinates;
	modelFinder->m_modelDefine.modelHeight = modelDefine->modelHeight;
	modelFinder->m_modelDefine.modelWidth = modelDefine->modelWidth;
	modelFinder->m_modelDefine.totalDegree = modelDefine->totalDegree;
	modelFinder->m_modelDefine.modelDefined = modelDefine->modelDefined;
	modelFinder->m_modelDefine.degreeStart = modelDefine->degreeStart;
	modelFinder->m_modelDefine.degreeEnd = modelDefine->degreeEnd;
	modelFinder->m_modelDefine.rotationResolution = modelDefine->rotationResolution;
	modelFinder->m_modelDefine.SobelLow = modelDefine->SobelLow;
	modelFinder->m_modelDefine.SobelHigh = modelDefine->SobelHigh;

	modelFinder->m_modelDefine.centerOfGravity.x = modelDefine->centerOfGravityX;
	modelFinder->m_modelDefine.centerOfGravity.y = modelDefine->centerOfGravityY;

	modelFinder->m_modelDefine.Release();

	modelFinder->m_modelDefine.cordinates = new CvPoint[modelDefine->modelWidth*modelDefine->modelHeight];
	modelFinder->m_modelDefine.edgeMagnitude = new double[modelDefine->modelWidth*modelDefine->modelHeight];
	modelFinder->m_modelDefine.edgeDerivativeX = new double[modelDefine->modelWidth*modelDefine->modelHeight];
	modelFinder->m_modelDefine.edgeDerivativeY = new double[modelDefine->modelWidth*modelDefine->modelHeight];

	for (int i = 0; i < modelDefine->noOfCordinates; i++)
	{
		modelFinder->m_modelDefine.cordinates[i].x = modelDefine->cordinatesX[i];
		modelFinder->m_modelDefine.cordinates[i].y = modelDefine->cordinatesY[i];
		modelFinder->m_modelDefine.edgeMagnitude[i]=modelDefine->edgeMagnitude[i];
		modelFinder->m_modelDefine.edgeDerivativeX[i] = modelDefine->edgeDerivativeX[i];
		modelFinder->m_modelDefine.edgeDerivativeY[i] = modelDefine->edgeDerivativeY[i];
	}*/

	modelFinder->CreateRotateModel();
	

}
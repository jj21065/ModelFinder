#include "pch.h"
#include <iostream>
#include "cvModelFinder.h"
#include <opencv2/highgui/highgui_c.h>

using namespace CLI;
using namespace System;
using namespace System::Drawing;
int main()
{
	//System::String^ filename = "D://work/Git/ModelFinder/ModelFinder_master/photo/CoAxisLight/black_No_Co.bmp";
	System::String^ filename = "D://work/Git/ModelFinder/ModelFinder_master/cvModelFinder/Capture/Test2.tif";
	//System::String^ golden = "D://work/Git/ModelFinder/ModelFinder_master/photo/CoAxisLight//golden.jpg";
	System::String^ golden = "D://work/Git/ModelFinder/ModelFinder_master/cvModelFinder/Capture/template1.jpg";
	ModelFinder^ modelFinder = gcnew ModelFinder();
	//Bitmap^ image = safe_cast<Bitmap^>(Image::FromFile("D://work/Git/ModelFinder/ModelFinder_master/cvModelFinder/Capture/template1.jpg"));
	Bitmap^ image2 = safe_cast<Bitmap^>(Image::FromFile(filename));
	Bitmap^ image = safe_cast<Bitmap^>(Image::FromFile(golden));

	modelFinder->SetModelPara(-15, 15, 0.051, 0.75);
	modelFinder->CreateModelFromImage(image);

	/*modelFinder->SerializeModel("template.mod");
	modelFinder->DeSerializeModel("template.mod");*/
	int high = 150; 
	int low = 120;
	modelFinder->SetROI(750, 550, 500, 500);
	modelFinder->SetSobelThreshold(low, high);
//	modelFinder->SetROI(100, 100, 400, 300);
	modelFinder->showCvImage = true;

	//modelFinder->ModelFind(image2);

	int eraserWidth = 20;
	int x = 31;
	int y = 30;
	while(1)
	{
		char c = cvWaitKey(4);
		if (c == 'm')
		{
			modelFinder->SerializeModel(".//testModel.mod");
		}
		if (c == ',')
		{
			modelFinder->DeSerializeModel(".//testModel.mod");
		}
		if (c == 's')
		{
			std::cout << "S" << std::endl;
			modelFinder->showCvImage = true;
			Bitmap^ image2 = safe_cast<Bitmap^>(Image::FromFile(filename));
			modelFinder->ModelFind(image2);
			std::cout <<std::endl << "ModelFind at (" << modelFinder->searchResult->X << "< " << modelFinder->searchResult->Y << ")"<<std::endl;
		}
		if (c == '+')
		{
			Bitmap^ image = safe_cast<Bitmap^>(Image::FromFile(filename));
			high += 10;
			low += 10;
			std::cout << std::endl << "high" << std::endl;
			modelFinder->SetSobelThreshold(high, low);
			modelFinder->CreateModelFromImage(image);
			
		}
		if (c == '-')
		{
			Bitmap^ image = safe_cast<Bitmap^>(Image::FromFile(golden));
			high-= 10;
			low -= 10;
			std::cout << std::endl << "low" << std::endl;
			modelFinder->SetSobelThreshold(high, low);
			modelFinder->CreateModelFromImage(image);
		
		}
		if (c == '5')
		{
			x -= 2;
			modelFinder->EraseModel(x, y, eraserWidth);
		}
		if (c == '2')
		{
			x += 2;
			modelFinder->EraseModel(x, y, eraserWidth);
		}
		if (c == '1')
		{
			y -= 2;
			modelFinder->EraseModel(x, y, eraserWidth);
		}
		if (c == '3')
		{
			y += 2;
			modelFinder->EraseModel(x, y, eraserWidth);
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
	int size = modelFinder->PrymidSize;
	modelDefine = gcnew ModelDefinDotNet();
	modelDefine->models = gcnew array<ModelDefinePyramidDotNet^>(size);
	for (int p = 0; p < size; p++) {
		modelDefine->models[p] = gcnew ModelDefinePyramidDotNet();
		modelDefine->models[p]->searshScore = modelFinder->m_modelDefine[p].searshScore;
		modelDefine->models[p]->noOfCordinates = modelFinder->m_modelDefine[p].noOfCordinates;
		modelDefine->models[p]->modelHeight = modelFinder->m_modelDefine[p].modelHeight;
		modelDefine->models[p]->modelWidth = modelFinder->m_modelDefine[p].modelWidth;
		modelDefine->models[p]->totalDegree = modelFinder->m_modelDefine[p].totalDegree;
		modelDefine->models[p]->modelDefined = modelFinder->m_modelDefine[p].modelDefined;
		modelDefine->models[p]->degreeStart = modelFinder->m_modelDefine[p].degreeStart;
		modelDefine->models[p]->degreeEnd = modelFinder->m_modelDefine[p].degreeEnd;
		modelDefine->models[p]->rotationResolution = modelFinder->m_modelDefine[p].rotationResolution;
		modelDefine->models[p]->SobelLow = modelFinder->m_modelDefine[p].SobelLow;
		modelDefine->models[p]->SobelHigh = modelFinder->m_modelDefine[p].SobelHigh;

		modelDefine->models[p]->centerOfGravityX = modelFinder->m_modelDefine[p].centerOfGravity.x;
		modelDefine->models[p]->centerOfGravityY = modelFinder->m_modelDefine[p].centerOfGravity.y;

		modelDefine->models[p]->cordinatesX = gcnew array<int>(modelDefine->models[p]->modelWidth * modelDefine->models[p]->modelHeight);
		modelDefine->models[p]->cordinatesY = gcnew array<int>(modelDefine->models[p]->modelWidth * modelDefine->models[p]->modelHeight);
		modelDefine->models[p]->edgeMagnitude = gcnew array<double>(modelDefine->models[p]->modelWidth * modelDefine->models[p]->modelHeight);
		modelDefine->models[p]->edgeDerivativeX = gcnew array<double>(modelDefine->models[p]->modelWidth * modelDefine->models[p]->modelHeight);
		modelDefine->models[p]->edgeDerivativeY = gcnew array<double>(modelDefine->models[p]->modelWidth * modelDefine->models[p]->modelHeight);

		for (int i = 0; i < modelDefine->models[p]->noOfCordinates; i++)
		{
			modelDefine->models[p]->cordinatesX[i] = modelFinder->m_modelDefine[p].cordinates[i].x;
			modelDefine->models[p]->cordinatesY[i] = modelFinder->m_modelDefine[p].cordinates[i].y;
			modelDefine->models[p]->edgeMagnitude[i] = modelFinder->m_modelDefine[p].edgeMagnitude[i];
			modelDefine->models[p]->edgeDerivativeX[i] = modelFinder->m_modelDefine[p].edgeDerivativeX[i];
			modelDefine->models[p]->edgeDerivativeY[i] = modelFinder->m_modelDefine[p].edgeDerivativeY[i];
		}
	}
	SerializeProject::SaveSerialize(modelDefine, filename);
}

void ModelFinder::DeSerializeModel(System::String^ filename)
{
	try {
		std::cout << "Start DeSerialize..";
		modelDefine = SerializeProject::LoadDeserialize(filename);
		int size = modelFinder->PrymidSize;
		for (int p = 0; p < size; p++) {
			
			modelFinder->m_modelDefine[p].searshScore = modelDefine->models[p]->searshScore;
			modelFinder->m_modelDefine[p].noOfCordinates = modelDefine->models[p]->noOfCordinates;
			modelFinder->m_modelDefine[p].modelHeight = modelDefine->models[p]->modelHeight;
			modelFinder->m_modelDefine[p].modelWidth = modelDefine->models[p]->modelWidth;
			modelFinder->m_modelDefine[p].totalDegree = modelDefine->models[p]->totalDegree;
			modelFinder->m_modelDefine[p].modelDefined = modelDefine->models[p]->modelDefined;
			modelFinder->m_modelDefine[p].degreeStart = modelDefine->models[p]->degreeStart;
			modelFinder->m_modelDefine[p].degreeEnd = modelDefine->models[p]->degreeEnd;
			modelFinder->m_modelDefine[p].rotationResolution = modelDefine->models[p]->rotationResolution;
			modelFinder->m_modelDefine[p].SobelLow = modelDefine->models[p]->SobelLow;
			modelFinder->m_modelDefine[p].SobelHigh = modelDefine->models[p]->SobelHigh;

			modelFinder->m_modelDefine[p].centerOfGravity.x = modelDefine->models[p]->centerOfGravityX;
			modelFinder->m_modelDefine[p].centerOfGravity.y = modelDefine->models[p]->centerOfGravityY;

			modelFinder->m_modelDefine[p].Release();

			modelFinder->m_modelDefine[p].cordinates = new CvPoint[modelDefine->models[p]->modelWidth * modelDefine->models[p]->modelHeight];
			modelFinder->m_modelDefine[p].edgeMagnitude = new double[modelDefine->models[p]->modelWidth * modelDefine->models[p]->modelHeight];
			modelFinder->m_modelDefine[p].edgeDerivativeX = new double[modelDefine->models[p]->modelWidth * modelDefine->models[p]->modelHeight];
			modelFinder->m_modelDefine[p].edgeDerivativeY = new double[modelDefine->models[p]->modelWidth * modelDefine->models[p]->modelHeight];

			for (int i = 0; i < modelDefine->models[p]->noOfCordinates; i++)
			{
				modelFinder->m_modelDefine[p].cordinates[i].x = modelDefine->models[p]->cordinatesX[i];
				modelFinder->m_modelDefine[p].cordinates[i].y = modelDefine->models[p]->cordinatesY[i];
				modelFinder->m_modelDefine[p].edgeMagnitude[i] = modelDefine->models[p]->edgeMagnitude[i];
				modelFinder->m_modelDefine[p].edgeDerivativeX[i] = modelDefine->models[p]->edgeDerivativeX[i];
				modelFinder->m_modelDefine[p].edgeDerivativeY[i] = modelDefine->models[p]->edgeDerivativeY[i];


			}
			modelFinder->CreateRotateModel(modelFinder->m_modelDefine[p]);
		}
	}
	catch (IOException^ ex)
	{
		throw ex;
	
	}
}
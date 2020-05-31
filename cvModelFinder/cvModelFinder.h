#pragma once
#include "NCC_ModelFinder.h"
using namespace System;
using namespace System::IO;
using namespace System::Drawing;
using namespace System::Runtime::Serialization::Formatters::Binary;
namespace CLI {
	public ref class SearchResult
	{
	public:
		double X;
		double Y;
		double Rotation;
		double scale = 1;
		double Score = 0;
		bool Result = false;

	};
	public ref class SearchROI
	{
	public:
		PointF point;
		double width;
		double height;
	};

	[Serializable]
	public ref class ModelDefineDotNet
	{
	public:
		double searshScore;
		int				noOfCordinates;		//Number of elements in coordinate array
		array<int>^ cordinatesX;		//Coordinates array to store model points	
		array<int>^ cordinatesY;		//Coordinates array to store model points	
		int				modelHeight;		//Template height
		int				modelWidth;			//Template width

		array<double>^edgeMagnitude;		//gradient magnitude
		array<double>^ edgeDerivativeX;	//gradient in X direction
		array<double>^ edgeDerivativeY;	//radient in Y direction	


		int totalDegree = 0;
		int centerOfGravityX;	//Center of gravity of template 
		int centerOfGravityY;	//Center of gravity of template 

		bool			modelDefined;
		double degreeStart, degreeEnd;
		double rotationResolution;

		int SobelLow = 100;
		int SobelHigh = 100;
	};

	public class SerializeProject
	{

	public:
		static System::String^ SaveSerialize(Object^ o, System::String^ filepath)
		{
			FileStream^ myFileStream = gcnew FileStream(filepath, FileMode::Create, FileAccess::ReadWrite);
			BinaryFormatter^ myBFormatter = gcnew BinaryFormatter();

			myBFormatter->Serialize(myFileStream, o);
			//Console.Wri1eLine("序列化完成，資料儲存於文字文件 sbinary.txt …….");
			myFileStream->Close();
			//XmlSerializer ser = new XmlSerializer(o.GetType());
			//StringBuilder sb = new StringBuilder();
			//StringWriter writer = new StringWriter(sb);
			//ser.Serialize(writer, o);
			return filepath;
		}

		static ModelDefineDotNet^ LoadDeserialize(System::String^ s)
		{
			ModelDefineDotNet^ o;
			//try
			//{

			FileStream^ myFileStream = gcnew FileStream(s, FileMode::Open, FileAccess::Read);
			BinaryFormatter^ myBFormatter = gcnew BinaryFormatter();
			//Console.WriteLine("開始還原序列化物件…….");
			o = (ModelDefineDotNet^)myBFormatter->Deserialize(myFileStream);
			//Console.WriteLine("物件完成還原…….");
			myFileStream->Close();
			//}
			//catch 
			//{
			//	//MessageBox.Show("Can't Find file!");
			//	//o = new DataFormat();
			//}
			return o;
		}


	};

	public ref class ModelFinder
	{
		// TODO: 請在此新增此類別的方法。
	private:
		//	Cpp::NCC_ModelFinder* modelFinder = nullptr;

	public:
		Cpp::NCC_ModelFinder* modelFinder = nullptr;
		SearchResult^ searchResult;
		SearchROI^ roi;
		ModelDefineDotNet^ modelDefine;
		ModelFinder();
		~ModelFinder();
		void SetROI(SearchROI ^_roi)
		{
			roi->point.X = _roi->point.X;
			roi->point.Y = _roi->point.Y;
			roi->width = _roi->width;
			roi->height = _roi->height;

			modelFinder->SetROI(roi->point.X, roi->point.Y, roi->width, roi->height);
		}
		void SetROI(double x, double y, double width, double height)
		{
			roi->point.X = x;
			roi->point.Y = y;
			roi->width = width;
			roi->height = height;
			modelFinder->SetROI(roi->point.X, roi->point.Y, roi->width, roi->height);
		}
		void DisableROI();
		void CreateModelFromImage(Bitmap^ _image);
		void ModelFind(Bitmap^ _image);
		void SetModelPara(double r1, double r2, double resolution, double score);
		void SetSobelThreshold(int high, int low);
		void EraseModel(int x, int y, int width);
		void DrawModel(Bitmap^ _image);
		void CreateRotateModel();
		void SerializeModel(System::String^ filename);
		void DeSerializeModel(System::String^ filename);
		bool showCvImage = true;
	};


}

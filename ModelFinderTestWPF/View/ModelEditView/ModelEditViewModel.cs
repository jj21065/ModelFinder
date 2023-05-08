using Emgu.CV;
using Emgu.CV.Structure;
using ModelFinder.Events;

using ModelFinder.Model.ImageProcess;
using ImageView;
using Microsoft.Win32;
using Prism.Commands;
using Prism.Events;
using Prism.Mvvm;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;
using CLI;

namespace ModelFinder.ViewModel
{
    public class ModelEditViewModel : BindableBase
    {
        private ImageProcessControl imageProcessControl;
        private ImageControlViewModel imageControlVM;
        private CLI.ModelFinder modelFinder = null;
        private IEventAggregator eventAggregator;
        private Mat loadImage;
        private Image<Bgr, byte> imgLoadImage;
        private int maskSize = 20;
        public ModelEditViewModel(ImageProcessControl imageProcessControl, ImageControlViewModel imageControlVM, IEventAggregator eventAggregator)
        {
            modelFinder = new CLI.ModelFinder();
            modelFinder.showCvImage = false;
            this.imageProcessControl = imageProcessControl;
            this.imageControlVM = imageControlVM;
            imageControlVM.IsShowSearchROI = false;
            imageControlVM.IsModifySearchingROI = false;
            imageControlVM.IsShowEllpiseROI = false;
            imageControlVM.IsModifyEllipseROI = false;
            this.eventAggregator = eventAggregator;
           
        }
        private int addShapeListBoxSelectedIndex = -1;
        public int AddShapeListBoxSelectedIndex
        {
            get => addShapeListBoxSelectedIndex;
            set
            {
                SetProperty(ref addShapeListBoxSelectedIndex, value);
                switch (AddShapeListBoxSelectedIndex)
                {
                    case -1:
                        imageControlVM.IsShowEllpiseROI = false;
                        imageControlVM.IsModifyEllipseROI = false;
                        imageControlVM.IsShowSearchROI = false;
                        imageControlVM.IsModifySearchingROI = false;
                        imageControlVM.IsShowMask = false;
                        break;
                    case 0:
                        imageControlVM.IsShowEllpiseROI = false;
                        imageControlVM.IsModifyEllipseROI = false;
                        imageControlVM.IsShowSearchROI = false;
                        imageControlVM.IsModifySearchingROI = false;
                        imageControlVM.IsShowMask = false;
                        break;
                    case 1:
                        imageControlVM.IsShowEllpiseROI = false;
                        imageControlVM.IsModifyEllipseROI = false;
                        imageControlVM.IsShowSearchROI = true;
                        imageControlVM.IsModifySearchingROI = true;
                        imageControlVM.IsShowMask = false;
                        break;
                    case 2:
                        imageControlVM.IsShowEllpiseROI = true;
                        imageControlVM.IsModifyEllipseROI = false;
                        imageControlVM.IsShowSearchROI = false;
                        imageControlVM.IsModifySearchingROI = false;
                        imageControlVM.IsShowMask = false;
                        break;
                    case 3:
                        imageControlVM.IsShowEllpiseROI = false;
                        imageControlVM.IsModifyEllipseROI = false;
                        imageControlVM.IsShowSearchROI = false;
                        imageControlVM.IsModifySearchingROI = false;
                        imageControlVM.IsShowMask = true;
                        break;
                }

                if (AddShapeListBoxSelectedIndex == 0)
                {
                    imageControlVM.StartAddingPointCurve();
                }
                else
                {
                    imageControlVM.StopAddingPointCurve();
                }
            }
        }

        private int thresholdLow = 90;
        public int ThresholdLow { get => thresholdLow; set => SetProperty(ref thresholdLow, value); }

        private int thresholdHigh = 100;
        public int ThresholdHigh { get => thresholdHigh; set => SetProperty(ref thresholdHigh, value); }

        private double rotationNegDegree = -3;
        public double RotationNegDegree { get => rotationNegDegree; set { CanUpdateModel = true; SetProperty(ref rotationNegDegree, value); } }

        private double rotationPosDegree = 3;
        public double RotationPosDegree { get => rotationPosDegree; set { CanUpdateModel = true; SetProperty(ref rotationPosDegree, value); } }

        private double rotationResolution = 0.05;
        public double RotationResolution { get => rotationResolution; set { CanUpdateModel = true; SetProperty(ref rotationResolution, value); } }

        private double modelSearchScore = 0.6;
        public double ModelSearchScore { get => modelSearchScore; set { CanUpdateModel = true; SetProperty(ref modelSearchScore, value); } }

        private bool canUpdateModel = false;
        public bool  CanUpdateModel { get => canUpdateModel; set => SetProperty(ref canUpdateModel, value); }

        public int MaskSize { get => maskSize; set { SetProperty(ref maskSize, value); eventAggregator.GetEvent<ModelEraserEvent>().Publish(MaskSize); } }

        private DelegateCommand loadImageCommand;
        public DelegateCommand LoadImageCommand
        {
            get
            {
                if (loadImageCommand == null)
                {
                    loadImageCommand = new DelegateCommand(() =>
                    {
                        var imagePath = string.Empty;
                        var openFileDialog = new Microsoft.Win32.OpenFileDialog();
                        if (true == openFileDialog.ShowDialog())
                        {
                            imagePath = openFileDialog.FileName;
                        }

                        if (!imageProcessControl.LoadImage(imagePath, ref loadImage))
                        {
                            return;
                        }
                       // CvInvoke.Resize(loadImage, loadImage, new System.Drawing.Size(loadImage.Width/8, loadImage.Height/8));
                        imgLoadImage = loadImage.ToImage<Bgr, byte>();
                        imageControlVM.CanvasHeight = loadImage.Height;
                        imageControlVM.CanvasWidth = loadImage.Width;
                        imageControlVM.ImageSource = imageProcessControl.ConvertImgToBitmapSource(imgLoadImage);
                    });
                }
                return loadImageCommand;
            }
        }

        private DelegateCommand saveModelCommand;
        public DelegateCommand SaveModelCommand
        {
            get
            {
                if (saveModelCommand == null)
                {
                    saveModelCommand = new DelegateCommand(() =>
                    {
                        SaveFileDialog saveFileDialog1 = new SaveFileDialog();
                        saveFileDialog1.Filter = "model(*.mod)|*.mod";
                        saveFileDialog1.Title = "Save an model File";
                        saveFileDialog1.InitialDirectory = @"C:\Automation\Gen4_Gluing\Models";
                        saveFileDialog1.AddExtension = true;
                        if (true == saveFileDialog1.ShowDialog())
                        {
                            // If the file name is not an empty string open it for saving.
                            if (saveFileDialog1.FileName != "")
                            {
                                 modelFinder.SerializeModel(saveFileDialog1.FileName);
                            }
                        }
                    
                    });
                }
                return saveModelCommand;
            }
        }

        private DelegateCommand loadModelCommand;
        public DelegateCommand LoadModelCommand
        {
            get
            {
                if (loadModelCommand == null)
                {
                    loadModelCommand = new DelegateCommand(() =>
                    {
                        var modelPath = string.Empty;
                        var openFileDialog = new Microsoft.Win32.OpenFileDialog();
                        openFileDialog.Filter = "model(*.mod)|*.mod|All files(*.*)|*.";
                        openFileDialog.InitialDirectory = @"C:\Automation\Gen4_Gluing\Models";
                        if (true == openFileDialog.ShowDialog())
                        {
                            modelPath = openFileDialog.FileName;
                        }

                      
                        try
                        {
                             modelFinder.DeSerializeModel(modelPath);
                            int height =  modelFinder.modelDefine.models[0].modelHeight;
                            int width =  modelFinder.modelDefine.models[0].modelWidth;

                            loadImage = Mat.Zeros(height, width, Emgu.CV.CvEnum.DepthType.Cv8U, 1);
                            imgLoadImage = loadImage.ToImage<Bgr, byte>();
                            imageControlVM.CanvasHeight = loadImage.Height;
                            imageControlVM.CanvasWidth = loadImage.Width;
                          
                             modelFinder.DrawModel(imgLoadImage.Bitmap);
                            imageControlVM.ImageSource = imageProcessControl.ConvertImgToBitmapSource(imgLoadImage);
                        
                        }catch(Exception ex)
                        {
                            Console.WriteLine(ex.ToString());
                        }
                    });
                  
                }
                return loadModelCommand;
            }
        }

        private DelegateCommand findModelCommand;
        public DelegateCommand FindModelCommand
        {
            get
            {
                if (findModelCommand == null)
                {
                    findModelCommand = new DelegateCommand(() =>
                    {
                        imgLoadImage = new Image<Bgr,byte>(loadImage.Bitmap);

                        if (imageControlVM.IsShowSearchROI)
                        {
                             modelFinder.SetROI(imageControlVM.RectRoi.X, imageControlVM.RectRoi.Y, imageControlVM.RectRoi.W, imageControlVM.RectRoi.H);
                             modelFinder.ModelFind(imgLoadImage.Bitmap);
                        }

                        imageControlVM.ImageSource = imageProcessControl.ConvertImgToBitmapSource(imgLoadImage);
                    });
                }
                return findModelCommand;
            }
        }

        private DelegateCommand sobelThresholdCommand;
        public DelegateCommand SobelThresholdCommand
        {
            get
            {
                if (sobelThresholdCommand == null)
                {
                    sobelThresholdCommand = new DelegateCommand(() =>
                    {
                         modelFinder.SetSobelThreshold(thresholdHigh, thresholdLow);
                        
                        if (imageControlVM.IsShowSearchROI)
                        {
                            Mat roi = new Mat(loadImage, new System.Drawing.Rectangle(imageControlVM.RectRoi.X, imageControlVM.RectRoi.Y, imageControlVM.RectRoi.W, imageControlVM.RectRoi.H));
                            loadImage = roi;
                            imgLoadImage = roi.ToImage<Bgr, byte>();
                        }
                        else
                        {
                            imgLoadImage = new Image<Bgr, byte>(loadImage.Bitmap);
                        }
                         modelFinder.SetModelPara(rotationNegDegree, rotationPosDegree, rotationResolution, modelSearchScore);
                         modelFinder.CreateModelFromImage(imgLoadImage.Bitmap);
                        // modelFinder.DrawModel(imgLoadImage.Bitmap);
                        imageControlVM.CanvasHeight = imgLoadImage.Height;
                        imageControlVM.CanvasWidth = imgLoadImage.Width;
                        imageControlVM.ImageSource = imageProcessControl.ConvertImgToBitmapSource(imgLoadImage);
                    });
                }
                return sobelThresholdCommand;
            }
        }

        private DelegateCommand updateModelParaCommand;
        public DelegateCommand UpdateModelParaCommand
        {
            get
            {
                if (updateModelParaCommand == null)
                {
                    updateModelParaCommand = new DelegateCommand(() =>
                    {
                        Console.WriteLine("update");
                        if ( modelFinder.modelDefine.models[0].modelDefined)
                        {
                            Console.WriteLine("update");
                             modelFinder.SetModelPara(rotationNegDegree, rotationPosDegree, rotationResolution, modelSearchScore);
                             modelFinder.CreateRotateModel();
                            Console.WriteLine("update");
                        }
                    });
                }
                return updateModelParaCommand;
            }
        }
        

        public void MouseMoveEvent(int x,int y , System.Windows.Input.MouseButtonState buttonState )
        {
            lock ("")
            {
                if (imageControlVM.IsShowMask)
                {
                    if (buttonState == System.Windows.Input.MouseButtonState.Pressed)
                    {
                         modelFinder.showCvImage = false;
                       
                        imgLoadImage = new Image<Bgr, byte>(loadImage.Bitmap);
                         modelFinder.EraseModel(y, x, MaskSize/2);
                        Console.WriteLine("XY={0},{1}", y, x);
                         modelFinder.DrawModel(imgLoadImage.Bitmap);
                        imageControlVM.ImageSource = imageProcessControl.ConvertImgToBitmapSource(imgLoadImage);
                        imgLoadImage.Dispose();
                    }
                }
            }
        }

        public void SetMaskSize( int size)
        {
            MaskSize = size;
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using Prism.Mvvm;
using System.Windows.Media.Imaging;
using Prism.Events;
using ModelFinder.Events;
using System.Windows.Media;
using System.Collections.ObjectModel;
using ModelFinder.View.ImageView;
using Emgu.CV;
using ModelFinder.Model.ImageProcess;

namespace ImageView
{
    public class ImageControlViewModel : BindableBase
    {
        private bool isSearchROI;
        private bool isShowSearchROI;
        private ROIRectangleModel rectRoi;
        //private ROIRectangleModel shapeRoi;
        private ROIRectangleModel ellipseRoi;
        private bool isModifySearchingROI;
        private Visibility searchingROIVisibility;
        private IEventAggregator eventAggregator;
        public Mat ImageMatToShow;
        
        private ImageProcessControl imageProcessControl;

        public Point CircleRoiCenterPoint;
        public double CircleRoiRadius;
        public double CircleRoiStartAngle;
        public double CircleRoiEndAngle;
       
        public  ImageControlViewModel(ImageProcessControl imageProcessContro, IEventAggregator eventAggregator)
        {
            rectRoi = new ROIRectangleModel { X = 200, Y = 200, W = 200, H = 200 };
            ellipseRoi = new ROIRectangleModel { X = 200, Y = 200, W = 100, H = 100 };
            this.eventAggregator = eventAggregator;
            this.imageProcessControl = imageProcessContro;
            eventAggregator.GetEvent<ImageControlViewCallbackEvent>().Subscribe(OnReceiveAddPointEvent);
            eventAggregator.GetEvent<ModelEraserEvent>().Subscribe(SetMaskSize);
         
        }       

        public Visibility SearchingROIVisibility
        {
            get => searchingROIVisibility;
            set => SetProperty(ref searchingROIVisibility, value);
        }


        private Visibility ellipseROIVisibility;
        public Visibility EllipseROIVisibility
        {
            get => ellipseROIVisibility;
            set => SetProperty(ref ellipseROIVisibility, value);
        }

        private Visibility maskROIVisibility = Visibility.Collapsed;
        public Visibility MaskROIVisibility
        {
            get => maskROIVisibility;
            set => SetProperty(ref maskROIVisibility, value);
        }

        public ROIRectangleModel RectRoi
        {
            get => rectRoi;
            set => SetProperty(ref rectRoi, value);
        }

        //public ROIRectangleModel ShapeRoi
        //{
        //    get => shapeRoi;
        //    set => SetProperty(ref shapeRoi, value);
        //}

        public ROIRectangleModel EllipseRoi
        {
            get => ellipseRoi;
            set => SetProperty(ref ellipseRoi, value);
        }

        private int mousex;
        public int MouseX { get=>mousex; set=>SetProperty(ref mousex,value); }
        private int mousey;
        public int MouseY { get => mousey; set => SetProperty(ref mousey, value); }

        private int maskx = 0;
        private int masky = 0;
        public int MaskX { get => maskx; set => SetProperty(ref maskx, value); }
        public int MaskY { get => masky; set => SetProperty(ref masky, value); }
        private int masksize = 20;
        public int MaskSize { get => masksize; set => SetProperty(ref masksize, value); }
        public bool IsModifySearchingROI
        {
            get => isModifySearchingROI;
            set => SetProperty(ref isModifySearchingROI, value);
        }


        private bool isModifyEllipseROI;
        public bool IsModifyEllipseROI
        {
            get => isModifyEllipseROI;
            set => SetProperty(ref isModifyEllipseROI, value);
        }

        public bool IsShowSearchROI
        {
            get => isShowSearchROI;

            set
            {
                isShowSearchROI = value;
                if (isShowSearchROI)
                {
                    SearchingROIVisibility = Visibility.Visible;
                }
                else
                {
                    SearchingROIVisibility = Visibility.Hidden;
                }

                RaisePropertyChanged("IsShowSearchROI");
            }
        }

        private bool isShowEllpiseROI;
        public bool IsShowEllpiseROI
        {
            get => isShowEllpiseROI;

            set
            {
                isShowEllpiseROI = value;
                if (isShowEllpiseROI)
                {
                    EllipseROIVisibility = Visibility.Visible;
                }
                else
                {
                    EllipseROIVisibility = Visibility.Hidden;
                }

                RaisePropertyChanged(nameof(IsShowEllpiseROI));
            }
        }

        private bool isShowMask;
        public bool IsShowMask {
            get => isShowMask;
            set
            {
                isShowMask = value;
                if (isShowMask)
                {
                    MaskROIVisibility = Visibility.Visible;
                }
                else
                {
                    MaskROIVisibility = Visibility.Hidden;
                }

                RaisePropertyChanged(nameof(IsShowMask));
            }
        }

        private BitmapSource imageSource;
        public BitmapSource ImageSource
        {
            get => imageSource;

            set
            {
                imageSource = value;
                RaisePropertyChanged(nameof(ImageSource));
            }
        }

        private double canvasWidth;
        public double CanvasWidth
        {
            get => canvasWidth;

            set
            {
                canvasWidth = value;
                RaisePropertyChanged(nameof(CanvasWidth));
            }
        }

        private double canvasHeight;
        public double CanvasHeight
        {
            get => canvasHeight;

            set
            {
                canvasHeight = value;
                RaisePropertyChanged(nameof(CanvasHeight));
            }
        }

        private ObservableCollection<Point> pointCollection;
        public ObservableCollection<Point> PointCollection
        {
            get => pointCollection;
            set => SetProperty(ref pointCollection, value);
        }

        private ObservableCollection<Point> startAnglePoint;
        public ObservableCollection<Point> StartAnglePoint
        {
            get => startAnglePoint;
            set => SetProperty(ref startAnglePoint, value);
        }

        private ObservableCollection<Point> endAnglePoint;
        public ObservableCollection<Point> EndAnglePoint
        {
            get => endAnglePoint;
            set => SetProperty(ref endAnglePoint, value);
        }

        public void ShowImage(Image<Emgu.CV.Structure.Bgr, Byte> loadImage)
        {
            CanvasHeight = loadImage.Height;
            CanvasWidth = loadImage.Width;
            ImageSource = new WriteableBitmap(imageProcessControl.ConvertImgToBitmapSource(loadImage));
            //ImageSource = imageProcessControl.ConvertImgToBitmapSource(loadImage);
        }

        public void ShowImage(Image<Emgu.CV.Structure.Gray, Byte> loadImage)
        {
            CanvasHeight = loadImage.Height;
            CanvasWidth = loadImage.Width;
            ImageSource = imageProcessControl.ConvertImgToBitmapSource(loadImage);
        }

        public void ShowImage(WriteableBitmap writeableBitmap)
        {
            CanvasHeight = writeableBitmap.Height;
            CanvasWidth = writeableBitmap.Width;
            ImageSource = writeableBitmap;
        }

        private void OnReceiveAddPointEvent(ImageControlViewCallbackEventData eventData)
        {
            if (eventData.Msg == "Update ObservableCollection")
            {
                PointCollection = eventData.EventDataObject as ObservableCollection<Point>;
            }            
        }

        public void StartAddingPointCurve()
        {
            eventAggregator.GetEvent<ImageControlViewEvent>().Publish("Start adding point curve");
        }

        public void StopAddingPointCurve()
        {
            eventAggregator.GetEvent<ImageControlViewEvent>().Publish("Stop adding point curve");
        }

        public void MouseMoveEvent(int x, int y, System.Windows.Input.MouseButtonState buttonState)
        {
            MouseX = (int)((double)x);
            MouseY = (int)((double)y);

            MaskX = MouseX - MaskSize / 2;
            MaskY = MouseY - MaskSize / 2;

            Console.WriteLine("{0},{1}", x, y);

        }
        public void StartAddStartAnglePoint()
        {
            eventAggregator.GetEvent<ImageControlViewEvent>().Publish("Start adding start angle point");
        }

        public void StartAddEndAnglePoint()
        {
            eventAggregator.GetEvent<ImageControlViewEvent>().Publish("Start adding end angle point");
        }
        public void SetMaskSize(int size)
        {
            MaskSize = size;
        }
    }
}

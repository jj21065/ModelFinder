using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Prism.Mvvm;

namespace ModelFinder.Interface
{
    public interface IImageProcessControl
    {
    }

    public class CircleSearchROI : BindableBase
    {
        private int x;
        public int X
        {
            get => x;
            set => SetProperty(ref x, value);
        }

        private int y;
        public int Y
        {
            get => y;
            set => SetProperty(ref y, value);
        }

        private int r;
        public int Radius
        {
            get => r;
            set => SetProperty(ref r, value);
        }

        private int startAngle;
        public int StartAngle
        {
            get => startAngle;
            set => SetProperty(ref startAngle, value);
        }

        private int endAngle;
        public int EndAngle
        {
            get => endAngle;
            set => SetProperty(ref endAngle, value);
        }
    }

    public  class ModelResult : BindableBase
    {
        private double x;
        public double X
        {
            get => x;
            set => SetProperty(ref x, value);
        }

        private double y;
        public double Y
        {
            get => y;
            set => SetProperty(ref y, value);
           
        }

        private double u;
        public double U
        {
            get => u;
            set => SetProperty(ref u, value);
        }
    }

    public class VisionSearchROI : BindableBase
    {
        private int height;
        public int Height
        {
            get => height;
            set => SetProperty(ref height, value);
        }
        private int width;
        public int Width
        {
            get => width;
            set => SetProperty(ref width, value);
        }
        private int x;
        public int XPosition
        {
            get => x;
            set => SetProperty(ref x, value);
        }

        private int y;
        public int YPosition
        {
            get => y;
            set => SetProperty(ref y, value);
        }
    }

    public class HsvMaskScalar : BindableBase
    {
        private double lowerValue1;
        public double LowerValue1
        {
            get => lowerValue1;
            set => SetProperty(ref lowerValue1, value);
        }
        private double lowerValue2;
        public double LowerValue2
        {
            get => lowerValue2;
            set => SetProperty(ref lowerValue2, value);
        }
        private double lowerValue3;
        public double LowerValue3
        {
            get => lowerValue3;
            set => SetProperty(ref lowerValue3, value);
        }
        private double upperValue1;
        public double UpperValue1
        {
            get => upperValue1;
            set => SetProperty(ref upperValue1, value);
        }
        private double upperValue2;
        public double UpperValue2
        {
            get => upperValue2;
            set => SetProperty(ref upperValue2, value);
        }
        private double upperValue3;
        public double UpperValue3
        {
            get => upperValue3;
            set => SetProperty(ref upperValue3, value);
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;

namespace ModelFinder.Model.ImageProcess.ROI
{
    public class EllipseROI
    {
        public Point CenterPt { get; private set; }
        public double Width { get; private set; }
        public double Height { get; private set; }

        private double aOfEllipse;
        private double bOfEllipse;
        public EllipseROI(Point centerPt, double width, double height)
        {
            CenterPt = centerPt;
            Width = width;
            Height = height;

            if (width >= height)
            {
                aOfEllipse = width / 2;
                bOfEllipse = height / 2;
            }
            else
            {
                aOfEllipse = height / 2;
                bOfEllipse = width / 2;
            }
        }

        public bool GetIsInEllipseRoi(Point pt)
        {
            bool result = false;
            try
            {
                var value = (Math.Pow(pt.X - CenterPt.X, 2) / Math.Pow(aOfEllipse, 2)) + (Math.Pow(pt.Y - CenterPt.Y, 2) / Math.Pow(bOfEllipse, 2));
                if(value <= 1)
                {
                    result = true;
                }
            }
            catch(Exception ex)
            {
                System.Windows.MessageBox.Show(ex.ToString());
            }

            return result;

        }


    }
}

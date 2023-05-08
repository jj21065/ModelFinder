using Emgu.CV.Structure;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Emgu.CV;
using System.Drawing;

namespace ModelFinder.Model.ImageProcess.ROI
{
    public class RectROI
    {
        public double x;
        public double y;
        public double w;
        public double h;
        public double theta;
        private RotatedRect rect;
        private PointF[] corner = new PointF[4];

        public RectROI(double centerX, double centerY, double rectWidth, double rectHeight, double angle)
        {
            x = centerX - rectWidth / 2;
            y = centerY - rectHeight / 2;
            w = rectWidth;
            h = rectHeight;
            rect = new RotatedRect();
            rect.Center = new System.Drawing.PointF((float)centerX, (float)centerY);
            rect.Size = new System.Drawing.SizeF((float)rectWidth, (float)rectHeight);
            this.theta = angle;
            rect.Angle = (float)theta;
            //var Corner1 = new PointF((float) (centerX - (rectWidth / 2)), (float) (centerY - (rectHeight / 2)));
            //var Corner2 = new PointF((float) (centerX + (rectWidth / 2)), (float) (centerY - (rectHeight / 2)));
            //var Corner3 = new PointF((float) (centerX - (rectWidth / 2)), (float) (centerY + (rectHeight / 2)));
            //var Corner4 = new PointF((float) (centerX + (rectWidth / 2)), (float) (centerY + (rectHeight / 2)));
            corner = CvInvoke.BoxPoints(rect);
        }

        public RotatedRect GetRotatedRect()
        {
            return rect;
        }

        public PointF[] GetCorners()
        {
            corner = CvInvoke.BoxPoints(rect);
            return corner;
        }

        public bool GetIsInRoi(PointF point)
        {
            var result = true;

            try
            {
                for (int i = 3; i >= 0; i--)
                {
                    int next = i - 1;
                    if (next < 0)
                    {
                        next = 3;
                    }
                    double[] v1 = { corner[i].X - point.X, corner[i].Y - point.Y, 0 };
                    double[] v2 = { corner[next].X - point.X, corner[next].Y - point.Y, 0 };
                    double z = DoCross(v1, v2);
                    if (z > 0)
                    {
                        result = false;
                        break;
                    }
                }

            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show(ex.ToString());
            }

            return result;
        }

        private double DoCross(double[] v1, double[] v2)
        {
            return v1[0] * v2[1] - v1[1] * v2[0];
        }
        
    }
}

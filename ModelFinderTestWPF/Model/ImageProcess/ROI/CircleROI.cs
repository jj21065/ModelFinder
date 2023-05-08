using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;
//using System.Windows;

namespace ModelFinder.Model.ImageProcess.ROI
{
    public class CircleROI
    {
        public PointF CenterPt { get; set; }
        public double Radius { get; set; }
        public double StartAngle { get; set; }
        public double EndAngle { get; set; }
        public int InterpolationCount { get; set; }
        public List<Point> CircleROIContourPointList { get; set; }

        public CircleROI()
        {
            CenterPt = new Point(500,500);
            Radius = 100;
            StartAngle = 90;
            EndAngle = 0;
            
        }
        public CircleROI(Point center, double radius, double startAngle, double endAngle, int interpolationCount)
        {
            CenterPt = center;
            Radius = radius;
            StartAngle = startAngle;
            EndAngle = endAngle;
            CircleROIContourPointList = GetInterpolationPointList(interpolationCount);
            
        }

        private List<Point> GetInterpolationPointList(int interpolationCount)
        {
            var pointList = new List<Point>();

            try
            {
                var start = StartAngle;
                var end = (start > EndAngle) ? EndAngle + 360 : EndAngle;
                var step = (end - start) / (float)interpolationCount;
                for (var t = start; t <= end; t += step)
                {
                    var x = CenterPt.X - Radius * Math.Cos(t * Math.PI / 180.0);
                    var y = CenterPt.Y + Radius * Math.Sign(t * Math.PI / 180.0);
                    pointList.Add(new Point((int)x, (int)y));
                }
            }
            catch(Exception ex)
            {
                System.Windows.MessageBox.Show("Error from GetInterpolationPointList, " + ex.ToString());
            }

            return pointList;
        }

        public bool GetIsInCircle(Point point)
        {
            var result = false;
            try
            {
                if(Math.Sqrt(Math.Pow(point.X - CenterPt.X, 2) + Math.Pow(point.Y -CenterPt.Y, 2)) <= Radius)
                {
                    result = true;
                }
            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show("Error from GetIsInCircle, " + ex.ToString());
            }
            return result;
        }
    }
}

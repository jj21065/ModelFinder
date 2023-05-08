using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ModelFinder.Model.ImageProcess.ROI
{
    public class CircleArcPath
    {
        private float startAngle;
        private float endAngle;
        private PointF centerPoint;
        private float radius;
        private List<PointF> pointsOnTheArcList;
        private List<float> tangentAngleList;

        public CircleArcPath(PointF centerPoint, float radius, float startAngle, float endAngle)
        {
            this.centerPoint = centerPoint;
            this.radius = radius;
            this.startAngle = startAngle;
            this.endAngle = endAngle;
        }

        public List<float> GetTangentAngleList(float angleStep)
        {
            DoCaculation(angleStep);
            return tangentAngleList;
        }

        public List<PointF> GetPointsOnTheArcList(float angleStep)
        {
            DoCaculation(angleStep);
            return pointsOnTheArcList;
        }

        public bool GetIsInCircleArc(PointF point)
        {
            var distance = Math.Sqrt(Math.Pow(point.X - centerPoint.X, 2) + Math.Pow(point.Y - centerPoint.Y, 2));
            if(distance <= radius)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        private void DoCaculation(float angleStep)
        {
            try
            {
                pointsOnTheArcList = new List<PointF>();
                tangentAngleList = new List<float>();

                for (double i = startAngle; i < endAngle; i = i + angleStep)
                {
                    var point = new PointF((float)(centerPoint.X + radius * Math.Cos(i * Math.PI / 180)), (float)(centerPoint.Y + radius * Math.Sin(i * Math.PI / 180)));
                    pointsOnTheArcList.Add(point);
                    tangentAngleList.Add((float) (i));
                }
            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show("Error from DoCaculation, ex:" + ex.ToString());
            }
        }
        
    }
}

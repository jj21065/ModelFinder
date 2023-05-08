using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Emgu;
using Emgu.CV;
using Emgu.Util;
using System.IO;
//using System.Windows;
using System.Windows.Media.Imaging;
using System.Runtime.InteropServices;
using Emgu.CV.CvEnum;
using Emgu.CV.Structure;
using System.Drawing;
using ModelFinder.Model.ImageProcess.ROI;
using Prism.Events;
using CLI;
using ModelFinder.Interface;

namespace ModelFinder.Model.ImageProcess
{
    public class ImageProcessControl
    {

        private IEventAggregator eventAggregator;
        public ImageProcessControl(IEventAggregator eventAggregator)
        {
            this.eventAggregator = eventAggregator;
        }


        public bool LoadImage(string path, ref Mat matLoadImage)
        {
            var isDone = false;

            try
            {
                if (File.Exists(path))
                {
                    matLoadImage = CvInvoke.Imread(path, Emgu.CV.CvEnum.ImreadModes.Color);
                    isDone = true;
                }
            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show("Error from LoadGoldenImage, error: " + ex.ToString());
            }

            return isDone;
        }

        public BitmapSource ConvertImgToBitmapSource(IImage img)
        {
            return BitmapSourceConvert.ToBitmapSource(img);
        }

        public Image<Gray, byte> SharpenImage(Image<Gray, byte> image, int w, int h, double sigma1, double sigma2, int k)
        {
            w = (w % 2 == 0) ? w - 1 : w;
            h = (h % 2 == 0) ? h - 1 : h;
            //apply gaussian smoothing using w, h and sigma 
            var gaussianSmooth = image.SmoothGaussian(w, h, sigma1, sigma2);
            //obtain the mask by subtracting the gaussian smoothed image from the original one 
            var mask = image - gaussianSmooth;
            //add a weighted value k to the obtained mask 
            mask *= k;
            //sum with the original image 
            image += mask;
            return image;
        }

        public void Opening(IInputArray inputImg, IOutputArray outputImg, int w, int h)
        {
            //outputImg = new Mat();
            var kernel = CvInvoke.GetStructuringElement(ElementShape.Ellipse, new System.Drawing.Size(w, h), new System.Drawing.Point(-1, -1));
            CvInvoke.MorphologyEx(inputImg, outputImg, MorphOp.Open, kernel, new System.Drawing.Point(-1, -1), 1, BorderType.Default, new Emgu.CV.Structure.MCvScalar());
        }

        public void Closing(IInputArray inputImg, IOutputArray outputImg, int w, int h)
        {
            //outputImg = new Mat();
            var kernel = CvInvoke.GetStructuringElement(ElementShape.Ellipse, new System.Drawing.Size(w, h), new System.Drawing.Point(-1, -1));
            CvInvoke.MorphologyEx(inputImg, outputImg, MorphOp.Close, kernel, new System.Drawing.Point(-1, -1), 1, BorderType.Default, new Emgu.CV.Structure.MCvScalar());
        }

        public void Dilate(IInputArray inputImg, IOutputArray outputImg, int w, int h)
        {
            //outputImg = new Mat();
            var kernel = CvInvoke.GetStructuringElement(ElementShape.Ellipse, new System.Drawing.Size(w, h), new System.Drawing.Point(-1, -1));
            CvInvoke.MorphologyEx(inputImg, outputImg, MorphOp.Dilate, kernel, new System.Drawing.Point(-1, -1), 1, BorderType.Default, new Emgu.CV.Structure.MCvScalar());
        }

        public void Erode(IInputArray inputImg, IOutputArray outputImg, int w, int h)
        {
            //outputImg = new Mat();
            var kernel = CvInvoke.GetStructuringElement(ElementShape.Ellipse, new System.Drawing.Size(w, h), new System.Drawing.Point(-1, -1));
            CvInvoke.MorphologyEx(inputImg, outputImg, MorphOp.Erode, kernel, new System.Drawing.Point(-1, -1), 1, BorderType.Default, new Emgu.CV.Structure.MCvScalar());
        }

        public void ConvertBgr2Gray(IInputArray inputImg, IOutputArray outputImg)
        {
            CvInvoke.CvtColor(inputImg, outputImg, ColorConversion.Bgr2Gray);
        }

        public void ConvertBgr2Hsv(IInputArray inputImg, IOutputArray outputImg)
        {
            CvInvoke.CvtColor(inputImg, outputImg, ColorConversion.Bgr2Hsv);
        }

        public void DrawCurve(Mat sourceImage, Mat destImage, List<PointF> pointList)
        {
            sourceImage.CopyTo(destImage);
            for (int i = 0; i < pointList.Count - 1; i++)
            {
                CvInvoke.Line(destImage, System.Drawing.Point.Round(pointList[i]), System.Drawing.Point.Round(pointList[i + 1]), new MCvScalar(33, 33, 133), 1);
            }
        }

        public void IsInROICheck(Mat src, Mat dst, ImageProcess.ROI.RectROI roi)
        {

            Mat tmpImg = null;
            src.CopyTo(tmpImg);
            //uchar* data = tmpImg.data;
            dst = Mat.Zeros(src.Cols, src.Rows, DepthType.Cv8U, 1);
            int nCols = roi.GetRotatedRect().MinAreaRect().Size.Width;
            int nRows = roi.GetRotatedRect().MinAreaRect().Size.Height;
            int startx = (int)roi.GetRotatedRect().Center.X - nCols / 2;
            int starty = (int)roi.GetRotatedRect().Center.Y - nRows / 2;

            byte[] srcData = new byte[src.Height * src.Width];
            Marshal.Copy(src.DataPointer, srcData, 0, src.Height * src.Width);

            byte[] dstData = new byte[dst.Height * dst.Width];
            Marshal.Copy(dst.DataPointer, dstData, 0, dst.Height * dst.Width);

            for (int i = 0; i < nRows; i++)
            {
                for (int j = 0; j < nCols; j++)
                {
                    if (j + startx > 0 && j + startx < dst.Cols && i + starty > 0 && i + starty < dst.Rows)
                        if (roi.GetIsInRoi(new PointF(j + startx, i + starty)))
                        {
                            dstData[(i + starty) * nCols + j + startx] = srcData[(i + starty) * nCols + j + startx];

                        }
                }
                //			data += nStep;
            }

        }

        public int RegionGrow(byte[] img1Data, ref byte[] img2Data, ref byte[] img3Data, int width, int height, ImageProcess.ROI.RectROI roi, int x, int y, int Threshold, CircleArcPath inner, CircleArcPath outter)
        {
            float Diff, Mean;
            int area = 0; ;

            /* Check to see if point already part of region */
            if (roi.GetIsInRoi(new PointF(x, y)))
            {
                if (img2Data[y * width * 3 + 3 * x] == 0 && img1Data[y * width + x] > 10)
                {
                    //if(img1Data[y * width + x] > 200)
                    //{
                    //    Console.WriteLine("Data > 200, * 0.9; ");
                    //    img1Data[y * width + x] = (byte) (img1Data[y * width + x] * 0.9);
                    //}
                    /* See if point is close enough to add */
                    if (x > 0)
                    {
                        Diff = img1Data[y * width + x] - img1Data[y * width + x - 1];
                        if (Diff < 0) Diff = -Diff;
                        if (Diff < Threshold)
                        {
                            if (inner.GetIsInCircleArc(new PointF(x, y)))
                            {
                                img2Data[y * width * 3 + 3 * x] = 255;
                                img2Data[y * width * 3 + 3 * x + 1] = 0;
                                img2Data[y * width * 3 + 3 * x + 2] = 0;
                                img3Data[y * width + x] = 255;
                            }
                            else if (!outter.GetIsInCircleArc(new PointF(x, y)))
                            {
                                img2Data[y * width * 3 + 3 * x] = 255;
                                img2Data[y * width * 3 + 3 * x + 1] = 0;
                                img2Data[y * width * 3 + 3 * x + 2] = 0;
                                img3Data[y * width + x] = 255;
                            }
                            else
                            {
                                img2Data[y * width * 3 + 3 * x] = 1;
                                img2Data[y * width * 3 + 3 * x + 1] = 0;
                                img2Data[y * width * 3 + 3 * x + 2] = 255;
                                img3Data[y * width + x] = 255;
                            }
                            var subArea = RegionGrow(img1Data, ref img2Data, ref img3Data, width, height, roi, x - 1, y, Threshold, inner, outter);
                            area = area + subArea + 1;
                        }
                    }
                    if (x < width - 2)
                    {
                        Diff = img1Data[y * width + x] - img1Data[y * width + x + 1];
                        if (Diff < 0) Diff = -Diff;
                        if (Diff < Threshold)
                        {
                            if (inner.GetIsInCircleArc(new PointF(x, y)))
                            {
                                img2Data[y * width * 3 + 3 * x] = 255;
                                img2Data[y * width * 3 + 3 * x + 1] = 0;
                                img2Data[y * width * 3 + 3 * x + 2] = 0;
                                img3Data[y * width + x] = 255;
                            }
                            else if (!outter.GetIsInCircleArc(new PointF(x, y)))
                            {
                                img2Data[y * width * 3 + 3 * x] = 255;
                                img2Data[y * width * 3 + 3 * x + 1] = 0;
                                img2Data[y * width * 3 + 3 * x + 2] = 0;
                                img3Data[y * width + x] = 255;
                            }
                            else
                            {
                                img2Data[y * width * 3 + 3 * x] = 1;
                                img2Data[y * width * 3 + 3 * x + 1] = 0;
                                img2Data[y * width * 3 + 3 * x + 2] = 255;
                                img3Data[y * width + x] = 255;
                            }
                            var subArea = RegionGrow(img1Data, ref img2Data, ref img3Data, width, height, roi, x + 1, y, Threshold, inner, outter);
                            area = area + subArea + 1;
                        }
                    }
                    if (y > 0)
                    {
                        Diff = img1Data[y * width + x] - img1Data[(y - 1) * width + x];
                        if (Diff < 0) Diff = -Diff;
                        if (Diff < Threshold)
                        {
                            if (inner.GetIsInCircleArc(new PointF(x, y)))
                            {
                                img2Data[y * width * 3 + 3 * x] = 255;
                                img2Data[y * width * 3 + 3 * x + 1] = 0;
                                img2Data[y * width * 3 + 3 * x + 2] = 0;
                                img3Data[y * width + x] = 255;
                            }
                            else if (!outter.GetIsInCircleArc(new PointF(x, y)))
                            {
                                img2Data[y * width * 3 + 3 * x] = 255;
                                img2Data[y * width * 3 + 3 * x + 1] = 0;
                                img2Data[y * width * 3 + 3 * x + 2] = 0;
                                img3Data[y * width + x] = 255;
                            }
                            else
                            {
                                img2Data[y * width * 3 + 3 * x] = 1;
                                img2Data[y * width * 3 + 3 * x + 1] = 0;
                                img2Data[y * width * 3 + 3 * x + 2] = 255;
                                img3Data[y * width + x] = 255;
                            }
                            var subArea = RegionGrow(img1Data, ref img2Data, ref img3Data, width, height, roi, x, y - 1, Threshold, inner, outter);
                            area = area + subArea + 1;
                        }
                    }
                    if (y < height - 2)
                    {
                        Diff = img1Data[y * width + x] - img1Data[(y + 1) * width + x];
                        if (Diff < 0) Diff = -Diff;
                        if (Diff < Threshold)
                        {
                            if (inner.GetIsInCircleArc(new PointF(x, y)))
                            {
                                img2Data[y * width * 3 + 3 * x] = 255;
                                img2Data[y * width * 3 + 3 * x + 1] = 0;
                                img2Data[y * width * 3 + 3 * x + 2] = 0;
                                img3Data[y * width + x] = 255;
                            }
                            else if (!outter.GetIsInCircleArc(new PointF(x, y)))
                            {
                                img2Data[y * width * 3 + 3 * x] = 255;
                                img2Data[y * width * 3 + 3 * x + 1] = 0;
                                img2Data[y * width * 3 + 3 * x + 2] = 0;
                                img3Data[y * width + x] = 255;
                            }
                            else
                            {
                                img2Data[y * width * 3 + 3 * x] = 1;
                                img2Data[y * width * 3 + 3 * x + 1] = 0;
                                img2Data[y * width * 3 + 3 * x + 2] = 255;
                                img3Data[y * width + x] = 255;
                            }
                            var subArea = RegionGrow(img1Data, ref img2Data, ref img3Data, width, height, roi, x, y + 1, Threshold, inner, outter);
                            area = area + subArea + 1;
                        }
                    }
                }
            }

            return area;
        }

        public float GetArea(Mat sourceImage, RectROI roi)
        {
            float sum = 0;

            try
            {
                int nCols = sourceImage.Cols;
                int nRows = sourceImage.Rows;
                int steps = sourceImage.Step;
                byte[] data = new byte[sourceImage.Height * sourceImage.Width * 3];
                Marshal.Copy(sourceImage.DataPointer, data, 0, sourceImage.Height * sourceImage.Width * 3);
                IntPtr dataPointer = sourceImage.DataPointer;

                for (int i = 0; i < nRows; i++)
                {
                    for (int j = 0; j < nCols; j++)
                    {
                        if (roi.GetIsInRoi(new PointF(j, i)))
                        {
                            if (data[sourceImage.Width * i + 3 * j] > 0)
                            {
                                sum++;
                                data[sourceImage.Width * i + 3 * j] = 0;
                                data[sourceImage.Width * i + 3 * j + 1] = 255;
                                data[sourceImage.Width * i + 3 * j + 2] = 0;
                            }
                        }
                    }
                }

            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show(ex.ToString());
            }


            return sum;
        }

        public int GetMaskArea(Mat sourceImage)
        {
            int sum = 0;

            try
            {
                int nCols = sourceImage.Cols;
                int nRows = sourceImage.Rows;
                int steps = sourceImage.Step;
                int nChannels = sourceImage.NumberOfChannels;
                byte[] data = new byte[sourceImage.Height * sourceImage.Width * nChannels];
                Marshal.Copy(sourceImage.DataPointer, data, 0, sourceImage.Height * sourceImage.Width * nChannels);

                for (int i = 0; i < nRows; i++)
                {
                    for (int j = 0; j < nCols; j++)
                    {
                        if (data[sourceImage.Width * i + nChannels * j] > 0)
                        {
                            sum++;
                        }
                    }
                }

            }
            catch (Exception ex)
            {
                System.Windows.MessageBox.Show(ex.ToString());
            }


            return sum;
        }

        public void GetTranform(PointF[] src, PointF[] dst, Mat srcImage, Mat dstImage)
        {
            try
            {
                ///  outPath = srcPath.ToList();
                ///  
                srcImage.CopyTo(dstImage);
                PointF vector1 = new PointF(src[1].X - src[0].X, src[1].Y - src[0].Y);
                PointF vector2 = new PointF(dst[1].X - dst[0].X, dst[1].Y - dst[0].Y);

                double deg1 = Math.Atan2(vector1.Y, vector1.X);
                double deg2 = Math.Atan2(vector2.Y, vector2.X);

                PointF dist = new PointF(dst[0].X - src[0].X, dst[0].Y - src[0].Y);

                double rotation = deg2 - deg1;
                Matrix<double> rotationMat = new Matrix<double>(3, 2);
                CvInvoke.GetRotationMatrix2D(new PointF((float)src[0].X, (float)src[0].Y), rotation, 1, rotationMat);

                CvInvoke.WarpAffine(dstImage, dstImage, rotationMat, new Size(dstImage.Cols, dstImage.Rows));
                Matrix<double> shiftMatrix = new Matrix<double>(3, 2)
                {
                    Data = new double[,] { { 1, 0, dist.X }, { 0, 1, dist.Y } }
                };
                Console.WriteLine(String.Format("{0},{1},{2}", shiftMatrix.Data[0, 0], shiftMatrix.Data[0, 1], shiftMatrix.Data[0, 2]));
                Console.WriteLine(String.Format("{0},{1},{2}", shiftMatrix.Data[1, 0], shiftMatrix.Data[1, 1], shiftMatrix.Data[1, 2]));

                //Matrix<double> AffineMat = rotationMat.Mul(shiftMatrix);
                CvInvoke.WarpAffine(dstImage, dstImage, shiftMatrix, new Size(dstImage.Cols, dstImage.Rows));
                //mat = AffineMat;


            }
            catch (Exception ex)
            {
                //Message.ListAdd(ex.ToString());
            }
        }

        public void HsvProcess(Mat inputImg, Mat outputImg, int hue, int saturation, int value, int hue2, int saturation2, int value2)
        {
            Mat mask = new Mat();
            Mat hsv = new Mat();
            // CvInvoke.Imshow("hsv", inputImg);
            try
            {
                CvInvoke.CvtColor(inputImg, hsv, ColorConversion.Bgr2Hsv);
                CvInvoke.InRange(hsv, new ScalarArray(new MCvScalar(hue, saturation, value)), new ScalarArray(new MCvScalar(hue2, saturation2, value2)), mask);
                Mat maskColor = new Mat();
                //CvInvoke.CvtColor(mask, maskColor, ColorConversion.Gray2Bgr);
                hsv.SetTo(new MCvScalar(0, 0, 0), mask);
                

                CvInvoke.CvtColor(hsv, outputImg, ColorConversion.Hsv2Bgr);

                CvInvoke.Subtract(inputImg, outputImg, outputImg);
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
            // CvInvoke.Imshow("hsv2", outputImg);
        }

        public void ExtractImageArea(Mat src, Mat dst, double Model1GoldenU, double Model1GoldenX, double Model1GoldenY, SearchResult modelResult, Interface.CircleSearchROI[] rois)
        {
            Size sz = src.Size;
            Mat mask = new Mat(sz, DepthType.Cv8U, 1);
        
            var angleDiff = -(modelResult.Rotation - Model1GoldenU);

            var rotationMat = new Mat();
            Mat sourceImage = new Image<Bgr, byte>(src.Bitmap).Mat;
            CvInvoke.GetRotationMatrix2D(new PointF((float)modelResult.X, (float)modelResult.Y), angleDiff, 1, rotationMat);
            Mat tempDest = new Mat(src.Size, src.Depth, src.NumberOfChannels);
            CvInvoke.WarpAffine(sourceImage, tempDest, rotationMat, src.Size);
            float offsetX = -(float)(modelResult.X - Model1GoldenX);
            float offsetY = -(float)(modelResult.Y - Model1GoldenY);

            Matrix<double> shiftMatrix = new Matrix<double>(3, 2)
            {
                Data = new double[,] { { 1, 0, offsetX }, { 0, 1, offsetY } }
            };
           
            for (int i = 0; i < rois.Length; i++)
            {
                Point center = new Point(rois[i].X,rois[i].Y);
                int radius = (int)rois[i].Radius;
                MCvScalar color = new MCvScalar(255, 255, 255);
                CvInvoke.Circle(mask, center, radius, color, -1);
            }
   
            src.CopyTo(dst, mask);

            CvInvoke.WarpAffine(dst, dst, shiftMatrix, src.Size);


            //CvInvoke.Imshow("mssk Image", dst);
        }

        public void ExtractImageArea(Mat src, Mat dst, Interface.CircleSearchROI[] rois)
        {
            Size sz = src.Size;
            Mat mask = new Mat(sz, DepthType.Cv8U, 1);
            
            var rotationMat = new Mat();
         
            for (int i = 0; i < rois.Length; i++)
            {
                Point center = new Point(rois[i].X, rois[i].Y);
                int radius = (int)rois[i].Radius;
                MCvScalar color = new MCvScalar(255, 255, 255);
                CvInvoke.Circle(mask, center, radius, color, -1);
            }

            src.CopyTo(dst, mask);

      
            //CvInvoke.Imshow("mssk Image", dst);
        }

        public CircleROI GetCircle3pt(PointF p0,PointF p1,PointF p2)
        {
            double a = p0.X - p1.X;// X1-X2
            double b = p0.Y - p1.Y;//Y1-Y2
            double c = p0.X - p2.X;//X1-X3
            double d = p0.Y - p2.Y;//Y1-Y3
            double aa = Math.Pow(p0.X, 2) - Math.Pow(p1.X, 2);//X1^2-X2^2
            double bb = Math.Pow(p1.Y, 2) - Math.Pow(p0.Y, 2);//Y2^2-Y1^2
            double cc = Math.Pow(p0.X, 2) - Math.Pow(p2.X, 2);//X1^2-X3^2
            double dd = Math.Pow(p2.Y, 2) - Math.Pow(p0.Y, 2);//Y3^2-Y1^2
            double E = (aa - bb) / 2;
            double F = (cc - dd) / 2;
            double resultY = (a * F - c * E) / (a * d - b * c);
            double resultX = (F * b - E * d) / (b * c - a * d);
            double resultR = Math.Sqrt((Math.Pow((p0.X - resultX), 2)) + (Math.Pow((p0.Y - resultY), 2)));
            var startAngle = Math.Atan2((p0.Y - resultY), (p0.X - resultX)) * 180 / Math.PI;
            var EndAngle = Math.Atan2((p2.Y - resultY), (p2.X - resultX)) * 180 / Math.PI;

            CircleROI circle = new CircleROI();
            circle.CenterPt = new PointF((float)resultX, (float)resultY) ;
            circle.Radius = (float)resultR;
            circle.StartAngle = startAngle;
            circle.EndAngle = EndAngle;

            return circle;
        }

        public bool GetCircle3PtFromCircleRoi(CircleROI circleRoi, out PointF p0, out PointF p1, out PointF p2)
        {
            p0 = new PointF();
            p1 = new PointF();
            p2 = new PointF();
            try
            {
                p0.X = (float) (circleRoi.CenterPt.X + circleRoi.Radius * Math.Cos(circleRoi.StartAngle / 180 * Math.PI));
                p0.Y = (float) (circleRoi.CenterPt.Y + circleRoi.Radius * Math.Sin(circleRoi.StartAngle / 180 * Math.PI));

                p2.X = (float) (circleRoi.CenterPt.X + circleRoi.Radius * Math.Cos(circleRoi.EndAngle / 180 * Math.PI));
                p2.Y = (float) (circleRoi.CenterPt.Y + circleRoi.Radius * Math.Sin(circleRoi.EndAngle / 180 * Math.PI));

                double newAngle = 0;
                if(circleRoi.StartAngle * circleRoi.EndAngle < 0)
                {
                    newAngle = circleRoi.StartAngle + circleRoi.EndAngle;
                }
                else
                {
                    System.Windows.MessageBox.Show("不支援畫弧的角度");
                }

                var p1X = circleRoi.CenterPt.X + circleRoi.Radius * Math.Cos(newAngle / 180 * Math.PI);
                var p1Y = circleRoi.CenterPt.Y + circleRoi.Radius * Math.Sin(newAngle / 180 * Math.PI);
                p1.X = (float)p1X;
                p1.Y = (float)p1Y;
                
                return true;
            }
            catch(Exception ex)
            {
                System.Windows.MessageBox.Show("Error from GetCircle3PtFromCircleRoi \r\n" + ex.ToString());
                return false;
            }
        }


        public Image<Bgr, byte> ImageRowDataToEmguBgr(byte[] imageBuffer, int width, int height)
        {
            Image<Bgr, byte> result = null;
            try
            {
                if (imageBuffer != null)
                {
                    result = new Image<Bgr, byte>(width, height);
                    var buffer = new byte[imageBuffer.Length];
                    var index = 0;
                    for (int i = 0; i < height; i++)
                    {
                        for (int j = 0; j < width; j += 1)
                        {
                            var pixelIndex = i * width * 3 + j * 3 + 2;
                            var pixelIndex2 = i * width * 3 + j * 3 + 1;
                            var pixelIndex3 = i * width * 3 + j * 3;
                            buffer[pixelIndex] = imageBuffer[index];
                            buffer[pixelIndex2] = imageBuffer[index + height * width];
                            buffer[pixelIndex3] = imageBuffer[index + height * width * 2];
                            index++;
                        }
                    }
                    result.Bytes = buffer;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error from ImageRowDataToEmguBgr: " + ex.ToString());
            }
            return result;
        }

        public Image<Gray, byte> ImageRowDataToEmguGray(byte[] imageBuffer, int width, int height)
        {
            Image<Gray, byte> result = null;
            try
            {
                if (imageBuffer != null)
                {
                    result = new Image<Gray, byte>(width, height);
                    
                    result.Bytes = imageBuffer;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error from ImageRowDataToEmguBgr: " + ex.ToString());
            }
            return result;
        }

        public static class BitmapSourceConvert
        {
            [DllImport("gdi32")]
            private static extern int DeleteObject(IntPtr o);

            public static BitmapSource ToBitmapSource(IImage image)
            {
                using (var source = image.Bitmap)
                {
                    var ptr = source.GetHbitmap();

                    var bs = System.Windows.Interop.Imaging.CreateBitmapSourceFromHBitmap(
                        ptr,
                        IntPtr.Zero,
                        System.Windows.Int32Rect.Empty,
                        System.Windows.Media.Imaging.BitmapSizeOptions.FromEmptyOptions());

                    DeleteObject(ptr);
                    return bs;
                }
            }
        }


    }
}

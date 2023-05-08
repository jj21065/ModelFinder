using DataMatrix.net;
using Emgu.CV;
using Emgu.CV.CvEnum;
using Emgu.CV.Structure;
using Emgu.CV.Util;
using ModelFinder.Interface;
using Prism.Events;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ZXing;
using ImageProcessModule.Models;
using Prism.Events;
using ModelFinder.Model.ImageProcess;

namespace ModelFinder.Model
{
    public class BarcodeControl
    {
        private IEventAggregator eventAggregator;
        private MILApplication milApplication;
        public BarcodeControl(IEventAggregator eventAggregator, MILApplication milApplication)
        {
            this.eventAggregator = eventAggregator;
            this.milApplication = milApplication;
        }
        /// <summary>
        /// QRCODE, DataMatrix available 
        /// </summary>
        /// <param name="image"></param>
        /// <returns></returns>
        public static string ZXingDecoder(Image<Bgr, byte> image, VisionSearchROI roi, ref Image<Gray, byte> roiImage)
        {
            string code = string.Empty;

            try
            {

                var gray = new Image<Gray, byte>(image.Bitmap);
                gray.ROI = new Rectangle(roi.XPosition, roi.YPosition, roi.Width, roi.Height);
                roiImage = gray.Copy();
               // roiImage = roiImage.ThresholdAdaptive(new Gray(255), Emgu.CV.CvEnum.AdaptiveThresholdType.GaussianC, Emgu.CV.CvEnum.ThresholdType.Binary, 49 new Gray(0));
                //CvInvoke.Imshow("grayImage", gray);
                //CvInvoke.Imshow("roiImage", roiImage);
                IOutputArray outputArray = new Mat();

                //roiImage = roiImage.ThresholdAdaptive(new Gray(255), AdaptiveThresholdType.MeanC, ThresholdType.Binary, 9, new Gray(5));
                roiImage._EqualizeHist();

                for (int i = 0; i < 4; i++)
                {
                    try
                    {
                        //CvInvoke.Imshow(i.ToString(), roiImage);
                        BarcodeReader barCodeReader = new BarcodeReader();
                        barCodeReader.TryInverted = true;
                        barCodeReader.Options.TryHarder = true;
                        barCodeReader.Options.PureBarcode = false;
                        barCodeReader.Options.PossibleFormats = new List<BarcodeFormat>();
                        barCodeReader.Options.PossibleFormats.Add(BarcodeFormat.DATA_MATRIX);
                        barCodeReader.AutoRotate = true;
                        //ZXing.Result[] resultmulti = barCodeReader.DecodeMultiple(gray.Bitmap);
                        ZXing.Result codeResult = barCodeReader.Decode(roiImage.Bitmap);
                        if (codeResult == null)
                        {
                            roiImage = roiImage.Rotate(90, new Gray());
                            Console.WriteLine("CodeResult no." + i + "] is null");
                            continue;
                        }
                        code = codeResult.Text;
                        break;
                    }
                    catch (Exception ex)
                    {
                        roiImage = roiImage.Rotate(90, new Gray());
                        Console.WriteLine("Error to rotate " + ex.ToString());
                    }
                }
            }
            catch(Exception ex)
            {
                Console.WriteLine("Error from ZXingDecoder \r\n" + ex.ToString());
            }
            return code;
        }
        /// <summary>
        /// Can find code in large image but too slow,, slower than Zxing
        /// </summary>
        /// <param name="image"></param>
        //public void DataMatrixRead(Image<Bgr,byte> image)
        //{
        //    try
        //    {

        //        DataMatrix.net.DmtxImageDecoder decoder = new DataMatrix.net.DmtxImageDecoder();

        //        var gray = new Image<Gray, byte>(image.Bitmap);

        //        List<string> str = decoder.DecodeImage(gray.Bitmap);
        //        if (str.Count > 0)
        //        {
        //            foreach (var l in str)
        //            {
        //                Console.WriteLine(l);
        //            }
        //        }
        //    }
        //    catch (Exception ex)
        //    {

        //    }
        //}

        public string MilCodeDecoder(Image<Bgr, byte> image, VisionSearchROI roi, ref Image<Gray, byte> codeImage)
        {
            string code = string.Empty;

            try
            {
                Console.WriteLine("In MilCodeDecoder");
                var gray = new Image<Gray, byte>(image.Bitmap);
                gray.ROI = new Rectangle(roi.XPosition, roi.YPosition, roi.Width, roi.Height);
                var roiImage = gray.Copy();

                //Console.WriteLine("Before loadToBuf");
               // milApplication.LoadToBuf(0, roiImage.Bytes);
                //Console.WriteLine("After loadToBuf");
                //var imageData =  milApplication.GetMonoImageBuffer1D(0, 0, 0, roiImage.Width, roiImage.Height);

                var imageProcessControl = new ImageProcessControl(eventAggregator);
                //var convertImage = imageProcessControl.ImageRowDataToEmguGray(imageData, roiImage.Width, roiImage.Height);

                codeImage = roiImage.Copy();
                //milApplication.CodeReaderInit();
                milApplication.CodeReaderInit();
                Console.WriteLine("roiImage.Bytes.Length:" + roiImage.Bytes.Length);
                //var data = new byte[roiImage.Cols * roiImage.Rows];
                var xLength = roiImage.Data.GetLength(1);
                var yLength = roiImage.Data.GetLength(0);
                // X須補到4的倍數
                code = milApplication.CodeReaderRun(roiImage.Bytes, xLength, yLength);
                



            }
            catch (Exception ex)
            {
                Console.WriteLine("Error from MilCodeDecoder \r\n" + ex.ToString());
            }
            return code;
        }
    }
}

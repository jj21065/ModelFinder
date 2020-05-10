using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace ModelFinderTestWPF
{
    /// <summary>
    /// MainWindow.xaml 的互動邏輯
    /// </summary>
  
    public partial class MainWindow : Window
    {
        public CLI.ModelFinder modelFinder = new CLI.ModelFinder();
        public MainWindow()
        {
            InitializeComponent();

            Bitmap image = new Bitmap("template1.jpg");
            if (image != null)
            {
                modelFinder.SetModelPara(-15, 15, 0.1, 0.6);
                modelFinder.SetROI(190, 190, 200, 200);
                modelFinder.SetSobelThreshold(160, 200);
                modelFinder.showCvImage = true;
                modelFinder.CreateModelFromImage(image);
                image.Dispose();
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            Bitmap image2 = new Bitmap("image3.jpg");
                if(image2 != null)
                    modelFinder.ModelFind(image2);
       
            Console.WriteLine("ModelFind at (" + modelFinder.searchResult.X + ", " + modelFinder.searchResult.Y + ")");
          
            image2.Dispose();
            //			std::cout <<std::endl << "ModelFind at (" << modelFinder->searchResult.X << "< " << modelFinder->searchResult.Y << <<")"<<std::endl;
        }
    }
}

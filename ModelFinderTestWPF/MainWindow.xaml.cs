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

            Bitmap image = new Bitmap(@".\\template1.jpg");
            if (image != null)
            {
                modelFinder.SetModelPara(-3, 3, 0.1, 0.9);
                modelFinder.SetROI(1100, 850, 400, 400);
                modelFinder.SetSobelThreshold(120, 130);
                modelFinder.showCvImage = true;
                modelFinder.CreateModelFromImage(image);
                image.Dispose();
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            Bitmap image2 = new Bitmap(".//Test_Golden.tif");
                if(image2 != null)
                    modelFinder.ModelFind(image2);
       
            Console.WriteLine("ModelFind at (" + modelFinder.searchResult.X + ", " + modelFinder.searchResult.Y + ")");
          
            image2.Dispose();
            //			std::cout <<std::endl << "ModelFind at (" << modelFinder->searchResult.X << "< " << modelFinder->searchResult.Y << <<")"<<std::endl;
        }

        private void SaveClick(object sender, RoutedEventArgs e)
        {
            modelFinder.SerializeModel(@"test.mod");
        }

        private void LoadClick(object sender, RoutedEventArgs e)
        {
            try
            {
                modelFinder.DeSerializeModel(@"test.mod");
            }catch(Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
        }
    }
}

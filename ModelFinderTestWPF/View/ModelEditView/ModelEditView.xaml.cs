using ImageView;
using ModelFinder.Model.ImageProcess;
using ModelFinder.ViewModel;
using Prism.Events;
using System;
using System.Collections.Generic;
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
using System.Windows.Shapes;

namespace ModelFinder.View.ModelEditView
{
    /// <summary>
    /// ModelEditView.xaml 的互動邏輯
    /// </summary>
    public partial class ModelEditView : Window
    {
        private IEventAggregator eventAggregator;
        public ModelEditView(ImageProcessControl imageProcessControl, IEventAggregator eventAggregator)
        {
            InitializeComponent();
            ImageControlViewModel imageControlVM = new ImageControlViewModel(imageProcessControl, eventAggregator);
            ModelEditViewModel modelEditViewModel = new ModelEditViewModel(imageProcessControl, imageControlVM, eventAggregator);
            ImageControlView imageControlView = new ImageControlView(eventAggregator, imageControlVM, modelEditViewModel, imageProcessControl);

            this.DataContext = modelEditViewModel;
            grid_AddImageView.Children.Add(imageControlView);
            this.eventAggregator = eventAggregator;
        }

        
    }
}

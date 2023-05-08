using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Prism.Mvvm;
namespace ImageView
{
    public class ROIRectangleModel: BindableBase
    {
        private int x;
        private int y;
        private int w;
        private int h;

        public int X
        {
            get => x;

            set
            {
                if (x != value)
                {
                    x = value;
                    RaisePropertyChanged();
                }
            }
        }

        public int Y
        {
            get => y;

            set
            {
                if (y != value)
                {
                    y = value;
                    RaisePropertyChanged();
                }
            }
        }

        public int W
        {
            get => w;

            set
            {
                if (w != value)
                {
                    w = value;
                    RaisePropertyChanged();
                }
            }
        }

        public int H
        {
            get => h;

            set
            {
                if (h != value)
                {
                    h = value;
                    RaisePropertyChanged();
                }
            }
        }

    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Prism.Events;
using System.Windows.Media;
using System.Collections.ObjectModel;
using System.Windows;

namespace ModelFinder.Events
{
    public class ImageControlViewCallbackEvent : PubSubEvent<ImageControlViewCallbackEventData>
    {

    }

    public class ImageControlViewCallbackEventData
    {
        public object EventDataObject { get; set; }
        public string Msg { get; set; }
    }
}

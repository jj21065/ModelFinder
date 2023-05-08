using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
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
using Prism.Events;

using System.Collections.ObjectModel;
using Emgu.CV;
using Emgu.CV.Structure;
using ModelFinder.Model.ImageProcess;
using ModelFinder.ViewModel;
using ModelFinder.Events;

namespace ImageView
{ 
    public partial class ImageControlView : UserControl
    {
        /// <summary>
        /// Specifies the current state of the mouse handling logic.
        /// </summary>
        private MouseHandlingMode mouseHandlingMode = MouseHandlingMode.None;

        /// <summary>
        /// The point that was clicked relative to the content that is contained within the ZoomAndPanControl.
        /// </summary>
        private System.Windows.Point origContentMouseDownPoint;

        /// <summary>
        /// Records which mouse button clicked during mouse dragging.
        /// </summary>
        private MouseButton mouseButtonDown;

        private HitType mouseHitType = HitType.None;
        private bool dragInProgress = false;
        private Point lastPoint;
        private IEventAggregator eventAggregator;
        private bool isAddingCurvePointList = false;
        private bool isAddingStartAnglePoint = false;
        private bool isAddingEndAnglePoint = false;
        private ObservableCollection<Point> observablePointCollection;
        public  delegate void MouseEvent(int x ,int y,MouseButtonState buttonState);
        public event MouseEvent MouseMoveEvent = null;
        public event MouseEvent MouseDownEvent = null;
        public event MouseEvent MouseUpEvent = null;
        private ImageControlViewModel imageControlVM;
        private ImageProcessControl imageProcessControl;
        private ModelEditViewModel modelEditViewModel;
        private Point startAnglePoint;
        private Point endAnglePoint;
        private Point centerPoint;
        private double startAngle;
        private double endAngle;
        private double radius;
        public ImageControlView(IEventAggregator eventAggregator, ImageControlViewModel imageControlVM, ModelEditViewModel modelEditViewModel, ImageProcessControl imageProcessControl)
        {
            InitializeComponent();
            this.imageControlVM = imageControlVM;
            this.imageProcessControl = imageProcessControl;
            this.modelEditViewModel = modelEditViewModel;
            zoomAndPanControl.ContentScaleChanged += (s, e) =>
            {
                txtbScaleRate.Text = (zoomAndPanControl.ContentScale * 100.0).ToString("###") + "%";
            };
            this.eventAggregator = eventAggregator;
            eventAggregator.GetEvent<ImageControlViewEvent>().Subscribe(OnReceiveImageControlViewEvent);
            observablePointCollection = new ObservableCollection<Point>();
            this.DataContext = imageControlVM;
            MouseMoveEvent = imageControlVM.MouseMoveEvent;
            
        }

        private enum HitType
        {
            None, Body, UL, UR, BL, BR, LL, RR, UU, BB
        }

        public void ZoomToFit()
        {
            zoomAndPanControl.AnimatedZoomToFit();
        }

        public void UpdateStatus()
        {
            // See how much the mouse has moved.
            foreach (var ui in content.Children)
            {
                if (ui is Image image)
                {
                    var point = Mouse.GetPosition((Image)ui);
                    if (point.X >= 0 && point.Y >= 0 && point.X < image.ActualWidth && point.Y < image.ActualHeight)
                    {
                        if (image.Source is WriteableBitmap im)
                        {
                            var bpp = im.Format.BitsPerPixel / 8;
                            var buffer = new byte[bpp];
                            System.Runtime.InteropServices.Marshal.Copy(im.BackBuffer + ((((int)point.Y * im.PixelWidth) * bpp) + ((int)point.X * bpp)), buffer, 0, bpp);
                            if (bpp == 1)
                            {
                                var grayValue = buffer[0];
                                txtbMouseColor.Text = string.Format("{0}, {0}, {0}", grayValue.ToString("000"));
                            }
                            else if (bpp == 3 || bpp == 4)
                            {
                                System.Runtime.InteropServices.Marshal.Copy(im.BackBuffer + ((((int)point.Y * im.PixelWidth) * bpp) + ((int)point.X * bpp)), buffer, 0, bpp);
                                var b = buffer[0];
                                var g = buffer[1];
                                var r = buffer[2];
                                txtbMouseColor.Text = string.Format("{0}, {1}, {2}", r.ToString("000"), g.ToString("000"), b.ToString("000"));
                            }
                            else
                            {
                                txtbMouseColor.Text = "000, 000, 000";
                            }
                        }

                        txtbMouseX.Text = point.X.ToString("0.00");
                        txtbMouseY.Text = point.Y.ToString("0.00");

                        break;
                    }
                }
            }
        }

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode", Justification = "Using in Xaml")]
        private HitType SetHitType(Rectangle rect, Point point)
        {
            if (rect == null)
            {
                return HitType.None;
            }

            var left = Canvas.GetLeft(rect);
            var top = Canvas.GetTop(rect);
            var p = rect.RenderTransform.Transform(new Point(left, top));
            left = p.X;
            top = p.Y;

            var right = left + rect.Width;
            var bottom = top + rect.Height;
            if (point.X < left)
            {
                return HitType.None;
            }

            if (point.X > right)
            {
                return HitType.None;
            }

            if (point.Y < top)
            {
                return HitType.None;
            }

            if (point.Y > bottom)
            {
                return HitType.None;
            }

            var gap = Math.Max(ActualWidth, ActualHeight) / 80;

            if (point.X - left < gap)
            {
                // Left edge.
                if (point.Y - top < gap)
                {
                    return HitType.UL;
                }

                if (bottom - point.Y < gap)
                {
                    return HitType.BL;
                }

                return HitType.LL;
            }
            else if (right - point.X < gap)
            {
                // Right edge.
                if (point.Y - top < gap)
                {
                    return HitType.UR;
                }

                if (bottom - point.Y < gap)
                {
                    return HitType.BR;
                }

                return HitType.RR;
            }

            if (point.Y - top < gap)
            {
                return HitType.UU;
            }

            if (bottom - point.Y < gap)
            {
                return HitType.BB;
            }

            return HitType.Body;
        }

        private HitType SetHitType(System.Windows.Shapes.Ellipse ellipse, Point point)
        {
            if (ellipse == null)
            {
                return HitType.None;
            }

            var left = Canvas.GetLeft(ellipse);
            var top = Canvas.GetTop(ellipse);
            var p = ellipse.RenderTransform.Transform(new Point(left, top));
            left = p.X;
            top = p.Y;

            var right = left + ellipse.Width;
            var bottom = top + ellipse.Height;
            if (point.X < left)
            {
                return HitType.None;
            }

            if (point.X > right)
            {
                return HitType.None;
            }

            if (point.Y < top)
            {
                return HitType.None;
            }

            if (point.Y > bottom)
            {
                return HitType.None;
            }

            var gap = Math.Max(ActualWidth, ActualHeight) / 80;

            if (point.X - left < gap)
            {
                // Left edge.
                if (point.Y - top < gap)
                {
                    return HitType.UL;
                }

                if (bottom - point.Y < gap)
                {
                    return HitType.BL;
                }

                return HitType.LL;
            }
            else if (right - point.X < gap)
            {
                // Right edge.
                if (point.Y - top < gap)
                {
                    return HitType.UR;
                }

                if (bottom - point.Y < gap)
                {
                    return HitType.BR;
                }

                return HitType.RR;
            }

            if (point.Y - top < gap)
            {
                return HitType.UU;
            }

            if (bottom - point.Y < gap)
            {
                return HitType.BB;
            }

            return HitType.Body;
        }

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode", Justification = "Using in Xaml")]
        private void SetMouseCursor()
        {
            // See what cursor we should display.
            var desired_cursor = Cursors.Arrow;
            switch (mouseHitType)
            {
                case HitType.None:
                    desired_cursor = Cursors.Arrow;
                    break;
                case HitType.Body:
                    desired_cursor = Cursors.ScrollAll;
                    break;
                case HitType.UL:
                    desired_cursor = Cursors.ScrollNW;
                    break;
                case HitType.UR:
                    desired_cursor = Cursors.ScrollNE;
                    break;
                case HitType.BL:
                    desired_cursor = Cursors.ScrollSW;
                    break;
                case HitType.BR:
                    desired_cursor = Cursors.ScrollSE;
                    break;
                case HitType.LL:
                    desired_cursor = Cursors.ScrollW;
                    break;
                case HitType.UU:
                    desired_cursor = Cursors.ScrollN;
                    break;
                case HitType.RR:
                    desired_cursor = Cursors.ScrollE;
                    break;
                case HitType.BB:
                    desired_cursor = Cursors.ScrollS;
                    break;
                default:
                    desired_cursor = Cursors.Arrow;
                    break;
            }

            // Display the desired cursor.
            if (Cursor != desired_cursor)
            {
                Cursor = desired_cursor;
            }
        }

        private void ZoomAndPanControl_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            e.Handled = true;
            if (e.Delta > 0)
            {
                var curContentMousePoint = e.GetPosition(content);
                ZoomIn(curContentMousePoint);
            }
            else if (e.Delta < 0)
            {
                var curContentMousePoint = e.GetPosition(content);
                ZoomOut(curContentMousePoint);
            }
        }

        private void ZoomOut(Point contentZoomCenter)
        {
            zoomAndPanControl.ZoomAboutPoint(zoomAndPanControl.ContentScale - 0.1, contentZoomCenter);
        }

        /// <summary>
        /// Zoom the viewport in, centering on the specified point (in content coordinates).
        /// </summary>
        private void ZoomIn(Point contentZoomCenter)
        {
            zoomAndPanControl.ZoomAboutPoint(zoomAndPanControl.ContentScale + 0.1, contentZoomCenter);
        }

        private void ZoomAndPanControl_MouseDown(object sender, MouseButtonEventArgs e)
        {
            content.Focus();
            Keyboard.Focus(content);

            Rectangle rect = null;
            System.Windows.Shapes.Ellipse ellipse = null;
            var isROIRectangleModify = ROIRectangle.Tag as bool?;
            //var isShapeROIRectangleModify = ShapeROIRectangle.Tag as bool?;
            var isROIEllipseModify = ROIEllipse.Tag as bool?;
            var isROIRectModifyAndNoneNull = isROIRectangleModify != null && (bool)ROIRectangle.Tag == true;
            var isRoiEllipseModifyAndNoneNull = isROIEllipseModify != null && (bool)ROIEllipse.Tag == true;

            if (isROIRectModifyAndNoneNull)
            {
                rect = ROIRectangle;
            }
            else if (isRoiEllipseModifyAndNoneNull)
            {
                ellipse = ROIEllipse;
            }
            else if (isAddingCurvePointList)
            {
                if (e.ChangedButton == MouseButton.Left)
                {
                    var ptX = (int)e.GetPosition(content).X;
                    var ptY = (int)e.GetPosition(content).Y;
                    observablePointCollection.Add(new Point(ptX, ptY));
                    var eventData = new ImageControlViewCallbackEventData()
                    {
                        EventDataObject = observablePointCollection,
                        Msg = "Update ObservableCollection"
                    };
                    eventAggregator.GetEvent<ImageControlViewCallbackEvent>().Publish(eventData);
                }
                else if (e.ChangedButton == MouseButton.Right)
                {
                    isAddingCurvePointList = false;
                    var eventData = new ImageControlViewCallbackEventData()
                    {
                        EventDataObject = new object(),
                        Msg = "Stop adding points"
                    };
                    eventAggregator.GetEvent<ImageControlViewCallbackEvent>().Publish(eventData);
                    var polyline = new Polyline()
                    {
                        Points = new System.Windows.Media.PointCollection(observablePointCollection),
                        Stroke = Brushes.Blue,
                        StrokeThickness = 2,
                    };
                    content.Children.Add(polyline);
                }
                //GluePathPolyline.Points.Add(e.GetPosition(content));
            }
            else
            {
                mouseButtonDown = e.ChangedButton;
                origContentMouseDownPoint = e.GetPosition(content);

                if ((Keyboard.Modifiers & ModifierKeys.Shift) != 0 &&
                    (e.ChangedButton == MouseButton.Left ||
                     e.ChangedButton == MouseButton.Right))
                {
                    // Shift + left- or right-down initiates zooming mode.
                    mouseHandlingMode = MouseHandlingMode.Zooming;
                }
                else if (mouseButtonDown == MouseButton.Left)
                {
                    // Just a plain old left-down initiates panning mode.
                    mouseHandlingMode = MouseHandlingMode.Panning;
                }

                if (mouseHandlingMode != MouseHandlingMode.None)
                {
                    // Capture the mouse so that we eventually receive the mouse up event.
                    zoomAndPanControl.CaptureMouse();
                    e.Handled = true;
                }
            }

            if (e.ClickCount == 2 && (isRoiEllipseModifyAndNoneNull || isROIRectModifyAndNoneNull))
            {
                if (rect != null)
                {
                    Canvas.SetLeft(rect, 0);
                    Canvas.SetTop(rect, 0);
                    rect.Width = ImgShow.Source.Width - 1;
                    rect.Height = ImgShow.Source.Height - 1;
                    return;
                }
            }
            
            mouseHitType = SetHitType(rect, Mouse.GetPosition(content));
            if (isRoiEllipseModifyAndNoneNull)
            {
                if (isAddingStartAnglePoint)
                {
                    imageControlVM.StartAnglePoint = new ObservableCollection<Point>();
                    startAnglePoint = new Point(e.GetPosition(content).X, e.GetPosition(content).Y);
                    imageControlVM.StartAnglePoint.Add(startAnglePoint);

                    var centerX = Canvas.GetLeft(ellipse) + (ellipse.Width / 2);
                    var centerY = Canvas.GetTop(ellipse) + (ellipse.Width / 2);
                    centerPoint = new Point(centerX, centerY);
                    startAngle = GetCircleAngle(centerPoint, startAnglePoint);
                    isAddingStartAnglePoint = false;
                }
                else if (isAddingEndAnglePoint)
                {
                    imageControlVM.EndAnglePoint = new ObservableCollection<Point>();
                    endAnglePoint = new Point(e.GetPosition(content).X, e.GetPosition(content).Y);
                    imageControlVM.EndAnglePoint.Add(endAnglePoint);

                    var centerX = Canvas.GetLeft(ellipse) + (ellipse.Width / 2);
                    var centerY = Canvas.GetTop(ellipse) + (ellipse.Width / 2);
                    centerPoint = new Point(centerX, centerY);
                    endAngle = GetCircleAngle(centerPoint, endAnglePoint);
                    isAddingEndAnglePoint = false;
                }
                else
                {
                    mouseHitType = SetHitType(ellipse, Mouse.GetPosition(content));
                }

                if (startAnglePoint != null && endAnglePoint != null && endAngle != 0 && startAngle != 0)
                {

                    var image = new Image<Bgr, byte>(BitmapFromSource(imageControlVM.ImageSource));
                    radius = (ellipse.Width / 2);
                    if(endAngle <= startAngle)
                    {
                        endAngle += 360;
                    }
                    for (double i = startAngle; i < endAngle; i = i + 0.5)
                    {
                        var point = new Point(centerPoint.X + radius * Math.Cos(i * Math.PI / 180), centerPoint.Y + radius * Math.Sin(i * Math.PI / 180));
                        image[(int)point.Y, (int)point.X] = new Bgr(255, 0, 0);
                    }
                    imageControlVM.ImageSource = imageProcessControl.ConvertImgToBitmapSource(image);
                    imageControlVM.IsShowEllpiseROI = false;
                    imageControlVM.IsModifyEllipseROI = false;

                    imageControlVM.CircleRoiCenterPoint = centerPoint;
                    imageControlVM.CircleRoiRadius = radius;
                    imageControlVM.CircleRoiStartAngle = startAngle;
                    imageControlVM.CircleRoiEndAngle = endAngle;
                }

            }
            SetMouseCursor();

            lastPoint = Mouse.GetPosition(content);
            dragInProgress = true;

            if(MouseDownEvent!=null)
            {
                MouseDownEvent((int)lastPoint.X, (int)lastPoint.Y, e.LeftButton);
            }
        }

        public static System.Drawing.Bitmap BitmapFromSource(BitmapSource bitmapsource)
        {
            System.Drawing.Bitmap bitmap;
            using (var outStream = new System.IO.MemoryStream())
            {
                BitmapEncoder enc = new BmpBitmapEncoder();
                enc.Frames.Add(BitmapFrame.Create(bitmapsource));
                enc.Save(outStream);
                bitmap = new System.Drawing.Bitmap(outStream);
            }
            return bitmap;
        }

        private double GetCircleAngle(Point pointStart, Point pointEnd)
        {
            var dX = pointEnd.X - pointStart.X;
            var dY = pointEnd.Y - pointStart.Y;

            var angle = Math.Atan(dY / dX) * 180 / Math.PI;
            if (dX < 0 && dY > 0)
            {
                angle = 180 + angle;
            }
            else if (dX < 0 && dY < 0)
            {
                angle = 180 + angle;
            }
            else if (dX > 0 && dY < 0)
            {
                angle = 360 + angle;
            }

            return angle;
        }
        
        private void ZoomAndPanControl_MouseUp(object sender, MouseButtonEventArgs e)
        {
            var isROIRectangleModify = ROIRectangle.Tag as bool?;
            var isROIEllipseModify = ROIEllipse.Tag as bool?;
            var isROIRectModifyAndNoneNull = isROIRectangleModify != null && (bool)ROIRectangle.Tag == true;
            //var isShapeRectModifyAndNoneNull = isShapeROIRectangleModify != null && (bool)ShapeROIRectangle.Tag == true;
            var isShapeRectModifyAndNoneNull = isROIEllipseModify != null && (bool)ROIEllipse.Tag == true;

            if (!isROIRectModifyAndNoneNull && !isShapeRectModifyAndNoneNull)
            {
                if (mouseHandlingMode != MouseHandlingMode.None)
                {
                    if (mouseHandlingMode == MouseHandlingMode.Zooming)
                    {
                        if (mouseButtonDown == MouseButton.Left)
                        {
                            // Shift + left-click zooms in on the content.
                            ZoomIn(origContentMouseDownPoint);
                        }
                        else if (mouseButtonDown == MouseButton.Right)
                        {
                            // Shift + left-click zooms out from the content.
                            ZoomOut(origContentMouseDownPoint);
                        }
                    }

                    zoomAndPanControl.ReleaseMouseCapture();
                    mouseHandlingMode = MouseHandlingMode.None;
                    e.Handled = true;
                }
            }
            else
            {
                dragInProgress = false;
            }
            if(MouseUpEvent!=null)
            {
          
                content.Focus();
                Keyboard.Focus(content);
                lastPoint = Mouse.GetPosition(content);
                MouseUpEvent((int)lastPoint.X, (int)lastPoint.Y, e.LeftButton);
            }
        }

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode", Justification = "Using in Xaml")]
        private void ZoomAndPanControl_MouseMove(object sender, MouseEventArgs e)
        {
            if (ImgShow.Source == null)
            {
                return;
            }

            Rectangle rect = null;
            System.Windows.Shapes.Ellipse ellipse = null;
            var isROIRectangleModify = ROIRectangle.Tag as bool?;
            var isROIEllipseModify = ROIEllipse.Tag as bool?;
            var isROIRectModifyAndNoneNull = isROIRectangleModify != null && (bool)ROIRectangle.Tag == true;
            var isROIEllipseModifyAndNoneNull = isROIEllipseModify != null && (bool)ROIEllipse.Tag == true;
            var mousePoint = e.GetPosition(content);

            if (!isROIRectModifyAndNoneNull && !isROIEllipseModifyAndNoneNull)
            {
                if (mousePoint.X < 0 || mousePoint.Y < 0 || mousePoint.X > ImgShow.Source.Width || mousePoint.Y > ImgShow.Source.Height)
                {
                    mousePoint.X = 0;
                    mousePoint.Y = 0;
                }

                if (mouseHandlingMode == MouseHandlingMode.Panning)
                {
                    // The user is left-dragging the mouse.
                    // Pan the viewport by the appropriate amount.
                    var curContentMousePoint = e.GetPosition(content);
                    var dragOffset = curContentMousePoint - origContentMouseDownPoint;
                    zoomAndPanControl.ContentOffsetX -= dragOffset.X;
                    zoomAndPanControl.ContentOffsetY -= dragOffset.Y;
                    e.Handled = true;
                }
            }
            else
            {
                if (isROIRectModifyAndNoneNull)
                {
                    rect = ROIRectangle;
                }

                if (isROIEllipseModifyAndNoneNull)
                {
                    ellipse = ROIEllipse;
                }

                if (dragInProgress && e.LeftButton == MouseButtonState.Pressed)
                {
                    // See how much the mouse has moved.
                    var point = Mouse.GetPosition(content);
                    var offset_x = point.X - lastPoint.X;
                    var offset_y = point.Y - lastPoint.Y;

                    // Get the rectangle's current position.
                    if (rect != null)
                    {
                        var new_x = Canvas.GetLeft(rect);
                        var new_y = Canvas.GetTop(rect);
                        var new_width = rect.Width;
                        var new_height = rect.Height;

                        // Update the rectangle.
                        switch (mouseHitType)
                        {
                            case HitType.Body:
                                new_x += offset_x;
                                new_y += offset_y;
                                break;
                            case HitType.UL:
                                new_x += offset_x;
                                new_y += offset_y;
                                new_width -= offset_x;
                                new_height -= offset_y;
                                break;
                            case HitType.UR:
                                new_y += offset_y;
                                new_width += offset_x;
                                new_height -= offset_y;
                                break;
                            case HitType.BL:
                                new_x += offset_x;
                                new_width -= offset_x;
                                new_height += offset_y;
                                break;
                            case HitType.BR:
                                new_width += offset_x;
                                new_height += offset_y;
                                break;
                            case HitType.LL:
                                new_x += offset_x;
                                new_width -= offset_x;
                                break;
                            case HitType.RR:
                                new_width += offset_x;
                                break;
                            case HitType.BB:
                                new_height += offset_y;
                                break;
                            case HitType.UU:
                                new_y += offset_y;
                                new_height -= offset_y;
                                break;
                        }

                        if ((new_x >= 0) && (new_y >= 0) && (new_width > 0) && (new_height > 0))
                        {
                            Canvas.SetLeft(rect, new_x);
                            Canvas.SetTop(rect, new_y);
                            rect.Width = new_width;
                            rect.Height = new_height;
                            lastPoint = point;
                        }
                    }

                    if (ellipse != null)
                    {
                        var new_x = Canvas.GetLeft(ellipse);
                        var new_y = Canvas.GetTop(ellipse);
                        var new_width = ellipse.Width;
                        var new_height = ellipse.Height;

                        // Update the rectangle.
                        switch (mouseHitType)
                        {
                            case HitType.Body:
                                new_x += offset_x;
                                new_y += offset_y;
                                break;
                            case HitType.UL:
                                new_x += offset_x;
                                new_y += offset_y;
                                new_width -= offset_x;
                                new_height -= offset_y;
                                break;
                            case HitType.UR:
                                new_y += offset_y;
                                new_width += offset_x;
                                new_height -= offset_y;
                                break;
                            case HitType.BL:
                                new_x += offset_x;
                                new_width -= offset_x;
                                new_height += offset_y;
                                break;
                            case HitType.BR:
                                new_width += offset_x;
                                new_height += offset_y;
                                break;
                            case HitType.LL:
                                new_x += offset_x;
                                new_width -= offset_x;
                                new_height -= offset_x;
                                break;
                            case HitType.RR:
                                new_width += offset_x;
                                new_height += offset_x;
                                break;
                            case HitType.BB:
                                new_height += offset_y;
                                new_width += offset_y;
                                break;
                            case HitType.UU:
                                new_y += offset_y;
                                new_height -= offset_y;
                                new_width -= offset_y;
                                break;
                        }

                        if ((new_x >= 0) && (new_y >= 0) && (new_width > 0) && (new_height > 0))
                        {
                            Canvas.SetLeft(ellipse, new_x);
                            Canvas.SetTop(ellipse, new_y);
                            ellipse.Width = new_width;
                            ellipse.Height = new_height;
                            lastPoint = point;
                        }
                    }


                }
                else
                {
                    mouseHitType = SetHitType(rect, Mouse.GetPosition(content));
                    if (ellipse != null)
                    {
                        if (isAddingStartAnglePoint)
                        {

                        }
                        else if (isAddingEndAnglePoint)
                        {

                        }
                        else
                        {
                            mouseHitType = SetHitType(ellipse, Mouse.GetPosition(content));
                        }
                    }
                    SetMouseCursor();
                }
            }

            UpdateStatus();
           // if (e.LeftButton == MouseButtonState.Pressed)
            {
                lock ("")
                {
                    if (MouseMoveEvent != null)
                        MouseMoveEvent((int)mousePoint.X, (int)mousePoint.Y, e.LeftButton);

                }
            }
            if (e.LeftButton == MouseButtonState.Pressed)
            {
                modelEditViewModel.MouseMoveEvent((int)mousePoint.X, (int)mousePoint.Y, System.Windows.Input.MouseButtonState.Pressed);
            }
        }

        private void BtnImgFit_Click(object sender, RoutedEventArgs e)
        {
            zoomAndPanControl.AnimatedZoomToFit();
        }

        private void BtnImgViewPlus_Click(object sender, RoutedEventArgs e)
        {
            ZoomIn(new Point(zoomAndPanControl.ContentZoomFocusX, zoomAndPanControl.ContentZoomFocusY));
        }

        private void BtnImgViewMinus_Click(object sender, RoutedEventArgs e)
        {
            ZoomOut(new Point(zoomAndPanControl.ContentZoomFocusX, zoomAndPanControl.ContentZoomFocusY));
        }

        private void ImgShow_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            zoomAndPanControl.ZoomToFit();
        }

        private void OnReceiveImageControlViewEvent(string msg)
        {
            if(msg == "Start adding point curve")
            {
                isAddingCurvePointList = true;
                observablePointCollection = new ObservableCollection<Point>();
            }

            if(msg == "Stop adding point curve")
            {
                isAddingCurvePointList = false;
                //observablePointCollection = new ObservableCollection<Point>();
            }

            if (msg == "Start adding start angle point")
            {
                isAddingStartAnglePoint = true;
            }

            if (msg == "Start adding end angle point")
            {
                isAddingEndAnglePoint = true;
            }

            if(msg == "Start adding mid angle point")
            {

            }
        }

        System.Windows.Shapes.Ellipse CreateEllipse(double width, double height, double desiredCenterX, double desiredCenterY)
        {
            var ellipse = new System.Windows.Shapes.Ellipse { Width = width, Height = height };
            var left = desiredCenterX - (width / 2);
            var top = desiredCenterY - (height / 2);

            ellipse.Margin = new Thickness(left, top, 0, 0);
            return ellipse;
        }
    }
}

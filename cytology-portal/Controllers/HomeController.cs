using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using System.IO;
using System.Drawing;
using ImageMagick;
using OpenCvSharp;
using BitMiracle.LibTiff.Classic;
using BitMiracle;
using System.Drawing.Imaging;

namespace cytology_portal.Controllers
{
    public class HomeController : Controller
    {
        public ActionResult Index()
        {
            cytology_portal.Models.CytologyViewModel model = new cytology_portal.Models.CytologyViewModel();
            string projectDirectory = Path.GetDirectoryName(AppDomain.CurrentDomain.SetupInformation.ConfigurationFile);
            string[] files = Directory.GetFiles(projectDirectory + "/Content/Images/");
            files.ToList();
            model.ImageNames = new SelectList(files);
            
            return View(model);
        }

        public ActionResult About()
        {
            ViewBag.Message = "Your application description page.";

            return View();
        }

        public ActionResult Contact()
        {
            ViewBag.Message = "Your contact page.";

            return View();
        }
         
        [HttpPost]
        public ActionResult SaveImage(HttpPostedFileBase file, cytology_portal.Models.CytologyViewModel model)
        {
            if (ModelState.IsValid && file != null && file.ContentType.StartsWith("image"))
            {
                string test = file.ContentType;
                string test2 = file.ContentType.FirstOrDefault().ToString();
                var originalFilename = Path.GetFileName(file?.FileName);
                //string fileId = Guid.NewGuid().ToString().Replace("-", "");
                //fileId = originalFilename + fileId;
                var path = Path.Combine(Server.MapPath("~/Content/Images/"), originalFilename);
                file.SaveAs(path);
                
                return RedirectToAction("Index");
            }
            return RedirectToAction("Index"); 
        }
         
        public ActionResult SegmentImage(cytology_portal.Models.CytologyViewModel model)
        {
            //          myBitmap = new Bitmap(@"C:\Documents and   
            //Settings\Joe\Pics\myPic.bmp");
            //         Graphics g = Graphics.FromImage(myBitmap);
            string projectDirectory = Path.GetDirectoryName(AppDomain.CurrentDomain.SetupInformation.ConfigurationFile);
            //Image img = Image.FromFile(projectDirectory + "/Content/Images/image3.png");
            //Graphics g = Graphics.FromImage(img);
            //g.DrawImage(img, 500, 500);
            var filename = Path.GetFileName(model.Filename);
            var path = Path.Combine(Server.MapPath("~/Content/Images/"), "Seg" + filename);
            //Image img = Image.FromFile(projectDirectory + "/Content/Images/" + filename);
            //img.Save(path);

            string[] arguments =
            {
                @"Sample Data\multipage.tif,1",
                "SplitTiffImage_2ndPage.tif"
            };
            
            //TiffCP.Program.Main(arguments);
            //BitMiracle.
            //System.Diagnostics.Process.Start("SplitTiffImage_2ndPage.tif");

            List<byte[]> paginatedData = new List<byte[]>();
            using (Image img = Image.FromFile(model.Filename))
            {
                paginatedData.Clear();
                int frameCount = img.GetFrameCount(FrameDimension.Page);
                for (int i = 0; i < frameCount; i++)
                {
                    img.SelectActiveFrame(new FrameDimension(img.FrameDimensionsList[0]), i);
                    using (MemoryStream memstr = new MemoryStream())
                    {
                        img.Save(memstr, ImageFormat.Tiff);
                        paginatedData.Add(memstr.ToArray());
                    }
                }
            }




            //using (Tiff image = Tiff.Open(model.Filename, "r"))
            //{
            //    if (image == null)
            //        return null;

            //    int dircount = 0;
            //    do
            //    {
            //        var path2 = Path.Combine(Server.MapPath("~/Content/Images/"), dircount + filename);
            //        Tiff image2 = Tiff.Open(path2, "w");
            //        dircount++;
            //    } while (image.ReadDirectory());

            //    System.Console.Out.WriteLine("{0} directories", dircount);
            //}


            //using (Tiff tif = Tiff.Open(model.Filename, "r"))
            //{
            //    short maxDirIndex = 0;
            //    int maxW = 0;
            //    int maxL = 0;
            //    do
            //    {
            //        FieldValue[] imageLength = tif.GetField(TiffTag.IMAGELENGTH);
            //        int  length1 = imageLength[0].ToInt();
            //        FieldValue[] imageWidth = tif.GetField(TiffTag.IMAGEWIDTH);
            //        int width1 = imageWidth[0].ToInt();
            //        if (length1 * width1 > maxL * maxW)
            //        {
            //            maxW = width1;
            //            maxL = length1;
            //            maxDirIndex = tif.CurrentDirectory();
            //        }
            //    } while (tif.ReadDirectory());
            //    tif.SetDirectory(maxDirIndex);
            //    // Find the width and height of the image
            //    FieldValue[] value = tif.GetField(TiffTag.IMAGEWIDTH);
            //    int width = value[0].ToInt();

            //    value = tif.GetField(TiffTag.IMAGELENGTH);
            //    int height = value[0].ToInt();



            //    // Read the image into the memory buffer 
            //    int[] raster = new int[height * width];
            //    if (!tif.ReadRGBAImage(width, height, raster))
            //    { 
            //        return null;
            //    }

            //    using (Bitmap bmp = new Bitmap(width, height, PixelFormat.Format32bppRgb))
            //    {
            //        Rectangle rect = new Rectangle(0, 0, bmp.Width, bmp.Height);

            //        BitmapData bmpdata = bmp.LockBits(rect, ImageLockMode.ReadWrite, PixelFormat.Format32bppRgb);
            //        byte[] bits = new byte[bmpdata.Stride * bmpdata.Height];

            //        for (int y = 0; y < bmp.Height; y++)
            //        {
            //            int rasterOffset = y * bmp.Width;
            //            int bitsOffset = (bmp.Height - y - 1) * bmpdata.Stride;

            //            for (int x = 0; x < bmp.Width; x++)
            //            {
            //                int rgba = raster[rasterOffset++];
            //                bits[bitsOffset++] = (byte)((rgba >> 16) & 0xff);
            //                bits[bitsOffset++] = (byte)((rgba >> 8) & 0xff);
            //                bits[bitsOffset++] = (byte)(rgba & 0xff);
            //                bits[bitsOffset++] = (byte)((rgba >> 24) & 0xff);
            //            }
            //        }

            //        System.Runtime.InteropServices.Marshal.Copy(bits, 0, bmpdata.Scan0, bits.Length);
            //        bmp.UnlockBits(bmpdata);
            //        var path3 = Path.Combine(Server.MapPath("~/Content/Images/"), "TiffTo32BitBitmapcyto.bmp");
            //        bmp.Save(path3);
            //        System.Diagnostics.Process.Start(path3);
            //    }
            //}


            using (MagickImage image = new MagickImage(model.Filename))
            {
                
                //image.RePage();
                image.Crop(model.X, model.Y, model.Width, model.Height);
                image.Segment(ColorSpace.Lab, model.ClusteringThreshold, model.SmoothingThreshold);
                var fileinfo = new FileInfo(path);
                image.Write(fileinfo);
                //var originalFilename = Path.GetFileName(model.Filename);
                //var path = Path.Combine(Server.MapPath("~/Content/Images/"), originalFilename);
            }
            //using (Mat image = new Mat(projectDirectory + "/Content/Images/image3.png"))
            //{
            //    using (Window window = new Window(image))
            //    {
            //        Window.WaitKey();
            //    }
            //}


            //string[] files = Directory.GetFiles(projectDirectory + "/Content/Images/");
            //foreach(var s in files)
            //{
            //    var name = Path.GetFileName(s.ToString());
            //}
            //Image.FromFile
            //    System.IO.i



            return RedirectToAction("Index");
        }
    }
}
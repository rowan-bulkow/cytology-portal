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
using System.Windows.Media.Imaging;

namespace cytology_portal.Controllers
{
    public class HomeController : Controller
    {
        public ActionResult Index(string imagePath = "", string filename = "", string histogram = "")
        {
            cytology_portal.Models.CytologyViewModel model = new cytology_portal.Models.CytologyViewModel();
            model.Filename = filename;
            if (histogram != "")
            {
                int[] histogramArray = { 0 };
            } 
            string projectDirectory = Path.GetDirectoryName(AppDomain.CurrentDomain.SetupInformation.ConfigurationFile);
            string[] files = Directory.GetFiles(projectDirectory + "/Content/Images/");
            for (int i = 0; i < files.Length; i++)
            {
                files[i] = Path.GetFileName(files[i]);
            }
            files.ToList();
            model.ImageNames = new SelectList(files);
            model.ImagePath = imagePath;
            int[] test = { 1, 3, 5, 7, 9 };
            model.Histogram = test;
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
                var path = Path.Combine(Server.MapPath("~/Content/Images/"), originalFilename);
                file.SaveAs(path);
                
                return RedirectToAction("Index");
            }
            return RedirectToAction("Index"); 
        }

        

        public ActionResult DisplayThumbnail(cytology_portal.Models.CytologyViewModel model)
        {
            int pages;
            var imagePath = "";

            var originalFilename = Path.GetFileName(model.Filename);
            var path = Path.Combine(Server.MapPath("~/Content/Images/"), originalFilename);
            var thumbPath = Path.Combine(Server.MapPath("~/Content/Images/thumbs/"), Path.GetFileNameWithoutExtension(model.Filename) + ".png");

            if (System.IO.File.Exists(thumbPath))
            {
                imagePath = "./Content/Images/thumbs/" + Path.GetFileNameWithoutExtension(model.Filename) + ".png";
            }
            else
            {
                using (Image image2 = Image.FromFile(path))
                {
                    pages = image2.GetFrameCount(FrameDimension.Page);
                    if (pages > 1)
                    {
                        image2.SelectActiveFrame(FrameDimension.Page, 1);
                    }
                    imagePath = "./Content/Images/thumbs/" + Path.GetFileNameWithoutExtension(model.Filename) + ".png";
                    var thumbnail = image2.GetThumbnailImage(360, 360, null, IntPtr.Zero);
                    thumbnail.Save(thumbPath, ImageFormat.Png);
                }
            }
            return RedirectToAction("Index", "Home", new { imagePath = imagePath , filename = model.Filename , histogram = model.Histogram});

        }

        public ActionResult SegmentImage(cytology_portal.Models.CytologyViewModel model)
        { 
            
            //scale selection based on 48000 x 48000
            double percentWidth = ((double)model.Width)/ 360;
            double newWidthDouble = 48000 * percentWidth;
            int newWidth = (int)(newWidthDouble);

            double percentHeight = ((double)model.Height) / 360;
            double newHeightDouble = 48000 * percentHeight;
            int newHeight = (int)(newHeightDouble);

            double percentX = ((double)model.X) / 360;
            double newXDouble = 48000 * percentX;
            int newX = (int)(newXDouble);

            double percentY = ((double)model.Y) / 360;
            double newYDouble = 48000 * percentY;
            int newY = (int)(newYDouble);


            //crop image based on what user selected
            string newFileName = Path.GetFileNameWithoutExtension(model.Filename) + model.Width + model.Height + model.X + model.Y;
            System.Diagnostics.Process cropProcess = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo startInfo1 = new System.Diagnostics.ProcessStartInfo();
            startInfo1.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
            startInfo1.FileName = "C:/Windows/System32/cmd.exe";
            startInfo1.Arguments = "/C C:/ImageMagick-7.0.7-Q16/magick.exe convert -define registry:temporary-path=./Content/Images/ -extract "+ newWidth + "x" + newHeight + "+" + newX + "+" + newY + " ./Content/Images/" + model.Filename + "[2] ../images/" + newFileName + ".tif";
            cropProcess.StartInfo = startInfo1;
            cropProcess.Start();
            cropProcess.WaitForExit();

            //segmentation will work if all the libraries are functional
            System.Diagnostics.Process segmentProcess = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo startInfo2 = new System.Diagnostics.ProcessStartInfo();
            startInfo2.WindowStyle = System.Diagnostics.ProcessWindowStyle.Normal;
            startInfo2.FileName = "cmd.exe";
            startInfo2.Arguments = "/c ../analyzer/segment -p ../images/" + newFileName + ".tif";
            segmentProcess.StartInfo = startInfo2;
            segmentProcess.Start();
            segmentProcess.WaitForExit();

            //read histogram data generated from segmentation code
            string line;
            List<int> nucleiSizeList = new List<int>();
            DirectoryInfo dirInfo = new DirectoryInfo("C:/Users/Joshua/source/repos/cytology-portal/analyzer/");
            FileInfo histogramFile = dirInfo.GetFiles("*.txt").OrderByDescending(f => f.LastWriteTime).First();
            System.IO.StreamReader file = new System.IO.StreamReader(histogramFile.FullName);
            while ((line = file.ReadLine()) != null)
            {
                nucleiSizeList.Add(Int32.Parse(line.Split('.')[0])); 
            } 
            file.Close(); 

            int[] histogram = new int[nucleiSizeList.Count];
            
            for(int i = 0; i < nucleiSizeList.Count; i++)
            {
                histogram[i] = nucleiSizeList.ElementAt(i);
            }
            model.Histogram = histogram;
            return View(model); 
        }
        
    }
}
using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using System.IO;
using System.Drawing;
using ImageMagick;
using OpenCvSharp;

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
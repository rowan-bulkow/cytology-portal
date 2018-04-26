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
        public ActionResult Index(string imagePath = "", string filename = "")
        {
            cytology_portal.Models.CytologyViewModel model = new cytology_portal.Models.CytologyViewModel();
            model.Filename = filename;
            string projectDirectory = Path.GetDirectoryName(AppDomain.CurrentDomain.SetupInformation.ConfigurationFile);
            string[] files = Directory.GetFiles(projectDirectory + "/Content/Images/");
            for (int i = 0; i < files.Length; i++)
            {
                files[i] = Path.GetFileName(files[i]);
            }
            files.ToList();
            model.ImageNames = new SelectList(files);
            model.ImagePath = imagePath;
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
                    var thumbnail = image2.GetThumbnailImage(300, 300, null, IntPtr.Zero);
                    thumbnail.Save(thumbPath, ImageFormat.Png);
                }
            }
            return RedirectToAction("Index", "Home", new { imagePath = imagePath , filename = model.Filename });

        }

        public ActionResult SegmentImage(cytology_portal.Models.CytologyViewModel model)
        { 
            //Integrate with c++ code for this task

            //string projectDirectory = Path.GetDirectoryName(AppDomain.CurrentDomain.SetupInformation.ConfigurationFile); 
            //var filename = Path.GetFileName(model.Filename);
            //var path = Path.Combine(Server.MapPath("~/Content/Images/results/"), "Seg" + filename); 
              
            //using (MagickImage image = new MagickImage("~/Content/Images/" + model.Filename))
            //{ 
            //    image.Crop(model.X, model.Y, model.Width, model.Height); 
            //    var fileinfo = new FileInfo(path);
            //    image.Write(fileinfo); 
            //}  
            return RedirectToAction("Index");
        }
        
    }
}
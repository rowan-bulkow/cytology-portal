using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using System.IO;

namespace cytology_portal.Controllers
{
    public class HomeController : Controller
    {
        public ActionResult Index()
        {
            return View();
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
        public ActionResult SaveImage(HttpPostedFileBase file)
        {
            if (ModelState.IsValid && file != null)
            {
                var originalFilename = Path.GetFileName(file?.FileName);
                //string fileId = Guid.NewGuid().ToString().Replace("-", "");
                //fileId = originalFilename + fileId;
                var path = Path.Combine(Server.MapPath("~/Content/Images/"), originalFilename);
                file.SaveAs(path);
                
                return RedirectToAction("Index");
            }
            return RedirectToAction("Index"); 
        }
    }
}
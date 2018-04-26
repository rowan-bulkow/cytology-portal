using System.Collections.Generic;
using System.Web.Mvc;

namespace cytology_portal.Models
{
    public class CytologyViewModel
    {
        public double SmoothingThreshold { get; set; }
        public double ClusteringThreshold { get; set; }
        public int X { get; set; }
        public int Y { get; set; }
        public int Height { get; set; }
        public int Width { get; set; }
        public string Filename { get; set; }
        public string ImagePath { get; set; }
        public SelectList ImageNames { get; set; } 
    }
}

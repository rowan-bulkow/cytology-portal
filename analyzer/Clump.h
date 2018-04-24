#include "opencv2/opencv.hpp"

using namespace std;

namespace segment
{
    // Clump stores information on a single cell clump within the image,
    // and houses a few related helper functions
    class Clump
    {
    public:
        // attributes
        cv::Mat* imgptr;
        vector<cv::Point> contour;
        cv::Rect boundingRect;
        vector<vector<cv::Point> > nucleiBoundaries;
        vector<cv::Point> nucleiCenters;

        // member functions
        // Given that imgptr and contour are defined, compute the bounding rect
        cv::Rect computeBoundingRect();
        // Mask the clump from the original image, return the result
        cv::Mat extractFull();
        // mask the clump from the image, then return image cropped to show only the clump
        cv::Mat extract();
        // If nucleiBoundaries are defined, compute the center of each nuclei
        vector<cv::Point> computeCenters();
    };
}

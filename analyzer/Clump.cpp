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
        cv::Mat extractFull(bool showBoundary=false);
        // mask the clump from the image, then return image cropped to show only the clump
        cv::Mat extract(bool showBoundary=false);
        // If nucleiBoundaries are defined, compute the center of each nuclei
        vector<cv::Point> computeCenters();
    };

    // Given that contour is defined, compute the bounding rect
    cv::Rect Clump::computeBoundingRect()
    {
        if(this->contour.empty()) std::cerr << "Contour must be defined before Clump::computeBoundingRect() can be run." << '\n';
        cv::Rect rect = cv::boundingRect(this->contour);
        this->boundingRect = rect;
        return rect;
    }

    // Mask the clump from the original image, return the result
    cv::Mat Clump::extractFull(bool showBoundary)
    {
        cv::Mat img = *this->imgptr;
        // this->imgptr->copyTo(img);

        // create clump mask
        cv::Mat mask = cv::Mat::zeros(img.rows, img.cols, CV_8U);
        cv::drawContours(mask, vector<vector<cv::Point> >(1, this->contour), 0, cv::Scalar(255), CV_FILLED);
        cv::Mat clumpFull = cv::Mat(img.rows, img.cols, CV_8U);
        img.copyTo(clumpFull, mask);

        if(showBoundary)
            cv::drawContours(clumpFull, vector<vector<cv::Point> >(1, this->contour), 0, cv::Scalar(255, 0, 255));

        // invert the mask and then invert the black pixels in the extracted image
        cv::bitwise_not(mask, mask);
        cv::bitwise_not(clumpFull, clumpFull, mask);

        return clumpFull;
    }

    // Mask the clump from the original image, then return a mat cropped to show
    // only this clump
    cv::Mat Clump::extract(bool showBoundary)
    {
        cv::Mat img = this->extractFull(showBoundary);
        if(this->boundingRect.empty()) std::cerr << "boundingRect must be defined before Clump::extract() can be run." << '\n';
        cv::Mat clump = cv::Mat(img, this->boundingRect);
        if(showBoundary)
            cv::drawContours(clump, vector<vector<cv::Point> >(1, this->contour), 0, cv::Scalar(255, 0, 255));
        return clump;
    }

    // If nucleiBoundaries are defined, compute the center of each nuclei
    vector<cv::Point> Clump::computeCenters()
    {
        // TODO stubbed method
        return vector<cv::Point>();
    }
}

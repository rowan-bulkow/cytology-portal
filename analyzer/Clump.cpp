#include "opencv2/opencv.hpp"
#include "Clump.h"

using namespace std;

namespace segment
{
    // Given that contour is defined, compute the bounding rect
    cv::Rect Clump::computeBoundingRect()
    {
        cv::Rect rect = cv::boundingRect(this->contour);
        this->boundingRect = rect;
        return rect;
    }
    // Mask the clump from the original image, return the result
    cv::Mat Clump::extractFull()
    {
        cv::Mat img;
        this->imgptr->copyTo(img);

        // create clump mask
        cv::Mat mask = cv::Mat::zeros(img.rows, img.cols, CV_8U);
        cv::drawContours(mask, vector<vector<cv::Point> >(1, this->contour), 0, cv::Scalar(255), CV_FILLED);
        img.copyTo(img, mask);

        // invert the mask and then invert the black pixels in the extracted image
        cv::bitwise_not(mask, mask);
        cv::bitwise_not(img, img, mask);

        return img;
    }
    // Mask the clump from the original image, then return a mat cropped to show
    // only this clump
    cv::Mat Clump::extract()
    {
        cv::Mat img = this->extractFull();
        cv::Mat clump = cv::Mat(img, this->boundingRect);
        return clump;
    }
    // If nucleiBoundaries are defined, compute the center of each nuclei
    vector<cv::Point> Clump::computeCenters()
    {
        // TODO stubbed method
        return vector<cv::Point>();
    }
}

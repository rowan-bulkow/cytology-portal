#include <stdlib.h>
#include "opencv2/opencv.hpp"
#include "Cell.cpp"

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
        cv::Mat clumpMat;
        vector<cv::Point> contour;
        vector<cv::Point> offsetContour;
        cv::Rect boundingRect;
        vector<vector<cv::Point> > nucleiBoundaries;
        vector<cv::Point> nucleiCenters;
        cv::Mat nucleiAssocs;
        unsigned int numCells;
        vector<Cell> cells;

        // member functions
        // Given that imgptr and contour are defined, compute the bounding rect
        cv::Rect computeBoundingRect();
        // Given the contour and bounding rect are defined, find the contour offset
        // by the bounding rect
        vector<cv::Point> computeOffsetContour();
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
        if(this->contour.empty())
            std::cerr << "Contour must be defined before Clump::computeBoundingRect() can be run." << '\n';
        this->boundingRect = cv::boundingRect(this->contour);
        return this->boundingRect;
    }

    vector<cv::Point> Clump::computeOffsetContour()
    {
        if(this->boundingRect.empty())
            std::cerr << "boundingRect must be defined before Clump::computeOffsetContour() can be run." << '\n';
        for(unsigned int i=0; i<this->contour.size(); i++)
            this->offsetContour.push_back(cv::Point(this->contour[i].x - this->boundingRect.x, this->contour[i].y - this->boundingRect.y));
        return this->offsetContour;
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
        if(this->boundingRect.empty())
            std::cerr << "boundingRect must be defined before Clump::extract() can be run." << '\n';
        cv::Mat clump = cv::Mat(img, this->boundingRect);
        if(showBoundary)
            cv::drawContours(clump, vector<vector<cv::Point> >(1, this->contour), 0, cv::Scalar(255, 0, 255));
        this->clumpMat = clump;
        return this->clumpMat;
    }

    // If nucleiBoundaries are defined, compute the center of each nuclei
    vector<cv::Point> Clump::computeCenters()
    {
        if(this->nucleiBoundaries.empty())
            cerr << "nucleiBoundaries must be defined and present before Clump::computeCenters() can be run." << "\n";
        this->nucleiCenters.clear();
        for(vector<cv::Point> nucleus : nucleiBoundaries)
        {
            double sumX = 0.0, sumY = 0.0;
            int count = 0;
            for(cv::Point p : nucleus)
            {
                sumX += p.x;
                sumY += p.y;
                count++;
            }
            this->nucleiCenters.push_back(cv::Point(sumX/count, sumY/count));
        }

        return this->nucleiCenters;
    }
}

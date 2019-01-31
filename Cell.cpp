#include <stdlib.h>
#include "opencv2/opencv.hpp"

using namespace std;

namespace segment
{
    class Cell
    {
    public:
        cv::Vec3b color;
        cv::Point nucleusCenter;
        cv::Point center;
        vector<cv::Point> nucleusBoundary;
        vector<cv::Point> boundary;

        cv::Vec3b generateColor();
        cv::Point computeCenter();
    };

    cv::Vec3b Cell::generateColor()
    {
        this->color = cv::Vec3b(rand()%150+50, rand()%150+50, rand()%150+50);
        return this->color;
    }

    // if the nucleusBoundary is defined, compute the center of the nucleus
    cv::Point Cell::computeCenter()
    {
        if(this->nucleusBoundary.empty())
            cerr << "nucleusBoundary must be defined and present before Cell::computeCenter() can be run." << "\n";
        double sumX = 0.0, sumY = 0.0;
        int count = 0;
        for(cv::Point p : nucleusBoundary)
        {
            sumX += p.x;
            sumY += p.y;
            count++;
        }
        this->nucleusCenter = cv::Point(sumX/count, sumY/count);

        return this->nucleusCenter;
    }
}

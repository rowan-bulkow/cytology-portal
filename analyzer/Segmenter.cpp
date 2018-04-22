#include <iostream>
#include <ctime>
#include <stdio.h>
#include "opencv2/opencv.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "SegmenterTools.cpp"
//extern "C" {
//    #include "vl/quickshift.h"
//}

using namespace std;

namespace segment
{
    class Segmenter
    {
    public:
        // Quickshift params
        int kernelsize = 2;
        int maxdist = 4;
        // Canny params
        int threshold1 = 20;
        int threshold2 = 40;
        // GMM params
        int maxGmmIterations = 10;
        // GMM post processing params
        double minAreaThreshold = 50.0;
        // MSER params
        int delta = 8, minArea = 15, maxArea = 100;
        double maxVariation = 0.5, minDiversity = 0.25;

    private:
        // internal attributes
        bool debug = false;
        cv::Scalar pink;
        int allContours = -1;
        bool totalTimed = true;

    public:
        // Constructors
        Segmenter() { setCommonValues(); }
        Segmenter(int kernelsize, int maxdist, int maxGmmIterations)
        {
            setCommonValues();
            this->kernelsize = kernelsize;
            this->maxdist = maxdist;
            this->maxGmmIterations = maxGmmIterations;
        }

        Segmenter(int kernelsize, int maxdist, int thres1, int thres2,  int maxGmmIterations, int minAreaThreshold, int delta, int minArea, int maxArea, double maxVariation, double minDiversity)
        {
            setCommonValues();
            this->kernelsize = kernelsize;
            this->maxdist = maxdist;
            this->maxGmmIterations = maxGmmIterations;
            this->threshold1 = thres1;
            this->threshold2 = thres2;
            this->minAreaThreshold = minAreaThreshold;
            this->delta = delta;
            this->minArea = minArea;
            this->maxArea = maxArea;
            this->maxVariation = maxVariation;
            this->minDiversity = minDiversity;
        }
        // Constructor helper
        void setCommonValues()
        {
            pink = cv::Scalar(255, 0, 255);
        }


        void runSegmentation(string fileName) {
            clock_t total = clock();
            clock_t start;
            double end;

            cv::Mat image = cv::imread(fileName);
            cv::Mat outimg;

            // meta-data about the image
            int channels = image.channels();
            int width = image.cols;
            int height = image.rows;
            if(debug) printf("Image data: (rows) %i (cols) %i (channels) %i\n", height, width, channels);

            segment::SegmenterTools segTools = segment::SegmenterTools();


            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            // Quickshift
            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            start = clock();
            if(debug) printf("Beginning quickshift...\n");

            cv::Mat postQuickShift = segTools.runQuickshift(image, kernelsize, maxdist);
            cv::imwrite("../images/quickshifted_cyto.png", postQuickShift);

            end = (clock() - start) / CLOCKS_PER_SEC;
            if(debug) printf("Quickshift complete, time:%f\n", end);


            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            // Canny Edge Detection
            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            start = clock();
            if(debug) printf("Beginning Edge Detection...\n");

            cv::Mat postEdgeDetection = segTools.runCanny(postQuickShift, threshold1, threshold2);
            cv::imwrite("../images/edgeDetectedEroded_cyto.png", postEdgeDetection);

            end = (clock() - start) / CLOCKS_PER_SEC;
            if(debug) printf("Edge Detection complete, time: %f\n", end);


            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            // Connected Components Analysis and Convex Hulls
            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            start = clock();
            if(debug) printf("Beginning CCA and building convex hulls...\n");

            // find contours
            vector<vector<cv::Point> > contours;
            cv::findContours(postEdgeDetection, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

            image.copyTo(outimg);
            outimg.convertTo(outimg, CV_8UC3);
            cv::drawContours(outimg, contours, allContours, pink);
            cv::imwrite("../images/contours_cyto.png", outimg);

            // find convex hulls
            vector<vector<cv::Point> > hulls(contours.size());
            for(unsigned int i=0; i<contours.size(); i++)
                cv::convexHull(cv::Mat(contours[i]), hulls[i], false);

            image.copyTo(outimg);
            outimg.convertTo(outimg, CV_8UC3);
            cv::drawContours(outimg, hulls, allContours, pink);
            cv::imwrite("../images/hulls_cyto.png", outimg);

            end = (clock() - start) / CLOCKS_PER_SEC;
            if(debug) printf("Finished with CCA and convex hulls, time: %f\n", end);


            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            // Gaussian Mixture Modeling
            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            start = clock();
            if(debug) printf("Beginning Gaussian Mixture Modeling...\n");

            cv::Mat gmmPredictions = segTools.runGmm(image, hulls, maxGmmIterations);
            cv::imwrite("../images/raw_gmm.png", gmmPredictions);

            end = (clock() - start) / CLOCKS_PER_SEC;
            if(debug) printf("Finished with Gaussian Mixture Modeling, time:%f\n", end);


            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            // GMM Post Processing
            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            start = clock();
            if(debug) printf("Beginning GMM Output post processing...\n");

            vector<vector<cv::Point> > clumpBoundaries = segTools.findFinalClumpBoundaries(gmmPredictions, minAreaThreshold);

            unsigned int numClumps = clumpBoundaries.size();
            image.convertTo(outimg, CV_8UC3);
            cv::drawContours(outimg, clumpBoundaries, allContours, pink);
            cv::imwrite("../images/clumpBoundaries.png", outimg);

            // extract each clump from the original image
            vector<cv::Mat> clumps = vector<cv::Mat>();
            for(unsigned int i=0; i<clumpBoundaries.size(); i++)
            {
                cv::Mat clump = segTools.extractClump(image, clumpBoundaries, i);
                clumps.push_back(clump);

                char buffer[200];
                sprintf(buffer, "../images/clumps/clump_%i.png", i);
                clump.convertTo(outimg, CV_8UC3);
                cv::imwrite(buffer, outimg);
            }

            end = (clock() - start) / CLOCKS_PER_SEC;
            if(debug) printf("Finished GMM post processing, clumps found:%i, time:%f\n", numClumps, end);


            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            // MSER for Nuclei Detection
            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            start = clock();
            if(debug) printf("Beginning MSER nuclei detection...\n");

            // this is getting a little ridiculous - should probably define a custom data type
            vector<vector<vector<cv::Point> > > nucleiBoundaries = vector<vector<vector<cv::Point> > >();
            for(unsigned int i=0; i<clumps.size(); i++)
            {
                vector<vector<cv::Point> > regions = segTools.runMser(clumps[i], delta, minArea, maxArea, maxVariation, minDiversity);
                nucleiBoundaries.push_back(regions);
                clumps[i].convertTo(outimg, CV_8UC3);
                cv::drawContours(outimg, regions, -1, cv::Scalar(255, 0, 255), 1);
                char buffer[200];
                sprintf(buffer, "../images/clumps/clump_%i_nuclei.png", i);
                cv::imwrite(buffer, outimg);
            }
            end = (clock() - start) / CLOCKS_PER_SEC;
            if(debug) printf("Finished MSER nuclei detection, time:%f\n", end);


            // clean up
            end = (clock() - total) / CLOCKS_PER_SEC;
            if(debug || totalTimed) printf("Segmentation finished, total time:%f\n", end);
        }
    };
}

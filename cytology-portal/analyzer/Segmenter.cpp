#include <iostream>
#include <ctime>
#include <stdio.h>
#include "opencv2/opencv.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "Clump.cpp"
#include "SegmenterTools.cpp"
extern "C" {
   #include "vl/quickshift.h"
}

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
        double minAreaThreshold = 200.0;
        // MSER params
        int delta = 8, minArea = 15, maxArea = 100;
        double maxVariation = 0.5, minDiversity = 0.25;

    private:
        // internal attributes
        bool debug = true;
        cv::Scalar pink;
        int allContours = -1;
        bool totalTimed = true;

    public:
        // Constructors
        Segmenter() { setCommonValues(); }
        Segmenter(int kernelsize, int maxdist)
        {
            setCommonValues();
            this->kernelsize = kernelsize;
            this->maxdist = maxdist;
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

            // check for image
            if(image.empty())
            {
                cerr << "Could not read image at: " << fileName << endl;
                exit(1);
            }

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

            cv::Mat postEdgeDetection = segTools.runCanny(postQuickShift, threshold1, threshold2, true);
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


            // TODO I think this is a good idea -- but at the moment the focus should
            // be on individual cell segmentation, not improving clump segmentation
            // cv::Mat preHulls = cv::Mat::zeros(height, width, CV_8U);
            // cv::drawContours(preHulls, contours, allContours, cv::Scalar(255, 255, 255), CV_FILLED);
            // cv::imwrite("../images/test_contoursfull.png", preHulls);
            // cv::erode(preHulls, preHulls, cv::Mat());
            // cv::imwrite("../images/test_contourseroded.png", preHulls);
            // cv::findContours(preHulls, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
            // image.copyTo(outimg);
            // outimg.convertTo(outimg, CV_8UC3);
            // cv::drawContours(outimg, contours, allContours, pink);
            // cv::imwrite("../images/test_erodedcontours_cyto.png", outimg);


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
            vector<Clump> clumps = vector<Clump>();
            for(unsigned int i=0; i<clumpBoundaries.size(); i++)
            {
                Clump clump;
                clump.imgptr = &image;
                clump.contour = vector<cv::Point>(clumpBoundaries[i]);
                clump.computeBoundingRect();
                clumps.push_back(clump);

                // char buffer[200];
                // sprintf(buffer, "../images/clumps/clump_%i.png", i);
                // clump.extract().convertTo(outimg, CV_8UC3);
                // cv::imwrite(buffer, outimg);
            }

            end = (clock() - start) / CLOCKS_PER_SEC;
            if(debug) printf("Finished GMM post processing, clumps found:%i, time:%f\n", numClumps, end);


            // clump extraction testing
            if(false)
            {
                for(unsigned int i=0; i<clumps.size(); i++)
                {
                    cv::Mat clump = clumps[i].extractFull(true);
                    char buffer[200];
                    sprintf(buffer, "../images/clumps/clump_%i_full.png", i);
                    cv::imwrite(buffer, clump);
                }
            }


            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            // MSER for Nuclei Detection
            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            start = clock();
            if(debug) printf("Beginning MSER nuclei detection...\n");

            for(unsigned int i=0; i<clumps.size(); i++)
            {
                cv::Mat clump = clumps[i].extract();
                clumps[i].nucleiBoundaries = segTools.runMser(clump, clumps[i].computeOffsetContour(),
                    delta, minArea, maxArea, maxVariation, minDiversity);
            }

            // remove clumps that don't have any nuclei
            int tempindex = 0;
            unsigned int numchecked = 0;
            unsigned int originalsize = clumps.size();
            while(clumps.size() > 0 && numchecked<originalsize)
            {
                if(clumps[tempindex].nucleiBoundaries.empty())
                {
                    clumps.erase(clumps.begin()+tempindex);
                    tempindex--;
                }
                tempindex++;
                numchecked++;
            }

            // write all the found clumps with nuclei
            for(unsigned int i=0; i<clumps.size(); i++)
            {
                cv::Mat clump = clumps[i].extract();
                char buffer[200];

                clump.convertTo(outimg, CV_8UC3);
                sprintf(buffer, "../images/clumps/clump_%i.png", i);
                cv::imwrite(buffer, outimg);

                clump.convertTo(outimg, CV_8UC3);
                cv::drawContours(outimg, clumps[i].nucleiBoundaries, allContours, pink);
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

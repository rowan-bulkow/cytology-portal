#include <iostream>
#include <ctime>
#include <stdio.h>
#include "opencv2/opencv.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "VLFeatWrapper.cpp"
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
        // post GMM numbers
        double minAreaThreshold = 50.0;

        // internal attributes
        bool debug = false;
        cv::Scalar pink;
        int allContours = -1;

        // Constructors
        Segmenter() { setCommonValues(); }
        Segmenter(int kernelsize, int maxdist, int maxGmmIterations)
        {
            setCommonValues();
            this->kernelsize = kernelsize;
            this->maxdist = maxdist;
            this->maxGmmIterations = maxGmmIterations;
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


            // Quickshift
            start = clock();
            if(debug) printf("Beginning quickshift...\n");

            cv::Mat postQuickShift = runQuickshift(image, kernelsize, maxdist);
            cv::imwrite("../images/quickshifted_cyto.png", postQuickShift);

            end = (clock() - start) / CLOCKS_PER_SEC;
            if(debug) printf("Quickshift complete, time:%f\n", end);


            // Canny Edge Detection
            start = clock();
            if(debug) printf("Beginning Edge Detection...\n");

            cv::Mat postEdgeDetection = runCanny(postQuickShift, threshold1, threshold2);
            cv::imwrite("../images/edgeDetectedEroded_cyto.png", postEdgeDetection);

            end = (clock() - start) / CLOCKS_PER_SEC;
            if(debug) printf("Edge Detection complete, time: %f\n", end);


            // Connected Components analysis and building convex hulls
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

            cv::Mat gmmPredictions = runGmm(image, hulls, maxGmmIterations);
            cv::imwrite("../images/raw_gmm.png", gmmPredictions);

            end = (clock() - start) / CLOCKS_PER_SEC;
            if(debug) printf("Finished with Gaussian Mixture Modeling, time:%f\n", end);


            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            // GMM Post Processing
            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            start = clock();
            if(debug) printf("Beginning GMM Output post processing...\n");

            vector<vector<cv::Point> > clumpBoundaries = findFinalClumpBoundaries(gmmPredictions, minAreaThreshold);

            unsigned int numClumps = clumpBoundaries.size();
            image.convertTo(outimg, CV_8UC3);
            cv::drawContours(outimg, clumpBoundaries, allContours, pink);
            cv::imwrite("../images/clumpBoundaries.png", outimg);

            // find the bounding rectangle for each clump, mask the original image, then extract the clump
            vector<cv::Rect> boundingRects = vector<cv::Rect>();
            vector<cv::Mat> clumps = vector<cv::Mat>();
            for(unsigned int i=0; i<clumpBoundaries.size(); i++)
            {
                // create a mask for each clump and apply it
                cv::Mat mask = cv::Mat::zeros(image.rows, image.cols, CV_8U);
                cv::drawContours(mask, clumpBoundaries, i, cv::Scalar(255), CV_FILLED);
                cv::Mat fullMasked = cv::Mat(image.rows, image.cols, CV_8U);
                fullMasked.setTo(cv::Scalar(255, 0, 255));
                image.copyTo(fullMasked, mask);
                // invert the mask and then invert the black pixels in the extracted image
                cv::bitwise_not(mask, mask);
                cv::bitwise_not(fullMasked, fullMasked, mask);

                // grab the bounding rect for each clump
                cv::Rect rect = cv::boundingRect(clumpBoundaries[i]);
                boundingRects.push_back(rect);

                // create mat of each clump
                cv::Mat clump = cv::Mat(fullMasked, rect);
                clumps.push_back(clump);

                char buffer[200];
                sprintf(buffer, "../images/clumps/clump_%i.png", i);
                clump.convertTo(outimg, CV_8UC3);
                cv::imwrite(buffer, outimg);
            }

            end = (clock() - start) / CLOCKS_PER_SEC;
            if(debug) printf("Finished GMM post processing, clumps found:%i, time:%f\n", numClumps, end);


            // MSER Magic Numbers
            // parameters for mser from trial and error
            int delta = 8, minArea = 15, maxArea = 100;
            double maxVariation = 0.5, minDiversity = 0.25;

            // MSER for nuclei Detection
            start = clock();
            if(debug) printf("Beginning MSER nuclei detection...");
            // this is getting a little ridiculous - should probably define a custom data type
            vector<vector<vector<cv::Point> > > nucleiBoundaries = vector<vector<vector<cv::Point> > >();
            for(unsigned int i=0; i<clumps.size(); i++)
            {
                vector<vector<cv::Point> > regions = runMser(clumps[i], delta, minArea, maxArea, maxVariation, minDiversity);
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
            if(debug) printf("Segmentation finished, total time:%f\n", end);
        }

        /*

        Returns:
        Params:
        */

        /*
        runQuickshift takes an image and params and runs Quickshift on it, using the VL_Feat implementation
        Returns:
            cv::Mat = image after quickshift is applied
        Params:
            cv::Mat img = the image
            int kernelsize = the kernel or window size of the quickshift applied
            int maxdist = the largest distance a pixel can be from it's root
        */
        cv::Mat runQuickshift(cv::Mat img, int kernelsize, int maxdist)
        {
            int channels = img.channels();
            int width = img.cols;
            int height = img.rows;

            cv::Mat tempMat;
            img.copyTo(tempMat);
            tempMat.convertTo(tempMat, CV_64FC3, 1/255.0);
            double* cvimg = (double*) tempMat.data;
            double* vlimg = (double*) calloc(channels*width*height, sizeof(double));

            // create VLFeatWrapper object
            segment::VLFeatWrapper vlf_wrapper = segment::VLFeatWrapper(width, height, channels);
            vlf_wrapper.debug = debug;
            vlf_wrapper.verifyVLFeat();

            // apply quickshift from VLFeat
            vlf_wrapper.convertOPENCV_VLFEAT(cvimg, vlimg);
            int superpixelcount = vlf_wrapper.quickshift(vlimg, kernelsize, maxdist);
            vlf_wrapper.convertVLFEAT_OPENCV(vlimg, cvimg);

            cv::Mat postQuickShift = cv::Mat(height, width, CV_64FC3, cvimg);
            cv::Mat outimg;
            postQuickShift.copyTo(outimg);
            outimg.convertTo(outimg, CV_8UC3, 255);
            free(vlimg);

            if(debug) printf("Super pixels found via quickshift: %i\n", superpixelcount);
            return outimg;
        }

        /*
        runCanny runs canny edge detection on an image, and dilates and erodes it to close holes
        Returns:
            cv::Mat = edges found post dilate/erode
        Params:
            cv::Mat img = image to find edged in
            int threshold1 = first threshold for the hysteresis procedure.
            int threshold2 = second threshold for the hysteresis procedure.
        */
        cv::Mat runCanny(cv::Mat img, int threshold1, int threshold2)
        {
            cv::Mat postEdgeDetection;
            img.copyTo(postEdgeDetection);
            cv::Mat blurred;
            cv::blur(img, blurred, cv::Size(3,3));
            cv::Canny(blurred, postEdgeDetection, threshold1, threshold2);

            // TODO these values for dilate and erode possibly should be configurable
            cv::dilate(postEdgeDetection, postEdgeDetection, cv::Mat(), cv::Point(-1, -1), 2);
            cv::erode(postEdgeDetection, postEdgeDetection, cv::Mat(), cv::Point(-1, -1), 2);

            return postEdgeDetection;
        }

        /*
        runGmm creates 2 Gaussian Mixture Models, one for cell pixels and one for background pixels,
        then returns the result of the labels generated by these models
        Returns:
            cv::Mat = labels found per pixel
        Params:
            cv::Mat img = image to process
            vector<vector<cv::Point> > hulls = convex hulls to provide initial labeling
            int maxGmmIterations = maximum number of iterations to allow the gmm to train
        */
        cv::Mat runGmm(cv::Mat img, vector<vector<cv::Point> > hulls, int maxGmmIterations)
        {
            int width = img.cols;
            int height = img.rows;

            cv::Mat gray;
            img.convertTo(gray, CV_8U);
            cv::cvtColor(gray, gray, CV_BGR2GRAY);
            gray.convertTo(gray, CV_64F, 1/255.0);

            // create initial probabilities based on convex hulls
            float initialProbs[width*height][2];
            for(int row=0; row < height; row++)
            {
                for(int col=0; col < width; col++)
                {
                    for(unsigned int hullIndex=0; hullIndex < hulls.size(); hullIndex++)
                    {
                        if(cv::pointPolygonTest(hulls[hullIndex], cv::Point2f(row, col), false) >= 0)
                        {
                            initialProbs[row + col*height][0] = 1;
                            initialProbs[row + col*height][1] = 0;
                        }
                        else
                        {
                            initialProbs[row + col*height][0] = 0;
                            initialProbs[row + col*height][1] = 1;
                        }
                    }
                }
            }
            gray = gray.reshape(0, gray.rows*gray.cols);
            cv::Mat initialProbMat(width*height, 2, CV_32F, initialProbs);

            cv::Mat outputProbs;
            cv::Mat labels;
            cv::Ptr<cv::ml::EM> cell_gmm;
            cv::TermCriteria termCrit = cv::TermCriteria();
            termCrit.type = cv::TermCriteria::COUNT;
            termCrit.maxCount = maxGmmIterations;
            cell_gmm = cv::ml::EM::create();
            cell_gmm->setTermCriteria(termCrit);
            cell_gmm->setClustersNumber(2);
            cell_gmm->trainM(gray, initialProbMat, cv::noArray(), labels, outputProbs);

            labels = labels.reshape(0, img.rows);
            cv::Mat outimg;
            labels.convertTo(outimg, CV_8U, 255);

            return outimg;
        }

        /*
        runMser takes an image and params and runs MSER algorithm on it, for nuclei detection
        Return:
            vector<vector<cv::Point> > = stable regions found
        Params:
            cv::Mat img = the image
            int delta = the # of iterations a region must remain stable
            int minArea = the minimum number of pixels for a viable region
            int maxArea = the maximum number of pixels for a viable region
            double maxVariation = the max amount of variation allowed in regions
            double minDiversity = the min diversity allowed in regions
        */
        vector<vector<cv::Point> > runMser(cv::Mat img, int delta, int minArea, int maxArea,
            double maxVariation, double minDiversity)
        {
            cv::Ptr<cv::MSER> ms = cv::MSER::create(delta, minArea, maxArea, maxVariation, minDiversity);
            cv::Mat tmp;
            img.convertTo(tmp, CV_8U);
            cv::cvtColor(tmp, tmp, CV_BGR2GRAY);
            vector<vector<cv::Point> > regions;
            vector<cv::Rect> mser_bbox;

            ms->detectRegions(tmp, regions, mser_bbox);

            return regions;
        }

        /*
        findFinalClumpBoundaries takes an image and a threshold and returns all the contours whose
        size is greater than the threshold
        Returns:
            vector<vector<cv::Point> > = the contours found
        Params:
            cv::Mat img = the input image
            int minAreaThreshold = the minimum area, all contours smaller than this are discarded
        */
        vector<vector<cv::Point> > findFinalClumpBoundaries(cv::Mat img, int minAreaThreshold)
        {
            // opencv wants to find white object on a black background,
            // so we want to invert the labels before findContours
            cv::bitwise_not(img, img);

            vector<vector<cv::Point> > contours;
            cv::findContours(img, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
            vector<vector<cv::Point> > clumpBoundaries = vector<vector<cv::Point> >();
            for(unsigned int i=0; i<contours.size(); i++)
            {
                vector<cv::Point> contour = contours[i];
                double area = cv::contourArea(contour);
                if(area > minAreaThreshold)
                {
                    clumpBoundaries.push_back(contour);
                }
            }

            return clumpBoundaries;
        }
    };
}

#include <iostream>
#include "opencv2/opencv.hpp"
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
            
        void runSegmentation(int kernelsize, int maxdist) {

            cv::Mat image = cv::imread("../images/cyto.tif");
            cv::imshow("pre", image);

            // meta-data about the image
            int channels = image.channels();
            int width = image.cols;
            int height = image.rows;

            // debug data
            printf("Image data: (rows) %i (cols) %i (channels) %i\n", height, width, channels);


            // quickshift
            printf("Beginning quickshift...\n");
            // convert to a format that VL_Feat can use
            cv::Mat tempMat;
            image.convertTo(tempMat, CV_64FC3, 1/255.0);
            double* cvimg = (double*) tempMat.data;
            double* vlimg = (double*) calloc(channels*width*height, sizeof(double));

            // create VLFeatWrapper object
            segment::VLFeatWrapper vlf_wrapper = segment::VLFeatWrapper(width, height, channels);
            vlf_wrapper.verifyVLFeat();

            // apply quickshift from VLFeat
            vlf_wrapper.convertOPENCV_VLFEAT(cvimg, vlimg);
            printf("Converted to VLFeat format\n");
            int superpixelcount = vlf_wrapper.quickshift(vlimg, kernelsize, maxdist);
            printf("Performed quickshift\n");
            vlf_wrapper.convertVLFEAT_OPENCV(vlimg, cvimg);
            printf("Converted to OpenCV format\n");

            cv::Mat postQuickShift = cv::Mat(height, width, CV_64FC3, cvimg);
            cv::imshow("quickshifted", postQuickShift);
            printf("Quickshift complete, super pixels found: %i\n", superpixelcount);

            cv::Mat outimg;
            postQuickShift.convertTo(outimg, CV_8U, 255);
            imwrite("../images/quickshifted_cyto.png", outimg);


            // apply Canny Edge Detection
            printf("Beginning Edge Detection...\n");
            cv::Mat postEdgeDetection;
            postEdgeDetection.create(postQuickShift.size(), postQuickShift.type());
            postEdgeDetection = cv::Scalar::all(0);
            cv::Mat blurred;
            cv::blur(postQuickShift, blurred, cv::Size(3,3));
            blurred.convertTo(blurred, CV_8UC3, 255);
            cv::Canny(blurred, postEdgeDetection, 20, 40);

            cv::imshow("edges", postEdgeDetection);
            printf("Edge Detection complete\n");
            postEdgeDetection.convertTo(outimg, CV_8U, 255);
            cv::imwrite("../images/edgeDetected_cyto.png", outimg);

            cv::dilate(postEdgeDetection, postEdgeDetection, cv::Mat(), cv::Point(-1, -1), 2);
            cv::erode(postEdgeDetection, postEdgeDetection, cv::Mat(), cv::Point(-1, -1), 2);

            imshow("post dilate/erode", postEdgeDetection);
            postEdgeDetection.convertTo(outimg, CV_8U, 255);
            imwrite("../images/edgeDetectedEroded_cyto.png", outimg);


            // find contours
            std::vector<vector<cv::Point> > contours;
            std::vector<cv::Vec4i> hierarchy;
            cv::findContours(postEdgeDetection, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
            cv::drawContours(image, contours, -1, cv::Scalar(255, 0, 255), 1.5);

            imshow("contours", image);
            // image.convertTo(image, CV_8U, 255);
            imwrite("../images/contours_cyto.png", image);


            // clean up
            free(vlimg);
            cv::waitKey(0);
        }
    };
}

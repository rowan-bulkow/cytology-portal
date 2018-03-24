// segment.cpp
// initial program to apply c++ tools from various libraries, including vlfeat and opencv
// to recreate the pipeline of cell segmentation described in the paper
// by Zhi Lu et al. :
// https://cs.adelaide.edu.au/~zhi/publications/paper_TIP_Jan04_2015_Finalised_two_columns.pdf

/*
The following is about linking libraries - still applicable, but hopefully
most of it is taken care of via `make`

Linking the vlfeat library requires some doing:

g++ segment.cpp -I$VLROOT -L$VLROOT/bin/glnxa64/ -lvl
where $VLROOT is the path to your vlroot source files, so ex: /usr/local/lib/vlfeat-0.9.21

then setup and env variable in the terminal,
export LD_LIBRARY_PATH=/usr/local/lib/vlfeat-0.9.21/bin/glnxa64
(or append this to the existing LD_LIBRARY_PATH if needed)

then run the executable and cross your fingers
*/

#include <iostream>
#include "opencv2/opencv.hpp"
extern "C" {
    #include "vl/quickshift.h"
}

using namespace std;
using namespace cv;

int main (int argc, const char * argv[]) {
    VL_PRINT ("Vlfeat successfully connected!\n");

    // kernelsize and maxdist are arguments that the quick shift algorithm requires
    int kernelsize = 4;
    int maxdist = 8;
    if(argc > 1)
    {
        kernelsize = atoi(argv[1]);
        maxdist = atoi(argv[2]);
        printf("Read args: kernelsize=%i maxdist=%i\n", kernelsize, maxdist);
    }

    Mat image = imread("../images/cyto.tif");
    imshow("pre", image);

    // meta-data about the image
    int channels = image.channels();
    int width = image.cols;
    int height = image.rows;

    // convert to a format that VL_Feat can use
    Mat tempMat;
    image.convertTo(tempMat, CV_64FC3, 1/255.0);
    double* cvimg = (double*) tempMat.data;
    double* vlimg = (double*) calloc(channels*width*height, sizeof(double));
    for(int c=0; c<channels; c++)
    {
        for(int col=0; col<width; col++)
        {
            for(int row=0; row<height; row++)
            {
                vlimg[c*width*height + row + col*height] = cvimg[(row + col*height)*channels + c];
            }
        }
    }

    // debug data
    printf("Image data: (rows) %i (cols) %i (channels) %i\n", height, width, channels);

    printf("Beginning quickshift...\n");

    VlQS* quickshift = vl_quickshift_new(vlimg, width, height, channels);
    vl_quickshift_set_kernel_size(quickshift, kernelsize);
    vl_quickshift_set_max_dist(quickshift, maxdist);
    vl_quickshift_process(quickshift);
    int* parents = vl_quickshift_get_parents(quickshift);
    double* dists = vl_quickshift_get_dists(quickshift);

    // debug data: find how many super pixels were identified
    int superpixelcount = 0;
    for(int i=0; i<width*height; i++)
    {
        if(dists[i] > maxdist)
            superpixelcount++;
    }
    printf("superpixelcount=%i\n", superpixelcount);

    // apply the shift found by vl_feat quickshift to the original image
    // (vl_feat does not do this automatically)
    for(int col=0; col<width; col++)
    {
        for(int row=0; row<height; row++)
        {
            // int linearIndex = row + col*height;
            int partialLinearIndex = row*width + col;
            int parentIndex = parents[partialLinearIndex];
            while(true)
            {
                if(dists[parentIndex] > maxdist)
                    break;
                parentIndex = parents[parentIndex];
            }

            for(int c=0; c<channels; c++)
            {
                int linearIndex = c*width*height + partialLinearIndex;
                vlimg[linearIndex] = vlimg[c*width*height + parentIndex];
            }
        }
    }

    // convert back to opencv and display
    for(int c=0; c<channels; c++)
    {
        for(int col=0; col<width; col++)
        {
            for(int row=0; row<height; row++)
            {
                cvimg[(row + col*height)*channels + c] = vlimg[c*width*height + row + col*height];
            }
        }
    }
    Mat postQuickShift = Mat(quickshift->height, quickshift->width, CV_64FC3, cvimg);
    imshow("quickshifted", postQuickShift);
    printf("Quickshift complete\n");

    cv::Mat outimg;
    postQuickShift.convertTo(outimg, CV_16U, 65535);
    imwrite("../images/quickshifted_cyto.png", outimg);

    // apply Canny Edge Detection
    printf("Beginning Edge Detection...\n");
    cv::Mat postEdgeDetection;
    postEdgeDetection.create(postQuickShift.size(), postQuickShift.type());
    postEdgeDetection = Scalar::all(0);
    cv::Mat blurred;
    cv::blur(postQuickShift, blurred, cv::Size(3,3));
    blurred.convertTo(blurred, CV_8UC3, 255);
    cv::Canny(blurred, postEdgeDetection, 20, 40);

    imshow("edges", postEdgeDetection);
    printf("Edge Detection complete\n");
    postEdgeDetection.convertTo(outimg, CV_16U, 65535);
    imwrite("../images/edgeDetected_cyto.png", outimg);

    cv::dilate(postEdgeDetection, postEdgeDetection, cv::Mat(), cv::Point(-1, -1), 2);
    cv::erode(postEdgeDetection, postEdgeDetection, cv::Mat(), cv::Point(-1, -1), 2);

    imshow("post dilate/erode", postEdgeDetection);
    postEdgeDetection.convertTo(outimg, CV_16U, 65535);
    imwrite("../images/edgeDetectedEroded_cyto.png", outimg);

    // find contours
    std::vector<vector<Point> > contours;
    std::vector<Vec4i> hierarchy;

    cv::findContours(postEdgeDetection, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    cv::drawContours(image, contours, -1, cv::Scalar(255, 0, 255), 1.5);

    imshow("contours", image);
    //image.convertTo(outimg, CV_16U, 65535);
    imwrite("../images/contours_cyto.png", image);

    // clean up
    vl_quickshift_delete(quickshift);
    free(vlimg);

    waitKey(0);
    return 0;
}

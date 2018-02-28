// segment.cpp
// initial program to apply c++ tools from various libraries, including vlfeat
// to recreate the pipeline of cell segmentation described in the paper
// by Zhi et al. :
// https://cs.adelaide.edu.au/~zhi/publications/paper_TIP_Jan04_2015_Finalised_two_columns.pdf

/*
The following is about linking libraries - still applicable, but hopefully
most of it is taken care of via `make`

Linking the vlfeat library requires some doing:

g++ segment.cpp -I$VLROOT -L$VLROOT/bin/glnxa64/ -lvl
where $VLROOT is the path to your vlroot source files, so ex: /usr/local/lib/vlfeat-0.9.21

then setup and env variable in the terminal,
export LD_LIBRARY_PATH=$VLROOT/bin/glnxa64
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
    Mat image = imread("../images/cyto.tif");

    imshow("pre", image);

    Mat toFloat;
    image.convertTo(toFloat, CV_64FC3, 1/255.0);

    double* vlimg = (double*) toFloat.data;

    int kernelsize = 2;
    int maxdist = 6;

    int channels = 3;
    int width = image.cols;
    int height = image.rows;
    printf("Image data: (rows) %i (cols) %i (channels) %i\n", height, width, channels);

    printf("Beginning quickshift...\n");

    VlQS* quickshift = vl_quickshift_new(vlimg, width, height, channels);
    vl_quickshift_set_kernel_size(quickshift, kernelsize);
    vl_quickshift_set_max_dist(quickshift, maxdist);
    vl_quickshift_process(quickshift);
    int* parents = vl_quickshift_get_parents(quickshift);
    double* dists = vl_quickshift_get_dists(quickshift);
    int superpixelcount = 0;
    for(int i=0; i<width*height; i++)
    {
        if(dists[i] > maxdist)
            superpixelcount++;
    }
    printf("superpixelcount=%i\n", superpixelcount);

    for(int col=0; col<width; col++)
    {
        for(int row=0; row<height; row++)
        {
            int linearIndex = row + col*height;
            int parentIndex = parents[linearIndex];
            while(true)
            {
                if(dists[parentIndex] > maxdist)
                    break;
                parentIndex = parents[parentIndex];
            }

            for(int c=0; c<channels; c++)
            {
                vlimg[linearIndex*3 + c] = vlimg[parentIndex*3 + c];
            }
        }
    }

    Mat postQuickShift = Mat(quickshift->height, quickshift->width, CV_64FC3, vlimg);
    imshow("quickshifted", postQuickShift);
    waitKey(0);

    printf("Quickshift complete\n");

    // get rid of the quickshift object
    vl_quickshift_delete(quickshift);

    //imwrite("images/quickshifted_cyto.tif", img_seg);

    return 0;
}

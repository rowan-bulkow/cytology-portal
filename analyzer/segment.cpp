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

    int kernelsize = 5;
    int maxdist = 10;

    int channels = 3;
    int width = image.cols;
    int height = image.rows;
    printf("Image data: (rows) %i (cols) %i (channels) %i\n", height, width, channels);

    printf("Beginning quickshift...");

    VlQS* quickshift = vl_quickshift_new(vlimg, width, height, channels);
    vl_quickshift_set_kernel_size(quickshift, kernelsize);
    vl_quickshift_set_max_dist(quickshift, maxdist);
    vl_quickshift_process(quickshift);
    int* parents = vl_quickshift_get_parents(quickshift);

    for(int i=0; ; i++)
    {
        if(parents[i] == '\0')
        {
            printf("Found the last index of parents at:%i\n", i);
            break;
        }
    }
    for(int i=0; ; i++)
    {
        if(vlimg[i] == '\0')
        {
            printf("Found the last index of vlimg at:%i\n", i);
            break;
        }
    }

    double* subset = (double*) calloc(sizeof(double), channels*width*height);
    for(int c=0; c<channels; c++)
    {
        for(int col=0; col<width; col++)
        {
            for(int row=0; row<height; row++)
            {
                int linearIndex = row + col*height + c*width*height;
                subset[linearIndex] = vlimg[linearIndex];
                if(row < 10 || col < 10)
                    subset[linearIndex] = 0.5;
                if(linearIndex%3 == 2)
                    subset[linearIndex] = 0.5;
            }
        }
    }
    Mat img_subset = Mat(height, width, CV_64FC3, subset);
    imshow("subset", img_subset);

    int maxval = 0;
    for(int i=0; i<width*height; i++)
    {
        if(parents[i] > maxval)
            maxval = parents[i];
    }
    printf("Max parent index found:%i\n", maxval);

    for(int c=0; c<channels; c++)
    {
        for(int col=0; col<width; col++)
        {
            for(int row=0; row<height; row++)
            {
                int linearIndex = row + col*height + c*width*height;
                if(linearIndex%3 == 0)
                {
                    int parentIndex = parents[col + row*height] + c*width*height;
                    vlimg[linearIndex] = vlimg[parentIndex];
                    vlimg[linearIndex+1] = vlimg[parentIndex+1];
                    vlimg[linearIndex+2] = vlimg[parentIndex+2];
                }
                //
                //
                // vlimg[row + col*height + c*width*height] = vlimg[parents[row + col*height] + c*width*height];
            }
        }
    }

    for(int col=0; col<width; col++)
    {
        for(int row=0; row<height; row++)
        {
            if(row < 10 && col < 10)
                printf("(%i)%i\t", row+col*height, parents[row + col*height]);
            else if(row == 10 && col < 10)
                printf("\n");
            else if(row > 10 && col > 10)
                break;
        }
    }
    Mat parentImg = Mat(height, width, CV_32S, parents);
    imshow("parentImg", parentImg);

    // maxval = 0;
    // for(int i=0; i < width*height; i++)
    // {
    //     int row = i%height;
    //     int col = floor(i/height);
    //     for(int c=0; c<channels; c++)
    //     {
    //         int linearIndex = c*width*height + row + col*height;
    //         linearIndex = row + col*height;
    //         vlimg[linearIndex] = vlimg[c*width*height + parents[i]];
    //         if(linearIndex > maxval)
    //             maxval = linearIndex;
    //     }
    // }
    // printf("Max index found:%i\n", maxval);

    Mat postQuickShift = Mat(quickshift->height, quickshift->width, CV_64FC3, vlimg);
    imshow("quickshifted", postQuickShift);
    waitKey(0);

    printf("Quickshift complete\n");

    // get rid of the quickshift object
    vl_quickshift_delete(quickshift);

    //imwrite("images/quickshifted_cyto.tif", img_seg);

    return 0;
}

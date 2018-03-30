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
#include "Segmenter.cpp"

using namespace std;

int main (int argc, const char * argv[])
{
    // kernelsize and maxdist are arguments that the quick shift algorithm requires
    int kernelsize = 4;
    int maxdist = 8;
    if(argc > 1)
    {
        kernelsize = atoi(argv[1]);
        maxdist = atoi(argv[2]);
        printf("Read args: kernelsize=%i maxdist=%i\n", kernelsize, maxdist);
    }

    segment::Segmenter seg = segment::Segmenter();
    seg.runSegmentation(kernelsize, maxdist);

    return 0;
}

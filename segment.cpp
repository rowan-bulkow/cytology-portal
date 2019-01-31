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

For command line arguments:
how to use reference: https://theboostcpplibraries.com/boost.program_options
get boost on linux: https://stackoverflow.com/questions/12578499/how-to-install-boost-on-ubuntu
		/use the apt-get version

compliation example: g++ -std=gnu++17 segment.cpp -l boost_program_options
	Make sure to include the "-l boost_program_options" otherwise you will get errors that do not make sense and the program will not work
*/

#include <iostream>
#include "opencv2/opencv.hpp"
#include "Segmenter.cpp"
#include <boost/program_options.hpp>

using namespace std;
using namespace boost::program_options;

int main (int argc, const char * argv[])
{
    // Default parameters
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

    try
    {
        options_description desc{"Options"};
        desc.add_options()
          ("help,h", "Help screen")
          ("maxVariation", value<float>()->default_value(maxVariation), "MaxVariation")
          ("minDiversity", value<float>()->default_value(minDiversity), "MinDiversity")
          ("minAreaThreshold", value<float>()->default_value(minAreaThreshold), "minAreaThreshold")
          ("kernelsize", value<int>()->default_value(kernelsize), "Kernelsize")
          ("maxdist", value<int>()->default_value(maxdist), "Maxdist")
          ("threshold1", value<int>()->default_value(threshold1), "threshold1")
          ("threshold2", value<int>()->default_value(threshold2), "threshold2")
          ("maxGmmIterations", value<int>()->default_value(maxGmmIterations), "maxGmmIterations")
          ("delta", value<int>()->default_value(delta), "delta")
          ("minArea", value<int>()->default_value(minArea), "minArea")
          ("maxArea", value<int>()->default_value(maxArea), "maxArea")
          ("photo,p", value<std::string>()->default_value("cyto.tif"), "photo");

        variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);
        notify(vm);

        segment::Segmenter seg = segment::Segmenter(
            vm["kernelsize"].as<int>(),
            vm["maxdist"].as<int>(),
            vm["threshold1"].as<int>(),
        	vm["threshold2"].as<int>(),
            vm["maxGmmIterations"].as<int>(),
            vm["minAreaThreshold"].as<float>(),
            vm["delta"].as<int>(),
        	vm["minArea"].as<int>(),
            vm["maxArea"].as<int>(),
            vm["maxVariation"].as<float>(),
            vm["minDiversity"].as<float>()
        );

        seg.runSegmentation(vm["photo"].as<std::string>());
    }
    catch (const error &ex)
    {
      std::cerr << ex.what() << '\n';
    }
}

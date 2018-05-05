# cytology-portal

A service for automated analysis and classification of cells in an image. Includes: Image Segmentation, Cell Segmentation, Cell Classification, and Web Portal for upload, management, and viewing.

## Cell Segmentation

Our intent: To Implement a version of the segmentation process described by Zhi Lu et al.
[An Improved Joint Optimization of Multiple Level Set Functions for the Segmentation of Overlapping Cervical Cells](https://cs.adelaide.edu.au/~zhi/publications/paper_TIP_Jan04_2015_Finalised_two_columns.pdf)

## Instructions

Segmenter External Dependencies
Boost: a C++ library
Source documentation: https://theboostcpplibraries.com/boost.program_options
Installed on linux using the following commands:

sudo apt-get install libboost-all-dev
aptitude search boost
Must compile with the “-l boost_program_options” flag

ImageMagick: 
https://www.imagemagick.org/script/download.php
Follow instructions provided at the url.

OpenCV: A C++ Computer Vision Library
Follow installation instructions on https://opencv.org/
Or, installation on Linux:

sudo apt-get update
 
sudo apt-get upgrade

Install dependencies
$ sudo apt-get install build-essential cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev

$ sudo apt-get install python3.5-dev python3-numpy libtbb2 libtbb-dev

$ sudo apt-get install libjpeg-dev libpng-dev libtiff5-dev libjasper-dev libdc1394-22-dev libeigen3-dev libtheora-dev libvorbis-dev libxvidcore-dev libx264-dev sphinx-common libtbb-dev yasm libfaac-dev libopencore-amrnb-dev libopencore-amrwb-dev libopenexr-dev libgstreamer-plugins-base1.0-dev libavutil-dev libavfilter-dev libavresample-dev

Get opencv, Install in the opt directory
$ sudo -s

$ cd /opt

/opt$ git clone https://github.com/Itseez/opencv.git

/opt$ git clone https://github.com/Itseez/opencv_contrib.git

Build and install
/opt$ cd opencv

/opt/opencv$ mkdir release

/opt/opencv$ cd release

/opt/opencv/release$ cmake -D BUILD_TIFF=ON -D WITH_CUDA=OFF -D ENABLE_AVX=OFF -D WITH_OPENGL=OFF -D WITH_OPENCL=OFF -D WITH_IPP=OFF -D WITH_TBB=ON -D BUILD_TBB=ON -D WITH_EIGEN=OFF -D WITH_V4L=OFF -D WITH_VTK=OFF -D BUILD_TESTS=OFF -D BUILD_PERF_TESTS=OFF -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D OPENCV_EXTRA_MODULES_PATH=/opt/opencv_contrib/modules /opt/opencv/

/opt/opencv/release$ make -j4

/opt/opencv/release$ make install

/opt/opencv/release$ ldconfig

/opt/opencv/release$ exit

/opt/opencv/release$ cd ~


VL Feat: A C Computer Vision Library
Follow instructions for installation of the C API from http://www.vlfeat.org/
The makefile should include an export of a needed library path, but doesn’t work on all machines, so you may need to set the environment variable:
LD_LIBRARY_PATH=/path/to/vlfeat/bin/systemarchitecture

Where the system architecture is one of the folders in the bin/ directory, so for 64-bit Ubuntu it’s glna64/.

Compiling the Segmenter
Once all external dependencies are installed and configured, compiling should be as easy as running the make file in the analyzer/ directory. This requires g++ and C++ Standard gnu 2014.

Running the Segmenter
After compilation, running the segmenter is done through a command line interface, with options specified in segmenter.cpp or through the ‘segmenter --help’ command.
Basic example:
./segmenter -p ../images/cyto.tif
This assumes you have an image, cyto.tif, in your images folder, and will output resulting images in the same images/ directory.

Web Portal
Clone the repository, and in Visual Studio the GitHub plugin can be used to connect with the repository. 
The following extra NuGet packages are installed on the website: 
LibTiff.Net by BitMiracle, 
jquery.jcrop.js by Kelly Hallman, Magick.Net-Q16-AnyCPU by Dirk Lemstra, (if there are errors involving Roslyn, Microsoft.CodeCom.Providers.DotNetCompilerPlatform may need to be updated),OpenCvSharp3-AnyCPU by shimat, and Jquery. 
Jcrop and Jquery are essential, and the others may be useful.
ImageMagick should be installed on your machine- follow instructions for installing from https://www.imagemagick.org/script/download.php.

Building and running the Web Application can be done through the standard .Net framework commands.


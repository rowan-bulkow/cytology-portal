# cytology-portal

A service for automated analysis and classification of cells in an image. Includes: Image Segmentation, Cell Segmentation, Cell Classification, and Web Portal for upload, management, and viewing.

## Cell Segmentation

Our intent: To Implement a version of the segmentation process described by Zhi Lu et al.
[An Improved Joint Optimization of Multiple Level Set Functions for the Segmentation of Overlapping Cervical Cells](https://cs.adelaide.edu.au/~zhi/publications/paper_TIP_Jan04_2015_Finalised_two_columns.pdf)

Using: C++, python

## Cell Classification

Not yet implemented :(

## Usage

OpenCV, VLFeat, and Boost must be installed:
- https://docs.opencv.org/4.0.1/df/d65/tutorial_table_of_content_introduction.html
- http://www.vlfeat.org/download.html
- https://www.boost.org/users/download/

`makefile` must be updated with the locations of these installations, their include & bin folders mainly.
`libvl.dylib` should be a symbolic link to the `libvl.dylib`, wherever your VLFeat was installed

Executing `make` from the project root directory should generate an executable, `segment`.
Running `segment` can be done by:
```
segment -p images/cyto.tif
```
With your image file path replaced. Defaults to `cyto.tif` in the same folder as the executable.

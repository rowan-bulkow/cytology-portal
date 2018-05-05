#include <iostream>
#include <ctime>
#include <stdio.h>
#include <fstream>
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
            cv::bitwise_not(gmmPredictions, gmmPredictions);
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
            cv::drawContours(outimg, clumpBoundaries, allContours, pink, 2);
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
                printf("Clump %u, nuclei boundaries found: %lu\n", i, clumps[i].nucleiBoundaries.size());
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


            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            // Cell Shape Priors / Initial Cell Segmentation
            // ***** ***** ***** ***** ***** ***** ***** ***** ***** ***** //
            start = clock();
            if(debug) printf("Beginning initial cell segmentation...\n");

            // run a find contour on the nucleiBoundaries to get them as contours, not regions
            for(unsigned int c=0; c<clumps.size(); c++)
            {
                Clump* clump = &clumps[c];
                cv::Mat regionMask = cv::Mat::zeros(clump->clumpMat.rows, clump->clumpMat.cols, CV_8U);
                cv::drawContours(regionMask, clump->nucleiBoundaries, allContours, cv::Scalar(255));
                cv::findContours(regionMask, clump->nucleiBoundaries, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
            }

            // populate the clump's vector of cells (this should probably be done earlier,
            // but I don't want to refactor right now)
            for(unsigned int c=0; c<clumps.size(); c++)
            {
                Clump* clump = &clumps[c];
                for(unsigned int n=0; n<clump->nucleiBoundaries.size(); n++)
                {
                    vector<cv::Point>* boundary = &clump->nucleiBoundaries[n];
                    Cell cell;
                    cell.nucleusBoundary = vector<cv::Point>(*boundary);
                    cell.computeCenter();
                    cell.generateColor();
                    clump->cells.push_back(cell);
                }
            }

            // sanity check: view all the cell array sizes for each clump
            for(unsigned int c=0; c<clumps.size(); c++)
            {
                Clump* clump = &clumps[c];
                printf("Clump: %u, # of nuclei boundaries: %lu, # of cells: %lu\n",
                    c, clump->nucleiBoundaries.size(), clump->cells.size());
            }

            /*
            Initial approach (grossly inefficient...but simple)
            for each clump:
                for each pixel:
                    associate with the closest nuclei with a direct line not leaving the clump:
                    measure distance to each nuclei
                    for closest one, check to make sure it has direct line not leaving clump
                    if it does, repeat with 2nd closest until you find one that works
            */
            for(unsigned int c=0; c<clumps.size(); c++)
            {
                Clump* clump = &clumps[c];
                clump->nucleiAssocs = cv::Mat::zeros(clump->clumpMat.rows, clump->clumpMat.cols, CV_8UC3);

                // typedef cv::Point3_<uint8_t> Pixel;
                // clump->nucleiAssocs.forEach<Pixel>([](Pixel &p, const int * position) -> void {
                //     // p.x = 255;
                //     // p.z = 255;
                // });

                // printf("Clump %i, # of cells: %lu\n", c, clump->cells.size());

                // debugging information
                int numPixelsOutside = 0, numPixelsAssigned = 0, numPixelsLost = 0;
                int pixelTotal = clump->clumpMat.rows * clump->clumpMat.cols;
                int pixelCount = 0;

                // for each pixel in the clump ...
                for(int row=0; row<clump->clumpMat.rows; row++)
                {
                    for(int col=0; col<clump->clumpMat.cols; col++)
                    {
                        pixelCount++;
                        // clump->nucleiAssocs.at<cv::Vec3b>(row, col) = cv::Vec3b(0, 0, 255);
                        // if the pixel is outside of the clump, ignore it
                        // TODO something is off here, seems like it should be row, col
                        cv::Point pixel = cv::Point(col, row);
                        if(cv::pointPolygonTest(clump->offsetContour, pixel, false) < 0)
                        {
                            // clump->nucleiAssocs.at<cv::Vec3b>(col, row) = cv::Vec3b(0, 255, 0);
                            numPixelsOutside++;
                        }
                        else
                        {
                            // find the distance to each nucleus
                            vector<pair<int, double>> nucleiDistances;
                            for(unsigned int n=0; n<clump->cells.size(); n++)
                            {
                                double distance = cv::norm(pixel - clump->cells[n].nucleusCenter);
                                nucleiDistances.push_back(pair<int, double>(n,distance));
                                // printf("pixel %i, %i, nucleus %i, distance %f\n", row, col, n, distance);
                            }
                            // order the nuclei indices by increasing distance
                            sort(nucleiDistances.begin(), nucleiDistances.end(), [=](pair<int, double>& a, pair<int, double>& b)
                            {
                                return a.second < b.second;
                            });

                            // then look at the closest and make sure it has a viable line
                            // while we don't have an answer, keep finding the closest and checking it
                            int nIndex = -1;
                            while(nIndex == -1 && !nucleiDistances.empty())
                            {
                                pair<int, double> closest = nucleiDistances[0];
                                nIndex = closest.first;
                                // printf("Checking pixel %i, %i against nucleus %i, distance %f\n", row, col, nIndex, closest.second);

                                // check points along the line between pixel and closest nuclei
                                for(float f=0.1; f<1.0; f+=0.1)
                                {
                                    float x = pixel.x*f + clump->cells[nIndex].nucleusCenter.x*(1-f);
                                    float y = pixel.y*f + clump->cells[nIndex].nucleusCenter.y*(1-f);
                                    // if the point is outside the clump, we haven't found our match
                                    if(cv::pointPolygonTest(clump->offsetContour, cv::Point(x, y), false) < 0)
                                    {
                                        nIndex = -1;
                                        nucleiDistances.erase(nucleiDistances.begin());
                                        break;
                                        // printf("Point %f, %f falls outside the clump\n", x, y);
                                    }
                                }
                            }
                            if(nIndex > -1)
                            {
                                clump->nucleiAssocs.at<cv::Vec3b>(row, col) = clump->cells[nIndex].color;
                                // clump->nucleiAssocs.data[(row + col*clump->nucleiAssocs.rows)*3 + 0] = clump->cells[nIndex].color[0];
                                // clump->nucleiAssocs.data[(row + col*clump->nucleiAssocs.rows)*3 + 1] = clump->cells[nIndex].color[1];
                                // clump->nucleiAssocs.data[(row + col*clump->nucleiAssocs.rows)*3 + 2] = clump->cells[nIndex].color[2];
                                numPixelsAssigned++;
                            }
                            else
                            {
                                clump->nucleiAssocs.at<cv::Vec3b>(row, col) = cv::Vec3b(255, 0, 0);
                                // clump->nucleiAssocs.data[(row + col*clump->nucleiAssocs.rows)*3 + 0] = 0;
                                // clump->nucleiAssocs.data[(row + col*clump->nucleiAssocs.rows)*3 + 1] = 0;
                                // clump->nucleiAssocs.data[(row + col*clump->nucleiAssocs.rows)*3 + 2] = 0;
                                numPixelsLost++;
                            }
                        }
                    }
                }

                if(debug)
                {
                    printf("For clump: %i, # of cells: %lu, %i pixels total, %i pixels assessed, %i pixels inside clump, %i pixels outside, %i pixels lost\n",
                        c, clump->cells.size(), pixelTotal, pixelCount, numPixelsAssigned, numPixelsOutside, numPixelsLost);
                }

                // print the potential assignment
                char buffer[200];
                cv::Mat temp;
                clump->nucleiAssocs.copyTo(temp);
                temp.convertTo(temp, CV_8UC3);
                sprintf(buffer, "../images/clumps/final_clump_%i_raw.png", c);
                cv::imwrite(buffer, temp);

                cv::Mat initialAssignments = segTools.runCanny(temp, threshold1, threshold2);
                vector<vector<cv::Point>> initialCells;
                cv::findContours(initialAssignments, initialCells, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

                clump->clumpMat.copyTo(outimg);
                outimg.convertTo(outimg, CV_8UC3);

                // cv::drawContours(outimg, vector<vector<cv::Point>>(1, clump->offsetContour), allContours, cv::Scalar(0, 255, 0), 1); // cv::Scalar(255, 0, 255)
                cv::drawContours(outimg, clump->nucleiBoundaries, allContours, pink, 2);
                cv::drawContours(outimg, initialCells, allContours, cv::Scalar(255, 0, 0), 2);

                sprintf(buffer, "../images/clumps/final_clump_%i_boundaries_1.png", c);
                cv::imwrite(buffer, outimg);
            }

            end = (clock() - start) / CLOCKS_PER_SEC;
            if(debug) printf("Finished initial cell segmentation, time:%f\n", end);

            // print the nuclei sizes
            fstream nucleiSizes("nucleiSizes.txt");
            for(Clump clump : clumps)
            {
                for(vector<cv::Point> nuclei : clump.nucleiBoundaries)
                {
                    nucleiSizes << cv::contourArea(nuclei) << endl;
                }
            }
            nucleiSizes.close();

            for(unsigned int c=0; c<clumps.size(); c++)
            {
                Clump* clump = &clumps[c];
                char buffer[200];
                cv::Mat temp;
                clump->nucleiAssocs.copyTo(temp);
                cv::Mat initialAssignments = segTools.runCanny(temp, threshold1, threshold2);
                vector<vector<cv::Point>> initialCells;
                cv::findContours(initialAssignments, initialCells, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

                clump->clumpMat.copyTo(outimg);
                outimg.convertTo(outimg, CV_8UC3);

                // cv::drawContours(outimg, vector<vector<cv::Point>>(1, clump->offsetContour), allContours, cv::Scalar(255, 0, 0), 1);
                cv::drawContours(outimg, clump->nucleiBoundaries, allContours, pink, 2);
                cv::drawContours(outimg, initialCells, allContours, cv::Scalar(255, 0, 0), 2);

                sprintf(buffer, "../images/clumps/final_clump_%i_boundaries.png", c);
                // cv::imwrite(buffer, outimg);
            }

            // clean up
            end = (clock() - total) / CLOCKS_PER_SEC;
            if(debug || totalTimed) printf("Segmentation finished, total time:%f\n", end);
        }
    };
}

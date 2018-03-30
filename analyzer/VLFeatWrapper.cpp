extern "C" {
    #include "vl/quickshift.h"
}

namespace segment
{
    class VLFeatWrapper
    {
        public:

        int width, height, channels;

        VLFeatWrapper(int width, int height, int channels)
        {
            this->width = width;
            this->height = height;
            this->channels = channels;
        }

        void verifyVLFeat()
        {
            VL_PRINT("VLFeat successfully connected!\n");
        }

        void convertOPENCV_VLFEAT(double* opencv, double* vlfeat)
        {
            for(int c=0; c<channels; c++)
            {
                for(int col=0; col<width; col++)
                {
                    for(int row=0; row<height; row++)
                    {
                        vlfeat[c*width*height + row + col*height] = opencv[(row + col*height)*channels + c];
                    }
                }
            }
        }

        void convertVLFEAT_OPENCV(double* vlfeat, double* opencv)
        {
            for(int c=0; c<channels; c++)
            {
                for(int col=0; col<width; col++)
                {
                    for(int row=0; row<height; row++)
                    {
                        opencv[(row + col*height)*channels + c] = vlfeat[c*width*height + row + col*height];
                    }
                }
            }
        }

        // applies a quick shift algorithm to the image, and returns the # of superpixels found
        int quickshift(double* image, int kernelsize, int maxdist)
        {
            VlQS* quickshift = vl_quickshift_new(image, width, height, channels);
            VL_PRINT("Created vl quickshift object\n");
            vl_quickshift_set_kernel_size(quickshift, kernelsize);
            vl_quickshift_set_max_dist(quickshift, maxdist);
            VL_PRINT("Set quickshift arguments\n");
            vl_quickshift_process(quickshift);
            VL_PRINT("Performed quickshift\n");
            int* parents = vl_quickshift_get_parents(quickshift);
            double* dists = vl_quickshift_get_dists(quickshift);
            VL_PRINT("Deleted quickshift object\n");

            // debug data: find how many super pixels were identified
            int superpixelcount = 0;
            for(int i=0; i<width*height; i++)
            {
                if(dists[i] > maxdist)
                    superpixelcount++;
            }

            // apply the shift found by vl_feat quickshift to the original image
            // (vl_feat does not do this automatically)
            for(int col=0; col<width; col++)
            {
                for(int row=0; row<height; row++)
                {
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
                        image[linearIndex] = image[c*width*height + parentIndex];
                    }
                }
            }
            vl_quickshift_delete(quickshift);
            VL_PRINT("Finished quickshift application process\n");

            return superpixelcount;
        }
    };
}

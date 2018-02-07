import cv2
import pymeanshift as meanshift

original_image = cv2.imread("cyto.tif")

(segmented_image, labels_image, number_regions) = meanshift.segment(original_image, spatial_radius=6,
                                                              range_radius=4.5, min_density=50)
cv2.imshow('mean shifted', segmented_image)
cv2.imwrite('shifted_cyto.tif', segmented_image)
cv2.waitKey(0)
cv2.destroyAllWindows()

# pymeanshift

Uses [pymeanshift](https://github.com/fjean/pymeanshift)
for a rudimentary superpixel creation algorithm.

Used [imagemagick](https://www.imagemagick.org/script/index.php) to crop a
portion of the original tif image we were given.

Command used:

`convert -extract 500x500+5000+5000 cyto_1_1.tif[3] cyto.tif`

Note: The 500x500+5000+5000 represents the size and offset of the cropped image,
and the [3] specifies the 3rd page of the tif file.

/*
 * hw4.c
 *
 *  Created on: 8/15/2014
 *      Author: Kieran Lynn
 */

#include "hw4.h"

/**
 * Loads an image (stored as an IplImage struct) for each
 * filename provided.
 * @param numImages		The number of images to load
 * @param filenames		A list of strings of the filenames to load
 */
IplImage** loadImages(int numImages, char ** fileNames) {
	IplImage** rv; // the return result
	int i; // used for looping

	// TODO: allocate an array of numImages IplImage* s
	rv = malloc(sizeof(IplImage*)*numImages);
	// TODO: iterate over each filenam
	for (i=0; i < numImages; i++){
		*(rv+i) = cvLoadImage(fileNames[i], CV_LOAD_IMAGE_COLOR);

		if(rv[i] == NULL){
			printf("couldnt load image\n");
			return NULL;
		}
	}
	// TODO: return results
	return rv;
}

/**
 * Computes the distance between two colors (stored as CvScalars).
 *
 * Given a CvScalar c, you can access the blue, green, and red (BGR) elements
 * via c.val[0], c.val[1], and c.val[2], respectively.
 *
 * This function computes the distance between two colors as the euclidean
 * distance between the two BGR vectors.
 *
 * @see http://en.wikipedia.org/wiki/Euclidean_distance
 *
 * @param c1	The first color
 * @param c2	The second color
 * @returns		The euclidean distance between the two 3-d vectors
 */
double colorDistance(CvScalar c1, CvScalar c2) {
	double d = 0; // the result
	int i; // an iterator
	double b = 0;
	// TODO: iterate over the dimensions and compute the sum
	for(i=0;i<3;i++){
		b = c2.val[i]-c1.val[i];
		d += b*b;
	}	

	// TODO: return the square root of the result.
 	d = sqrt(d);
	return d;
}

/**
 * Splits up an image into numColumns by numRows sub-images and returns the results.
 *
 * @param src	The source image to split
 * @param numColumns	The number of columns to split into
 * @param numRows 		The number of rows to split into
 * @returns				A numColumns x numRows array of IplImages
 */
IplImage ** getSubImages(IplImage* src, int numColumns, int numRows) {
	int cellWidth, cellHeight, y, x, i;
	IplImage ** rv;
	CvSize s = cvGetSize(src);

	// Compute the cell width and the cell height
	cellWidth = s.width / numColumns;
	cellHeight = s.height / numRows;

	// Allocate an array of IplImage* s
	rv = malloc(sizeof(IplImage*) * numColumns * numRows);
	if (rv == NULL) {
		fprintf(stderr, "Could not allocate rv array\n");
	}

	// Iterate over the cells
	i = 0;
	for (y = 0; y < s.height; y += cellHeight)
		for (x = 0; x < s.width; x += cellWidth) {
			// Create a new image of size cellWidth x cellHeight and
			// store it in rv[i]. The depth and number of channels should
			// be the same as src.
			rv[i] = cvCreateImage(cvSize(cellWidth, cellHeight), src->depth,
					src->nChannels);
			if (rv[i] == NULL) {
				fprintf(stderr, "Could not allocate image %d\n", i);
			}

			// set the ROI of the src image
			cvSetImageROI(src, cvRect(x, y, cellWidth, cellHeight));

			// copy src to rv[i] using cvCopy, which obeys ROI
			cvCopy(src, rv[i], NULL);

			// reset the src image roi
			cvResetImageROI(src);

			// increment i
			i++;
		}

	// return the result
	return rv;
}

/**
 * Finds the CvScalar in colors closest to t using the colorDistance function.
 * @param t		 	The color to look for
 * @param scolors	The colors to look through
 * @param numColors	The length of scolors
 * @returns			The index of scolors that t is closest to
 * 					(i.e., colorDistance( t, scolors[result]) <=
 * 					colorDistance( t, scolors[i]) for all i != result)
 */
int findClosest(CvScalar t, CvScalar* scolors, int numColors) {
	int rv = 0, // return value
		 i; // used to iterate
	double d, // stores the result of distance
	       m = colorDistance(t, scolors[0]); // the current minmum distance
	// TODO: iterate over scolors
	for(i=1;i<numColors;i++){
		// TODO: compute the distance between t and scolors[i]
		d = colorDistance(t, scolors[i]);
		// TODO: check if the distance is less then current minimum
		if(d<m){
			rv = i;
			m = d;
		}
	}
	return rv;	
}

/**
 * For each image provided, computes the average color vector
 * (represented as a CvScalar object).
 *
 * @param images	The images
 * @param numImages	The length of images
 * @returns 		An numImages length array of CvScalars were rv[i] is the average color in images[i]
 */
CvScalar* getAvgColors(IplImage** images, int numImages) {
	CvScalar* rv;
	int i;
	// TODO: create return vector
	rv = malloc(sizeof(CvScalar)*numImages);
	// TODO: iterate over images and compute average color
	for(i=0;i<numImages;i++){
		// TODO: for each image, compute the average color (hint: use cvAvg)
		rv[i] = cvAvg(images[i],NULL);
	}
	// TODO: return result
	return rv;
}

/**
 * Given an ordered list of images (iclosest), creates a
 * numColumns x numRows grid in a new image, copies each image in, and returns the result.
 *
 * Thus, if numColumns is 10, numRows is 5, and each iclosest image is 64x64, the resulting image
 * would be 640x320 pixels.
 *
 * @param iclostest		A numColumns x numRows list of images in row-major order to be put into the resulting image.
 * @param numColumns  	Number of horizontal cells
 * @param numRows		Number of vertical cells
 */
IplImage* stitchImages(IplImage** iclosest, int numColumns, int numRows) {
	int j, // for iterating over the rows
	    i, // for iterating over the columns
		 totalWidth,totalHeight,
		 q; 
	// TODO: using cvGetSize, get the size of the first image in iclosest.
	// remember all of the images should be the same size
	CvSize size = cvGetSize(*iclosest);
	// TODO: Compute the size of the final destination image.
	totalWidth = size.width*numColumns;
	totalHeight  = size.height*numRows;
	// TODO: allocate the return image. This can be potentially large, so
	// you should make sure the result is not null
	
	IplImage *rv;
	if( (rv= cvCreateImage(cvSize(totalWidth, totalHeight), (*iclosest)->depth, (*iclosest)->nChannels)) == NULL) {
		printf("Couldn't malloc memory for final image. Exiting...\n");
		exit(EXIT_FAILURE);
	}
	q=0;//iterator through icloset array
	// TODO: iterate over each cell and copy the closest image into it
	for(j=0;j<numRows;j++){
		for(i=0;i<numColumns;i++){
			// TODO: set the ROI of the result
			cvSetImageROI(rv,cvRect(i*size.width,j*size.height,size.width,size.height));
			// TODO: copy the proper image into the result
			cvCopy(iclosest[q],rv,NULL);
			// TODO: reset the ROI of the result
			cvResetImageROI(rv);
			q++;
		}
	}
	// TODO: return the result
	return rv;
}

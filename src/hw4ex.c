/*
 * hw4.c
 *
 *  Created on: Mar 10, 2013
 *      Author: lou
 */

// Images from http://press.liacs.nl/mirflickr/#sec_download
#include "hw4.h"

#define VIS_SIZE 10

int main(int argc, char ** argv) {
	int numberOfImages, // the number of images loaded to make the mosaic
			numColumns, // the number of horizontal images in the result
			numRows, // the number of vertical images in the result
			i; // just an iterator
	IplImage *src, // the source image
			*res, // the resulting mosaic image
			*smallRes; // the resulting mosaic image that is only 2x the size of the original
	IplImage **timages, // the small images loaded from the disk
			**subImages, // small images created from the source image
			**closest; // the pictures in timates closest to the subImages
	CvScalar *tcolors, // the average color of each thumbnail image
			*scolors; // the average color of the sub images of the source image
	// Load the source image
	src = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
	// get the number of rows and columns
	numColumns = atoi(argv[2]);
	numRows = atoi(argv[3]);

	// make sure we have an even separation
	assert(src->width%numColumns==0);
	assert(src->height%numRows==0);

	// The number of images passed in
	numberOfImages = argc - 4;

	// Load the thumbnail images
	printf("Loading images\n");
	if ((timages = loadImages(numberOfImages, argv + 4)) == NULL ) {
		fprintf(stderr, "Could not load images!");
		return EXIT_FAILURE;
	}
	// compute the average colors of the loaded thumbnails
	tcolors = getAvgColors(timages, numberOfImages);

	printf("Computing sub images\n");
	// create the sub images
	subImages = getSubImages(src, numColumns, numRows);
	// compute the average colors of the sub images
	printf("Computing sub image colors\n");
	scolors = getAvgColors(subImages, numColumns * numRows);
	
	printf("Finding closest images\n");
	// get the closest image to each subimage
	closest = malloc(sizeof(IplImage*) * numColumns * numRows);
#pragma omp parallel for
	for (i = 0; i < numColumns * numRows; i++) {
		int ci = findClosest(scolors[i], tcolors, numberOfImages);
		closest[i] = timages[ci];
	}
	printf("Closest Images found\n");
	// stitch the result
	res = stitchImages(closest, numColumns, numRows);
	printf( "Images stitched\n");

	// create a smaller version of the result for visualization
	smallRes = cvCreateImage(cvSize((src->width)*VIS_SIZE, (src->height)*VIS_SIZE), src->depth, src->nChannels);
	cvResize(res, smallRes, CV_INTER_CUBIC);
	printf("saving scaled image to Mosiac.jpg\n");
	cvSaveImage("Mosiac.jpg", smallRes, NULL );

	return EXIT_SUCCESS;
}

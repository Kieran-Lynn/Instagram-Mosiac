/*
 * hw4.h
 *
 *  Created on: Mar 10, 2013
 *      Author: lou
 */

#ifndef HW4_H_
#define HW4_H_

#include "cv.h"
#include "highgui.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"

/**
 * Loads an image (stored as an IplImage struct) for each
 * filename provided.
 * @param numImages		The number of images to load
 * @param filenames		A list of strings of the filenames to load
 */
IplImage** loadImages(int numImages, char ** fileNames);

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
double colorDistance(CvScalar c1, CvScalar c2);

/**
 * Splits up an image into numColumns by numRows sub-images and returns the results.
 *
 * @param src	The source image to split
 * @param numColumns	The number of columns to split into
 * @param numRows 		The number of rows to split into
 * @returns				A numColumns x numRows array of IplImages
 */
IplImage ** getSubImages(IplImage* src, int numColumns, int numRows);

/**
 * Finds the CvScalar in colors closest to t using the colorDistance function.
 * @param t		 	The color to look for
 * @param scolors	The colors to look through
 * @param numColors	The length of scolors
 * @returns			The index of scolors that t is closest to
 * 					(i.e., colorDistance( t, scolors[result]) <=
 * 					colorDistance( t, scolors[i]) for all i != result)
 */
int findClosest(CvScalar t, CvScalar* scolors, int numColors);
/**
 * For each image provided, computes the average color vector
 * (represented as a CvScalar object).
 *
 * @param images	The images
 * @param numImages	The length of images
 * @returns 		An numImages length array of CvScalars were rv[i] is the average color in images[i]
 */
CvScalar* getAvgColors(IplImage** images, int numImages);

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
IplImage* stitchImages(IplImage** iclosest, int numColumns, int numRows);
#endif /* HW4_H_ */

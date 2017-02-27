/*
Author: Chun-Wei Chiang
Object: canny detection of the image and find the circle shape via hough transform
*/

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sys/time.h>
#include <algorithm>
using namespace std;
 
using namespace cv;

Mat src;
Mat detected_edges;
Mat dst;

int edgeThresh = 1;
/*
If an edge pixel’s gradient value is higher than the high threshold value, it is marked as a strong edge pixel. 
If an edge pixel’s gradient value is smaller than the high threshold value and larger than the low threshold value, it is marked as a weak edge pixel. 
If an edge pixel's value is smaller than the low threshold value, it will be suppressed. 
Normailly the high Threshold is 2-3 times to low threshold.
*/
int lowThreshold = 70;
int ratio = 3;
/*
size of sobel filter size
*/
int kernel_size = 3;
const int HoughTreshold = 75;

const char* window_name = "original image";
const char* window_name2 = "canny edge detect";
const char* window_name3 = "hough detect";
const char* source_address = "/home/chunwei/computerVision/ShapeDetection/3circle.jpg";
static unsigned int*** Accumulator; //(a,b,r)
 
void edgefinder(int, void*){

	// Reduce noise with a Gaussian kernel 5*5 with sigma 1.3
	GaussianBlur( src, detected_edges, Size(5,5), 1.3, 1.3);
	// Canny detector
	Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );
	// Show the edge image
	imshow( window_name2, detected_edges );
	imwrite ("edge.jpg ", detected_edges);
	cout << detected_edges.cols<< "   " << detected_edges.rows << endl;

}

void houghTrasform(unsigned char* hough_src){
	
	cv::cvtColor(src,dst,CV_GRAY2RGB);

	int w = detected_edges.cols;
	int h = detected_edges.rows;
	int radius = min(w,h)/2;

	// //initialize the matrix
	Accumulator = new unsigned int **[h];
	for(int i = 0 ; i < h; i++){
		Accumulator[i] = new unsigned int *[w];
		for(int j = 0 ; j < w ; j++){
			Accumulator[i][j] = new unsigned int [radius];
			for(int k = 0 ; k < radius; k++){
				Accumulator[i][j][k] = 0; 
			}
		}
	}

	int r;

	cout << "start hough transform" <<endl;
	clock_t start_time = clock();

	//traditional Hough transform to detect circle.
	for(int i = 0 ; i < h; i++){
		for(int j = 0; j < w ; j++){
			
			// cout << hough_src [ (i*w) + j] << " " ;

			if(hough_src [ (i*w) + j]>250){ //edge
				for(int a = 0; a < h ; a++){
					for (int b = 0 ; b < w ; b++){
						r = (int) sqrt( (i-a)*(i-a) + (j-b)*(j-b) );
						if( r >0 && r < radius){
							Accumulator[a][b][r]++;
						}
					}
				}
			}
		}
	}

	for(int i = 0; i < h; i++){
		for(int j = 0; j < w; j++){
			for(int k = 0; k < radius; k++){
				if(Accumulator[i][j][k] > HoughTreshold){
					//To reduce the duplicate cirlce in (9*9)
					int max = Accumulator[i][j][k];
					for(int li = -4 ; li <= 4; li++ ){
						for(int lj = -4 ; lj <= 4; lj++){
							for(int lk = -4 ; lk <= 4; lk++){
								
								if(i+li >= 0 && i+li < h && j+lj >=0 && j+lj < w && k+lk > 0 && k + lk < radius){
									
									if(Accumulator[i+li][j+lj][k+lk] > max ){
										max = Accumulator[i+li][j+lj][k+lk];
										li = lj = lk = 5;
									}
								}
							}
						}
					}

					if(max > Accumulator[i][j][k]) continue;

					//To avoid two points have the same accumulator, and they would draw two circles at the near place.
					Accumulator[i][j][k]++;
					cout << "(" << i << "," << j << "," << k << ")" << endl;
					cv::circle(dst, Point(j,i), k, Scalar(0,0,255), 1);
				}
			}
		}
	}

	clock_t end_time = clock();
	cout << "It takes " << (end_time - start_time) * 1000 / CLOCKS_PER_SEC << " ms to do the hough transform." << endl;
	cout << "end draw circle" << endl;

	imshow( window_name3, dst);
	imwrite ("output1.jpg ", dst);
}
 
int main(){
 

	// Load an image in gray scale
	src = imread(source_address,0 );
 
	if( !src.data ){ 
		cout << "cannot find the image"<<endl;
		return -1; 
	}
 	
 	// Create a matrix of the same type and size as src (for dst)
	
	// Create a window
	namedWindow( window_name, CV_WINDOW_AUTOSIZE );
	namedWindow( window_name2, CV_WINDOW_AUTOSIZE );
	namedWindow( window_name3, CV_WINDOW_AUTOSIZE );

	// Show the original image
	imshow( window_name, src );
	//find the edge
	edgefinder(0, 0);
	houghTrasform(detected_edges.data);
	// Wait until user exit program by pressing a key
	waitKey(0);
 
 return 0;
}


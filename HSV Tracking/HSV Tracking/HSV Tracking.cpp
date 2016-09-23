#include "stdafx.h"
#include <iostream>
#include <stdlib.h>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\imgcodecs.hpp>


using namespace cv;
using namespace std;

Mat src, hsv, dst;

//void makeTrackbar();
void hsvThresh();
void contours();


int main() {
	src = imread("C:/Users/Jake/Desktop/circles.jpg", 1);
	cvtColor(src, hsv, CV_RGB2HSV);

	imshow("Original", src);
	imshow("HSV", hsv);
	hsvThresh();
	contours();
	

	waitKey(0);
	return 0;

}

/*void makeTrackbar() {
	int Hmax = 255;
	int Hmin = 0;
	int Smax = 255;
	int Smin = 0;
	int Vmax = 255;
	int Vmin = 0;

	cvCreateTrackbar("H Max", "Threshold", &Hmax, 255);

}*/

void hsvThresh() {
	
	inRange(hsv,Scalar(100,100,100),Scalar(255,255,255),dst);
	imshow("Threshold Image", dst);
	dst.convertTo(dst, CV_8U);
}

void contours() {
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	findContours(dst, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
}
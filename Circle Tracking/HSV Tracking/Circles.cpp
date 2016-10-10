#include "stdafx.h"	//only needed for Visual Studio
#include "opencv2/imgcodecs.hpp"	//I forget what each of these librarys do
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <numeric>
#include <algorithm>
#include <iterator>

using namespace cv;
using namespace std;

vector<double> filter(vector<double> v) {	//Filters extra circles out
	double sum = std::accumulate(v.begin(), v.end(), 0.0);	//Calculate mean
	double mean = sum / v.size();
	double sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
	double stdev = std::sqrt(sq_sum / v.size() - mean * mean);	//Calculate standard deviation
	double hi = mean + stdev;
	double low = mean - stdev;
	vector<double> fil;
	for (int i = 0; i < v.size(); i++) {
		if ((v[i] > low) && (v[i] < hi))	//Filter out values
			fil.push_back(v[i]);
	}
	return fil;	//return new vector
}
double stddev(double mean, vector<double> v){		//Calculates Standard Deviation of a vector
	double sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
	double stdev = std::sqrt(sq_sum / v.size() - mean * mean);
	return stdev;
}

int main(int argc, char** argv)
{
	ofstream cir("C:/Users/Jake/Desktop/Circle.txt");
	VideoCapture cap(0);	//Captures video from specified device
	while (1) {
		Mat img;
		cap.read(img);	//Read data from video frame
		Mat gray;	
		cvtColor(img, gray, COLOR_BGR2GRAY);	//Converts image to grayscale
		medianBlur(gray, gray, 5);		//Blurs image to reduce noise
		vector<Vec3f> circles;	//Vector to hold values from circle detection
		HoughCircles(gray, circles, HOUGH_GRADIENT, 1,	//Detect circles in image
			gray.rows / 8, // change this value to detect circles with different distances to each other
			100, 30, 1, 50 // change the last two parameters
						   // (min_radius & max_radius) to detect larger circles
		);

		vector<double> xcoord;  //vector used to store x-coordinates of circles
		vector<double> ycoord;  //vector used to store y-coordinates of circles
		for (size_t i = 0; i < circles.size(); i++) {
			Vec3i c = circles[i];
			circle(img, Point(c[0], c[1]), c[2], Scalar(0, 0, 255), 3, LINE_AA);	//Draws detected circles in image
			//circle(img, Point(c[0], c[1]), 2, Scalar(255, 0, 0), 3, LINE_AA);
			xcoord.push_back(c[0]); //store x values in xcoord
			ycoord.push_back(c[1]); //store y values in ycoord
		}
		vector<double> x = filter(xcoord);
		vector<double> y = filter(ycoord);
		if ((x.size() == 6) || (y.size() == 6))
			for(int i = 0; i<7; i++)
				circle(img, Point(x[i], y[i]), 2, Scalar(255, 0, 0), 3, LINE_AA);

		imshow("detected circles", img); //Show detected circles
		if(waitKey(100)=='q')	//hold 'q' to quit program
			break;

	}
	return 0;
}

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

vector<double> filter(vector<double> u, vector<double> v) {	//Filters extra circles out
	double devscale = 1.5;
	double sumv = std::accumulate(v.begin(), v.end(), 0.0);	//Calculate mean
	double meanv = sumv / v.size();
	double sq_sumv = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
	double stdevv = std::sqrt(sq_sumv / v.size() - meanv * meanv);	//Calculate standard deviation
	double hiv = meanv + devscale*stdevv;
	double lowv = meanv - devscale*stdevv;
	double sumu = std::accumulate(u.begin(), u.end(), 0.0);	//Calculate mean
	double meanu = sumu / u.size();
	double sq_sumu = std::inner_product(u.begin(), u.end(), u.begin(), 0.0);
	double stdevu = std::sqrt(sq_sumu / u.size() - meanu * meanu);	//Calculate standard deviation
	double hiu = meanu + devscale*stdevu;
	double lowu = meanu - devscale*stdevu;
	vector<double> fil;
	//cout << u.size() << " + " << v.size() << endl;
	for (int i = 0; i < (v.size()); i++) {
		if (((u[i] > lowu) && (u[i] < hiu)) && ((v[i] > lowv) && (v[i] < hiv))) {
			fil.push_back(u[i]);
			fil.push_back(v[i]);
		}
	}
	return fil;	//return new vector
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
			//circle(img, Point(c[0], c[1]), c[2], Scalar(0, 0, 255), 3, LINE_AA);	//Draws detected circles in image
			//circle(img, Point(c[0], c[1]), 2, Scalar(255, 0, 0), 3, LINE_AA);
			xcoord.push_back(c[0]); //store x values in xcoord
			ycoord.push_back(c[1]); //store y values in ycoord
		}
		vector<double> filr = filter(xcoord, ycoord);
		cout << filr.size() << endl;
		if (filr.size()==12){
			//for (int i = 0; i < 12; i + 2)
				//circle(img, Point(filr[i], filr[i+1]), 2, Scalar(255, 0, 0), 3, LINE_AA);
				cout << "target" << endl;
		}

		imshow("detected circles", img); //Show detected circles
		if(waitKey(100)=='q')	//hold 'q' to quit program
			break;

	}
	return 0;
}

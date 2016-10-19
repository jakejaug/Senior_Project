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
#include <math.h>

using namespace cv;
using namespace std;

vector<double> filter(vector<double> u, vector<double> v, Mat img) {	//Filters extra circles out
	double devscale = 3;
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
	double s = (stdevu + stdevv) / 2;  //average standard deviation of x and y
	s = s*1.8;
	if(s>0)
		circle(img, Point(meanu, meanv), s, Scalar(255, 0, 0), 3, LINE_AA);	//draw circle representing std
	for (int i = 0; i < (v.size()); i++) {
		double r = sqrt(pow(u[i] - meanu, 2) + pow(v[i] - meanv, 2));	//fill vector with values within std
		if (r < s) {
			fil.push_back(u[i]);
			fil.push_back(v[i]);
		}

	}
	return fil;	//return new vector
}

vector<double> sort(vector<double> f) {		//sorts the coordinates found from circle tracking
	double x[6];
	double y[6];
	int j = 0;
	for (int i = 0; i < 6; i++) {
		x[i] = f[j];
		y[i] = f[j + 1];
		j=j+2;
	}
	double sx=0;
	double sy=0;
	for (int i = 0; i < 6; i++) {
		sx += x[i];
		sy += y[i];
	}
	double mx = sx / 6.0;	//find mean coordinate values
	double my = sy / 6.0;
	double r[6];
	for (int i = 0; i < 6; i++) {
		r[i] = sqrt(pow(x[i]-mx, 2) + pow(y[i]-my, 2));
	}
	double sml = r[0];
	int p;
	for (int i = 0; i < 6; i++) {
		if (sml >= r[i]) {
			sml = r[i];
			p = i;
		}
	}
	vector<double> sort;	//add circle closest to mean first
	sort.push_back(x[p]);
	sort.push_back(y[p]);
	double theta[5];
	double x2[5];
	double y2[5];
	int l = 0;
	cout << "start" << endl;
	for (int i = 0; i < 6; i++) {
		if (i != p) {
			theta[l] = atan2((y[i]-my), (x[i]-mx));		//determine radial position of the remaining circles
			//cout << theta[i] << endl;
			x2[l] = x[i];
			y2[l] = y[i];
			l++;
			//cout << x2[i] << "  " << y2[i] << endl;
		}
	}

	bool swap = true;
	int k = 0;
	while(swap) {		//sort circle coordinates by increasing radial value
		swap = false;
		k++;
		for (int j = 0; j < 5-k; j++)
		{
			if (theta[j] > theta[j + 1])
			{
				double temp = theta[j + 1];
				double tempx = x2[j + 1];
				double tempy = y2[j + 1];
				theta[j + 1] = theta[j];
				x2[j + 1] = x2[j];
				y2[j + 1] = y2[j];
				theta[j] = temp;
				x2[j] = tempx;
				y2[j] = tempy;
				swap = true;
				
			}
			//cout << x2[j] << "  " << y2[j] << endl;
		}
		//cout << x2[i] << "  " << y2[i] << endl;
	}
	for (int i = 0; i < 5; i++) {	//add sorted coordinates to vector
		cout << theta[i] << " " << (x2[i]-mx) <<" " <<(y2[i]-my)<<endl;
		sort.push_back(x2[i]);
		sort.push_back(y2[i]);
	}
	return sort;
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
		int ptsx[] = { 276, 277, 308, 275, 300, 238};
		int ptsy[] = { 162, 187, 173, 152, 143, 138};
		//for (int i = 0; i < 6; i++) {
			//circle(img, Point(ptsx[i], ptsy[i]), 1, Scalar(255, 255, 0), 3, 8, 0);
		//}
		vector<double> filr = filter(xcoord, ycoord, img);	//use statistics to filter out unwanted circles
		//cout << filr.size() << endl;
		int count2 = 0;
		if (filr.size() == 12) {
			//cir << "Filtered:" << endl;
			//for (int i = 0; i < 6; i++) {
			//	cir << "x:  " << filr[count2] << "y:  " << filr[count2 + 1] << endl;
				//count2 = count2 + 2;
			//}
			vector<double> s = sort(filr);	//sort filtered circle into form of target feature vector
			int count = 0;
			//cir << "Start Sort" << endl;
			for (int i = 0; i < 6; i++) {
				circle(img, Point(s[count], s[count+1]), 1, Scalar(255, 0, 0), 3, 8, 0);	//Draw center point for current feature vector
				cir << s[count] << " " << s[count + 1] << " ";
				count = count + 2;
			}
			//cir << endl;
			//for (int i = 0; i < 12; i++)
				//cir << filr[i] << "	";
			
		}
		cir << endl;
		

		imshow("detected circles", img); //Show detected circles
		if(waitKey(100)=='q')	//hold 'q' to quit program
			break;

	}
	return 0;
}

//#include "stdafx.h"	//only needed for Visual Studio
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <math.h>
#include <Eigen/Dense>

using namespace cv;
using namespace std;
using namespace Eigen;

//filters out circles outside of the target
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

//sorts the coordinates found from circle tracking
MatrixXd sort(vector<double> f) {		
	MatrixXd sort(12, 1);
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
	//find mean coordinate values
	double mx = sx / 6.0;	
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
	//add circle closest to mean first
	sort.row(0) << x[p];
	sort.row(1) << y[p];
	double theta[5];
	double x2[5];
	double y2[5];
	int l = 0;
	//determine radial position of the remaining circles
	for (int i = 0; i < 6; i++) {
		if (i != p) {
			theta[l] = atan2((y[i]-my), (x[i]-mx));
			x2[l] = x[i];
			y2[l] = y[i];
			l++;
		}
	}

	bool swap = true;
	int k = 0;
	//sort circle coordinates by increasing radial value
	while(swap) {
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
		}
	}
	//add sorted coordinates to matrix
	int count = 2;
	for (int i = 0; i < 5; i++) {	
		sort.row(count) << x2[i];
		count++;
		sort.row(count) << y2[i];
		count++;
	}
	return sort;
}


int main(int argc, char** argv)
{
	//ofstream cir("C:/Users/Jake/Desktop/Circle.txt");
	//Captures video from specified device
	VideoCapture cap(0);
	//Create place holder values
	MatrixXd fc(12,1);
	MatrixXd fp(12, 1);
	fp << 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0;
	MatrixXd thec(3, 1);
	thec << 0, 0, 0;
	MatrixXd thep(3, 1);
	thep << 0, 0, 0;
	MatrixXd jac(12, 3);
	jac << 89, 0, 7, 0, 71, -8, 72, 0, -3, 0, 51, 13, 94, 0, 6, 0, 16, 6, 116, 0, -3, 0, 48, -25,
		101, 0, 4, 0, 107, -37, 71, 0, 4, 0, 107, 4;
	//jac << -103, 1, -99, 3, -37, 3, -115, 13, -119, -3, -33, 1, -108, 0, -110, -9, -33, -13, -100,
		//-10, -96, -10, -38, -10, -91, -5, -83, 8, -40, 6, -111, 5, -107, 11, -37, 23;
	MatrixXd P(3, 3);
	P << 1, 0, 0, 0, 1, 0, 0, 0, 1;
	double lamb = 0.5;
	MatrixXd lam(1, 1);
		lam << lamb;
	bool init = false;
	while (1) {
		Mat img;
		//Read data from video frame
		cap.read(img);	
		Mat gray;
		//Converts image to grayscale
		cvtColor(img, gray, COLOR_BGR2GRAY);
		//Blurs image to reduce noise
		medianBlur(gray, gray, 5);
		//Vector to hold values from circle detection
		vector<Vec3f> circles;	
		//Detect circles in image
		HoughCircles(gray, circles, HOUGH_GRADIENT, 1,	gray.rows / 8, 100, 30, 1, 50);
		//vectors used to store x and Y coordinates of circles
		vector<double> xcoord;  
		vector<double> ycoord;
		for (size_t i = 0; i < circles.size(); i++) {
			Vec3i c = circles[i];
			//Draws detected circles in image
			circle(img, Point(c[0], c[1]), c[2], Scalar(0, 0, 255), 3, LINE_AA);
			//stores coordinate values in vector
			xcoord.push_back(c[0]);
			ycoord.push_back(c[1]);
		}
		//create target feature vector
		int ptsx[] = { 285, 157, 282, 422, 391, 193 };
		int ptsy[] = { 215, 165, 79, 158, 310, 311 };
		MatrixXd tfv(12,1);	
		tfv << 285, 215, 157, 165, 282, 79, 422, 158, 391, 310, 193, 311;
		for (int i = 0; i < 6; i++) {
			circle(img, Point(ptsx[i], ptsy[i]), 1, Scalar(255, 255, 0), 3, 8, 0);
		}
		//use statistics to filter out unwanted circles
		vector<double> filr = filter(xcoord, ycoord, img);
		int count2 = 0;
		//Check if 6 circles remain after filtering
		if (filr.size() == 12) {
			//sort filtered circle into form of current feature vector
			fc = sort(filr);	
			int count = 0;
			for (int i = 0; i < 6; i++) {
				//circle(img, Point(fc.row(count).sum(), fc.row(count+1).sum(), 1, Scalar(255, 0, 0), 3, 8, 0);	//Draw center point for current feature vector
				//cir << fc.row(count) << endl << fc.row(count+1) << endl;								//Needs updating
				count = count + 2;
			}
			//cir << endl;
			if (init == false) {
				fp = fc;
				init = true;
			}
			else {
				//Begin Uncalibrated visual servoing
				MatrixXd df(12, 1);
				df = fc - fp;
				//find e
				MatrixXd e(12, 1);
				e = tfv - fc;
				//update fp
				fp = fc;
				//Find h
				MatrixXd h(3, 1);
				h = thec - thep;
				//update Jacobian Matrix
				RowVectorXd var(1);
				var = (lam + h.transpose()*P*h).inverse();
				jac = jac + (var.sum())*(df - jac*h)*h.transpose()*P;
				//cout << jac << endl << endl;
				//Update P
				MatrixXd a1(3,3);
				//a1 << 1, 1, 1, 1, 1, 1, 1, 1, 1;
				MatrixXd a2(1,1);
				a2 = ((lam + h.transpose()*P*h).inverse());
				a1 << a2.row(0), a2.row(0), a2.row(0), a2.row(0), a2.row(0), a2.row(0), a2.row(0), a2.row(0), a2.row(0);
				P = 1/lamb*((P-(a1)*(P*h*h.transpose()*P)));
				//P = lam.inverse()*((P - (lam + h.transpose()*P*h).inverse())*(P*h*h.transpose()*P));
				//Update motor commands
				MatrixXd theta(3, 1);
				theta = thep - (jac.transpose()*jac).inverse()*jac.transpose()*e;
				thep = thec;
				thec = theta;
				cout << endl << thec << endl;
			}
		}
		//Show detected circles
		imshow("detected circles", img); 
		//hold 'q' to quit program
		if(waitKey(10)=='q')	
			break;
	}
	return 0;
}

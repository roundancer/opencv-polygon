#include <cv.h>
#include <opencv.hpp>
#include <highgui.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

using namespace cv;
using namespace std;

void main() {

	//  ��������� ��������
	Mat src = imread("img1.jpg", 1);

	//  ������������ � HSV
	Mat hsv;
	cvtColor(src, hsv, CV_RGB2HSV);

	//  ��������� �� ������
	Mat hsv_channels[3];
	split(hsv, hsv_channels);
	//  ���������� ������
	imshow("H", hsv_channels[0]);
	imshow("S", hsv_channels[1]);
	imshow("V", hsv_channels[2]);

	//  ������ ������� �� ������������
	Mat s_range;
	inRange(hsv_channels[1], 0, 60, s_range);
	//  ���������� ��������� �������
	imshow("S_Range", s_range);

	//  ���������� ��� �� ������� �� ������������
	Mat canny;
	Canny(s_range, canny, 100, 200);
	//  ���������� ��������� ������ ���
	imshow("Canny", canny);

	//  ���� �������
	vector< vector<Point> > contours;
	findContours(s_range, contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

	//  ������ �������� ��� �����
	Mat mask = Mat::zeros(canny.rows, canny.cols, CV_8UC1);

	// CV_FILLED fills the connected components found
	drawContours(mask, contours, -1, Scalar(255), CV_FILLED);
	//  fillPoly(mask, contours, Scalar(255), 8);
	imshow("Mask", mask);

	std::ofstream fout;
	fout.open("edges_area.txt");
	for (unsigned int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > 4)
		{
			for (unsigned int j = 0; j < contours[i].size(); j++)
			{
				std::cout << "Point(x,y)=" << contours[i][j] << std::endl;
				fout << "Point(x,y)=" << contours[i][j] << "\n";
			}

			std::cout << "# of contour points: " << contours[i].size() << std::endl;
			fout << "@@@@@@@@@@@@@ of contour points: " << contours[i].size() << "\n";

			std::cout << " Area: " << contourArea(contours[i]) << std::endl;
			fout << "########################## Area: " << contourArea(contours[i]) << "\n" << "\n";
		}
	}
	fout.close();

	//  normalize so imwrite(...)/imshow(...) shows the mask correctly!
	//  normalize(mask.clone(), mask, 0.0, 255.0, CV_MINMAX, CV_8UC1);

	//  imwrite("canny.jpg", canny);
	//  imwrite("mask.jpg", mask);

	waitKey();
	return 0;
}
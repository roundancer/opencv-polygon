#include <cv.h>
#include <opencv.hpp>
#include <highgui.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

int main() {

//  загружаем исходник
	Mat src = imread("test0_copy.jpg", 1);
	namedWindow("src", 0);
	imshow("src", src);

//  конвертируем в HSV
	Mat hsv;
	cvtColor(src, hsv, CV_RGB2HSV);

//  разбиваем на каналы
	Mat hsv_channels[3];
	split(hsv, hsv_channels);
//  показываем каналы
	namedWindow("H", 0);
	namedWindow("S", 0);
	namedWindow("V", 0);
	imshow("H", hsv_channels[0]);
	imshow("S", hsv_channels[1]);
	imshow("V", hsv_channels[2]);

//	делаем выборку по насыщенности
	Mat s_range;
	inRange(hsv_channels[1], 0, 60, s_range);
//	показываем результат выборки
	namedWindow("S_Range", 0);
	imshow("S_Range", s_range);

//	проходимся ДГК по выборке по насыщенности
	Mat canny;
	Canny(s_range, canny, 100, 200);
//	показываем результат работы ДГК
	namedWindow("Canny", 0);
	imshow("Canny", canny);

//	ищем контуры
	vector< vector<Point> > contours;
	Mat hierarchy;
	findContours(s_range, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

//	создаём картинку под маску
	Mat mask = Mat::zeros(src.rows, src.cols, CV_8UC1);

	std::ofstream fout;
	fout.open("edges_area.txt");
	for (unsigned int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > 6)
		{
			double varContourArea = contourArea(contours[i]);
			// исходя из определённых расчётов относительно длины моей ноги и разрешения камеры
			// мы приходим к выводу, что 815 пикселей соответствуют 33 сантиметрам
			// следовательно 664225 кв. пикселя = 1089 кв. сантимера
			// а значит делим получившуюся площадь в кв. пикселях на известное число соответствия пикселей сантиметрам кв.
			// и умножаем на число соответствия сантиметрам, что даёт нам искомую площадь в кв. сантиметрах
			double areaInSm = varContourArea / 664225;
			areaInSm = areaInSm * 1089;

			//вывод информации и отрисовка контура выполняется только при площади контура более 50 кв. см.
			if (areaInSm > 50)
			{
				for (unsigned int j = 0; j < contours[i].size(); j++)
				{
					cout << "Point(x,y)=" << contours[i][j] << endl;
					fout << "Point(x,y)=" << contours[i][j] << endl;
				}
				
				cout << endl << "# of contour points: " << contours[i].size() << endl;
				cout << "Area in px: " << varContourArea << endl;
				cout << "Area in cm: " << areaInSm << endl;
				cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl << endl;

				fout << endl << "# of contour points: " << contours[i].size() << endl;
				fout << "Area in px: " << varContourArea << endl;
				fout << "Area in cm: " << areaInSm << endl << endl;
				fout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl << endl;
			}
		}
	}
	fout.close();

//	normalize so imwrite(...)/imshow(...) shows the mask correctly!
//	normalize(mask.clone(), mask, 0.0, 255.0, CV_MINMAX, CV_8UC1);

//	рисуем контуры
//	CV_FILLED заполняет найденные контуры
	drawContours(mask, contours, -1, Scalar(255), CV_FILLED);

//	отображаем нарисованные контуры
	namedWindow("Mask", 0);
	imshow("Mask", mask);

//	сохраняем необходимые изображения
	imwrite("canny.jpg", canny);
	imwrite("mask.jpg", mask);

	waitKey();
	return 0;
}
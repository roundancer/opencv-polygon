#include <cv.h>
#include <opencv.hpp>
#include <highgui.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

//функция вывода информации о площади контуров
void areaOutput(vector< vector<Point> > contours) {
	ofstream fout;
	fout.open("edges_area.txt");
	for (unsigned int i = 0; i < contours.size(); i++) {
		if (contours[i].size() > 6) {
			double varContourArea = contourArea(contours[i]);
			// исходя из определённых расчётов относительно длины моей ноги и разрешения камеры
			// мы приходим к выводу, что 815 пикселей соответствуют 33 сантиметрам
			// следовательно 664225 кв. пикселя = 1089 кв. сантимера
			// а значит делим получившуюся площадь в кв. пикселях на известное число соответствия пикселей сантиметрам кв.
			// и умножаем на число соответствия сантиметрам, что даёт нам искомую площадь в кв. сантиметрах
			double areaInSm = (varContourArea / 664225) * 1089;

			//вывод информации выполняется только при площади контура более 50 кв. см.
			if (areaInSm > 50) {
				/*for (unsigned int j = 0; j < contours[i].size(); j++) {
					cout << "Point(x,y)=" << contours[i][j] << endl;
					fout << "Point(x,y)=" << contours[i][j] << endl;
				}*/

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
}

int main() {

	//загружаем исходник
	Mat src = imread("test0_copy.jpg", 1);
	namedWindow("src", 0);
	imshow("src", src);

	//конвертируем в HSV
	Mat hsv;
	cvtColor(src, hsv, CV_RGB2HSV);

	//разбиваем на каналы
	Mat hsv_channels[3];
	split(hsv, hsv_channels);

	//делаем выборку по насыщенности
	Mat s_range;
	inRange(hsv_channels[1], 0, 60, s_range);
	//показываем результат выборки
	namedWindow("S_Range", 0);
	imshow("S_Range", s_range);

	//проходимся ДГК по выборке по насыщенности
	Mat canny;
	Canny(s_range, canny, 100, 200, 3, true);//работает он странно, к слову, для работы с маской где перепад яркости имеет бинарную основу

	//показываем результат работы ДГК
	namedWindow("Canny", 0);
	imshow("Canny", canny);

	//ищем контуры
	vector< vector<Point> > contours;
	Mat hierarchy;
	findContours(s_range, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

	//создаём картинку под маску
	Mat mask = Mat::zeros(src.rows, src.cols, CV_8UC1);
	//рисуем контуры
	drawContours(mask, contours, -1, Scalar(255), CV_FILLED);//CV_FILLED заполняет найденные контуры

	//вывод данных о площади
	areaOutput(contours);
	
	//клонируем маску
	Mat cloneMask = mask.clone();
	//клонируем оригинал
	Mat cloneSrc = src.clone();
	//массив матриц для хранения каналов оригинала
	Mat srcRGBChannels[3];
	split(cloneSrc, srcRGBChannels);
	//массив матриц под хранение каналов с применением альфа смешивания
	Mat alphaBlending[3];
	//матрица для сведения каналов с альфа смешиванием
	Mat alphaBlendingMerged;

	

	/*  namedWindow("alphaBlending_0", 0);
	namedWindow("alphaBlending_1", 0);
	namedWindow("alphaBlending_2", 0);
	imshow("alphaBlending_0", alphaBlending[0]);
	imshow("alphaBlending_1", alphaBlending[1]);
	imshow("alphaBlending_2", alphaBlending[2]); */

	//  отображаем нарисованные контуры
	namedWindow("Mask", 0);
	imshow("Mask", mask);

	//  сохраняем необходимые изображения
	imwrite("canny.jpg", canny);
	imwrite("mask.jpg", mask);

	waitKey();
	return 0;
}
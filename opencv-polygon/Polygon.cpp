#include <cv.h>
#include <opencv.hpp>
#include <highgui.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdarg.h>

using namespace cv;
using namespace std;

//функция вывода изображения в именованном окне (Named Window Image Show)
void namedWIS(string windowName, Mat src, int flags) {
	//содаём именованное окно
	namedWindow(windowName, flags);
	//отображаем изображение в окне
	imshow(windowName, src);
};

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

//альфа-смешивание изображений с одинаковым или разным кол-вом каналов
void alphaBlending(Mat src1, Mat src2, double alpha, double beta, double gamma)
{
	//количество каналов 1-го изображения
	int src1_NoC = src1.channels();
	cout << "NoC src1 = " << src1_NoC << endl;
	//количество каналов 2-го изображения
	int src2_NoC = src2.channels();
	cout << "NoC src2 = " << src2_NoC << endl;
	//определяем максимальное количество каналов (maximum number of
	//channels)
	int max_NoC;
	//и количество каналов второго по канальности изображения для
	//возможного ветвления АС
	int second_NoC;
	if (src1_NoC > src2_NoC) {
		max_NoC = src1_NoC;
		second_NoC = src2_NoC;
	}
	else {
		max_NoC = src2_NoC;
		second_NoC = src1_NoC;
	}

	//матрица для готового изображения с АС
	Mat aplphaBlendinged = Mat::zeros(src1.rows, src1.cols, src1.type());

	//если количество каналов одинаково
	if (src1_NoC == src2_NoC)
	{
		//применям функцию взвешенной поэлементной суммы двух массивов, что играет роль альфа-смешивания
		addWeighted(src1, alpha, src2, beta, gamma, aplphaBlendinged);
	}

	//если количество каналов неодинаково
	if (src1_NoC != src2_NoC)
	{	
		//динамический массив под каналы 1-го изображения
		Mat *src1_Channels = new Mat [src1_NoC];
		//разделяем каналы 1-го изображения
		split(src1, src1_Channels);

		//динамический массив под каналы 2-го изображения
		Mat *src2_Channels = new Mat [src2_NoC];
		//разделяем каналы 2-го изображения
		split(src2, src2_Channels);

		//динамический массив матриц под каналы с АС
		 Mat *channelsFAB = new Mat[max_NoC];

		 //если коичество каналов второго изображения
		 //равно единицы, то применяем поканальное альфа-смешивание:
		 if (second_NoC = 1) {
			 //поочерёдно смешиваем каналы первого изобажения с каналом
			 //второго изображения
			 for (int i = 0; i < max_NoC; i++) {
				 addWeighted(src1_Channels[i], alpha, src2, beta, gamma, channelsFAB[i]);
			 }
			 //сведение каналов с альфа-смешиванием
			 merge(channelsFAB, 3, aplphaBlendinged);
			 namedWIS("Alpha blendinged", aplphaBlendinged, 0);
		 }
		 else
		 {
			 for (int i = 0; i < second_NoC; i++)
			 {
				 for (int j = 0; j < max_NoC; j++)
				 {
					 channelsFAB[j] = src1_Channels[j].clone();
					 addWeighted(channelsFAB[j], alpha, src2_Channels[i], beta, gamma, channelsFAB[j]);
				 }
			 }
			 merge(channelsFAB, 3, aplphaBlendinged);
			 namedWIS("Alpha blendinged", aplphaBlendinged, 0);
		 }
	}
}

int main()
{

	//загружаем исходник
	Mat src = imread("test0_copy.jpg", 1);
	namedWIS("src", src, 0);

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
	namedWIS("S_Range", s_range, 0);

	//проходимся ДГК по выборке по насыщенности
	Mat canny;
	//работает он странно, к слову, для работы с маской где перепад яркости
	//имеет бинарную основу
	Canny(s_range, canny, 100, 200, 3, true);

	//показываем результат работы ДГК
	namedWIS("Canny", canny, 0);

	//ищем контуры
	vector< vector<Point> > contours;
	Mat hierarchy;
	findContours(s_range, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

	//создаём картинку под маску
	Mat mask = Mat::zeros(src.rows, src.cols, CV_8UC1);
	//рисуем контуры, CV_FILLED заполняет найденные контуры
	drawContours(mask, contours, -1, Scalar(255), CV_FILLED);

	//вывод данных о площади
	areaOutput(contours);

	//  отображаем нарисованные контуры
	namedWIS("Mask", mask, 0);

	//применяем альфа-смешивание
	alphaBlending(src, mask, 0.5, 0.5, 0.0);

	//  сохраняем необходимые изображения
	imwrite("canny.jpg", canny);
	imwrite("mask.jpg", mask);

	waitKey();
	return 0;
}
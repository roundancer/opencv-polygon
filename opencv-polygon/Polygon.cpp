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

//загружаем исходник
Mat src = imread("01_copy.jpg", 1);
//для изображения в формате HSV
Mat hsv;
//матрица под канал тона (hue)
Mat h_channel;
//матрица под выборку тона
Mat h_range;
//матрица для хранения канала насыщенности (saturation)
Mat s_channel;
//матрица под выборку по насыщенности
Mat s_range;
//матрица под ДГК
Mat canny;
//матрица под контуры
vector< vector<Point> > contours;
//матрица под иерархию
Mat hierarchy;
//матрица под маску
Mat global_mask;
//матрица для готового изображения с АС
Mat aplphaBlendinged;
//динамический массив под каналы 1-го изображения c AC
Mat *src1_Channels;
//динамический массив под каналы 2-го изображения c AC
Mat *src2_Channels;
//динамический массив матриц под каналы с АС
Mat *channelsFAB;

//для хранения промежуточной позиции ползунков нижней и верхней границы
int lowerBoundPosition_SRange = 0;
int upperBoundPosition_SRange = 32;

//для хранения промежуточной позиции ползунков нижней и верхней тона
int lowerBoundPosition_HRange = 91;
int upperBoundPosition_HRange = 166;

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
				cout << "Площадь in px: " << varContourArea << endl;
				cout << "Площадь in cm: " << areaInSm << endl;
				cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl << endl;

				fout << endl << "# of contour points: " << contours[i].size() << endl;
				fout << "Площадь in px: " << varContourArea << endl;
				fout << "Площадь in cm: " << areaInSm << endl << endl;
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
	//cout << "NoC src1 = " << src1_NoC << endl;
	//количество каналов 2-го изображения
	int src2_NoC = src2.channels();
	//cout << "NoC src2 = " << src2_NoC << endl;
	//определяем максимальное количество каналов (maximum Number of
	//Channels)
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
	//aplphaBlendinged = Mat::zeros(src1.rows, src1.cols, src1.type());

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
		if (!src1_Channels)
		{
			src1_Channels = new Mat[src1_NoC];
		}

		//разделяем каналы 1-го изображения
		split(src1, src1_Channels);

		//динамический массив под каналы 2-го изображения
		if (!src2_Channels)
		{
			src2_Channels = new Mat[src2_NoC];
		}
		//разделяем каналы 2-го изображения
		split(src2, src2_Channels);

		if (!channelsFAB)
		{
			channelsFAB = new Mat[max_NoC];
		}

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
			imshow("Alpha blendinged", aplphaBlendinged);
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
			imshow("Alpha blendinged", aplphaBlendinged);
		}
	}
}

//функция-обработчик ползунка нижней границы тона
void minHueLevel(int lowerPos, void*)
{
	inRange(h_channel, lowerPos, upperBoundPosition_HRange, h_range);
	lowerBoundPosition_HRange = lowerPos;
};

//функция-обработчик ползунка верхней границы тона
void maxHueLevel(int upperPos, void*)
{
	inRange(h_channel, lowerBoundPosition_HRange, upperPos, h_range);
	upperBoundPosition_HRange = upperPos;
}

//функция-обработчик ползунка нижней границы насыщенности
void minSaturationLevel(int lowerPos, void*)
{
	inRange(s_channel, lowerPos, upperBoundPosition_SRange, s_range);
	lowerBoundPosition_SRange = lowerPos;
};
//функция-обработчик ползунка верхней границы насыщенности
void maxSaturationLevel(int upperPos, void*)
{
	inRange(s_channel, lowerBoundPosition_SRange, upperPos, s_range);
	upperBoundPosition_SRange = upperPos;
}

int main(int argc, char* argv[])
{
	//окна для отображения картнки
	namedWindow("src", 0);
	imshow("src", src);
	namedWindow("S_Range", 0);
	namedWindow("H_Range", 0);
	namedWindow("Canny", 0);
	namedWindow("Mask", 0);
	namedWindow("Alpha blendinged", 0);

	//конвертируем в HSV	
	cvtColor(src, hsv, CV_RGB2HSV);
	//разбиваем на каналы
	Mat hsv_channels[3];
	split(hsv, hsv_channels);
	//передаём канал S в предопределённую переменную
	h_channel = hsv_channels[0];
	s_channel = hsv_channels[1];

	//ползунок нижней границы выборки по насыщенности
	createTrackbar("НГН", "S_Range", &lowerBoundPosition_SRange, 255, minSaturationLevel);

	//ползунок верхней границы выборки по насыщенности
	createTrackbar("ВГН", "S_Range", &upperBoundPosition_SRange, 255, maxSaturationLevel);

	//ползунок нижней границы выборки по насыщенности
	createTrackbar("НГТ", "H_Range", &lowerBoundPosition_HRange, 255, minHueLevel);

	//ползунок верхней границы выборки по насыщенности
	createTrackbar("ВГТ", "H_Range", &upperBoundPosition_HRange, 255, maxHueLevel);

	//выборка по тону
	inRange(h_channel, lowerBoundPosition_HRange, upperBoundPosition_HRange, h_range);

	//выборка по насыщенности
	inRange(s_channel, lowerBoundPosition_SRange, upperBoundPosition_SRange, s_range);

	while (true) {
		//создаём картинку под маску
		Mat mask = Mat::zeros(src.rows, src.cols, CV_8UC1);
		imshow("S_Range", s_range);
		imshow("H_Range", h_range);
		//проходимся ДГК по выборке по насыщенности
		//работает он странно, к слову, для работы с маской  
		//где перепад яркости имеет бинарную основу
		Canny(s_range, canny, 100, 200, 3, true);
		//показываем результат работы ДГК
		imshow("Canny", canny);
		//ищем контуры
		findContours(s_range, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
		//рисуем контуры, CV_FILLED заполняет найденные контуры
		drawContours(mask, contours, -1, Scalar(255), CV_FILLED);
		//отображаем нарисованные контуры
		imshow("Mask", mask);

		//применяем альфа-смешивание
		alphaBlending(src, h_range, 0.5, 0.5, 0.0);

		char c = cvWaitKey(33);
		if (c == 27)
		{
			global_mask = mask;
			break;
		}
		mask.release();
	}

	//вывод данных о площади
	areaOutput(contours);

	//сохраняем необходимые изображения
	imwrite("canny.jpg", canny);
	imwrite("mask.jpg", global_mask);

	waitKey();

	return 0;
}
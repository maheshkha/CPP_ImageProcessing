/*

Mahesh Khadatare

Image Processing in C++ Using MS Visual Studio 2013

*/

#include <stdlib.h>
#include <iostream>
#include "ImageProcess.h"
#include <cmath>
using namespace std;

Image::Image()
/* Creates an Image 0x0 */
{
	m_N = 0;
	m_M = 0;
	m_Q = 0;

	m_pixelVal = NULL;
}

Image::Image(int numRows, int numCols, int grayLevels)
/* Creates an Image of numRows x numCols and creates the arrays for it*/
{

	m_N = numRows;
	m_M = numCols;
	m_Q = grayLevels;

	m_pixelVal = new int *[m_N];
	for (int i = 0; i < m_N; i++)
	{
		m_pixelVal[i] = new int[m_M];
		for (int j = 0; j < m_M; j++)
			m_pixelVal[i][j] = 0;
	}
}

Image::~Image()
/*destroy image*/
{
	m_N = 0;
	m_M = 0;
	m_Q = 0;

	for (int i = 0; i < m_N; i++)
		delete m_pixelVal[m_N];

	delete m_pixelVal;
}

Image::Image(const Image& oldImage)
/*copies oldImage into new Image object*/
{
	m_N = oldImage.m_N;
	m_M = oldImage.m_M;
	m_Q = oldImage.m_Q;

	m_pixelVal = new int*[m_N];
	for (int i = 0; i < m_N; i++)
	{
		m_pixelVal[i] = new int[m_M];
		for (int j = 0; j < m_M; j++)
			m_pixelVal[i][j] = oldImage.m_pixelVal[i][j];
	}
}

void Image::operator=(const Image& oldImage)
/*copies oldImage into whatever you = it to*/
{
	m_N = oldImage.m_N;
	m_M = oldImage.m_M;
	m_Q = oldImage.m_Q;

	m_pixelVal = new int*[m_N];
	for (int i = 0; i < m_N; i++)
	{
		m_pixelVal[i] = new int[m_M];
		for (int j = 0; j < m_M; j++)
			m_pixelVal[i][j] = oldImage.m_pixelVal[i][j];
	}
}

void Image::setImageInfo(int numRows, int numCols, int maxVal)
/*sets the number of rows, columns and graylevels*/
{
	m_N = numRows;
	m_M = numCols;
	m_Q = maxVal;
}

void Image::getImageInfo(int &numRows, int &numCols, int &maxVal)
/*returns the number of rows, columns and gray levels*/
{
	numRows = m_N;
	numCols = m_M;
	maxVal = m_Q;
}

int Image::getPixelVal(int row, int col)
/*returns the gray value of a specific pixel*/
{
	return m_pixelVal[row][col];
}


void Image::setPixelVal(int row, int col, int value)
/*sets the gray value of a specific pixel*/
{
	m_pixelVal[row][col] = value;
}

bool Image::inBounds(int row, int col)
/*checks to see if a pixel is within the image, returns true or false*/
{
	if (row >= m_N || row < 0 || col >= m_M || col < 0)
		return false;

	return true;
}

void Image::getSubImage(int upperLeftRow, int upperLeftCol, int lowerRightRow,
	int lowerRightCol, Image& oldImage)
	/*Pulls a sub image out of oldImage based on users values, and then stores it
	in oldImage*/
{
	int width, height;

	width = lowerRightCol - upperLeftCol;
	height = lowerRightRow - upperLeftRow;

	Image tempImage(height, width, m_Q);

	for (int i = upperLeftRow; i < lowerRightRow; i++)
	{
		for (int j = upperLeftCol; j < lowerRightCol; j++)
			tempImage.m_pixelVal[i - upperLeftRow][j - upperLeftCol] = oldImage.m_pixelVal[i][j];
	}

	oldImage = tempImage;
}

int Image::meanGray()
/*returns the mean gray levels of the Image*/
{
	int totalGray = 0;

	for (int i = 0; i < m_N; i++)
	{
		for (int j = 0; j < m_M; j++)
			totalGray += m_pixelVal[i][j];
	}

	int cells = m_M * m_N;

	return (totalGray / cells);
}

void Image::enlargeImage(int value, Image& oldImage)
/*enlarges Image and stores it in tempImage, resizes oldImage and stores the
larger image in oldImage*/
{
	int rows, cols, gray;
	int pixel;
	int enlargeRow, enlargeCol;

	rows = oldImage.m_N * value;
	cols = oldImage.m_M * value;
	gray = oldImage.m_Q;

	Image tempImage(rows, cols, gray);

	for (int i = 0; i < oldImage.m_N; i++)
	{
		for (int j = 0; j < oldImage.m_M; j++)
		{
			pixel = oldImage.m_pixelVal[i][j];
			enlargeRow = i * value;
			enlargeCol = j * value;
			for (int c = enlargeRow; c < (enlargeRow + value); c++)
			{
				for (int d = enlargeCol; d < (enlargeCol + value); d++)
				{
					tempImage.m_pixelVal[c][d] = pixel;
				}
			}
		}
	}

	oldImage = tempImage;
}

void Image::shrinkImage(int value, Image& oldImage)
/*Shrinks image as storing it in tempImage, resizes oldImage, and stores it in
oldImage*/
{
	int rows, cols, gray;

	rows = oldImage.m_N / value;
	cols = oldImage.m_M / value;
	gray = oldImage.m_Q;

	Image tempImage(rows, cols, gray);

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
			tempImage.m_pixelVal[i][j] = oldImage.m_pixelVal[i * value][j * value];
	}
	oldImage = tempImage;
}

void Image::reflectImage(bool flag, Image& oldImage)
/*Reflects the Image based on users input*/
{
	int rows = oldImage.m_N;
	int cols = oldImage.m_M;
	Image tempImage(oldImage);
	if (flag == true) //horizontal reflection
	{
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
				tempImage.m_pixelVal[rows - (i + 1)][j] = oldImage.m_pixelVal[i][j];
		}
	}
	else //vertical reflection
	{
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
				tempImage.m_pixelVal[i][cols - (j + 1)] = oldImage.m_pixelVal[i][j];
		}
	}

	oldImage = tempImage;
}

void Image::translateImage(int value, Image& oldImage)
/*translates image down and right based on user value*/
{
	int rows = oldImage.m_N;
	int cols = oldImage.m_M;
	int gray = oldImage.m_Q;
	Image tempImage(m_N, m_M, m_Q);

	for (int i = 0; i < (rows - value); i++)
	{
		for (int j = 0; j < (cols - value); j++)
			tempImage.m_pixelVal[i + value][j + value] = oldImage.m_pixelVal[i][j];
	}

	oldImage = tempImage;
}

void Image::rotateImage(int theta, Image& oldImage)
/*based on users input and rotates it around the center of the image.*/
{
	int r0, c0;
	int r1, c1;
	int rows, cols;
	rows = oldImage.m_N;
	cols = oldImage.m_M;
	Image tempImage(rows, cols, oldImage.m_Q);

	float rads = (theta * 3.14159265) / 180.0;

	r0 = rows / 2;
	c0 = cols / 2;

	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < cols; c++)
		{
			r1 = (int)(r0 + ((r - r0) * cos(rads)) - ((c - c0) * sin(rads)));
			c1 = (int)(c0 + ((r - r0) * sin(rads)) + ((c - c0) * cos(rads)));

			if (inBounds(r1, c1))
			{
				tempImage.m_pixelVal[r1][c1] = oldImage.m_pixelVal[r][c];
			}
		}
	}

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			if (tempImage.m_pixelVal[i][j] == 0)
				tempImage.m_pixelVal[i][j] = tempImage.m_pixelVal[i][j + 1];
		}
	}
	oldImage = tempImage;
}

Image Image::operator+(const Image &oldImage)
/*adds images together, half one image, half the other*/
{
	Image tempImage(oldImage);

	int rows, cols;
	rows = oldImage.m_N;
	cols = oldImage.m_M;

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
			tempImage.m_pixelVal[i][j] = (m_pixelVal[i][j] + oldImage.m_pixelVal[i][j]) / 2;
	}

	return tempImage;
}

Image Image::operator-(const Image& oldImage)
/*subtracts images from each other*/
{
	Image tempImage(oldImage);

	int rows, cols;
	rows = oldImage.m_N;
	cols = oldImage.m_M;
	int tempGray = 0;

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{

			tempGray = abs(m_pixelVal[i][j] - oldImage.m_pixelVal[i][j]);
			if (tempGray < 35)// accounts for sensor flux
				tempGray = 0;
			tempImage.m_pixelVal[i][j] = tempGray;
		}

	}

	return tempImage;
}

void Image::negateImage(Image& oldImage)
/*negates image*/
{
	int rows, cols, gray;
	rows = m_N;
	cols = m_M;
	gray = m_Q;

	Image tempImage(m_N, m_M, m_Q);

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
			tempImage.m_pixelVal[i][j] = -(m_pixelVal[i][j]) + 255;
	}

	oldImage = tempImage;
}

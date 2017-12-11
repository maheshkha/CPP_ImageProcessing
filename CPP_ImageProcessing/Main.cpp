#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstdlib>

#include "ImageProcess.h"

#include "PGM_IO.h"
#include "PPM_IO.h"
using namespace std;

int readImageHeader(char[], int&, int&, int&, bool&);
int readImage(char[], Image&);
int writeImage(char[], Image&);

static void filterGauss3x1(unsigned char * pImgDst, const unsigned char * pImgSrc,
	const int width, const int height)
{

	for (int y = 0; y < height; y++)
	{
		unsigned char * pDst = pImgDst + y* width + 1;
		const unsigned char *pSrc = pImgSrc + y* width;

		for (int x = 1; x < width - 1; x++)
		{
			const unsigned int sum = (int)pSrc[0] + 2 * (int)pSrc[1] + (int)pSrc[2];
			*pDst = sum / 4;
			pDst++;
			pSrc++;
		}
	}
}


static void filterGauss1x3(unsigned char * pImgDst, const unsigned char * pImgSrc,
	const int width, const int height)
{

	for (int y = 1; y < height - 1; y++)
	{
		unsigned char * pDst = pImgDst + y* width;
		const unsigned char *pSrc = pImgSrc + y* width;

		for (int x = 0; x < width; x++)
		{
			const unsigned int sum = (int)pSrc[-width] + 2 * (int)pSrc[0] + (int)pSrc[width];
			*pDst = sum / 4;
			pDst++;
			pSrc++;
		}
	}
}

static void filterGaussian3x3(unsigned char * pImg, const int width, const int height)
{
	unsigned char * pFltY = new unsigned char[width*height];

	filterGauss1x3(pFltY, pImg, width, height);

	// copy border
	memcpy(pFltY, pImg, width);
	memcpy(pFltY + width*(height - 1), pImg + width*(height - 1), width);

	filterGauss3x1(pImg, pFltY, width, height);

	delete pFltY;
}

int main(int argc, char* argv[])
{
	//////////////////////////////////////////////////////////////////////////
	// Read gray-value image
	//////////////////////////////////////////////////////////////////////////
	unsigned char * pImage = 0;
	int width, height;

	bool readOk = readPGM( "eyes_dark.pgm", &pImage, width, height );

	if( ! readOk )
	{
		printf( "Reading image failed!\n");
		printf( "Press any key to exit.\n" );
		getchar();
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale image by 1/2
	//////////////////////////////////////////////////////////////////////////
	const int widthScl = width / 2;
	const int heightScl = height / 2;

	unsigned char * pScaledImage = new unsigned char[ widthScl * heightScl ];

	for( int y = 0; y < heightScl; y++ )
	{
		for( int x = 0; x < widthScl; x++ )
		{
			pScaledImage[ x + y*widthScl ] = pImage[ 2*x + 2*y*width ];
		}
	}

	writePGM( "half.pgm", pScaledImage, widthScl, heightScl );


	//////////////////////////////////////////////////////////////////////////
	// Filter image with 3x3 Gaussian kernel
	//////////////////////////////////////////////////////////////////////////
	filterGaussian3x3( pScaledImage, widthScl, heightScl );

	writePGM( "halfFiltered.pgm", pScaledImage, widthScl, heightScl );


	//////////////////////////////////////////////////////////////////////////
	// Compute histogram / cut upper and lower 5% of gray-values
	//////////////////////////////////////////////////////////////////////////
	unsigned int histogram[256];
	memset( histogram, 0, 256*sizeof(unsigned int) );

	// calculate histogram
	for( int i = 0; i < widthScl*heightScl; i++ )
		++histogram[ pScaledImage[i] ];

	// determine lower and upper bound for histogram stretch
	const float			cutOffPercentage = 0.05;
	unsigned char		lowerBound, upperBound;
	unsigned int		histAccu = 0;
	const unsigned int	lowerPercentile = cutOffPercentage * widthScl*heightScl;
	const unsigned int	upperPercentile = (1-cutOffPercentage) * widthScl*heightScl;

	for( int h = 0; h < 256 ; h++ )
	{
		histAccu += histogram[h];
		if( histAccu <= lowerPercentile )
		{
			lowerBound = h;
			continue;
		}
		if( histAccu >= upperPercentile )
		{
			upperBound = h;
			break;
		}
	}

	// assign new gray-values from linear mapping between lower and upper bound
	const float histScale = 255. / (upperBound - lowerBound);
	for( int i = 0; i < widthScl*heightScl; i++ )
	{
		const int newVal = histScale * ( (int)pScaledImage[i] - lowerBound );
		pScaledImage[i] = std::min<int>( 255, std::max<int>( 0, newVal ) );
	}

	writePGM( "histogram.pgm", pScaledImage, widthScl, heightScl );


	//////////////////////////////////////////////////////////////////////////
	// Compute image energy from gradients
	//////////////////////////////////////////////////////////////////////////
	unsigned char * energy = new unsigned char[ widthScl * heightScl ];
	memset( energy, 0, widthScl*heightScl );

	for (int y = 1; y < heightScl-1; ++y)
	{
		const int rowOffset = y * widthScl;
		for (int x = 1; x < widthScl-1; ++x)
		{
			const int gradX = pScaledImage[rowOffset+x+1] - pScaledImage[rowOffset+x-1];
			const int gradY = pScaledImage[rowOffset+x+widthScl] - pScaledImage[rowOffset+x-widthScl];
			energy[rowOffset+x] = sqrt( (float)(gradX * gradX + gradY * gradY) ) / sqrt(2.f);
		}
	}

	writePGM( "energy.pgm", energy, widthScl, heightScl );


	//////////////////////////////////////////////////////////////////////////
	// Segment high energy areas by Thresholding
	//////////////////////////////////////////////////////////////////////////
	for( int i = 0; i < widthScl*heightScl; i++ )
	{
		if( energy[i] > 30 )
			energy[i] = 255;
		else
			energy[i] = 0;
	}
	writePGM( "energyThresh.pgm", energy, widthScl, heightScl );

	delete energy;
	delete pScaledImage;
	free( pImage );


#if 1 // color
	//////////////////////////////////////////////////////////////////////////
	// Read color (RGB) image
	//////////////////////////////////////////////////////////////////////////
	rtcvRgbaValue * pRgbImage = 0;
	//readOk = readPPM( "../eyes_color.ppm", &pRgbImage, width, height );
	readOk = readPPM( "../HSV_cone.ppm", &pRgbImage, width, height ); // proof of concept ;)

	if( ! readOk )
	{
		printf( "Reading color image failed!\n");
		printf( "Press any key to exit.\n" );
		getchar();
		return -1;
	}


	//////////////////////////////////////////////////////////////////////////
	// Determine the dominant color from a Hue-histogram
	//////////////////////////////////////////////////////////////////////////

	// Compute hue-histogram with 8 bins
	unsigned int hueHist[8] = {0,0,0,0,0,0,0,0};
	for( int i = 0; i < width*height; i++ )
		++hueHist[ pRgbImage[i].getHue() >> 5 ];

	// Find maximum in histogram
	int maxHist = 0;
	int maxHue = -1;
	for( int h = 0; h < 8; h++ )
	{
		if( hueHist[h] > maxHist )
		{
			maxHist = hueHist[h];
			maxHue = h;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Segment dominant color (and neighbors) in HSV color space
	//////////////////////////////////////////////////////////////////////////
	unsigned char * hueSeg = new unsigned char[ width*height ];
	for( int i = 0; i < width*height; i++ )
	{
		const unsigned char hue = pRgbImage[i].getHue() >> 5; // in bins
		const unsigned char sat = pRgbImage[i].getSat();
		const unsigned char val = pRgbImage[i].getV();
		if( hue == maxHue && sat > 100 && val > 100 )
			hueSeg[i] = 255;
		else
			hueSeg[i] = 0;
	}

	writePGM( "../hueSegmentation.pgm", hueSeg, width, height );

	delete hueSeg;
	free( pRgbImage );
#endif
	printf( "Finished! Press any key.\n" );
	getchar();

	return 0;
}
#if 0 // old code
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstdlib>

#include "ImageProcess.h"


using namespace std;

int readImageHeader(char[], int&, int&, int&, bool&);
int readImage(char[], Image&);
int writeImage(char[], Image&);

int main(int argc, char *argv[])
{
	int M, N, Q; // rows, cols, grayscale
	int val;
	bool type;
	int mean;
	int choice = 1;
	int value = 0;
	bool able;
	int flipChoice = 0;
	bool flag;
	int translate = 0;
	int rotate = -1;
	int subULR, subULC, subLRR, subLRC;
	int r, c;



	// read image header
	readImageHeader(argv[1], N, M, Q, type);

	// allocate memory for the image array
	Image image(N, M, Q);
	//Image outImage();
	Image tempImage(image);
	Image secImage(image);

	// read image
	readImage(argv[1], image);

	//print menu until user quits
	while (choice > 0 && choice <= 13)
	{
		able = false;

		cout << "What would you like to do:" << endl;

		//menu changes as paramenters that are passed to program change
		if (argc == 3) // one input one output image
		{
			cout << "[1] Get a Sub Image" << endl;
			cout << "[2] Enlarge Image" << endl;
			cout << "[3] Shrink Image" << endl;
			cout << "[4] Reflect Image" << endl;
			cout << "[5] Translate Image" << endl;
			cout << "[6] Rotate Image" << endl;
			cout << "[7] Negate Image" << endl << endl;
		}
		else if (argc == 2) // one input image
		{
			cout << "[8] Get the Mean of the gray values" << endl;
			cout << "[9] Get Info Of Image" << endl;
			cout << "[10] Set The Value Of A Pixel" << endl;
			cout << "[11] Get the Value Of A Pixel" << endl << endl;
		}
		else if (argc == 4) // two input images, one output image
		{
			cout << "[12] Add Images" << endl;
			cout << "[13] Subtract Images" << endl << endl;
		}
		else
			cout << "You passed too many or too few images into the program." << endl;
		cout << "[0] Quit" << endl;
		cin >> choice;

		switch (choice)
		{
		case 1: // subimage
			cout << "You are getting a sub image.  Please have the upper left and lower right ";
			cout << "coordinates ready." << endl;
			cout << "Please enter the upper left row coordinate: ";
			cin >> subULR;
			cout << "Please enter the upper left column coordinate: ";
			cin >> subULC;
			cout << "Please enter the lower right row coordinate: ";
			cin >> subLRR;
			cout << "Please enter the lower right column coordinate: ";
			cin >> subLRC;

			image.getSubImage(subULR, subULC, subLRR, subLRC, image);
			writeImage(argv[2], image);

			break;
		case 2: //enlarge image
			while (able == false)
			{
				cout << "How many times larger do you want the image?" << endl;
				cin >> value;

				if (value > 5 || value < 1)
				{
					cout << "You have either entered a value that is too ";
					cout << "big, or too small, please reenter" << endl;
					able = false;
				}
				else
					able = true;
			}

			image.enlargeImage(value, image);
			writeImage(argv[2], image);
			break;
		case 3: // shrink image
			while (able == false)
			{
				cout << "How many times smaller do you want the image?" << endl;
				cin >> value;

				if (value > 5 || value < 1)
				{
					cout << "You have either entered a value that is too ";
					cout << "big, or too small, please reenter" << endl;
					able = false;
				}
				else
					able = true;
			}

			image.shrinkImage(value, image);
			writeImage(argv[2], image);
			break;
		case 4: // reflect image
			while (flipChoice < 1 || flipChoice > 2)
			{
				cout << "Do you want the image flipped horizontally or vertically?" << endl;
				cout << "[1] Horizontally" << endl;
				cout << "[2] Vertically" << endl;
				cin >> flipChoice;
			}
			if (flipChoice == 1)
				flag = true;
			else
				flag = false;

			image.reflectImage(flag, image);
			writeImage(argv[2], image);
			break;
		case 5: //translate
			image.getImageInfo(N, M, Q);
			while (translate < 1 || translate > N || translate > M)
			{
				cout << "How far would you like to translate the image?" << endl;
				cin >> translate;
			}

			image.translateImage(translate, image);
			writeImage(argv[2], image);
			break;
		case 6: //rotate
			while (rotate < 0)
			{
				cout << "Enter to what degree you want to rotate the image:" << endl;
				cin >> rotate;
			}
			image.rotateImage(rotate, image);
			writeImage(argv[2], image);

			break;
		case 7: //negate
			image.negateImage(image);
			writeImage(argv[2], image);
			break;
		case 8: //mean gray
			mean = image.meanGray();
			cout << "The mean value of the grey in image is: " << mean << endl << endl;
			break;
		case 12:   //add
			readImageHeader(argv[2], N, M, Q, type);

			// allocate memory for the image array
			secImage.setImageInfo(N, M, Q);

			// read image
			readImage(argv[2], secImage);

			tempImage = image + secImage;
			writeImage(argv[3], tempImage);
			break;
		case 13:  //subtract
			readImageHeader(argv[2], N, M, Q, type);

			// allocate memory for the image array
			secImage.setImageInfo(N, M, Q);

			// read image
			readImage(argv[2], secImage);

			tempImage = image - secImage;
			writeImage(argv[3], tempImage);
			break;
		case 9: //get info
			image.getImageInfo(N, M, Q);
			cout << "Info for " << argv[1] << endl;
			cout << "Rows: " << N << endl;
			cout << "Columns: " << M << endl;
			cout << "Max Values of Gray: " << Q << endl << endl;
			break;
		case 10: // change pixel
			cout << "Please enter the row of pixel you want to change: ";
			cin >> r;
			cout << "Please enter the column of pixel you want to change: ";
			cin >> c;
			cout << "Please enter the value you want to change the pixel: ";
			cin >> val;
			image.setPixelVal(r, c, val);
			writeImage(argv[1], image);
			break;
		case 11: //info for pixel
			cout << "Please enter the row of pixel you want to get: ";
			cin >> r;
			cout << "Please enter the column of pixel you want to get: ";
			cin >> c;
			val = image.getPixelVal(r, c);
			cout << "The value of " << r << "," << c << " is " << val << endl << endl;
			break;
		default:
			cout << "You have chosen to close the progam." << endl;
			break;
		}
	}
	system("PAUSE");


	return 0;
}

int readImage(char fname[], Image& image)
{
	int i, j;
	int N, M, Q;
	unsigned char *charImage;
	char header[100], *ptr;
	ifstream ifp;

	ifp.open(fname, ios::in | ios::binary);

	if (!ifp)
	{
		cout << "Can't read image: " << fname << endl;
		exit(1);
	}

	// read header

	ifp.getline(header, 100, '\n');
	if ((header[0] != 80) || (header[1] != 53))
	{
		cout << "Image " << fname << " is not PGM" << endl;
		exit(1);
	}

	ifp.getline(header, 100, '\n');
	while (header[0] == '#')
		ifp.getline(header, 100, '\n');

	M = strtol(header, &ptr, 0);
	N = atoi(ptr);

	ifp.getline(header, 100, '\n');
	Q = strtol(header, &ptr, 0);

	charImage = (unsigned char *) new unsigned char[M*N];

	ifp.read(reinterpret_cast<char *>(charImage), (M*N)*sizeof(unsigned char));

	if (ifp.fail())
	{
		cout << "Image " << fname << " has wrong size" << endl;
		exit(1);
	}

	ifp.close();

	//
	// Convert the unsigned characters to integers
	//

	int val;

	for (i = 0; i<N; i++)
		for (j = 0; j<M; j++)
		{
			val = (int)charImage[i*M + j];
			image.setPixelVal(i, j, val);
		}

	delete[] charImage;


	return (1);

}

int readImageHeader(char fname[], int& N, int& M, int& Q, bool& type)
{
	int i, j;
	unsigned char *charImage;
	char header[100], *ptr;
	ifstream ifp;

	ifp.open(fname, ios::in | ios::binary);

	if (!ifp)
	{
		cout << "Can't read image: " << fname << endl;
		exit(1);
	}

	// read header

	type = false; // PGM

	ifp.getline(header, 100, '\n');
	if ((header[0] == 80) && (header[1] == 53))
	{
		type = false;
	}
	else if ((header[0] == 80) && (header[1] == 54))
	{
		type = true;
	}
	else
	{
		cout << "Image " << fname << " is not PGM or PPM" << endl;
		exit(1);
	}

	ifp.getline(header, 100, '\n');
	while (header[0] == '#')
		ifp.getline(header, 100, '\n');

	M = strtol(header, &ptr, 0);
	N = atoi(ptr);

	ifp.getline(header, 100, '\n');

	Q = strtol(header, &ptr, 0);

	ifp.close();

	return(1);
}

int writeImage(char fname[], Image& image)
{
	int i, j;
	int N, M, Q;
	unsigned char *charImage;
	ofstream ofp;

	image.getImageInfo(N, M, Q);

	charImage = (unsigned char *) new unsigned char[M*N];

	// convert the integer values to unsigned char

	int val;

	for (i = 0; i<N; i++)
	{
		for (j = 0; j<M; j++)
		{
			val = image.getPixelVal(i, j);
			charImage[i*M + j] = (unsigned char)val;
		}
	}

	ofp.open(fname, ios::out | ios::binary);

	if (!ofp)
	{
		cout << "Can't open file: " << fname << endl;
		exit(1);
	}

	ofp << "P5" << endl;
	ofp << M << " " << N << endl;
	ofp << Q << endl;

	ofp.write(reinterpret_cast<char *>(charImage), (M*N)*sizeof(unsigned char));

	if (ofp.fail())
	{
		cout << "Can't write image " << fname << endl;
		exit(0);
	}

	ofp.close();

	delete[] charImage;

	return(1);

}
#endif
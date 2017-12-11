#ifndef __PPM_IO_H__
#define __PPM_IO_H__
//=================================================================================
//=================================================================================
///
/// \file	 PPM_IO.h
///
///
///
//=================================================================================
//=================================================================================


#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "string.h"

union rtcvRgbaValue
{
	int m_Val;
	struct
	{
		unsigned char m_b;
		unsigned char m_g;
		unsigned char m_r;
		unsigned char m_a;
	};
	unsigned char byte[4];
	
	rtcvRgbaValue () : m_r(0), m_g(0), m_b(0), m_a(255){}
	rtcvRgbaValue (unsigned char gray) : m_r(gray), m_g(gray), m_b(gray), m_a(255){}
	rtcvRgbaValue (unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) : m_r(r), m_g(g), m_b(b), m_a(a){}
	rtcvRgbaValue (int val) : m_Val(val){}

	unsigned char getHue() const
	{
		const int v = std::max<unsigned char>( m_r, (std::max<unsigned char>(m_g, m_b)) );
		if (0 == v)
			return 0;

		const int rgbMin = std::min<unsigned char>( m_r, (std::min<unsigned char>(m_g, m_b)) );
		const int rgbDiff = (int)v - rgbMin;

		if (0 == rgbDiff)
			return 0;

		int h;

		if ( v == m_r )
		{
			h = (43 * ((int)m_g - (int)m_b)) / rgbDiff;
		}
		else if ( v == m_g )
		{
			h = 85 + (43 * ((int)m_b - (int)m_r)) / rgbDiff;
		}
		else  // ( v == m_b )
		{
			h = 171 + (43 * ((int)m_r - (int)m_g)) / rgbDiff;
		}

		return h;
	}

	unsigned char getSat() const
	{
		int max = m_r;
		int min = m_r;

		// find maximum and minimum RGB values
		if (m_g > max)
		{
			max = m_g;
		} else {
			min = m_g;
		}

		if (m_b > max)
		{
			max = m_b;
		} else if (m_b < min) {
			min = m_b;
		}

		if (0 == max)
			return 0;

		const int delta = max - min;

		return ( (255*delta) / max );
	}

	unsigned char getV() const
	{
		return ( std::max<unsigned char>( m_r, (std::max<unsigned char>(m_g, m_b)) ) );
	}

	bool operator==(const rtcvRgbaValue & otherVal)
	{
		return ( (m_r == otherVal.m_r) && (m_g == otherVal.m_g)&& (m_b == otherVal.m_b) && (m_a == otherVal.m_a) );
	}
};


static void writePPM(const char * fileName, const rtcvRgbaValue * pImg, const int sx, const int sy, bool switchRB = false)
{
	unsigned char * pTmpBuffer = (unsigned char*)malloc(sx*sy*3*sizeof(unsigned char));
	for(int y = 0; y < sy; ++y)
	{
		for(int x = 0; x < sx; ++x)
		{
			const rtcvRgbaValue & val = *pImg++;
			pTmpBuffer[y*sx*3+x*3] = switchRB ? val.m_b : val.m_r;
			pTmpBuffer[y*sx*3+x*3+1] = val.m_g;
			pTmpBuffer[y*sx*3+x*3+2] = switchRB ? val.m_r : val.m_b;
		}
	}


	FILE* fp = fopen(fileName, "wb");
	if( !fp ) return;

	fprintf(fp,"P6\n%d %d\n255\n", sx, sy);

	if( 1 != fwrite(pTmpBuffer, sx*sy*3*sizeof(unsigned char), 1, fp) )
	{
		fclose(fp);
		return;
	}

	fclose(fp);


	free(pTmpBuffer);


	return;
}


static bool readPPM( const char * fileName, rtcvRgbaValue ** ppImg, int & sx, int & sy, bool switchRB = false )
{
	FILE * fp = fopen(fileName, "rb");
	if( !fp )
		return false;

	char format[16];
	fscanf(fp, "%s\n", &format); 
	
	char tmpCharBuf[256];
	
	fpos_t position;
	fgetpos (fp, &position);

	while ( true )
	{
		(void)fgets( tmpCharBuf, 255, fp );
		const int cnt = strncmp( tmpCharBuf, "#", 1 );
		if (0 != cnt)
		{
			fsetpos(fp, &position);
			break;
		}
		else
		{
			fgetpos (fp, &position);
		}
	}

	int maxVal;

	const int nParamRead1 = fscanf( fp, "%d %d\n", &sx, &sy );
	const int nParamRead2 = fscanf( fp, "%d\n", &maxVal );  // 255 TODO: checken!

		
	if ( (nParamRead1 != 2) || (nParamRead2 != 1) )
	{
		fclose(fp);
		return false;
	}

	unsigned char * pTmpBuffer = (unsigned char*)malloc(sx*sy*3*sizeof(unsigned char));

	*ppImg = (rtcvRgbaValue*)realloc(*ppImg, sx*sy*sizeof(rtcvRgbaValue));
	
	fseek(fp, (sx*sy*3*sizeof(unsigned char)), SEEK_END);

	const int readcount = fread(pTmpBuffer, sx*sy*3*sizeof(unsigned char), 1, fp);
	if (1 !=  readcount)
	{
		free( pTmpBuffer );
		fclose(fp);
		return false;
	}

	const int szImg = sx * sy;
	for( int i= 0; i < szImg; ++i )
	{
		(*ppImg)[i].m_r = switchRB ? pTmpBuffer[3*i+2] : pTmpBuffer[3*i];
		(*ppImg)[i].m_g = pTmpBuffer[3*i+1];
		(*ppImg)[i].m_b = switchRB ? pTmpBuffer[3*i] : pTmpBuffer[3*i+2];
		(*ppImg)[i].m_a = 255;
	}

	free( pTmpBuffer );

	fclose(fp);
	
	return true;
}

#endif
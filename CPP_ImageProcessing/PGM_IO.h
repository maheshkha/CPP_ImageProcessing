#ifndef __PGM_IO_H__
#define __PGM_IO_H__
//=================================================================================
//=================================================================================
///
/// \file	 PGM_IO.h
///
///
///
//=================================================================================
//=================================================================================


#include <limits>
#include <algorithm>
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstdlib>

static bool readPGM( const char * fileName, unsigned char ** ppData, int & sx, int & sy )
{
	if( ppData == 0 )
		return false;

	FILE * fp = fopen(fileName, "rb");
	
	if ( 0 == fp )
		return false;

	int nGrayValues;
	
	char format[16];
	fscanf(fp, "%s\n", &format);
	
	// Kommentar-Zeilen überlesen
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

	const int nParamRead1 = fscanf( fp, "%d %d\n", &sx, &sy );
	const int nParamRead2 = fscanf( fp, "%d\n", &nGrayValues );

	if ( (nParamRead1 != 2) || (nParamRead2 != 1) )
	{
		fclose(fp);
		return false;
	} else {
		if ( (0 == strncmp("P2", format, 2)) && (4096 == nGrayValues) )
		{
			const double fact = 255./4095.;
			*ppData = (unsigned char*)realloc(*ppData, sx*sy*sizeof(unsigned char) );
			int idx = 0;
			for (int y = 0; y < sy; ++y)
			{
				for (int x = 0; x < sx; ++x)
				{
					int val;
					fscanf (fp, "%d", &val);
					(*ppData)[idx++] = (unsigned char)floor( 0.5 + (double)val * fact );
				}
			}
		}
		else if ( (0 == strncmp("P5", format, 2)) && ( (255 == nGrayValues) || (256 == nGrayValues) ) )
		{
			*ppData = (unsigned char*)realloc(*ppData, sx*sy*sizeof(unsigned char) );
			fseek(fp, -sx*sy*sizeof(unsigned char), SEEK_END);
			
			const int readcount = (int)( fread(*ppData, sx*sizeof(unsigned char), sy, fp) );
			if (sy != readcount)
			{
				fclose(fp);
				return false;
			}
		}
		else
		{
			fclose(fp);
			return false;
		}
	}
	
	fclose(fp);

	return true;
};


static bool writePGM(const char * fileName, const unsigned char * pData, const unsigned int sx, const unsigned int sy)
{
	FILE* fp = fopen(fileName, "wb");
	if( !fp )
		return false;

	fprintf(fp,"P5\n%d %d\n255\n", sx, sy);

	if( 1 != fwrite(pData, sx*sy, 1, fp) )
	{
		fclose(fp);
		return false;
	}

	fclose(fp);
	return true;
};


#endif
// 
#include "stdafx.h"
#include <GL/glut.h>
#include <GL/gl.h>

#include <GL/glaux.h>  // 
#include <algorithm>
#include <stdio.h>
#include "stdlib.h"
#include "math.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>   // include important windows stuff

#define BITMAP_ID            0x4D42 // universal id for a bitmap
using namespace std;

char* filename_BMP = (char*) malloc(30*sizeof(char));

bool writeBMP(const char filename[], unsigned char* data, unsigned int w, unsigned int h);

bool screenshot( char* filename)
{
	GLenum lastBuffer;
	GLbyte* pBits = 0; // ͼ������
	unsigned long lImageSize;
	GLint iViewport[4]; // ��ͼ��С

	glGetIntegerv(GL_VIEWPORT, iViewport);
	lImageSize = iViewport[2] * iViewport[3] * 3;
	pBits = (GLbyte*)new unsigned char[lImageSize];
	if (!pBits)
	return false;

	// ��color buffer�ж�ȡ����
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

	//
	glGetIntegerv(GL_READ_BUFFER, (GLint*)&lastBuffer);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, iViewport[2], iViewport[3], GL_BGR_EXT, GL_UNSIGNED_BYTE, pBits);
	glReadBuffer(lastBuffer);

	if (writeBMP(filename, (unsigned char*)pBits, iViewport[2], iViewport[3]))
	{ 
		return true;
	}
	else
	{
		return false;
	}
}


bool writeBMP(const char filename[], unsigned char* data, unsigned int w, unsigned int h)
{
	std::ofstream out_file;

	/** ���data */
	if(!data) 
	{
		std::cerr << "data corrupted! " << std::endl;
		out_file.close();
		return false;
	}

	/** ����λͼ�ļ���Ϣ��λͼ�ļ�ͷ�ṹ */
	BITMAPFILEHEADER header;
	BITMAPINFOHEADER bitmapInfoHeader;

	//unsigned char textureColors = 0;/**< ���ڽ�ͼ����ɫ��BGR�任��RGB */

	/** ���ļ�,�������� */
	out_file.open(filename, std::ios::out | std::ios::binary);
	if (!out_file)
	{
		std::cerr << "Unable to open file " << filename << std::endl;
		return false;
	}

	/** ���BITMAPFILEHEADER */
	header.bfType = BITMAP_ID;
	header.bfSize = w*h*3 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	header.bfReserved1 = 0;
	header.bfReserved2 = 0;
	header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	/** д��λͼ�ļ�ͷ��Ϣ */
	out_file.write((char*)&header, sizeof(BITMAPFILEHEADER));

	/** ���BITMAPINFOHEADER */
	bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfoHeader.biWidth = w;
	bitmapInfoHeader.biHeight = h;
	bitmapInfoHeader.biPlanes = 1;
	bitmapInfoHeader.biBitCount = 24;
	bitmapInfoHeader.biCompression = BI_RGB; // BI_RLE4 BI_RLE8
	bitmapInfoHeader.biSizeImage = w * h * 3; // ��ѹ������ΪBI_RGB��Ҳ��������Ϊ0
	bitmapInfoHeader.biXPelsPerMeter = 0;
	bitmapInfoHeader.biYPelsPerMeter = 0;
	bitmapInfoHeader.biClrUsed = 0;
	bitmapInfoHeader.biClrImportant = 0;
	/** д��λͼ�ļ���Ϣ */
	out_file.write((char*)&bitmapInfoHeader, sizeof(BITMAPINFOHEADER));

	/** ��ָ���Ƶ����ݿ�ʼλ�� */
	out_file.seekp(header.bfOffBits, std::ios::beg);

	/** д��ͼ������ */
	out_file.write((char*)data, bitmapInfoHeader.biSizeImage);

	out_file.close();
	return true;
}
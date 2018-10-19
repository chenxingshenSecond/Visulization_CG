// #pragma once
#ifndef  _IP_H_
#define  _IP_H_

#include "stdafx.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glaux.h>

#include <windows.h>   // include important windows stuff
#define   EPS    0.00001
#define   GU_BITMAP_ID            0x4D42 // universal id for a bitmap
#define   BOOL int 

using namespace std;



class IP_Image
{
public:	
	int WIDTH ;        // 图像宽度
	int HEIGHT;        // 图像高度
	double PPI ;       // LCD的PPI 
	double LENSPITCH ;      // LCD的PPI 
	int    LENSTYPE  ;      // 透镜的形状
	double LENSTHICK ;
	unsigned char* DATA ;          // IP存储
	
	char  * IP_FILE_NAME ;  // 最终文件名字
	int   LENS_NUMBER_X;
	int   LENS_NUMBER_Y;
	int   RADIUS_X;
	int   RADIUS_Y;
	double PIXEL_DX ;
	// 物体凸出平面的距离
	double FLYOUT_DISTANCE ;
	GLfloat WINDOW_SIZESET;
	float  **Rotation_Matrix;
	double RATIO ;  // 物体显示的比例
	double SCALE ;  // 实际IPAD坐标系 到 OpenGL图像坐标系
	// 不初始化的东西
	GLenum lastBuffer;           // 读数据用的缓冲句柄
	unsigned char * pBits;        // Perspective图像数据的暂时存储
	unsigned long lImageSize;    
	GLint iViewport[4]; // 视图大小
	double LENSPITCHY;
	// 
	int FIRST_FLAG_OF_IP;  // 初始化为0，表征第几次进入像素分配Function
	float *subimage_Indexs;
	int   DIRECTION_TEST[2];
	float Magnitudes_4_Interp [4] ;
public:	
	IP_Image(); // 构造函数
	IP_Image(int width,int height, double ppi, double lenspitch, double lenstype,double lensthick, double flyout_distance,char* filename ); // 构造函数
	// ********************************************************** // 
	void Rotation_Matrix_Update(float eyex , float eyey, float eyez, float centerx, float centery, float centerz, float upx,float upy,float upz); // 
	void Cross_Matrix( GLfloat*Matrix ,GLfloat*Matrix1,GLfloat*Matrix2 ) ; 
	BOOL IP_make_FCN(int i,int j , int sizex, int sizey); // 
	void Pixel_Redistribution(int i,int j);
	void Normalize_Fcn(float *Vector) ;
	void Cal_Index(float x, float y, float z);
	void Interp_Bilinear_Fcn () ;
	void SET_IMAGE_SIZE (int x_init, int y_init, int x_width, int y_height) ; 
	bool IPwriteBMP();
	~IP_Image(); // 析构函数
	void IP_Parameter_Show();
};
#endif

//  包围起来 利于工作
#pragma region IPHEADER
#include "stdafx.h"     // 任何cpp和头文件都需要加如
#include "IP_header.h"

// 默认construction 函数
#pragma region  IP_Image::IP_Image()
IP_Image::IP_Image()
{
	// test
	WIDTH        = 2048 ; 
	HEIGHT       = 1536 ; 
	PPI          = 264  ;  
	LENSPITCH    = 2.32 ;
	LENSTYPE     = 1      ; 
	LENSPITCHY   = LENSPITCH*sqrt(3.0)/2.0;
	LENSTHICK    = 2.4;
	PIXEL_DX      = (25.4/PPI);    // PIXEL的尺寸
	FLYOUT_DISTANCE = 0.18;
	
	DATA = (unsigned char*) calloc(WIDTH*HEIGHT*3,sizeof(unsigned char)) ;
	
	LENS_NUMBER_X = WIDTH * PIXEL_DX/LENSPITCH;                 // X方向的透镜排数
	LENS_NUMBER_Y = HEIGHT* PIXEL_DX/LENSPITCH*2.0/sqrt(3.0);   // Y方向的透镜排数
	RADIUS_X      = LENSPITCH / PIXEL_DX / 2.0       ;          // X方向的半径
	RADIUS_Y      = LENSPITCH / PIXEL_DX / sqrt(3.0) ;          // Y方向最多的个数

	IP_FILE_NAME   = "Guowen_IP.bmp";
	WINDOW_SIZESET = 0.8 ;              // 设定IP过程中窗口大小
	RATIO          = 1.3 ;              // 设定物体尺寸等于IPad的几分之几
	SCALE          = (0.5/WINDOW_SIZESET*iViewport[2])/(PIXEL_DX*HEIGHT*RATIO ) ;  
	// 假定windowsize=0.7f*2，和窗口尺寸 400*400 物体大小是1
	Rotation_Matrix = (float  **) malloc(3*sizeof(float *));
	for (int i=0;i<3;i++) //
		Rotation_Matrix[i]=(float *) malloc(3*sizeof(float)); 
	//
	FIRST_FLAG_OF_IP = 0 ;                                    // 像素分配运行次数标志
	subimage_Indexs  = (float*)calloc(8 , sizeof(float));     //  存储插值可能下标
	DIRECTION_TEST[0] =-1 ; 
	DIRECTION_TEST[1] =-1 ;
}
#pragma endregion

#pragma region  IP_Image::IP_Image 2
// 显式construction 函数
IP_Image::IP_Image(int width,int height, double ppi, double lenspitch,double lenstype,
double lensthick, double flyout_distance ,char* filename)
{
	WIDTH  = width;
	HEIGHT = height;
	PPI    = ppi;      // pixel per inch
	LENSPITCH = lenspitch;   // 透镜横向间距
	LENSTYPE  = lenstype ;   // 透镜类型
	LENSTHICK = lensthick;   // 透镜厚度 
	FLYOUT_DISTANCE = flyout_distance ;
	PIXEL_DX  = 25.4/PPI ;   // PIXEL的尺寸 mm
	LENSPITCHY   = LENSPITCH*sqrt(3.0)/2.0;

	DATA = (unsigned char*) calloc(WIDTH*HEIGHT*3,sizeof(unsigned char)) ;

	LENS_NUMBER_X = WIDTH *PIXEL_DX/LENSPITCH;                // X方向的透镜个数
	LENS_NUMBER_Y = HEIGHT* PIXEL_DX/LENSPITCH*2.0/sqrt(3.0); // Y方向的透镜排数  // Modified 2015-07-28 
	RADIUS_X      = LENSPITCH / PIXEL_DX / 2.0       ;        // X方向的半径
	RADIUS_Y      = LENSPITCH / PIXEL_DX / sqrt(3.0) ;        // Y方向最多的个数
	IP_FILE_NAME  = filename;
	
	RATIO        = 1; // 设定物体尺寸等于IPad的几分之几
	SCALE        = (0.5/0.7*iViewport[2])/(PIXEL_DX*HEIGHT*RATIO ) ;  
	// 假定windowsize=0.7f*2，和窗口尺寸 400*400 物体大小是1
	Rotation_Matrix = (float  **) malloc(3*sizeof(float *));
	for (int i=0;i<3;i++) //
		Rotation_Matrix[i]=(float *) malloc(3*sizeof(float)); 
	//
	FIRST_FLAG_OF_IP =0;            // 像素分配运行次数标志
	subimage_Indexs  = (float*)calloc(2,sizeof(float)); // 
}
#pragma endregion

#pragma region  IP_Image::IP_Image()  // // 写IP图像程序
bool IP_Image::IPwriteBMP()
{
unsigned char* data = DATA ; 
unsigned int w = WIDTH;
unsigned int h = HEIGHT;
std::ofstream out_file;
/** 检查data */
if(!data) 
{
	std::cerr << "data corrupted! " << std::endl;
	out_file.close();
	return false;
}

/** 创建位图文件信息和位图文件头结构 */
BITMAPFILEHEADER header;
BITMAPINFOHEADER bitmapInfoHeader;
//unsigned char textureColors = 0;/**< 用于将图像颜色从BGR变换到RGB */

/** 打开文件,并检查错误 */
out_file.open(IP_FILE_NAME, std::ios::out | std::ios::binary);
if (!out_file)
{
	std::cerr << "Unable to open file " << IP_FILE_NAME << std::endl;
	return false;
}

/** 填充BITMAPFILEHEADER */
header.bfType = GU_BITMAP_ID;  // 国文防止串扰引入
header.bfSize = w*h*3 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
header.bfReserved1 = 0;
header.bfReserved2 = 0;
header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
/** 写入位图文件头信息 */
out_file.write((char*)&header, sizeof(BITMAPFILEHEADER));

/** 填充BITMAPINFOHEADER */
bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
bitmapInfoHeader.biWidth = w;
bitmapInfoHeader.biHeight = h;
bitmapInfoHeader.biPlanes = 1;
bitmapInfoHeader.biBitCount = 24;
bitmapInfoHeader.biCompression = BI_RGB; // BI_RLE4 BI_RLE8
bitmapInfoHeader.biSizeImage = w * h * 3; // 当压缩类型为BI_RGB是也可以设置为0
bitmapInfoHeader.biXPelsPerMeter = 0;
bitmapInfoHeader.biYPelsPerMeter = 0;
bitmapInfoHeader.biClrUsed = 0;
bitmapInfoHeader.biClrImportant = 0;
/** 写入位图文件信息 */
out_file.write((char*)&bitmapInfoHeader, sizeof(BITMAPINFOHEADER));

/** 将指针移到数据开始位置 */
out_file.seekp(header.bfOffBits, std::ios::beg);

/** 写入图像数据 */
out_file.write((char*)data, bitmapInfoHeader.biSizeImage);

out_file.close();
return true;
}
#pragma endregion


// 析构函数
IP_Image::~IP_Image()
{
	free(DATA);            // 动态分配的数组释放 
	for (int i=0;i<3;i++)  // 动态分配的数组释放
	{
	   free(Rotation_Matrix[i]);
	}
	free(Rotation_Matrix); 
	free(pBits);
	free(subimage_Indexs);
}

void IP_Image::IP_Parameter_Show()
{
	cout<<" 图像宽度 ="<<WIDTH <<endl ;       // 
	cout<<" 图像高度 ="<<HEIGHT<<endl ;       // 图像高度
	cout<< " PPI =   " <<PPI   <<endl ;       // LCD的PPI 

	cout<<"LENSPITCH间距 = "   <<LENSPITCH     <<"mm"<<endl ;
	cout<<"LENSTHICK厚度 = "   <<LENSTHICK     <<"mm"<<endl ;
	cout<<"最终文件名字 = "    <<IP_FILE_NAME        <<endl;
	cout<<"LENS_NUMBER_X = "   <<LENS_NUMBER_X <<"个"<<endl;
	cout<<"LENS_NUMBER_Y = "   <<LENS_NUMBER_Y <<"个"<<endl;
	cout<<"RADIUS_X = "        <<RADIUS_X      <<"个"<<endl;
	cout<<"RADIUS_Y = "        <<RADIUS_Y      <<"个"<<endl;
	cout<<"PIXEL_DX"           <<PIXEL_DX      <<"mm"<<endl;
	cout<<"物体凸出平面的距离 FLYOUT_DISTANCE"<<FLYOUT_DISTANCE<<endl;
	// 
	cout<<"SCALE = "<<SCALE<<endl;
	cout<<"RATIO = "<<RATIO<<endl;
 }


void IP_Image::Normalize_Fcn(float *Vector)
{
	// cout<<"Vector[0]="<<Vector[0]<<"  Vector[1]="<<Vector[1]<<"  Vector[2]="<<Vector[2]<<endl;
	float sum = sqrt ( Vector[0]*Vector[0] + Vector[1]*Vector[1] + Vector[2]*Vector[2] ) ;
	Vector[0] = Vector[0] / sum ;
	Vector[1] = Vector[1] / sum ;
	Vector[2] = Vector[2] / sum ;
}


// 向量求叉乘
void IP_Image::Cross_Matrix( GLfloat*Matrix ,GLfloat*Matrix1,GLfloat*Matrix2)
{
	Matrix[0] = 1 * ( Matrix1[1]*Matrix2[2]-Matrix1[2]*Matrix2[1] );
	Matrix[1] = 1 * ( Matrix1[0]*Matrix2[2]-Matrix1[2]*Matrix2[0] );
	Matrix[2] = 1 * ( Matrix1[1]*Matrix2[0]-Matrix1[0]*Matrix2[1] );
	double sum = sqrt(  Matrix[0] *Matrix[0]  +  Matrix[1] *Matrix[1] +  Matrix[2] *Matrix[2] );
	Matrix[1]  = Matrix[1] /sum;
	Matrix[0]  = Matrix[0] /sum;
	Matrix[2]  = Matrix[2] /sum;
}

 #pragma endregion 

// 旋转矩阵每次更新一下即可
void IP_Image::Rotation_Matrix_Update(float eyex,float  eyey,float  eyez,float  centerx, float centery, float centerz, float upx,float  upy,float  upz) // 根据摄像机位置和目标位置计算旋转矩阵
{ // 下午按照格式写出来  
    Rotation_Matrix[2][0] = centerx -eyex ;
	Rotation_Matrix[2][1] = centery -eyey ;
	Rotation_Matrix[2][2] = centerz -eyez ;  // Z'轴
	Normalize_Fcn( Rotation_Matrix[2] );  // 获得归一化的Z'轴
	float up_dot_Z_axis = (upx*Rotation_Matrix[2][0]+upy*Rotation_Matrix[2][1]+upz*Rotation_Matrix[2][2]); 
	Rotation_Matrix[1][0] = upx -Rotation_Matrix[2][0]*up_dot_Z_axis ;
	Rotation_Matrix[1][1] = upy -Rotation_Matrix[2][1]*up_dot_Z_axis ;
	Rotation_Matrix[1][2] = upz -Rotation_Matrix[2][2]*up_dot_Z_axis ; // Y'轴
	Normalize_Fcn( Rotation_Matrix[1] );  // 获得归一化的Y'轴
	Cross_Matrix(Rotation_Matrix[0],Rotation_Matrix[1],Rotation_Matrix[2]); // 获得X'轴
}

// IP 生产程序
BOOL IP_Image::IP_make_FCN(int xx,int yy,int sizex, int sizey)
{
	// *******************  Store the Matrix after sheer **********************  //
	FIRST_FLAG_OF_IP++;
	if( FIRST_FLAG_OF_IP==1 ) // 如果是第一次进入 则进行一些初始化的操作
	{
		lImageSize = iViewport[2] * iViewport[3] * 3; 
		cout<<"iViewport[2] = "<<iViewport[2]<<endl; // 
		cout<<"iViewport[3] = "<<iViewport[3]<<endl; 
		cout<<"lImageSize = "<<lImageSize<<endl;
		pBits = (unsigned char*)new unsigned char[lImageSize];
		if (!pBits)
		return false;
		system("pause"); // 查看参数 
	}
	
	//****************************************************_***************// 
	// 从color buffer中读取数据
	glPixelStorei(GL_PACK_ALIGNMENT, 1);  // OpenGL版本的IP的读取数据的函数在自己内部所以不受制于Screen_Shot header
	glPixelStorei(GL_PACK_ROW_LENGTH, 0); 
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
	//
	glGetIntegerv(GL_READ_BUFFER, (GLint*)&lastBuffer);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, iViewport[2], iViewport[3], GL_BGR_EXT, GL_UNSIGNED_BYTE, pBits); // GL_UNSIGNED_BYTE 
	glReadBuffer(lastBuffer);
	Pixel_Redistribution( xx, yy); // 
	return 0;
}

void IP_Image::Pixel_Redistribution(int xx,int yy)
{
	float CENTER1[2] ; // 透镜中心
	int   INDEX_i,INDEX_j ;       // DATA中的索引
	int   index_ij[8] ;         // pBits中的索引
	
	for(int i=0;i<LENS_NUMBER_X;i++)
	{
		for(int j=0;j<LENS_NUMBER_Y;j+=2)
		{
			// 第一排
			INDEX_j    = (i+0.0)*LENSPITCH /PIXEL_DX + DIRECTION_TEST[0] * xx ; // x方向 对应于图像中的j 即图像的列
			INDEX_i    = (j+0.0)*LENSPITCHY/PIXEL_DX + DIRECTION_TEST[1] * yy ; // y方向 对应于图像中的i 即图像的行
			if( INDEX_j>=0 && INDEX_i>=0  &&  INDEX_j< WIDTH  && INDEX_i <HEIGHT)
			{
				CENTER1[0] = (i+0.0)*LENSPITCH*SCALE  - WIDTH  * PIXEL_DX/2.0*SCALE  ; // x方向
			    CENTER1[1] = (j+0.0)*LENSPITCHY*SCALE - HEIGHT * PIXEL_DX/2.0*SCALE  ; // y方向

				Cal_Index( CENTER1[0] ,  CENTER1[1], 0.0f );   // 计算在Othographic图中的Index
				Interp_Bilinear_Fcn ();                        // 计算出插值系数  Magnitudes_4_Interp 

				index_ij[0] = subimage_Indexs[0];    // 取整
	            index_ij[1] = subimage_Indexs[1];    // 取整

				index_ij[2] = subimage_Indexs[0];    // 取整
	            index_ij[3] = subimage_Indexs[1]+1;  // 取整

				index_ij[4] = subimage_Indexs[0]+1;  // 取整
	            index_ij[5] = subimage_Indexs[1];    // 取整

				index_ij[6] = subimage_Indexs[0]+1;  // 取整
	            index_ij[7] = subimage_Indexs[1]+1;  // 取整

				if(index_ij[1]>=0 && index_ij[1]< iViewport[2] && index_ij[0]>=0 && index_ij[0]< iViewport[3] )
				{
					DATA[INDEX_i*3*WIDTH + INDEX_j*3 + 0  ] = Magnitudes_4_Interp[0] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + 0 ]   + 
						                                      Magnitudes_4_Interp[1] * (float)pBits[ index_ij[3]*iViewport[2]*3 + index_ij[2]*3 + 0 ]   + 
															  Magnitudes_4_Interp[2] * (float)pBits[ index_ij[5]*iViewport[2]*3 + index_ij[4]*3 + 0 ]   + 
															  Magnitudes_4_Interp[3] * (float)pBits[ index_ij[7]*iViewport[2]*3 + index_ij[6]*3 + 0 ]   ;

					DATA[INDEX_i*3*WIDTH + INDEX_j*3 + 1  ] = Magnitudes_4_Interp[0] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + 1 ]   + 
						                                      Magnitudes_4_Interp[1] * (float)pBits[ index_ij[3]*iViewport[2]*3 + index_ij[2]*3 + 1 ]   + 
															  Magnitudes_4_Interp[2] * (float)pBits[ index_ij[5]*iViewport[2]*3 + index_ij[4]*3 + 1 ]   + 
															  Magnitudes_4_Interp[3] * (float)pBits[ index_ij[7]*iViewport[2]*3 + index_ij[6]*3 + 1 ]   ;

					DATA[INDEX_i*3*WIDTH + INDEX_j*3 + 2  ] = Magnitudes_4_Interp[0] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + 2 ]   + 
						                                      Magnitudes_4_Interp[1] * (float)pBits[ index_ij[3]*iViewport[2]*3 + index_ij[2]*3 + 2 ]   + 
															  Magnitudes_4_Interp[2] * (float)pBits[ index_ij[5]*iViewport[2]*3 + index_ij[4]*3 + 2 ]   + 
															  Magnitudes_4_Interp[3] * (float)pBits[ index_ij[7]*iViewport[2]*3 + index_ij[6]*3 + 2 ]   ;
				}
			}
			// 第二排
			INDEX_j    = (i+0.5)*LENSPITCH /PIXEL_DX + DIRECTION_TEST[0] * xx ; // x方向 对应于图像中的j 即图像的列
			INDEX_i    = (j+1.0)*LENSPITCHY/PIXEL_DX + DIRECTION_TEST[1] * yy ; // y方向 对应于图像中的i 即图像的行
			if( INDEX_j>=0 && INDEX_i>=0  &&  INDEX_j< WIDTH  && INDEX_i <HEIGHT)
			{
				CENTER1[0] = (i+0.5)*LENSPITCH*SCALE  - WIDTH  * PIXEL_DX/2.0*SCALE  ;   // 
			    CENTER1[1] = (j+1.0)*LENSPITCHY*SCALE - HEIGHT * PIXEL_DX/2.0*SCALE  ;   // 
				// 由于Ipad放在xy平面上，而camtarget在z轴上，与Ipad所在面相同，因此z上面填0
				Cal_Index( CENTER1[0] ,  CENTER1[1], 0.0f );  
				Interp_Bilinear_Fcn ();  // 计算出插值系数  Magnitudes_4_Interp 

				index_ij[0] = subimage_Indexs[0];    // 取整
	            index_ij[1] = subimage_Indexs[1];    // 取整

				index_ij[2] = subimage_Indexs[0];    // 取整
	            index_ij[3] = subimage_Indexs[1]+1;  // 取整

				index_ij[4] = subimage_Indexs[0]+1;  // 取整
	            index_ij[5] = subimage_Indexs[1];    // 取整

				index_ij[6] = subimage_Indexs[0]+1;  // 取整
	            index_ij[7] = subimage_Indexs[1]+1;  // 取整

				if(index_ij[1]>=0 && index_ij[1]< iViewport[2] && index_ij[0]>=0 && index_ij[0]< iViewport[3] )
				{
					DATA[INDEX_i*3*WIDTH + INDEX_j*3 + 0  ] = Magnitudes_4_Interp[0] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + 0 ]   + 
						                                      Magnitudes_4_Interp[1] * (float)pBits[ index_ij[3]*iViewport[2]*3 + index_ij[2]*3 + 0 ]   + 
															  Magnitudes_4_Interp[2] * (float)pBits[ index_ij[5]*iViewport[2]*3 + index_ij[4]*3 + 0 ]   + 
															  Magnitudes_4_Interp[3] * (float)pBits[ index_ij[7]*iViewport[2]*3 + index_ij[6]*3 + 0 ]   ;

					DATA[INDEX_i*3*WIDTH + INDEX_j*3 + 1  ] = Magnitudes_4_Interp[0] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + 1 ]   + 
						                                      Magnitudes_4_Interp[1] * (float)pBits[ index_ij[3]*iViewport[2]*3 + index_ij[2]*3 + 1 ]   + 
															  Magnitudes_4_Interp[2] * (float)pBits[ index_ij[5]*iViewport[2]*3 + index_ij[4]*3 + 1 ]   + 
															  Magnitudes_4_Interp[3] * (float)pBits[ index_ij[7]*iViewport[2]*3 + index_ij[6]*3 + 1 ]   ;

					DATA[INDEX_i*3*WIDTH + INDEX_j*3 + 2  ] = Magnitudes_4_Interp[0] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + 2 ]   + 
						                                      Magnitudes_4_Interp[1] * (float)pBits[ index_ij[3]*iViewport[2]*3 + index_ij[2]*3 + 2 ]   + 
															  Magnitudes_4_Interp[2] * (float)pBits[ index_ij[5]*iViewport[2]*3 + index_ij[4]*3 + 2 ]   + 
															  Magnitudes_4_Interp[3] * (float)pBits[ index_ij[7]*iViewport[2]*3 + index_ij[6]*3 + 2 ]   ;
				}

				//if(index_ij[1]>=0 && index_ij[1]< iViewport[2] && index_ij[0]>=0 && index_ij[0]< iViewport[3] )
				//{
				//	DATA[INDEX_i*3*WIDTH + INDEX_j*3 + 0  ] = Magnitudes_4_Interp[0] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + 0 ]   +         // index_ij[1] mean the y direction  //  Modified 15-07-28
				//		                                      Magnitudes_4_Interp[1] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + iViewport[2]*3 ]   + 
				//											  Magnitudes_4_Interp[2] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + 3 ]   + 
				//											  Magnitudes_4_Interp[3] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + iViewport[2]*3 +1*3 ]   ;

				//	DATA[INDEX_i*3*WIDTH + INDEX_j*3 + 1  ] = Magnitudes_4_Interp[0] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + 0     + 1  ]   + 
				//		                                      Magnitudes_4_Interp[1] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + iViewport[2]*3 + 1  ]   + 
				//											  Magnitudes_4_Interp[2] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + 3     + 1  ]   + 
				//											  Magnitudes_4_Interp[3] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + iViewport[2]*3 +1*3 + 1  ]   ;

				//	DATA[INDEX_i*3*WIDTH + INDEX_j*3 + 2  ] = Magnitudes_4_Interp[0] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + 0     + 2  ]   + 
				//		                                      Magnitudes_4_Interp[1] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + iViewport[2]*3 + 2  ]   + 
				//											  Magnitudes_4_Interp[2] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + 3     + 2  ]   + 
				//											  Magnitudes_4_Interp[3] * (float)pBits[ index_ij[1]*iViewport[2]*3 + index_ij[0]*3 + iViewport[2]*3 +1*3 + 2  ]   ;

				//}
			}
		}
	}
}


void IP_Image:: Cal_Index(float x, float y, float z)
{
	// ************************  最终正确的版本 ************************ //
	subimage_Indexs[0] =  -(Rotation_Matrix[0][0]*x + Rotation_Matrix[0][1]*y+ Rotation_Matrix[0][2]*z) + (iViewport[2]-1)/2.0; // Modified 15-07-28
	subimage_Indexs[1] =  +(Rotation_Matrix[1][0]*x + Rotation_Matrix[1][1]*y+ Rotation_Matrix[1][2]*z) + (iViewport[3]-1)/2.0; // Modified 15-07-28
}


void IP_Image::Interp_Bilinear_Fcn ()  // 
{
	float Sum = 0;
	float x_1 = (subimage_Indexs[0]-int(subimage_Indexs[0]+0 ));  // 
	float x_2 = (subimage_Indexs[0]-int(subimage_Indexs[0]+1 ));  //

	float y_1 = (subimage_Indexs[1]-int(subimage_Indexs[1]+0 ));  // 
	float y_2 = (subimage_Indexs[1]-int(subimage_Indexs[1]+1 ));  // 

	Magnitudes_4_Interp[0] = 1/( x_1*x_1+(y_1*y_1) + EPS ) ;
	Magnitudes_4_Interp[1] = 1/( x_1*x_1+(y_2*y_2))+ EPS ;
	Magnitudes_4_Interp[2] = 1/( x_2*x_2+(y_1*y_1) + EPS) ;
	Magnitudes_4_Interp[3] = 1/( x_2*x_2+(y_2*y_2) + EPS) ;

	Sum  = Sum + Magnitudes_4_Interp[0] ;
	Sum  = Sum + Magnitudes_4_Interp[1] ;
	Sum  = Sum + Magnitudes_4_Interp[2] ;
	Sum  = Sum + Magnitudes_4_Interp[3] ;

	Magnitudes_4_Interp[0] = Magnitudes_4_Interp[0] / Sum;
	Magnitudes_4_Interp[1] = Magnitudes_4_Interp[1] / Sum;
	Magnitudes_4_Interp[2] = Magnitudes_4_Interp[2] / Sum;
	Magnitudes_4_Interp[3] = Magnitudes_4_Interp[3] / Sum;
}


void IP_Image::SET_IMAGE_SIZE (int x_init, int y_init, int x_width, int y_height) 
{
	iViewport[0] = x_init ;  // 暂定 x_init 是横向起始位置 // Modified 15-07-28
	iViewport[1] = y_init ;  // 暂定 y_init 是纵向起始位置 // Modified 15-07-28
	iViewport[2] = x_width ; // 暂定 x_width 是横向宽度    // Modified 15-07-28
	iViewport[3] = y_height; // 暂定 y_height是纵向高度    // Modified 15-07-28
	RATIO        = 1; // 设定物体尺寸等于IPad的几分之几
	SCALE        = (0.5/0.7*iViewport[3])/(PIXEL_DX*HEIGHT*RATIO ) ;  // Modified 15-07-28 Vital important
}

// Height_Map.cpp : 定义控制台应用程序的入口点。
#include "stdafx.h"

#include "IP_header.h"
#include "Screen_Shot.h"
#include "LoadObj.h"

#define SQRT3 1.732050807568877
 // 大量代码折叠里面
IP_Image *ip_image = new IP_Image();
int FIRST_TIME_FLAG = 0 ;
#pragma region Rotation_Control  
TriangleMesh mesh;
static GLfloat spin_x=0.0, spin_y=0.0 ;
int    Latest_State = 1 ;
int    Last_xy[2],Now_xy[2] ;
double ANGLE_SUB = 20;
GLuint  texture[1]  ;

float Move_Z= 0.0 ;
float Rotation_INdex = 0;

GLfloat Transform_Matrix[16]={1.0f, 0.0f,  0.0f,  0.0f,				// NEW: Final Transform
                              0.0f, 1.0f,  0.0f,  0.0f,
                              0.0f,  0.0f,  1.0f,  0.0f,
                              0.0f,  0.0f,  0.0f,  1.0f } ;
GLfloat *Now_Matrix = (GLfloat *) calloc(9,sizeof(GLfloat));

double rotxyz[3] ;  
// glMatrix g;  glMultMatrixf(Transform_Matrix);	
//变量定义 
GLdouble Orth_Window=1.0;
void myDisplay();
void IP_MAKE_Display();
//double 
void Refresh_4f_Transform_Matrix(GLfloat* Transform_Matrix, GLfloat* RotationMatrix)
{
	GLfloat Result[9];
	for (int i=0;i<3;i++)
	{
		for (int j=0;j<3;j++)
	    {
		   Result[i*3+j] = Transform_Matrix[i*4+j];
		}
	}
	for (int i=0;i<3;i++)
	{
		for (int j=0;j<3;j++)
	    {
			Transform_Matrix[i*4+j] = 0 ;
			for (int k=0;k<3;k++){
		        Transform_Matrix[i*4+j] =  Transform_Matrix[i*4+j] + Result[i*3+k] * RotationMatrix[k*3+j];
			}
		}
	}
}
//  
void Cross_Matrix( GLfloat*Matrix ,GLfloat*Matrix1,GLfloat*Matrix2)
{
	Matrix[0] = 1 * ( Matrix1[1]*Matrix2[2]-Matrix1[2]*Matrix2[1] );
	Matrix[1] = 1 * ( Matrix1[0]*Matrix2[2]-Matrix1[2]*Matrix2[0] );
	Matrix[2] = 1 * ( Matrix1[1]*Matrix2[0]-Matrix1[0]*Matrix2[1] );
	double sum = sqrt(  Matrix[0] *Matrix[0]  +  Matrix[1] *Matrix[1] +  Matrix[2] *Matrix[2] );
	Matrix[1]  = Matrix[1] /sum;
	Matrix[0]  = Matrix[0] /sum;
	Matrix[2]  = Matrix[2] /sum;
}

void Roderigues(GLfloat* RotationMatrix,GLfloat *NowRotation_Matrix , double* rotxyz)
{
	double TMP[3] ;

	TMP[0] = rotxyz[0];
	TMP[1] = rotxyz[1];
	TMP[2] = rotxyz[2];
	if(TMP[0]*TMP[0] + TMP[1]*TMP[1] + TMP[2]*TMP[2] > 0 ) 
	{
		GLfloat theta= sqrt( rotxyz[0]*rotxyz[0] + rotxyz[1]*rotxyz[1] + rotxyz[2]*rotxyz[2] );
		rotxyz[0] = rotxyz[0] / theta ;
		rotxyz[1] = rotxyz[1] / theta ;
		rotxyz[2] = rotxyz[2] / theta ;
		// Matlab Version
		//rotation=cos(theta)*eye(3)+(1-cos(theta))*r*r'+[0 -r(3) r(2); r(3) 0  -r(1); -r(2) r(1) 0]*sin(theta);
		RotationMatrix[0] = cos(theta)+ (1-cos(theta))*rotxyz[0]*rotxyz[0] + 0        *sin(theta);
		RotationMatrix[1] = 0         + (1-cos(theta))*rotxyz[0]*rotxyz[1] - rotxyz[2]*sin(theta);
		RotationMatrix[2] = 0         + (1-cos(theta))*rotxyz[0]*rotxyz[2] + rotxyz[1]*sin(theta);

		RotationMatrix[3] = 0         + (1-cos(theta))*rotxyz[1]*rotxyz[0] + rotxyz[2]*sin(theta);
		RotationMatrix[4] = cos(theta)+ (1-cos(theta))*rotxyz[1]*rotxyz[1] + 0        *sin(theta);
		RotationMatrix[5] = 0         + (1-cos(theta))*rotxyz[1]*rotxyz[2] - rotxyz[0]*sin(theta);

		RotationMatrix[6] = 0         + (1-cos(theta))*rotxyz[2]*rotxyz[0] - rotxyz[1]*sin(theta);
		RotationMatrix[7] = 0         + (1-cos(theta))*rotxyz[2]*rotxyz[1] + rotxyz[0]*sin(theta);
		RotationMatrix[8] = cos(theta)+ (1-cos(theta))*rotxyz[2]*rotxyz[2] + 0        *sin(theta);
	}
	else 
	{
		RotationMatrix[0] = 1.0;
		RotationMatrix[1] = 0;
		RotationMatrix[2] = 0;

		RotationMatrix[3] = 0;
		RotationMatrix[4] = 1.0;
		RotationMatrix[5] = 0;

		RotationMatrix[6] = 0;
		RotationMatrix[7] = 0;
		RotationMatrix[8] = 1.0;
	}
}

// Load Bitmaps And Convert To Textures
GLvoid LoadGLTextures()
{
	// Load Texture
	AUX_RGBImageRec *texture1;
	texture1 = auxDIBImageLoad(L"Data/yb_10k.bmp");
	if (!texture1) exit(1);
	// Create Texture
	glGenTextures(1, &texture[0]);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, texture1->sizeX, texture1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, texture1->data);
};


void InitGL ( GLvoid )     // Create Some Everyday Functions
{
	LoadGLTextures();                                    // 读入纹理 read texture
	glShadeModel(GL_SMOOTH);						 	 // Enable Smooth Shading
	glEnable(GL_DEPTH_TEST);						 	 // Enables Depth Testing
	glEnable (GL_COLOR_MATERIAL );                       // 跟踪颜色
	glEnable(GL_TEXTURE_2D);                             // 启动2D纹理映射
    GLfloat ambient[] = { 0.5, 0.5, 0.5, 1.0 };          
    GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat position[] = { 0.0, 2.0, 10.0, 11.0 };
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);  // Setup The Ambient Light设置环境光
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);  // Setup The Diffuse Light设置漫射光
	glLightfv(GL_LIGHT1, GL_POSITION,position);  // Position The Light设置光源位置
	glEnable(GL_LIGHTING);     // 启动光照
    glEnable(GL_LIGHT1);     // Enable Light One启用一号光源
	GLfloat pos[4] = { -2.8, 5., 1.8, 1.};
	glLightfv (GL_LIGHT1, GL_POSITION, pos);
	glEnable (GL_LINE_SMOOTH);
    glHint (GL_LINE_SMOOTH, GL_NICEST);
	glEnable (GL_POLYGON_SMOOTH);
	glHint (GL_POLYGON_SMOOTH,GL_NICEST ); 
	glEnable (GL_POINT_SMOOTH);
	glHint (GL_POINT_SMOOTH,GL_NICEST ); 
}

void spinDisplay(void)
{
	 spin_x =  -(Now_xy[0]- Last_xy[0])/400.0f*4; 
	 spin_y =  -(Now_xy[1]- Last_xy[1])/400.0f*4;
	 rotxyz[0] =   spin_y;
	 rotxyz[1] =   spin_x;
	 rotxyz[2] =   0 ;
	 
	 Last_xy[0] = Now_xy[0];
	 Last_xy[1] = Now_xy[1];   //cout<<"spin_x =　"<<spin_x << "   spin_y ="<<spin_y<<endl;
	 Roderigues(Now_Matrix , Transform_Matrix , rotxyz);          // Transform_Matrix包含前一个时刻的旋转矩阵，Now_Matrix是增长部分
	 Refresh_4f_Transform_Matrix( Transform_Matrix, Now_Matrix);  // glutPostRedisplay(); // 5.31 chenguowen
	 myDisplay();
}

//  
void  Reshape(GLsizei width,GLsizei height)
{
    GLfloat w_aspect = (double) width / height;
	int w_width = width;
	int w_height = height;
	ip_image->SET_IMAGE_SIZE(0,0,w_width,w_height); // 
	cout<<"********************************** Size of Window by glGetIntegerv ******************"<<endl;

	cout<<"WIDTH of Window="<<ip_image->iViewport[2]<<"\tHeight of Window = "<<ip_image->iViewport[3]<<endl;
	int a[4];                      // 
	glGetIntegerv(GL_VIEWPORT,a);  // 
	cout<<"WIDTH  of Window="<<a[2]<<"\tHeight of Window = "<<a[3]<<endl;
	cout<<"Origin of Window="<<a[0]<<"\tOrigin of Window = "<<a[1]<<endl;

	if (w_aspect>1)  // 原因在此 我把ViewPort进行了一些处理 （正方形化）
	{
		glViewport( (w_width-w_height)/2, 0, w_width-(w_width-w_height), w_height);
	}
	else
	{
		glViewport( 0,-(w_width-w_height)/2,  w_width, w_height+(w_width-w_height));
	}
}


void mouse(int botton,int state,int x,int y)
{
 switch(botton)
 {
 case GLUT_LEFT_BUTTON:   //当单击鼠标左键时开始旋转
   if(state==GLUT_DOWN )
   { 
	   Latest_State =1; 
	   Last_xy[0] = x;
	   Last_xy[1] = y;
   }
   if(state==GLUT_UP && Latest_State==1 )
   {
	   Now_xy[0]= x; 
	   Now_xy[1]= y;
	   Latest_State = 0;
	   spinDisplay();
   }
   break;

 case GLUT_MIDDLE_BUTTON:  
   if(state==GLUT_DOWN)
    glutIdleFunc(0);
   break;
 default:
  break;

 }
}
void Mouse_motion(int x,int y )
{
	if(Latest_State==1 )
   { 
	   Now_xy[0]= x; 
	   Now_xy[1]= y;
	   spinDisplay();
   }
}
// te
void SpecicalKey(int key, int x, int y) 
{  
   switch (key) {  
	case GLUT_KEY_DOWN:
		Move_Z = Move_Z-0.3;
		glutPostRedisplay(); 
		break;
	case GLUT_KEY_UP:  
		 Move_Z = Move_Z+0.3;
	     cout<<Move_Z <<endl;
	     glutPostRedisplay();       
         break;                     // 失败的尝试是   // GLUT_KEY_DOWNVK_UP 
  case 'z':
  glutIdleFunc(NULL);
   break; 
      case 27:  
         exit(0);  
         break;  
   }  
}  

//  
void keyboard(unsigned char key, int x, int y)  
{  
   switch (key) {  
      case 'w':  
		 Move_Z = Move_Z+0.3;
	     //cout<<Move_Z <<endl;
	     glutPostRedisplay(); 
         break;  
     case 's':  
		 Move_Z = Move_Z-0.3;
		 glutPostRedisplay(); 
         break;  
	 case'd':
		 Rotation_INdex = Rotation_INdex - 3;
	     cout<<Rotation_INdex <<endl;
	     myDisplay();
         break;  
	 case'a':
		 Rotation_INdex = Rotation_INdex + 3;
		 glutPostRedisplay(); 
         break;  

	 case 'i':
		 IP_MAKE_Display();
		 break;

	case'b':  //
		cin.getline(filename_BMP,30);
		screenshot(filename_BMP);
        break;  
	/*case GLUT_KEY_DOWN:
		Move_Z = Move_Z-3;
		glutPostRedisplay(); 
		break;
	case GLUT_KEY_F6:  
		 Move_Z = Move_Z+3;
	     cout<<Move_Z <<endl;
	     glutPostRedisplay();         // GLUT_KEY_DOWNVK_UP 
         break;   */                  // 失败的尝试是 因为位置不对 应放在SpecialFunc里面
  case 'z':
  glutIdleFunc(NULL);
   break; 
      case 27:  
         exit(0);  
         break;  
   }  
}  


GLvoid draw_Scene()
{
    glBindTexture(GL_TEXTURE_2D, texture[0]);      // 选择纹理
	for (int i=0;i<mesh.faces.size() ;i++)
	{
		glBegin(GL_TRIANGLES);
		int a = mesh.faces[i].v[0]-1,b = mesh.faces[i].v[1]-1, c=mesh.faces[i].v[2]-1;
		int D = mesh.faces[i].Define_index[0]-1,E = mesh.faces[i].Define_index[1]-1, F=mesh.faces[i].Define_index[2]-1;
		glTexCoord2f(mesh.Texture_2D[D].x , mesh.Texture_2D[D].y  );  
		glVertex3f(mesh.verts[a].x , mesh.verts[a].y ,mesh.verts[a].z);   // 
		glTexCoord2f(mesh.Texture_2D[E].x , mesh.Texture_2D[E].y  );  
		glVertex3f(mesh.verts[b].x, mesh.verts[b].y ,mesh.verts[b].z);
		glTexCoord2f(mesh.Texture_2D[F].x , mesh.Texture_2D[F].y  );  
		glVertex3f(mesh.verts[c].x, mesh.verts[c].y ,mesh.verts[c].z);
		glEnd();
	}
}
#pragma endregion 

// 制作IP的显示函数
void IP_MAKE_Display() // 
{
	//ip_image->IP_Parameter_Show(); //参数展示
	//ip_image->IPwriteBMP();
	cout<<"请输入飞出距离"<<endl; 
	cin>>ip_image->FLYOUT_DISTANCE ; //
	double DISTANCE = 10 ;
	GLfloat Window_size_ortho = ip_image->WINDOW_SIZESET;         // 窗口大小orthographic

	for( int xx= -ip_image->RADIUS_X ; xx <ip_image->RADIUS_X; xx++)
	{
		for (int yy=-ip_image->RADIUS_Y ; yy <ip_image->RADIUS_Y; yy++)
		{
			if( (xx+0.5)/SQRT3 + (yy+0.5) <= +ip_image->RADIUS_Y + 0.5  &&    -(xx+0.5)/SQRT3+(yy+0.5) <= + ip_image->RADIUS_Y + 0.5 && 
				(xx+0.5)/SQRT3 + (yy+0.5) >= -ip_image->RADIUS_Y - 0.5  &&    -(xx+0.5)/SQRT3+(yy+0.5) >= - ip_image->RADIUS_Y - 0.5 )
			{   // 清理深度缓存 和 颜色缓存
				glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
				glMatrixMode(GL_PROJECTION);              // 调整相机声明
				glLoadIdentity();                         // 回复原位
				// 采用正投影相机
				glOrtho(-Window_size_ortho,Window_size_ortho,-Window_size_ortho,Window_size_ortho,1.0f,120.0f); 
				// 核心部分在此处,计算相机位置
				GLdouble  eyex=xx*DISTANCE*ip_image->PIXEL_DX/ip_image->LENSTHICK;  // 
				GLdouble  eyey=yy*DISTANCE*ip_image->PIXEL_DX/ip_image->LENSTHICK;  //   
				GLdouble  eyez=0, centerx=0, centery =0, centerz = - DISTANCE  , upx = 0, upy =1, upz= 0;
				// 核心部分在此处,设置相机位置
				gluLookAt( eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
				ip_image->Rotation_Matrix_Update( eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz); // 根据摄像机位置和目标位置计算旋转矩阵
				glMatrixMode(GL_MODELVIEW);                         //  对模型在世界坐标系中的位置进行设置
				glLoadIdentity();									//  Reset The Current Modelview Matrix 首先设置为重合
				glTranslatef(0.0f,.0f,ip_image->FLYOUT_DISTANCE -DISTANCE);             //  在Z轴方向上往负向移动10f
				glPushMatrix();                                     //  将当前的model矩阵矩阵压入堆栈 
				glMultMatrixf(Transform_Matrix);	                //  堆栈顶端的矩阵右乘上旋转矩阵Transform_Matrix 
			
				draw_Scene();                                        // 绘制小孩的程序

				glPopMatrix();                                      // 将模型矩阵出栈 
				glutSwapBuffers();
				cout<<"xx="<<xx<<"   yy="<<yy<<endl; 
				ip_image->IP_make_FCN(xx,yy,ip_image->iViewport[2],ip_image->iViewport[3]);  
			}
		}
	}
	ip_image->IPwriteBMP();
}
// 正常显示函数
void myDisplay()
{
	if(FIRST_TIME_FLAG==0)  {	ip_image->IP_Parameter_Show();}   //参数展示
	FIRST_TIME_FLAG++;
	double DISTANCE =5;
	// 清理深度缓存 和 颜色缓存
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
	glMatrixMode(GL_PROJECTION);              // 调整相机声明
	glLoadIdentity();                         // 回复原位
	GLfloat Window_size_ortho = 0.2f;         // 窗口大小orthographic
	// 采用正投影相机
	glFrustum(-Window_size_ortho,Window_size_ortho,-Window_size_ortho,Window_size_ortho,2.0f,120.0f); 
	// 核心部分在此处,计算相机位置
	GLdouble  eyex=0 ;  // 
	GLdouble  eyey=0 ;  //   
	GLdouble  eyez=0, centerx=0, centery =0, centerz = - DISTANCE  , upx = 0, upy =1, upz= 0;
	// 核心部分在此处,设置相机位置
	gluLookAt( eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
			
	glMatrixMode(GL_MODELVIEW);                         //  对模型在世界坐标系中的位置进行设置
	glLoadIdentity();									//  Reset The Current Modelview Matrix 首先设置为重合
	glTranslatef(0.0f,.0f,Move_Z-DISTANCE);             //  在Z轴方向上往负向移动10f
	glPushMatrix();                                     //  将当前的model矩阵矩阵压入堆栈 
	glMultMatrixf(Transform_Matrix);	                //  堆栈顶端的矩阵右乘上旋转矩阵Transform_Matrix 
	draw_Scene();                                        // 绘制小孩的程序
	glPopMatrix();                                      // 将模型矩阵出栈 
	glutSwapBuffers();

}
  
int _tmain(int argc, char** argv)
{
	 AUX_RGBImageRec *texture1;
	 std::string filename = "Data\\yb_10k.obj";    // 数据量较小 Data\\spongebob_bind.obj
	 loadObj(filename ,mesh);

	 cout<<ip_image->HEIGHT<<endl;
	 ip_image->SET_IMAGE_SIZE(100,100,100,100) ;

	 glutInit(&argc, argv);
     glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE); //GLUT_SINGLE
	 glutInitWindowPosition(ip_image->iViewport[0], ip_image->iViewport[1]);   // All control by IP_Image class
     glutInitWindowSize    (ip_image->iViewport[2], ip_image->iViewport[3]);   // All control by IP_Image class
     glutCreateWindow("第一个OpenGL程序");
	 InitGL();
     glutDisplayFunc(&myDisplay);
	 glutMotionFunc(Mouse_motion);  // 如果鼠标在移动中 则调用
	 glutMouseFunc(mouse);          // 如果鼠标抬起放下 则调用
	 glutReshapeFunc(Reshape);
     glutKeyboardFunc(keyboard);    // Press specific key 则调用
	 glutSpecialFunc(SpecicalKey);  // 特殊按键
     glutMainLoop();
	 return 0;
}
///////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//
//-----------H----H--X----X-----CCCCC----22222----0000-----0000------11----------//
//----------H----H----X-X-----C--------------2---0----0---0----0--1--1-----------//
//---------HHHHHH-----X------C----------22222---0----0---0----0-----1------------//
//--------H----H----X--X----C----------2-------0----0---0----0-----1-------------//
//-------H----H---X-----X---CCCCC-----222222----0000-----0000----1111------------//
//-------------------------------------------------------------------------------//
//----------------------------------------------------- http://hxc2001.free.fr --//
///////////////////////////////////////////////////////////////////////////////////
//
// <<The Minitel Demo>> A demo for the French Minitel terminal.
//
// Written by: Jean François DEL NERO
//
// You are free to do what you want with this code.
// A credit is always appreciated if you include it into your prod :)
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////

typedef struct dot_
{
	char x;
	char y;
	char z;
}dot;

typedef struct polyg_
{
	dot point[3];
	char vectricemask;

}polyg;

typedef  struct _data3dtype
{
   unsigned char nbfaces;
   unsigned char nbvertex;
   char * vertex;
  unsigned char * faces;
}data3dtype;

typedef  struct _d3dtype
{
   unsigned char nbfaces;
   char * vertex;
}d3dtype;

void Show3DPoint(dot * point,dot2d * point2D);
void rotateX(dot * point,unsigned char xang);
void rotateY(dot * point,unsigned char yang);
void rotateZ(dot * point,unsigned char zang);
void drawpolygone(char * polygone,short xrotate,short yrotate,short zrotate,char state);
void drawobject(d3dtype * obj,short xrotate,short yrotate,short zrotate,char state)     __reentrant;

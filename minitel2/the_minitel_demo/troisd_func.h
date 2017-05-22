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
	int8_t x;
	int8_t y;
	int8_t z;
}dot;

typedef struct polyg_
{
	dot point[3];
	int8_t vectricemask;

}polyg;

typedef  struct _data3dtype
{
   uint8_t nbfaces;
   uint8_t nbvertex;
   int8_t * vertex;
   uint8_t * faces;
}data3dtype;

typedef  struct _d3dtype
{
   uint8_t nbfaces;
   int8_t * vertex;
}d3dtype;

void Show3DPoint(dot * point,dot2d * point2D);
void rotateX(dot * point,int8_t xang);
void rotateY(dot * point,int8_t yang);
void rotateZ(dot * point,int8_t zang);
void drawpolygone(int8_t * polygone,int16_t xrotate,int16_t yrotate,int16_t zrotate,int8_t state);
void drawobject(d3dtype * obj,int16_t xrotate,int16_t yrotate,int16_t zrotate,int8_t state)     __reentrant;

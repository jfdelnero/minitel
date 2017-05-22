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

typedef struct dot2d_
{
	int8_t x;
	int8_t y;
}dot2d;

void setpixel(uint8_t x,uint8_t y, uint8_t state);
void setpixelFast(uint8_t x,uint8_t y);
void Ligne(dot2d * pointA,dot2d * pointB,int8_t state)  __reentrant;
void LigneFast(dot2d * pointA,dot2d * pointB)  __reentrant;
void cercle(int16_t rayon,int16_t x_centre,int16_t y_centre,uint8_t state)  __reentrant;

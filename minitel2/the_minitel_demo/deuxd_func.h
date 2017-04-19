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
	char x;
	char y;
}dot2d;

void setpixel(unsigned char x,unsigned char y, unsigned char state);
void setpixelFast(unsigned char x,unsigned char y);
void Ligne(dot2d * pointA,dot2d * pointB,char state)  __reentrant;
void LigneFast(dot2d * pointA,dot2d * pointB)  __reentrant;
void cercle(short rayon,short x_centre,short y_centre,unsigned char state)  __reentrant;

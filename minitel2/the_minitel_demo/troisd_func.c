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

#include <stdint.h>
#include "deuxd_func.h"
#include "troisd_func.h"
#include "cossintab.h"


const int16_t	Xoff = 64;
const int16_t	Yoff = 64;
const int16_t	Zoff = 128;

//Proc ; 3D -> 2D

void Show3DPoint(dot * point,dot2d * point2D)
{
	point2D->x=(point->x<<6) / (point->z+Zoff);
	point2D->y=(point->y<<6) / (point->z+Zoff);

	point2D->x=point2D->x+(80/2);
	point2D->y=point2D->y+(100/2);
}


void rotateX(dot * point,int8_t xang)
{
// Rotate around x-axis
// YT = Y * COS(xang) - Z * SIN(xang) / 256
// ZT = Y * SIN(xang) + Z * COS(xang) / 256
// Y = YT
// Z = ZT
	int16_t yt;
	int16_t zt;
	int8_t cosv;
	int8_t sinv;

	cosv=SinCos[xang];
	xang=xang+64;
	sinv=SinCos[xang];

	yt=point->y * cosv;
	yt=yt - (point->z * sinv);
	yt=yt>>7;

	zt=point->y * sinv;
	zt=zt + (point->z * cosv);
	zt=zt>>7;

	point->y=yt;
	point->z=zt;

}



void rotateY(dot * point,int8_t yang)
{
// Rotate around y-axis
// XT = X * COS(yang) - Z * SIN(yang) / 256
// ZT = X * SIN(yang) + Z * COS(yang) / 256
// X = XT
// Z = ZT
	int16_t xt;
	int16_t zt;
	int8_t cosv;
	int8_t sinv;

	cosv=SinCos[yang];
	yang=yang+64;
	sinv=SinCos[yang];

	xt=point->x * cosv;
	xt=xt - (point->z * sinv);
	xt=xt>>7;

	zt=point->x * sinv;
	zt=zt + (point->z * cosv);
	zt=zt>>7;

	point->x=xt;
	point->z=zt;

}

void rotateZ(dot * point,int8_t zang)
{
// Rotate around z-axis
// XT = X * COS(zang) - Y * SIN(zang) / 256
// YT = X * SIN(zang) + Y * COS(zang) / 256
// X = XT
// Y = YT
	int16_t xt;
	int16_t yt;
	int8_t cosv;
	int8_t sinv;

	cosv=SinCos[zang];
	zang=zang+64;
	sinv=SinCos[zang];

	xt=point->x * cosv;
	xt=xt - (point->y * sinv);
	xt=xt>>7;

	yt=point->x * sinv;
	yt=yt + (point->y * cosv);
	yt=yt>>7;

	point->x=xt;
	point->y=yt;

}

void drawpolygone(int8_t * polygone,int16_t xrotate,int16_t yrotate,int16_t zrotate,int8_t state)  /* __reentrant		  */
{
	int8_t i,c;

	dot temppoint[3];
	dot2d points2d[3];

	for(i=0;i<3;i++)
	{

		c=0;
		switch(i)
		{
			case 0:
				if( polygone[3] || polygone[11])
				{
					c=1;
				}
			break;

			case 1:
				if( polygone[3] || polygone[7])
				{
					c=1;
				}
			break;

			case 2:
				if( polygone[7] || polygone[11])
				{
					c=1;
				}
			break;
		}

		if(c)
		{
			temppoint[i].x=polygone[(i<<2)+0];//polygone->point[0].x;
			temppoint[i].y=polygone[(i<<2)+1];//polygone->point[0].y;
			temppoint[i].z=polygone[(i<<2)+2];//polygone->point[0].z;

			//rotat
			rotateX(&temppoint[i],xrotate);
			rotateY(&temppoint[i],yrotate);
			rotateZ(&temppoint[i],zrotate);

			Show3DPoint(&temppoint[i],&points2d[i]);
		}
	}

	if(polygone[3])
	{
		LigneFast(&points2d[0],&points2d[1]);
		//setpixelFast(points2d[0].x,points2d[0].y);
		//setpixelFast(points2d[1].x,points2d[1].y);
	}

	if(polygone[7])
	{
		LigneFast(&points2d[1],&points2d[2]);
		//setpixelFast(points2d[2].x,points2d[2].y);
		//setpixelFast(points2d[1].x,points2d[1].y);
	}

	if(polygone[11])
	{
		LigneFast(&points2d[2],&points2d[0]);
		//setpixelFast(points2d[0].x,points2d[0].y);
		// setpixelFast(points2d[2].x,points2d[2].y);
	}
}

void drawobject(d3dtype * obj,int16_t xrotate,int16_t yrotate,int16_t zrotate,int8_t state)     __reentrant
{
	uint8_t i;
	for(i=0;i<obj->nbfaces;i++)
	{
		drawpolygone(&obj->vertex[i<<4],xrotate,yrotate,zrotate,state);
	}
}

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

#include <8052.h>
#include "deuxd_func.h"
#include "minitel_reg.h"
#include "minitel_hw.h"

extern volatile unsigned char buffernb;


const unsigned char div10tab[]=
{
	0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,
	3,3,3,3,3,3,3,3,3,3,
	4,4,4,4,4,4,4,4,4,4,
	5,5,5,5,5,5,5,5,5,5,
	6,6,6,6,6,6,6,6,6,6,
	7,7,7,7,7,7,7,7,7,7,
	8,8,8,8,8,8,8,8,8,8,
	9,9,9,9,9,9,9,9,9,9,
	10,10,10,10,10,10,10,10,10,10,
	11,11,11,11,11,11,11,11,11,11,
	12,12,12,12,12,12,12,12,12,12,
	13,13,13,13,13,13,13,13,13,13,
	14,14,14,14,14,14,14,14,14,14,
	15,15,15,15,15,15,15,15,15,15,
	16,16,16,16,16,16,16,16,16,16,
	17,17,17,17,17,17,17,17,17,17,
	18,18,18,18,18,18,18,18,18,18,
	19,19,19,19,19,19,19,19,19,19,
	20,20,20,20,20,20,20,20,20,20,
	21,21,21,21,21,21,21,21,21,21,
	22,22,22,22,22,22,22,22,22,22,
	23,23,23,23,23,23,23,23,23,23,
	24,24,24,24,24,24,24,24,24,24,
	25,25,25,25,25,25,25,25,25,25
};

const unsigned char multentab[]=
{
	0,10,20,30,40,50,60,70,80,90,100,110,120
};


void setpixel(unsigned char x,unsigned char y, unsigned char state)
{
	unsigned char i,j,c,k,d;

	if(x<80 && y<100)
	{
		i=div10tab[y];//y/10;
		k=y-multentab[i];

		j=x>>3;

		d=0x01<<(x&0x7);

		c=j+multentab[i];

		if(c>3) c=32+(c-4);

		WAIT_TS9347;

		TS9347_R6= (0x20)|(c>>2);
		TS9347_R7= buffernb | (k<<2)|(c&0x3);
		TS9347_ER0=CMD_TBM| 0x08;

		WAIT_TS9347;

		if(state)
			TS9347_R1=  d | TS9347_R1;
		else
			TS9347_R1= (~d) & TS9347_R1;

		TS9347_ER0=CMD_TBM;
	}
}


void setpixelFast(unsigned char x,unsigned char y)
{
	unsigned char i,c,k;

	i=div10tab[y];//y/10;
	k=y-multentab[i];

	c=(x>>3)+multentab[i];

	if(c&0xFC) c=c+28;

	WAIT_TS9347;

	TS9347_R6= (0x20)|(c>>2);
	TS9347_R7= buffernb | (k<<2)|(c&0x3);
	TS9347_ER0=CMD_TBM| 0x08;

	WAIT_TS9347;
	TS9347_R1=  (0x01<<(x&0x7)) | TS9347_R1;

	TS9347_ER0=CMD_TBM;
}


dot2d points2d[3];

void Ligne(dot2d * pointA,dot2d * pointB,char state) __reentrant
{
	char first_x,last_x;
	char first_y,last_y;
	char dx;
	char dy;
	char inc1,inc2;
	char sub,remain,error;
	char x1,x2,y1,y2;

	x1=  pointA->x;
	x2=  pointB->x;
	y1=  pointA->y;
	y2=  pointB->y;

    if(x1<x2)
	{
		first_x=x1;
		last_x=x2;
		first_y=y1;
		last_y=y2;
	}
	else
	{
		first_x=x2;
		last_x=x1;
		first_y=y2;
		last_y=y1;
	}

	dx=last_x-first_x;
	dy=last_y-first_y;
	if ((!dx)&&(!dy))
	{
		return; // rien a tracer
	}


	if (dy<0)
	{
		dy=-dy;
		inc1=-1;
		inc2=1;
	}
	else
	{
		inc1=1;
		inc2=1;
	}


	if (dx>dy)
	{
		sub=dx-dy;
		error=dy-(dx>>1);
		remain=(dx+1)>>1;

		do
		{
			setpixel(first_x,first_y,state);
			setpixel(last_x,last_y,state);
			first_x+=inc2;
			last_x-=inc2;
			if (error>=0)
			{
				first_y+=inc1;
				last_y-=inc1;
				error-=sub;
			}
			else error+=dy;
		} while (--remain>0);

		if (!(dx&1)) setpixel(first_x,first_y,state);

		return;
	}
	else
	{
		sub=dy-dx;
		error=dx-(dy>>1);
		remain=(dy+1)>>1;

		do
		{
			setpixel(first_x,first_y,state);
			setpixel(last_x,last_y,state);
			first_y+=inc1;
			last_y-=inc1;

			if (error>=0)
			{
				first_x+=inc2;
				last_x-=inc2;
				error-=sub;
			}
			else error+=dx;

		} while (--remain>0);
		if (!(dy&1)) setpixel(first_x,first_y,state);

		return;
	}

}

void LigneFast(dot2d * pointA,dot2d * pointB) __reentrant
{
	char first_x,last_x;
	char first_y,last_y;
	char dx;
	char dy;
	char inc1,inc2;
	char sub,remain,error;
	char x1,x2,y1,y2;

	x1=  pointA->x;
	x2=  pointB->x;
	y1=  pointA->y;
	y2=  pointB->y;

    if(x1<x2)
	{
		first_x=x1;
		last_x=x2;
		first_y=y1;
		last_y=y2;
	}
	else
	{
		first_x=x2;
		last_x=x1;
		first_y=y2;
		last_y=y1;
	}

	dx=last_x-first_x;
	dy=last_y-first_y;
	if ((!dx)&&(!dy))
	{
		return; // rien a tracer
	}


	if (dy<0)
	{
		dy=-dy;
		inc1=-1;
		inc2=1;
	}
	else
	{
		inc1=1;
		inc2=1;
	}


	if (dx>dy)
	{
		sub=dx-dy;
		error=dy-(dx>>1);
		remain=(dx+1)>>1;

		if( inc1>0)
		{
			do
			{
				setpixelFast(first_x,first_y);
				setpixelFast(last_x,last_y);
				first_x++;
				last_x--;
				if (error>=0)
				{
					first_y++;
					last_y--;
					error-=sub;
				}
				else error+=dy;
			} while (--remain>0);
		}
		else
		{
			do
			{
				setpixelFast(first_x,first_y);
				setpixelFast(last_x,last_y);
				first_x++;
				last_x--;
				if (error>=0)
				{
					first_y--;
					last_y++;
					error-=sub;
				}
				else error+=dy;
			} while (--remain>0);
		}

		if (!(dx&1)) setpixelFast(first_x,first_y);

		return;
	}
	else
	{
		sub=dy-dx;
		error=dx-(dy>>1);
		remain=(dy+1)>>1;

		if( inc1>0)
		{
			do
			{
				setpixelFast(first_x,first_y);
				setpixelFast(last_x,last_y);
				first_y++;
				last_y--;

				if (error>=0)
				{
					first_x++;
					last_x--;
					error-=sub;
				}
				else error+=dx;

			} while (--remain>0);
		}
		else
		{
			do
			{
				setpixelFast(first_x,first_y);
				setpixelFast(last_x,last_y);
				first_y--;
				last_y++;

				if (error>=0)
				{
					first_x++;
					last_x--;
					error-=sub;
				}
				else error+=dx;

			} while (--remain>0);

		}
		if (!(dy&1)) setpixelFast(first_x,first_y);

		return;
	}

}

void cercle(short rayon,short x_centre,short y_centre,unsigned char state)   __reentrant
{
	short x,y,d;

	x=0;
	y=rayon;
	d=rayon;
	while(y>=x)
	{
		setpixel(x+x_centre , y+y_centre,state);
		setpixel(y+x_centre , x+y_centre,state);
		setpixel(-x+x_centre , y+y_centre,state);
		setpixel(-y+x_centre , x+y_centre,state);
		setpixel(x+x_centre , -y+y_centre,state);
		setpixel(y+x_centre , -x+y_centre,state);
		setpixel(-x+x_centre , -y+y_centre,state);
		setpixel(-y+x_centre , -x+y_centre,state);

		if(d >= 2*x)
		{
			d=d-2*x-1;
			x++;
		}
		else
		{
			if( d < 2*(rayon-y) )
			{
				d=d+2*y-1;
				y--;

			}
			else
			{
				d=d+2*(y-x-1);
				y--;
				x++;
			}
		}
	}
	return;
}
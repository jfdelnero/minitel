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
#include <math.h>
#include "demo_minitel.h"

#include "minitel_reg.h"
#include "minitel_hw.h"
#include "data_bmp_nolife_bmp.h"
#include "data_bmp_Image3_raw.h"
#include "data_bmp_Image4_raw.h"
#include "data_bmp_Image5_raw.h"
#include "data_bmp_mac_raw.h"

#include "data_bmp_theminiteldemo_200x40_raw.h"

#include "demotxt.h"

#include "deuxd_func.h"
#include "troisd_func.h"

#include "obj3D_Box01.h"
#include "obj3D_Sphere01.h"
#include "obj3D_Cylinder01.h"
#include "obj3D_Prism01.h"
#include "obj3D_Piramid01.h"
#include "obj3D_Hedra01.h"

#include "nyan_cat.h"

const unsigned char * spritestab[]={ data_nolife_bmp,  data_bmpImage3_raw,  data_bmpImage4_raw,data_bmpImage5_raw};

volatile unsigned char hwctrlstatus;
volatile char musicptr;
volatile char musicptr2;

volatile unsigned short timer_tick;
volatile unsigned char start_tick;
volatile unsigned short tick_to_wait;
volatile unsigned char buffernb;

enum DEMOSTATE
{
     STARTSCREEN,
     ENDSCREEN,
     SCREEN2,
     SCREEN3,
     SCREEN4,
     SCREEN5,
	 SCREENNYAN
};

const unsigned char initstarstab[25]=
{
	37,4,26,11,33,20,26,35,
	15,47,19,45,13,28,36,34,
	11,0,47,32,42,34,40,0,20
};

const polyg testpg=
{
	{
		{20,-10,60},
		{10,30,40},
		{-50,20,20}
	}
};

const unsigned char txt[]=" Minitel Demo Test ! (c)HxC2001 ";

void wait(unsigned short ms)
{
	start_tick=0;
	tick_to_wait=ms;

	start_tick=1;
	do
	{
		ms--;
	}while(start_tick);

}

// music tick int
void music_timer_isr (void) __interrupt (TF1_VECTOR) //__using (1)
{
	if(musicptr2>2)
	{
		musicptr2=0;
		RTS_MODEM=1;

		if(musicptr&0x4)
		{
			write_to_modem(0x3,RWLO);
			write_to_modem(0x4,RPTF);
			//write_to_modem_dtmf(musicptr<<2,RDTMF);
		}
		else
		{
			write_to_modem(0x3,RWLO);
			write_to_modem(0x8,RPTF);
			//write_to_modem_dtmf(musicptr,RDTMF);
		}
		musicptr++;
	}

	musicptr2++;

    TH1 = 0x07;
    TL1 = 0xF0;
}

// timer tick int
void timer_isr (void) __interrupt (TF0_VECTOR) //__using (1)
{
	timer_tick++;

	if(start_tick)
	{
		tick_to_wait--;
		if(!tick_to_wait)
		{
			start_tick=0;
		}
	}
	TH0=0x1F;
	TL0=0xF0;
}

void nyan_cat_sc()
{
	int x,y,xstart,ystart,i,j,k,l,m;
	unsigned char c;

	xstart=0;
	ystart=8;//8+0;
	x=0;
	y=0;
	i=0;
	c=0;
	k=0;
	l=0;

	TS9347_R1=0x7f;//txt_start[i];
	TS9347_R2=0x01;


	for(j=0;j<24;j++)
	{
		WAIT_TS9347;
		set_ptr(0+xstart,j+ystart,0,0);
		for(i=0;i<40;i++)
		{
			WAIT_TS9347;

			TS9347_R3=0x01 | (0x10);
			TS9347_ER0=CMD_TLM|0x01;
			k++;
		}
	}

	for(l=0;l<256;l++)
	{
		for(j=0;j<22;j++)
		{
			WAIT_TS9347;
			set_ptr(0,1+j+ystart,0,0);
			for(i=0;i<40;i++)
			{
				//write_to_modem_dtmf(2,RDTMF);
				WAIT_TS9347;

				TS9347_R3=0x01 | (nyan_cat[k]<<4);
				TS9347_ER0=CMD_TLM|0x01;
				k++;
			}
		}

		/*timer_tick=0;
		do{
		}while(timer_tick<2);*/

		for(m=0;m<5000;m++)
		{
			//nop();
		}

		k=40*22*((l%6));
	}


}

void start_screen()
{
	int x,y,xstart,ystart,i,j,k;
	int scroll_start_offset_a,scroll_start_offset_b,numberofline;
	unsigned int last;

	clearscreen();
	xstart=0;
	ystart=8;//8+0;
	x=0;
	y=0;
	last=0;
	i=0;
	k=0;

	timer_tick=0;
	do{
	}while(timer_tick<1);

	i=0;
	while(txt_start[i])
	{
		if(txt_start[i]=='\n')
		{
			x=0;
			y++;
		}
		else
		{
			WAIT_TS9347;
			set_ptr(x+xstart,y+ystart,0,0);

			write_to_modem_dtmf(2,RDTMF);
			WAIT_TS9347;

			TS9347_R1=txt_start[i];
			TS9347_R2=0x01;
			TS9347_R3=0x71;
			TS9347_ER0=CMD_TLM|0x01;

			x++;
			timer_tick=0;
			do{
			}while(timer_tick<1);

			write_to_modem(2,RDTMF);
		}
		i++;
	}
	wait(130);

	clearscreen();
	setcharset(data_bmptheminiteldemo_200x40_raw,25,4);
	fillmosaic(7,10,25,4,0);

	wait(30);


	scroll_start_offset_a=0;
	scroll_start_offset_b=0;
	y=0;
	j=0;
	k=0;
	numberofline=1;

	do
	{
		do
		{
			i=scroll_start_offset_a+y;
			i=i%20;
			k=scroll_start_offset_b+y;
			k=k%20;

			WAIT_TS9347;
			TS9347_R2=0x01;
			TS9347_R3=0x37;
			set_ptr(x+xstart,y+ystart,0,0);

			x=0;
			TS9347_R3=0x07 | ((i&7)<<4);
			while(x<40)
			{
				WAIT_TS9347;
				TS9347_R1=txt_scroll[i];
				TS9347_ER0=CMD_TLM|0x01;
				i++;
				if(!txt_scroll[i]) i=0;
				x++;
			}

			x=0;
			WAIT_TS9347;
			set_ptr(x+xstart,(23-y)+ystart,0,0);

			TS9347_R3=0x07 | ((i&7)<<4);
			while(x<40)
			{
				WAIT_TS9347;
				TS9347_R1=txt_scroll[k];
				TS9347_ER0=CMD_TLM|0x01;

				k++;
				if(!txt_scroll[k]) k=0;

				x++;
			}

			y++;
		}while(  y<numberofline);

		scroll_start_offset_a++;
		if(  scroll_start_offset_a>=20 )
                     scroll_start_offset_a=0;
		scroll_start_offset_b=20-scroll_start_offset_a;

		wait(1);
		y=0;
		j++;
		if(j>16)
		{
			if(numberofline<10 )
				numberofline++;
			j=0;
		}

		if(numberofline>=10 ) last++;

	}while(numberofline<10 || last<128);
}


void starfield() __reentrant
{
	unsigned char i,j,t;
	unsigned char starttab[25];
	unsigned short k;

	buffernb=0;
	clearcharset(0);

	for(i=0;i<25;i++)
	{
		starttab[i]=initstarstab[i];
	}

	for(j=0;j<3;j++)
	{
		for(i=0;i<4;i++)
		{
			fillmosaic(10*i,10*j,10,10,0);
		}
	}


	for(k=0;k<1524;k++)
	{
		for(i=0;i<25;i++)
		{
			t=starttab[i];
			setpixel(t,i*4, 0);

			switch(i&3)
			{
				case 0:
					t--;
					if(t>=80) t=79;
					break;
				case 1:
					if((k&3)==2 || (k&3)==3 || (k&3)==1)
					{
						t--;
						if(t>=80) t=79;
					}
					break;
				case 2:
					if((k&3)==2 || (k&3)==3)
					{
						t--;
						if(t>=80) t=79;
					}
					break;
				case 3:
					if((k&3)==3)
					{
						t--;
						if(t>=80) t=79;
					}
					break;
			}

			starttab[i]=t;
			setpixel(t,i*4, 1);
		}
	}
}

int mandel()   __reentrant
{
	// screen ( integer) coordinate
	int iX,iY;
	unsigned char i,j;
	const int iXmax = 80;
	const int iYmax = 100;
	//world ( float) coordinate = parameter plane
	float Cx,Cy;
	const float CxMin=-2.5;
	const float CxMax=1.5;
	const float CyMin=-2.0;
	const float CyMax=2.0;

	float PixelWidth=(CxMax-CxMin)/iXmax;
	float PixelHeight=(CyMax-CyMin)/iYmax;
	// color component ( R or G or B) is coded from 0 to 255
	// it is 24 bit color RGB file
	float Zx, Zy;
	float Zx2, Zy2; // Zx2=Zx*Zx;  Zy2=Zy*Zy

	int Iteration;
	const int IterationMax=20;
	// bail-out value , radius of circle ;
	const float EscapeRadius=1000;
	float ER2=EscapeRadius*EscapeRadius;

	clearcharset(0);

	for(j=0;j<3;j++)
	{
		for(i=0;i<4;i++)
		{

			fillmosaic(10*i,10*j,10,10,0);
		}
	}

	// compute and write image data bytes to the file
	for(iY=0;iY<iYmax;iY++)
	{
		Cy=CyMin + iY*PixelHeight;
		if(Cy>=0)
		{
			if (Cy< PixelHeight/2) Cy=0.0; /* Main antenna */
		}
		else
		{

			if (-Cy< PixelHeight/2) Cy=0.0; /* Main antenna */
		}

		for(iX=0;iX<iXmax;iX++)
		{
			Cx=CxMin + iX*PixelWidth;
			/* initial value of orbit = critical point Z= 0 */
			Zx=0.0;
			Zy=0.0;
			Zx2=Zx*Zx;
			Zy2=Zy*Zy;
			/* */
			for (Iteration=0;Iteration<IterationMax && ((Zx2+Zy2)<ER2);Iteration++)
			{
				Zy=2*Zx*Zy + Cy;
				Zx=Zx2-Zy2 +Cx;
				Zx2=Zx*Zx;
				Zy2=Zy*Zy;
			};
			/* compute  pixel color (24 bit = 3 bajts) */
			if (Iteration==IterationMax)
			{ /*  interior of Mandelbrot set = black */
				//setpixel(iX,iY, 0);
			}
			/* exterior of Mandelbrot set = LSM */
			else if ((Iteration%2)==0)
			{
				//setpixel(iX,iY, 0);
			}
			else
			{
				setpixel(iX,iY, 1);
			};
			/*write color to the file*/
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////
// Main function
//////////////////////////////////////////////////////////////////

void main(void)
{

	unsigned char demostate;
	unsigned short i,j,k,l;

	init_ts9347();
	init_modem();
	hwctrlstatus=hwctrlstatus &(~( HW_CTRL_MODDTMF));
	hw_ctrl_reg=hwctrlstatus;

	// init timer + it
	IE=0;
	timer_tick=0;
	TMOD=0x11;
	TR1=1;
	TR0=1;
	ET1=1;
	ET0=1;
	EA=1;
	buffernb=0;
	fillmosaic(15,5,10,10,0);

	demostate=STARTSCREEN;
	do
	{
		switch(demostate)
		{
		case STARTSCREEN:
			clearscreen();
			setcharset1010(data_bmpmac_raw);

			for(j=0;j<12;j++)
			{
				for(i=0;i<4;i++)
				{
					fillmosaic(i*10,j*2,10,2,0);
				}
			}

			wait(150);

			fillmosaic(15,7,10,10,0);

			wait(100);

			start_screen();
			demostate=SCREENNYAN;
			break;

		case ENDSCREEN:
			clearscreen();
			mandel();
			setcharset1010(data_bmpImage3_raw);

			for(j=0;j<3;j++)
			{
				for(i=0;i<4;i++)
				{
					fillmosaic(10*i,10*j,10,10,0);
				}
			}

			for(i=0;i<64;i++)
			{
				setcharset1010(spritestab[i&3]);
			}
			wait(80);

			demostate=STARTSCREEN;
			break;

		case SCREENNYAN:
			clearscreen();
			nyan_cat_sc();

			demostate=SCREEN2;

		break;

		case SCREEN2:

			starfield();

			demostate=SCREEN3;


			break;

		case SCREEN3:

			buffernb=0;
			clearcharset(0);

			for(j=0;j<3;j++)
			{
				for(i=0;i<4;i++)
				{
					fillmosaic(10*i,10*j,10,10,0);
				}
			}

			for(j=0;j<80;j++)
			{
				for(i=0;i<99;i++)
				{
					//Ligne(0,0,0,79-i);
					//Ligne(79,i,j,0,1);
					//Ligne(79,i,j,0,0);
				}
			}
			//wait(80);

			demostate=SCREEN4;

			break;
		case SCREEN4:

			buffernb=0;
			clearcharset(0);
			buffernb=0x40;
			clearcharset(0);
			buffernb=0x00;

			for(j=0;j<3;j++)
			{
				for(i=0;i<4;i++)
				{
					fillmosaic(10*i,10*j,10,10,0);
				}
			}

			for(k=0;k<128;k++)
			{
				buffernb=(k&1)<<6;

				clearcharset(0);
				drawobject(&data3d_Hedra01,k,k*3,k*2,1);

				setbank(k&1);
			}

			for(k=0;k<128;k++)
			{
				buffernb=(k&1)<<6;

				clearcharset(0);
				drawobject(&data3d_Piramid01,k,k*3,k*2,1);

				setbank(k&1);
			}


			for(k=0;k<128;k++)
			{
				buffernb=(k&1)<<6;

				clearcharset(0);
				drawobject(&data3d_Prism01,k,k*3,k*2,1);

				setbank(k&1);
			}

			for(k=0;k<128;k++)
			{
				buffernb=(k&1)<<6;

				clearcharset(0);
				drawobject(&data3d_Box01,k,k*3,k*2,1);

				setbank(k&1);
			}

			for(k=0;k<128;k++)
			{
				buffernb=(k&1)<<6;

				clearcharset(0);
				drawobject(&data3d_Cylinder01,k,k*3,k*2,1);

				setbank(k&1);
			}

			buffernb=0;
			setbank(0);
			demostate=SCREEN5;

			break;

		case SCREEN5:

			buffernb=0;
			clearcharset(0);
			buffernb=0x40;
			clearcharset(0);
			buffernb=0x00;

			for(j=0;j<3;j++)
			{
				for(i=0;i<4;i++)
				{
					fillmosaic(10*i,10*j,10,10,0);
				}
			}

			for(k=0;k<40;k++)
			{
				cercle(k,40,50,1);
			}

			for(k=0;k<40;k++)
			{
				cercle(k,40,50,0);
			}

			i=0;
			j=20;
			for(k=0;k<400;k++)
			{
				cercle(j,40,50,1);
				cercle(i,40,50,0);

				i++;
				if(i>=65) i=0;

				j++;
				if(j>=65) j=0;
			}

			i=0;
			j=1;
			for(l=0;l<10;l++)
			{
				clearcharset(0);
				for(k=0;k<40;k++)
				{
					cercle(j,40,50,1);
//					cercle(i,40,50,0);

					i=i+2;
					if(i>=65) i=0;

					j=j+2;
					if(j>=65) j=l&1;

				}

				j=l&1;
			}

			demostate=ENDSCREEN;
			break;

		}
	}while(1);
}

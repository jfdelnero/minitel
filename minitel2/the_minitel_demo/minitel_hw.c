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
#include <8052.h>
#include "minitel_hw.h"
#include "minitel_reg.h"

extern volatile uint8_t buffernb;
extern volatile uint8_t hwctrlstatus;

//////////////////////////////////////////////////////////////////
// Video functions
//////////////////////////////////////////////////////////////////
/*
void wait_video(void) _naked
{
	while( VIDEO_R0&0x80);
}*/

void clearscreen(void)
{
	int j,i;
	// clear screen
	for(j=8;j<25+8;j++)
	{
		for(i=0;i<40;i++)
		{
			//wait_video();
			WAIT_VIDEO_BUSY;
			set_ptr(i,j,0,0);

			VIDEO_R1=' ';
			VIDEO_R2=0x01;
			VIDEO_R3=0x37;
			VIDEO_ER0=CMD_TLM|0x01;
		}
	}
}

void setbank(int8_t bank)
{

	/*
	wait_video();
	VIDEO_R1=0x08 | ((bank&1)<<5);
	VIDEO_ER0=CMD_IND|ROR_REG;
	*/

	//wait_video();
	WAIT_VIDEO_BUSY;

	if(bank==0)
		VIDEO_R1=0x26;
	else
		VIDEO_R1=0x72;

	VIDEO_R6=0x0;
	VIDEO_R7=0x0;
	VIDEO_R5=0x0;
	VIDEO_R4=0x0;
	VIDEO_ER0=CMD_IND|DOR_REG;

}

void init_video(void)
{
	VIDEO_ER0=CMD_NOP;
	//wait_video();
	WAIT_VIDEO_BUSY;

#if defined(MINITEL_NFZ330)
	WAIT_VIDEO_BUSY;
	VIDEO_R1=0x10;
	VIDEO_ER0=CMD_IND|TGS_REG;

	WAIT_VIDEO_BUSY;
	VIDEO_R1=0x06;
	VIDEO_ER0=CMD_IND|PAT_REG;
#elif defined(MINITEL_NFZ400)
	WAIT_VIDEO_BUSY;
	VIDEO_R1=0x0C;
	VIDEO_ER0=CMD_IND|TGS_REG;

	//wait_video();
	WAIT_VIDEO_BUSY;
	VIDEO_R1=0x0C;
	VIDEO_ER0=CMD_IND|TGS_REG;

	WAIT_VIDEO_BUSY;
	VIDEO_R1=0x02;
	VIDEO_ER0=CMD_IND|PAT_REG;
#else
#error Please define one of the MINITEL_* macros
#endif

	WAIT_VIDEO_BUSY;
	VIDEO_R1=0x00;
	VIDEO_ER0=CMD_IND|MAT_REG;

	WAIT_VIDEO_BUSY;
	VIDEO_ER0=CMD_VRM;

	VIDEO_R6=0;
	VIDEO_R7=0;

	WAIT_VIDEO_BUSY;
	VIDEO_R1=0x08;
	VIDEO_ER0=CMD_IND|ROR_REG;

	WAIT_VIDEO_BUSY;
	VIDEO_R1=0x26;
	VIDEO_R6=0x0;
	VIDEO_R7=0x0;
	VIDEO_R5=0x0;
	VIDEO_R4=0x0;
	VIDEO_ER0=CMD_IND|DOR_REG;

	clearscreen();
}

void set_ptr(uint8_t X,uint8_t Y,uint8_t D,uint8_t B)
{
	VIDEO_R6= (D<<5)|(Y&0x1F);
	VIDEO_R7=(B<<6)|(X&0x3F);
}

void fillmosaic(uint8_t x,uint8_t y,uint8_t xsize,uint8_t ysize,uint8_t page) __reentrant
{
	uint8_t i,j,c;

	c=0;

	VIDEO_R3=0x37;

	for(i=0;i<ysize;i++)
	{
		for(j=0;j<xsize;j++)
		{

			WAIT_VIDEO_BUSY;
			set_ptr(j+x,i+8+y,0,0);

			WAIT_VIDEO_BUSY;
			VIDEO_R1=c;
			VIDEO_R2=0xA5;

			VIDEO_ER0=CMD_TLM|0x01;
			c++;
			if(c==4)  c=32;
		}
	}
}


void setcharset1010(const int8_t * databuffer) __reentrant
{
	uint8_t i,j,c,k;
	uint16_t ptr;
	c=0;
	VIDEO_R6=0x0;
	VIDEO_R7=0x0;
	ptr=0;
	for(i=0;i<10;i++)
	{
		for(k=0;k<10;k++)
		{
			for(j=0;j<10;j++)
			{
				c=j+(i*10);
				if(c>3) c=32+(c-4);
				WAIT_VIDEO_BUSY;
				VIDEO_R1= databuffer[ptr];
				VIDEO_R6= (0x20)|(c>>2);
				VIDEO_R7= 0x00 | (k<<2)|(c&0x3);
				VIDEO_ER0=CMD_TBM;
				ptr++;
			}
		}
	}
}


void setcharset(int8_t * databuffer,uint8_t xsize,uint8_t ysize) __reentrant
{
	uint8_t i,j,c,k;
	uint16_t ptr;
	c=0;
	VIDEO_R6=0x0;
	VIDEO_R7=0x0;
	ptr=0;
	for(i=0;i<ysize;i++)
	{
		for(k=0;k<10;k++)
		{
			for(j=0;j<xsize;j++)
			{
				c=j+(i*xsize);
				if(c>3) c=32+(c-4);
				WAIT_VIDEO_BUSY;
				VIDEO_R1= databuffer[ptr];
				VIDEO_R6= (0x20)|(c>>2);
				VIDEO_R7= 0x00 | (k<<2)|(c&0x3);
				VIDEO_ER0=CMD_TBM;
				ptr++;
			}
		}
	}
}

void clearcharset(uint8_t ligne) __reentrant
{


	uint8_t i,j,c,k;
	uint16_t ptr;

	c=0;
	VIDEO_R6=0x0;
	VIDEO_R7=0x0;
	ptr=0;

	WAIT_VIDEO_BUSY;
	VIDEO_R1= ligne;

	for(k=0;k<10;k++)
	{
		for(j=0;j<10;j++)
		{
			c=j;
			if(c>3) c=32+(c-4);
			WAIT_VIDEO_BUSY;

			VIDEO_R6= (0x20)|(c>>2);
			VIDEO_R7= buffernb | (k<<2)|(c&0x3);
			VIDEO_ER0=CMD_TBM;
			ptr++;
		}
	}

	for(i=1;i<10;i++)
	{
		for(k=0;k<10;k++)
		{
			for(j=0;j<10;j++)
			{
				c=28 + j+(i*10);
				//if(c>3) c=32+(c-4);
				WAIT_VIDEO_BUSY;

				VIDEO_R6= (0x20)|(c>>2);
				VIDEO_R7= buffernb | (k<<2)|(c&0x3);
				VIDEO_ER0=CMD_TBM;
				ptr++;
			}
		}
	}
}

#if defined(MINITEL_NFZ400)
//////////////////////////////////////////////////////////////////
// Modem functions
//////////////////////////////////////////////////////////////////
#pragma save
#pragma nooverlay
void write_to_modem(uint8_t datavalue,uint8_t address)
{
	uint8_t j;
	unsigned hwctrlstatus_backup;

	RTS_MODEM=1;

	hwctrlstatus_backup=hwctrlstatus;
	hwctrlstatus=hwctrlstatus &(~( HW_CTRL_MCBC | HW_CTRL_MODDTMF));
	hw_ctrl_reg=hwctrlstatus;


	for(j=0;j<4;j++)
	{
		if(datavalue&(0x01<<j))
		{
			PRD_MODEM=1;
		}
		else
		{
			PRD_MODEM=0;
		}
		RTS_MODEM=0;
		RTS_MODEM=1;
	}

	for(j=0;j<4;j++)
	{
		if(address&(0x01<<j))
		{
			PRD_MODEM=1;
		}
		else
		{
			PRD_MODEM=0;
		}
		RTS_MODEM=0;

		RTS_MODEM=1;

	}


	hwctrlstatus=hwctrlstatus_backup;
	hw_ctrl_reg=hwctrlstatus;

}
#pragma restore

#pragma save
#pragma nooverlay
void write_to_modem_dtmf(uint8_t datavalue,uint8_t address)
{
	uint8_t j;
	unsigned hwctrlstatus_backup;

	RTS_MODEM=1;

	hwctrlstatus_backup=hwctrlstatus;
	hwctrlstatus=hwctrlstatus &(~( HW_CTRL_MCBC | HW_CTRL_MODDTMF));
	hw_ctrl_reg=hwctrlstatus;


	for(j=0;j<4;j++)
	{
		if(datavalue&(0x01<<j))
		{
			PRD_MODEM=1;
		}
		else
		{
			PRD_MODEM=0;
		}
		RTS_MODEM=0;
		RTS_MODEM=1;

	}

	for(j=0;j<4;j++)
	{
		if(address&(0x01<<j))
		{
			PRD_MODEM=1;
		}
		else
		{
			PRD_MODEM=0;
		}
		RTS_MODEM=0;

		if(j!=3)
			RTS_MODEM=1;

	}

	hwctrlstatus=hwctrlstatus_backup;
	hw_ctrl_reg=hwctrlstatus;

}
#pragma restore

void init_modem(void)
{
	hwctrlstatus=HW_CTRL_CTRON | HW_CTRL_MCBC | HW_CTRL_MODDTMF;
	hw_ctrl_reg=hwctrlstatus;

	write_to_modem(0x0,RPROG);
	write_to_modem(0x0,RATTE);
	write_to_modem(0x3,RWLO);//0x3
	write_to_modem(0x0,RPTF);
}
#endif

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
void wait_ts9347(void) _naked
{
	while( TS9347_R0&0x80);
}*/

void clearscreen(void)
{
	int j,i;
	// clear screen
	for(j=8;j<25+8;j++)
	{
		for(i=0;i<40;i++)
		{
			//wait_ts9347();
			WAIT_TS9347;
			set_ptr(i,j,0,0);

			TS9347_R1=' ';
			TS9347_R2=0x01;
			TS9347_R3=0x37;
			TS9347_ER0=CMD_TLM|0x01;
		}
	}
}

void setbank(int8_t bank)
{

	/*
	wait_ts9347();
	TS9347_R1=0x08 | ((bank&1)<<5);
	TS9347_ER0=CMD_IND|ROR_REG;
	*/

	//wait_ts9347();
	WAIT_TS9347;

	if(bank==0)
		TS9347_R1=0x26;
	else
		TS9347_R1=0x72;

	TS9347_R6=0x0;
	TS9347_R7=0x0;
	TS9347_R5=0x0;
	TS9347_R4=0x0;
	TS9347_ER0=CMD_IND|DOR_REG;

}

void init_ts9347(void)
{
	TS9347_ER0=CMD_NOP;
	//wait_ts9347();
	WAIT_TS9347;

	//wait_ts9347();
	WAIT_TS9347;
	TS9347_R1=0x0C;
	TS9347_ER0=CMD_IND|TGS_REG;

	//wait_ts9347();
	WAIT_TS9347;
	TS9347_R1=0x0C;
	TS9347_ER0=CMD_IND|TGS_REG;

	WAIT_TS9347;
	TS9347_R1=0x02;
	TS9347_ER0=CMD_IND|PAT_REG;

	WAIT_TS9347;
	TS9347_R1=0x00;
	TS9347_ER0=CMD_IND|MAT_REG;

	WAIT_TS9347;
	TS9347_ER0=CMD_VRM;

	TS9347_R6=0;
	TS9347_R7=0;

	WAIT_TS9347;
	TS9347_R1=0x08;
	TS9347_ER0=CMD_IND|ROR_REG;

	WAIT_TS9347;
	TS9347_R1=0x26;
	TS9347_R6=0x0;
	TS9347_R7=0x0;
	TS9347_R5=0x0;
	TS9347_R4=0x0;
	TS9347_ER0=CMD_IND|DOR_REG;

	clearscreen();
}

void set_ptr(uint8_t X,uint8_t Y,uint8_t D,uint8_t B)
{
	TS9347_R6= (D<<5)|(Y&0x1F);
	TS9347_R7=(B<<6)|(X&0x3F);
}

void fillmosaic(uint8_t x,uint8_t y,uint8_t xsize,uint8_t ysize,uint8_t page) __reentrant
{
	uint8_t i,j,c;

	c=0;

	TS9347_R3=0x37;

	for(i=0;i<ysize;i++)
	{
		for(j=0;j<xsize;j++)
		{

			WAIT_TS9347;
			set_ptr(j+x,i+8+y,0,0);

			WAIT_TS9347;
			TS9347_R1=c;
			TS9347_R2=0xA5;

			TS9347_ER0=CMD_TLM|0x01;
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
	TS9347_R6=0x0;
	TS9347_R7=0x0;
	ptr=0;
	for(i=0;i<10;i++)
	{
		for(k=0;k<10;k++)
		{
			for(j=0;j<10;j++)
			{
				c=j+(i*10);
				if(c>3) c=32+(c-4);
				WAIT_TS9347;
				TS9347_R1= databuffer[ptr];
				TS9347_R6= (0x20)|(c>>2);
				TS9347_R7= 0x00 | (k<<2)|(c&0x3);
				TS9347_ER0=CMD_TBM;
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
	TS9347_R6=0x0;
	TS9347_R7=0x0;
	ptr=0;
	for(i=0;i<ysize;i++)
	{
		for(k=0;k<10;k++)
		{
			for(j=0;j<xsize;j++)
			{
				c=j+(i*xsize);
				if(c>3) c=32+(c-4);
				WAIT_TS9347;
				TS9347_R1= databuffer[ptr];
				TS9347_R6= (0x20)|(c>>2);
				TS9347_R7= 0x00 | (k<<2)|(c&0x3);
				TS9347_ER0=CMD_TBM;
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
	TS9347_R6=0x0;
	TS9347_R7=0x0;
	ptr=0;

	WAIT_TS9347;
	TS9347_R1= ligne;

	for(k=0;k<10;k++)
	{
		for(j=0;j<10;j++)
		{
			c=j;
			if(c>3) c=32+(c-4);
			WAIT_TS9347;

			TS9347_R6= (0x20)|(c>>2);
			TS9347_R7= buffernb | (k<<2)|(c&0x3);
			TS9347_ER0=CMD_TBM;
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
				WAIT_TS9347;

				TS9347_R6= (0x20)|(c>>2);
				TS9347_R7= buffernb | (k<<2)|(c&0x3);
				TS9347_ER0=CMD_TBM;
				ptr++;
			}
		}
	}
}

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


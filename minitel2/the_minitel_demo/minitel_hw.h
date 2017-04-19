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

void clearscreen(void);
void setbank(char bank);
void init_ts9347(void);
void set_ptr(unsigned char X,unsigned char Y,unsigned char D,unsigned char B);
void fillmosaic(unsigned char x,unsigned char y,unsigned char xsize,unsigned char ysize,unsigned char page) __reentrant;
void setcharset(char * databuffer,unsigned char xsize,unsigned char ysize) __reentrant;
void setcharset1010(const char * databuffer) __reentrant;

void clearcharset(unsigned char ligne) __reentrant;
void write_to_modem(unsigned char datavalue,unsigned char address);
void write_to_modem_dtmf(unsigned char datavalue,unsigned char address);
void init_modem(void);

#define WAIT_TS9347 while(TS9347_R0&0x80);

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
void setbank(int8_t bank);
void init_video(void);
void set_ptr(uint8_t X,uint8_t Y,uint8_t D,uint8_t B);
void fillmosaic(uint8_t x,uint8_t y,uint8_t xsize,uint8_t ysize,uint8_t page) __reentrant;
void setcharset(int8_t * databuffer,uint8_t xsize,uint8_t ysize) __reentrant;
void setcharset1010(const int8_t * databuffer) __reentrant;

void clearcharset(uint8_t ligne) __reentrant;

#if defined(MINITEL_NFZ330)
#define XTAL_HZ 11059200

#elif defined(MINITEL_NFZ400)
#define XTAL_HZ 14318181

void write_to_modem(uint8_t datavalue,uint8_t address);
void write_to_modem_dtmf(uint8_t datavalue,uint8_t address);
void init_modem(void);
#endif

#define TIMER16_RELOAD_VALUE(us) (uint16_t)(0x10000 - (float)(us) * XTAL_HZ / 1000000 / 12)

#define WAIT_VIDEO_BUSY while(VIDEO_R0&0x80);

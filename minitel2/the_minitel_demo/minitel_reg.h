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

#if defined(MINITEL_NFZ330)
////////////////////////////////////////////////////////////////////
// Minitel 1B Hardware definition.
////////////////////////////////////////////////////////////////////

volatile __xdata __at 0xDF20 uint8_t VIDEO_R0;
volatile __xdata __at 0xDF21 uint8_t VIDEO_R1;
volatile __xdata __at 0xDF22 uint8_t VIDEO_R2;
volatile __xdata __at 0xDF23 uint8_t VIDEO_R3;
volatile __xdata __at 0xDF24 uint8_t VIDEO_R4;
volatile __xdata __at 0xDF25 uint8_t VIDEO_R5;
volatile __xdata __at 0xDF26 uint8_t VIDEO_R6;
volatile __xdata __at 0xDF27 uint8_t VIDEO_R7;
volatile __xdata __at 0xDF28 uint8_t VIDEO_ER0;
volatile __xdata __at 0xDF29 uint8_t VIDEO_ER1;
volatile __xdata __at 0xDF2A uint8_t VIDEO_ER2;
volatile __xdata __at 0xDF2B uint8_t VIDEO_ER3;
volatile __xdata __at 0xDF2C uint8_t VIDEO_ER4;
volatile __xdata __at 0xDF2D uint8_t VIDEO_ER5;
volatile __xdata __at 0xDF2E uint8_t VIDEO_ER6;
volatile __xdata __at 0xDF2F uint8_t VIDEO_ER7;

#elif defined(MINITEL_NFZ400)
////////////////////////////////////////////////////////////////////
// Minitel 2 Hardware definition.
// (c) 2010 Jean-François DEL NERO / HxC2001
////////////////////////////////////////////////////////////////////

#define HW_CTRL_MCBC 0x01
#define HW_CTRL_MODDTMF 0x02

#define HW_CTRL_CTRON 0x08
#define HW_CTRL_COILON 0x20
volatile __xdata __at 0x2000 uint8_t hw_ctrl_reg;

volatile __xdata __at 0x4020 uint8_t VIDEO_R0;
volatile __xdata __at 0x4021 uint8_t VIDEO_R1;
volatile __xdata __at 0x4022 uint8_t VIDEO_R2;
volatile __xdata __at 0x4023 uint8_t VIDEO_R3;
volatile __xdata __at 0x4024 uint8_t VIDEO_R4;
volatile __xdata __at 0x4025 uint8_t VIDEO_R5;
volatile __xdata __at 0x4026 uint8_t VIDEO_R6;
volatile __xdata __at 0x4027 uint8_t VIDEO_R7;
volatile __xdata __at 0x4028 uint8_t VIDEO_ER0;
volatile __xdata __at 0x4029 uint8_t VIDEO_ER1;
volatile __xdata __at 0x402A uint8_t VIDEO_ER2;
volatile __xdata __at 0x402B uint8_t VIDEO_ER3;
volatile __xdata __at 0x402C uint8_t VIDEO_ER4;
volatile __xdata __at 0x402D uint8_t VIDEO_ER5;
volatile __xdata __at 0x402E uint8_t VIDEO_ER6;
volatile __xdata __at 0x402F uint8_t VIDEO_ER7;

///////////////////////////////////////////////////////////
// Modem
///////////////////////////////////////////////////////////

#define RXD_MODEM P3_3   // Modem -> CPU
#define RTS_MODEM P1_4   // CPU -> Modem
#define TXD_MODEM P1_3   // CPU -> Modem
#define PRD_MODEM P1_2   // CPU -> Modem
#define DCD_MODEM P1_1   // Modem -> CPU
#define ZCO_MODEM P3_2   // Modem -> CPU

// Note ENP set to high -> the register input is connected to PRD.

#define RPROG 0x0
#define RDTMF 0x1
#define RATTE 0x2
#define RWLO  0x3
#define RPTF  0x4
#define RHDL  0x5
#define RPRX  0x6
#define RPROGB  0x7

#else
#error Please define one of the MINITEL_* macros
#endif

///////////////////////////////////////////////////////////
// Video chip regs
//
#define CMD_IND 0x80
#define CMD_TLM 0x00
#define CMD_TLA 0x20
#define CMD_TSM 0x60
#define CMD_TSA 0x70
#define CMD_KRS 0x40
#define CMD_KRL 0x50
#define CMD_TBM 0x30
#define CMD_TBA 0x30
#define CMD_MVB 0xD0
#define CMD_MVD 0xE0
#define CMD_MVT 0xF0
#define CMD_CLL 0xF0
#define CMD_CLS 0x60
#define CMD_VSM 0x90
#define CMD_VRM 0x90
#define CMD_INY 0xB0
#define CMD_NOP 0x90
#define EXEC_CMD 0x08

#define ROM_REG 0x00
#define TGS_REG 0x01
#define MAT_REG 0x02
#define PAT_REG 0x03
#define DOR_REG 0x04
#define ROR_REG 0x07
#define READ_REG 0x08

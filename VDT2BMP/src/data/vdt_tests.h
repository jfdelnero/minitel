extern unsigned char vdt_test_001[1709];
extern unsigned char vdt_test_002[1459];
extern unsigned char vdt_test_003[1636];

unsigned char * vdt_test_pages[]=
{
	vdt_test_003,
	vdt_test_001,
	vdt_test_002,
	NULL
};

int vdt_test_pages_size[]=
{
	sizeof(vdt_test_003),
	sizeof(vdt_test_001),
	sizeof(vdt_test_002),
	0
};
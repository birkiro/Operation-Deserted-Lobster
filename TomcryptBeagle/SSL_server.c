#include <tomcrypt.h>

int main(void)
{
	unsigned char key[16], IV[16], buffer[512];
	symmetric_CTR ctr;
	int x, err;

	if(register_cipher(&twofish_desc) == -1)
	{
		printf()
	}
}

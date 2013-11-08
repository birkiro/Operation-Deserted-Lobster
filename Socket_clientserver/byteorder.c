#include <stdio.h>

int main(void)
{
	/*
	 * This cute little program helps us determine our host byte order
	 *
	 * e store the two-byte value 0x0102 in the short integer and then
	 * look at the two consecutive bytes, c[0] (the address A) and c[1]
	 * (the address A+1) to determine the byte order.
	 */
    union {
        short s;
        char c[sizeof(short)];
    }un;

    un.s = 0x0102;

    if (sizeof(short) == 2) {
        if (un.c[0] == 1 && un.c[1] == 2)
            printf("big-endian\n");
        else if (un.c[0] == 2 && un.c[1] == 1)
            printf("little-endian\n");
        else
            printf("unknown\n");
   } else{
        printf("sizeof(short) = %d\n", sizeof(short));
   }
   exit(0);
}

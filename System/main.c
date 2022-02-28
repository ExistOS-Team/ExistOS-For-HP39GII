
#include <stdint.h>

char s[] = "012345678901234567890123456789012345678901234567890123456789\n";

    volatile uint8_t *p = (uint8_t *)0x80070000;
    volatile uint32_t *t = (uint32_t *)0x80070018;

    volatile uint32_t *digms = (uint32_t *)0x8001C0C0;

volatile void delay(uint32_t us)
{
    uint32_t start = *digms;

    while((*digms - start) < us)
    {
        ;
    }
}

void main()
{
    uint32_t i = 0;
    for(;;){
        while(*t & 4);
        delay(1000);
        *p = s[i++];

        if(i > sizeof(s))
        {
            i = 0;
        }
    }
/*
    for(int i =0; i<sizeof(s); i++){
        while(*t & 4);

        *p = s[i];
    }
*/
    while(1);


}
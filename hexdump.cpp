#include <cstdio>
#include <stdint.h>

void hexdump(const void *ptr, size_t len)
{
    uint8_t *p = (uint8_t*) ptr;
    for (size_t i = 0; i < len; i++)
    {
        printf("%02x ", p[i]);
        if (i && i % 16 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
}


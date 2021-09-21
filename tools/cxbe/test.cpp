#include <cstdint>

typedef struct _m_eight {
    std::uint8_t m_type1 : 1;
    std::uint8_t m_len : 3;
    std::uint8_t m_data : 4;
} m_eight;

typedef struct _m_sixteen {
    std::uint16_t m_type1 : 1;
    std::uint16_t m_type2 : 1;
    std::uint16_t m_len : 10;
    std::uint16_t m_data : 4;
} m_sixteen;

typedef union _logo_rle {
    m_eight m_eight;
    m_sixteen m_sixteen;
} logo_rle;

// ******************************************************************
// * standard typedefs
// ******************************************************************
typedef signed int     sint;
typedef unsigned int   uint;

typedef char           int08;
typedef short          int16;
typedef long           int32;

typedef unsigned char  uint08;
typedef unsigned short uint16;
typedef unsigned long  uint32;

typedef signed char    sint08;
typedef signed short   sint16;
typedef signed long    sint32;

// ******************************************************************
// * func : import_logo (100x17 8bit grayscale -> LogoBitmap format)
// ******************************************************************
void cxbx_xbe::import_logo(const uint08 x_gray[100*17])
{
    char *rle = get_logo();     // get raw logo data bytes

    // calculate maximum bitmap size supported by the existing file
    uint32 max_size = m_header.m_logo_bitmap_size;

    while(rle[max_size] == 0)
        max_size++;

    if(rle == 0)
    {
        if(get_error() == 0)
            set_error("logo bitmap could not be imported (not enough space in file?)", false);

        return;
    }

    // clear old bitmap data area
    {
        for(uint32 x=0;x<max_size;x++)
            rle[x] = 0;
    }

    uint32 offs = 0;

    for(uint32 v=1;v<100*17;offs++)
    {
        char color = x_gray[v] >> 4;

        uint32 len = 1;

        while(++v<100*17-1 && len < 1024 && color == x_gray[v] >> 4)
            len++;

        if(offs >= max_size)
        {
            set_error("not enough room to write logo bitmap", true);
            return;
        }

        logo_rle *cur = (logo_rle *)&rle[offs];

        if(len <= 7)
        {
            cur->m_eight.m_type1 = 1;
            cur->m_eight.m_len = len;
            cur->m_eight.m_data = color;
        }
        else
        {
            cur->m_sixteen.m_type1 = 0;
            cur->m_sixteen.m_type2 = 1;
            cur->m_sixteen.m_len = len;
            cur->m_sixteen.m_data = color;
            offs++;
        }
    }

    m_header.m_logo_bitmap_size = offs;
}

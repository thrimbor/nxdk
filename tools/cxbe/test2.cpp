#include <cstdint>

typedef struct __attribute__((packed, aligned(1))) _m_eight {
    std::uint8_t m_type1 : 1;
    std::uint8_t m_len : 3;
    std::uint8_t m_data : 4;
} m_eight_t;

typedef struct __attribute__((packed, aligned(1))) _m_sixteen {
    std::uint16_t m_type1 : 1;
    std::uint16_t m_type2 : 1;
    std::uint16_t m_len : 10;
    std::uint16_t m_data : 4;
} m_sixteen_t;

typedef union __attribute__((packed, aligned(1))) _logo_rle {
    m_eight_t m_eight;
    m_sixteen_t m_sixteen;
} logo_rle;

#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

// ******************************************************************
// * func : export_logo (LogoBitmap format -> 100x17 8bit grayscale)
// ******************************************************************
int main ()
{
    std::ifstream xbe;
    xbe.open("default.xbe", xbe.binary | xbe.in);
    xbe.seekg(0x170, xbe.beg);
    std::uint32_t bitmapoffset;
    std::uint32_t bitmapsize;
    xbe.read((char*)&bitmapoffset, 4);
    xbe.read((char*)&bitmapsize, 4);
    bitmapoffset -= 0x10000;
    xbe.seekg(bitmapoffset, xbe.beg);

    std::vector<std::uint8_t> bitmap;
    bitmap.resize(bitmapsize);
    std::cout << "reading from offset: " << std::hex << xbe.tellg() << std::endl;
    xbe.read((char*)bitmap.data(), bitmapsize);

    std::array<std::uint8_t, 100*17> x_gray;
    // in that rare case that we have no logo bitmap, we should
    // just clear it to black.
    memset(x_gray.data(), 0, 100*17);

    std::uint32_t o = 0;

    for (std::uint32_t v = 0; v < bitmapsize; v++) {
        std::uint32_t len = 0, data = 0;

        logo_rle *cur = (logo_rle *)&bitmap[v];

        //std::cout << "checking out value " << std::hex << (unsigned int)bitmap[v] << std::endl;

        if(cur->m_eight.m_type1)
        {
            len     = cur->m_eight.m_len;
            data    = cur->m_eight.m_data;
        }
        else
        {
            if(cur->m_sixteen.m_type2)
            {
                len     = cur->m_sixteen.m_len;
                data    = cur->m_sixteen.m_data;
                v      += 1;
            }
        }

        for(std::uint32_t j=0;j<len;j++)
        {
            o++;

            //std::cout << "writing pixel " << o << std::endl;

            if(o < 100*17)
                x_gray[o] = (char)(data << 4); // could use (int)(data * 15.0 + .5);
        }
    }

    std::ofstream out;
    out.open("out.pgm", out.binary | out.out);
    out.write("P5\x0A", 3);
    out.write("100\x0A", 4);
    out.write("17\x0A", 3);
    out.write("255\x0A", 4);
    out.write((char*)x_gray.data(), 100*17);
}

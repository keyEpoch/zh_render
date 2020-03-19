#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <fstream>

class TGAImage {

protected:
    unsigned char* data;
    int width;
    int height;
    int bytespp;     // (Format) channel

    bool load_rle_data(std::ifstream &in);
    bool unload_rle_data(std::ofstream &out);

    enum Format {GRAYSCALE=1, RBG=3, RGBA=4};   // dims, or channel

    // override constructors
    TGAImage();
    TGAImage(int w, int h, int bpp);
    TGAImage(const TGAImage& img);

    

    ~TGAImage();
};

struct TGAColor {
    // struct 缺省是 public，class 缺省是 private
    union {
        struct {
            unsigned char b, g, r, a;
        };
        unsigned char raw[4];
        unsigned int val;
    };
    int bytespp;

    // constructor
    TGAColor() 
    : val(0), bytespp(1) {}

    TGAColor(unsigned char R, unsigned char G, unsigned B, unsigned A)
    : b(B), g(G), r(R), a(A), bytespp(4) {}

    TGAColor(int v, int bpp)  
    : val(v), bytespp(bpp) {}

    TGAColor(const TGAColor& c) 
    : val(c.val), bytespp(c.bytespp) {}

    TGAColor(const unsigned char* p, int bpp) 
    : val(0), bytespp(bpp) {
        for (int i = 0; i < bpp; ++i)
            raw[i] = p[i];
    }

    // override the = op
    TGAColor& operator=(const TGAColor& c) {
        if (this != &c) {
            bytespp = c.bytespp;
            val = c.val;
        }
        return *this;
    } 
};


#endif
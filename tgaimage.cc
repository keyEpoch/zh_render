#include <iostream>
#include <string.h>
#include <time.h>
#include <math.h>

#include "tgaimage.h"


// constructors
TGAImage::TGAImage()
: data(NULL), width(0), height(0), bytespp(0) {}

TGAImage::TGAImage(int w, int h, int bpp) 
: data(NULL), width(w), height(h), bytespp(bpp) {
    unsigned long nbytes = width*height*bytespp;
    data = new unsigned char[nbytes];
    memset(data, 0, nbytes);
}

TGAImage::TGAImage(const TGAImage& img) {
    width = img.width;
    height = img.height;
    bytespp = img.bytespp;
    unsigned long nbytes = width*height*bytespp;
    data = new unsigned char[nbytes];
    memcpy(data, img.data, nbytes);
}

TGAImage::~TGAImage() {
    if (data) delete[] data;
}
// #include <vector>
// #include <cmath>
// #include <cstdlib>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "shader.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);

Model* model;

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(p0.x-p1.x)<std::abs(p0.y-p1.y)) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x>p1.x) {
        std::swap(p0, p1);
    }

    for (int x=p0.x; x<=p1.x; x++) {
        float t = (x-p0.x)/(float)(p1.x-p0.x);
        int y = p0.y*(1.-t) + p1.y*t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}



Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
}

float dot_product(Vec3f va, Vec3f vb) {
    return va.x * vb.x + va.y * vb.y + va.z * vb.z;
}

int main(int argc, char** argv) {
    if (2 > argc) {
        std::cerr << "need obj file!" << std::endl;
        return 1;
    }

    // render round one: obtain shadow_buffer
    

    // render round two: obtain shadow shader


    return 0;
}
#pragma once

#include "tgaimage.h"
#include "geometry.h"
#include "model.h"

const int width  = 800;
const int height = 800;

class BaseShader {

public:
    virtual ~BaseShader();
    // vertex shader 
    virtual Vec4f vertex(int iface, int nthvert, Model* model) = 0;
    // fragment shader
    virtual bool fragment(Vec3f bary, TGAColor& c, Model* model) = 0;
};

class OnlyTexShader : public BaseShader {

public:
    mat<2, 3, float> triangle_uvs; 

    virtual Vec4f vertex(int iface, int nthvert, Model* model);

    virtual bool fragment(Vec3f bary, TGAColor& c, Model* model);
};

void triangle(Vec3f* pts, BaseShader& shader, TGAImage& image, float* zbuffer, Model* model);

Vec3f bary_centric(Vec2f A, Vec2f B, Vec2f C, Vec3i P);
#pragma once

#include "tgaimage.h"
#include "geometry.h"
#include "model.h"

const int width  = 800;
const int height = 800;

extern Vec3f light_dir;
extern Vec3f eye;
extern Vec3f center;
extern Vec3f up;

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;

/* three important matrix */
// transfer eye(change the O point)
void lookat(Vec3f eye, Vec3f center, Vec3f up);

// after lookat(), the camera is always on z-axis
void projection(float coeff = 0.f);   // coeff = -1/c

// transfer to [(x, y), (x+w, y+h)] from [(-1, -1), (1, 1)]
void viewport(int x, int y, int w, int h);


class BaseShader {

public:
    virtual ~BaseShader();
    // vertex shader 
    virtual Vec4f vertex(int iface, int nthvert, Model* model) = 0;
    // fragment shader
    virtual bool fragment(Vec3f bary, TGAColor& c, Model* model) = 0;
};

class OnlyTexShader : public BaseShader {
// 正交的shader
public:
    mat<2, 3, float> triangle_uvs; 

    virtual Vec4f vertex(int iface, int nthvert, Model* model);

    virtual bool fragment(Vec3f bary, TGAColor& c, Model* model);
};

class CommonShader : public BaseShader {

public:
    mat<2, 3, float> varying_uv;
    mat<4, 3, float> varying_triangle;   // 齐次坐标
    mat<3, 3, float> varying_norm;
    mat<3, 3, float> back_homo_triangle;

    virtual Vec4f vertex(int iface, int nthvert, Model* model);

    virtual bool fragment(Vec3f bary, TGAColor& c, Model* model);
};

// void triangle(Vec3f* pts, BaseShader& shader, TGAImage& image, float* zbuffer, Model* model);
void triangle(mat<4, 3, float>& pts, BaseShader& shader, TGAImage& image, float* zbuffer, Model* model);

Vec3f bary_centric(Vec2f A, Vec2f B, Vec2f C, Vec2i P);
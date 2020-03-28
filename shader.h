#pragma once

#include "tgaimage.h"
#include "geometry.h"
#include "model.h"

const int width  = 800;
const int height = 800;

Vec3f light_dir(1, 1, 1);
Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 1);
Vec3f up(0, 1, 0);

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

// three important matrix
void viewport(int x, int y, int w, int h);
void projection(float coeff = 0.f);   // coeff = -1/c
void lokkat(Vec3f eye, Vec3f center, Vec3f up);

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

    virtual Vec4f vertex(int iface, int nthvert, Model* model) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        
        Vec4f nm_embeding = embed<4, 3, float>(model->normal(iface, nthvert), 0.f);
        Vec3f transposed_norm = proj<3, 4, float>((Projection * ModelView).invert_transpose() * nm_embeding);

        varying_norm.set_col(nthvert, transposed_norm);

        // homogeneous coordinates
        Vec4f homogeneous_vertex = Projection * ModelView * embed<4, 3, float>(model->vert(iface, nthvert), 1.f);
        varying_triangle.set_col(nthvert, homogeneous_vertex);

        back_homo_triangle.set_col(nthvert, proj<3, 4, float>(homogeneous_vertex / homogeneous_vertex[3]));

        return homogeneous_vertex;
    }

    bool fragment(Vec3f bary, TGAColor& c, Model* model) {
        

    }
};

void triangle(Vec3f* pts, BaseShader& shader, TGAImage& image, float* zbuffer, Model* model);

Vec3f bary_centric(Vec2f A, Vec2f B, Vec2f C, Vec3f P);
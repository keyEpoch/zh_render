#include <cmath>
#include <limits>
#include "shader.h"


Vec3f light_dir(1, 1, 1);
Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 1);
Vec3f up(0, 1, 0);

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

BaseShader::~BaseShader() {}

Vec4f OnlyTexShader::vertex(int iface, int nthvert, Model* model) {
    triangle_uvs.set_col(nthvert, model->uv(iface, nthvert));

    mat<1, 4, float> trash;
    return trash[0];
}
    
bool OnlyTexShader::fragment(Vec3f bary, TGAColor& c, Model* model) {
    // 2x3 3x1 
    Vec2f uv = triangle_uvs * bary;  // override * between mat and vec
    // between mat and vec will cause fucking ambiguous fault
    // Vec2f uv;
    // uv[0] = triangle_uvs[0] * bary;
    // uv[1] = 1.f - uv[1];
    c = model->diffuse(uv);
    return false;
}

Vec4f CommonShader::vertex(int iface, int nthvert, Model* model) {
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

bool CommonShader::fragment(Vec3f bary, TGAColor& c, Model* model) {
    Vec3f bn = (varying_norm * bary).normalize();
    Vec2f uv = varying_uv * bary;

    mat<3, 3, float> A;
    A[0] = back_homo_triangle.col(1) - back_homo_triangle.col(0);
    A[1] = back_homo_triangle.col(2) - back_homo_triangle.col(0);
    A[2] = bn;

    mat<3, 3, float> AI = A.invert();

    Vec3f i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
    Vec3f j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);

    mat<3,3,float> B;
    B.set_col(0, i.normalize());
    B.set_col(1, j.normalize());
    B.set_col(2, bn);

    Vec3f n = (B * model->normal(uv)).normalize();

    float diff = std::max(0.f, n * light_dir);
    // 算那么多就是为了计算光强
    c = model->diffuse(uv) * diff;

    return false;

}


void triangle(mat<4, 3, float>& pts, BaseShader& shader, TGAImage& image, float* zbuffer, Model* model) {

}

/*
// this method to fill color in triangle can run parallel, so can us gpu for multi-threads
void triangle(Vec3f* pts, BaseShader& shader, TGAImage& image, float* zbuffer, Model* model) {
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    
    mat<3, 2, float> pts2;
    for (int i = 0; i < 3; ++i) 
        pts2.set_row(i, pts[i]);

    
    // pts coords are screen coords
    for (int i = 0; i < 3; i++) {

        // for (int j = 0; j < 2; j++) { 
        //     // it will raise an error: expression is not assignable
        //     bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]));
        //     bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        
        bboxmin.x = std::max(0.f,      std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0.f,      std::min(bboxmin.y, pts[i].y));
        
        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    } // get the minimum region, for I need to judge if the points 
      // in the region is contained by the triangle
     
    TGAColor color;
    Vec3f P;  // P is supposed to be inside triangle
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bary = bary_centric(pts2[0], pts2[1], pts2[2], P);   // bary_centric 就是用来判断某点是不是在三角形内
            // judge if triangle contains P 
            if (bary.x < 0 || bary.y < 0 || bary.z < 0) 
                continue;
            
            P.z = 0;
            shader.fragment(bary, color, model);
            for (int i=0; i<3; i++) 
                // P.z得到的就是 P 的z坐标
                // 因为bary_centric得到的就是 (1-u-v, u, v)
                // P = (1 - u - v) * A + u * B + v * C
                P.z += pts[i][2]*bary[i];   

            if (zbuffer[int(P.x+P.y*width)] < P.z) {
                zbuffer[int(P.x+P.y*width)] = P.z;

                image.set(P.x, P.y, color);
            }
        }
    }
}
*/

Vec3f bary_centric(Vec2f A, Vec2f B, Vec2f C, Vec3f P) {
    Vec3f s[2];
    Vec3f ret;
    for (int i=2; i--; ) {
        s[i][0] = C[i]-A[i];
        s[i][1] = B[i]-A[i];
        s[i][2] = A[i]-P[i];
    }
    Vec3f u = cross_product(s[0], s[1]);
    if (std::abs(u[2])>1e-4) {   // u[2] 一定不能为 0
        ret.x = 1.f-(u.x+u.y)/u.z;
        ret.y = u.y/u.z;
        ret.z = u.x/u.z;
        return ret;
    }
    return Vec3f(-1,1,1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

/* three important matrix */
// transfer eye(change the O point)
void lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye-center).normalize();
    Vec3f x = cross_product(up,z).normalize();
    Vec3f y = cross_product(z,x).normalize();
    Matrix Minv = Matrix::identity();
    Matrix Tr   = Matrix::identity();
    for (int i=0; i<3; i++) {
        Minv[0][i] = x[i];
        Minv[1][i] = y[i];
        Minv[2][i] = z[i];
        Tr[i][3] = -center[i];
    }
    ModelView = Minv*Tr;
}

// after lookat(), the camera is always on z-axis
void projection(float coeff) {   // coeff = -1/c
    Projection = Matrix::identity();
    Projection[3][2] = coeff;
}   

// transfer to [(x, y), (x+w, y+h)] from [(-1, -1), (1, 1)]
void viewport(int x, int y, int w, int h) {
    Viewport = Matrix::identity();
    Viewport[0][3] = x+w/2.f;
    Viewport[1][3] = y+h/2.f;
    Viewport[2][3] = 1.f;
    Viewport[0][0] = w/2.f;
    Viewport[1][1] = h/2.f;
    Viewport[2][2] = 0;
}
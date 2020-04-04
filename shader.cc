#include <cmath>
#include <limits>
#include "shader.h"


Vec3f light_dir(0.f, 0.f, -1.f);
Vec3f eye(0, 0, -3);
Vec3f center(0, 0, 0);
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

    // varying_norm.set_col(nthvert, proj<3>((Projection*ModelView).invert_transpose()*embed<4>(model->normal(iface, nthvert), 0.f)));
    
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
    // 算那么多就是为了计算光强diff

    c = model->diffuse(uv) * diff;

    return false;

}


void triangle(mat<4, 3, float>& clipc, BaseShader& shader, TGAImage& image, float* zbuffer, Model* model) {
    
    mat<3,4,float> pts = (Viewport*clipc).transpose(); // transposed to ease access to each of the points
    mat<3,2,float> pts2;
    Vec4f tmp;
    Vec2f tmp2;
    for (int i=0; i<3; i++) 
        pts2[i] = proj<2, 4, float>(pts[i]/pts[i][3]);

    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts2[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts2[i][j]));
        }
    }
    Vec2i P;
    TGAColor color;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            Vec3f bc_screen  = bary_centric(pts2[0], pts2[1], pts2[2], P);
            Vec3f bc_clip    = Vec3f(bc_screen.x/pts[0][3], bc_screen.y/pts[1][3], bc_screen.z/pts[2][3]);
            bc_clip = bc_clip/(bc_clip.x+bc_clip.y+bc_clip.z);
            float frag_depth = clipc[2]*bc_clip;
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0 || zbuffer[P.x+P.y*image.get_width()]>frag_depth) continue;
            bool discard = shader.fragment(bc_clip, color, model);
            if (!discard) {
                zbuffer[P.x+P.y*image.get_width()] = frag_depth;
                image.set(P.x, P.y, color);
            }
        }
    }
}


Vec3f bary_centric(Vec2f A, Vec2f B, Vec2f C, Vec2i P) {
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
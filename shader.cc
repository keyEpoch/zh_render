#include <cmath>
#include <limits>
#include "shader.h"


Vec3f light_dir(1, 1, 0);
Vec3f eye(1, 1, 4);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

float* shadow_buffer = NULL;
float* z_buffer = NULL;

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
    B.set_col(2, bn);     // B就是TBN矩阵，tangent的z轴就是由三个端点的normal和bary算出来的normal

    // TBN矩阵，用三个点的normal，uv，vertex，外加三角形内部的一点P就可以求出来了

    // normal map 加载的是tangent坐标系的 normal 向量
    // 这里是将每个点的normal，都从tangent坐标系转到跟light_dir一样的坐标系下去
    Vec3f n = (B * model->normal(uv)).normalize();   

    float diff = std::max(0.f, n * light_dir);
    // 算那么多就是为了计算光强diff

    c = model->diffuse(uv) * diff;

    return false;
}

Vec4f DepthShader::vertex(int iface, int nthvert, Model* model) {
    
    Vec4f gl_vertex = embed<4, 3, float>(model->vert(iface, nthvert), 1.f);

    gl_vertex = Viewport*Projection*ModelView*gl_vertex;
    // std::cout << gl_vertex << std::endl;
    varying_triangle.set_col(nthvert, embed<3, 4, float>(gl_vertex/gl_vertex[3]));
    
    return gl_vertex;
}

bool DepthShader::fragment(Vec3f bary, TGAColor& color, Model* model) {
    Vec3f p = varying_triangle*bary;
    color = TGAColor(255, 255, 255) * (p.z / depth);
    
    return false;
}

Vec4f ShadowShader::vertex(int iface, int nthvert, Model* model) {
    varying_uv.set_col(nthvert, model->uv(iface, nthvert));
    Vec4f gl_vertex = Viewport*Projection*ModelView*embed<4, 3, float>(model->vert(iface, nthvert), 1.f);
    varying_triangle.set_col(nthvert, proj<3, 4, float>(gl_vertex/gl_vertex[3]));
    return gl_vertex;
}

bool ShadowShader::fragment(Vec3f bary, TGAColor& color, Model* model) {
    Vec2i uv = varying_uv * bary;

    // light_dir p
    Vec4f ld_p = uniform_Mshadow*embed<4, 3, float>(varying_triangle*bary, 1.f);
    ld_p = ld_p / ld_p[3];
    int idx = int(ld_p[0]) + int(ld_p[1]) * width;
    // define shadow
    float shadow = .3f + .7f * (ld_p[2] >= shadow_buffer[idx]);

    // transfer normal to eye coordinate
    Vec3f n = proj<3, 4, float>(uniform_MIT*embed<4, 3, float>(model->normal(uv)));
    n = n.normalize();
    Vec3f l = proj<3, 4, float>(uniform_M*embed<4, 3, float>(light_dir, 0));
    l = l.normalize();
    float tmp = n*l*2.f;
    Vec3f r = (n*tmp - l).normalize();   // reflect light

    float spec = pow(std::max(r.z, 0.f), model->specular(uv));
    float diff = std::max(0.f, n*l);
    TGAColor c = model->diffuse(uv);

    for (int i = 0; i < 3; ++i) 
        color[i] = std::min<float>(20 + c[i]*shadow*(1.2f*diff + .6f*spec), 255);
    return false;
}

/*
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
*/

void triangle(Vec4f* pts, BaseShader& shader, TGAImage& image, float* zbuffer, Model* model) {
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j]/pts[i][3]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j]/pts[i][3]);
        }
    }
    Vec2i P;
    Vec3f bary;
    int frag_depth;
    TGAColor color;
    
    for (P.x = bboxmin.x; P.x <= bboxmax.x; ++P.x) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; ++P.y) {
            bary = bary_centric(proj<2, 4, float>(pts[0]/pts[0][3]), proj<2, 4, float>(pts[1]/pts[1][3]), \
            proj<2, 4, float>(pts[2]/pts[2][3]), P);
            float z = pts[0][2] * bary.x + pts[1][2] * bary.y + pts[2][2] * bary.z;
            float w = pts[0][3] * bary.x + pts[1][3] * bary.y + pts[2][3] * bary.z;
            frag_depth = z / w;
            
            // judge if P in triangle && frag_depth
            if (bary.x < 0 || bary.y < 0 || bary.z < 0 || zbuffer[P.x + P.y * image.get_width()] > frag_depth)
                continue;
            bool discard = shader.fragment(bary, color, model);
            
            if (!discard) {
                zbuffer[P.x + P.y * image.get_width()] = frag_depth;
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
// transfer eye(change the o point)
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
// void viewport(int x, int y, int w, int h) {
//     Viewport = Matrix::identity();
//     Viewport[0][3] = x+w/2.f;
//     Viewport[1][3] = y+h/2.f;
//     Viewport[2][3] = 1.f;    // 不用考虑z轴的时候，就可以设置成1
//     Viewport[0][0] = w/2.f;
//     Viewport[1][1] = h/2.f;
//     Viewport[2][2] = 0;
// }

void viewport(int x, int y, int w, int h) {
    Viewport = Matrix::identity();
    Viewport[0][3] = x+w/2.f;
    Viewport[1][3] = y+h/2.f;
    Viewport[2][3] = depth/2.f;
    Viewport[0][0] = w/2.f;
    Viewport[1][1] = h/2.f;
    Viewport[2][2] = depth/2.f;
}
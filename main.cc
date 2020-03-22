// #include <vector>
// #include <cmath>
// #include <cstdlib>
#include <limits>
#include "tgaimage.h"
#include "model.h"
// #include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
Model *model = NULL;
const int width  = 800;
const int height = 800;

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

Vec3f cross_product(Vec3f v1, Vec3f v2) {
    return Vec3f(v1.y * v2.z - v1.z * v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

Vec3f bary_centric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f s[2];
    for (int i = 2; i--; ) {
        s[i].x = C[i] - A[i];
        s[i].y = B[i] - A[i];
        s[i].z = A[i] - P[i];
    }
    Vec3f u = cross_product(s[0], s[1]);
    
    if (std::abs(u[2]) > 1e-2) // 需要数学知识了
        return Vec3f(1.f - (u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
    return Vec3f(-1, 1, 1);
}

/*
// fill by drawing the line segments in triangle
void triangle_1(Vec2f t0, Vec2f t1, Vec2f t2, TGAImage &image, TGAColor color) {
    if (t0.y==t1.y && t0.y==t2.y) return; // i dont care about degenerate triangles
    if (t0.y>t1.y) std::swap(t0, t1);
    if (t0.y>t2.y) std::swap(t0, t2);
    if (t1.y>t2.y) std::swap(t1, t2);
    int total_height = t2.y-t0.y;
    for (int i=0; i<total_height; i++) {
        bool second_half = i>t1.y-t0.y || t1.y==t0.y;
        int segment_height = second_half ? t2.y-t1.y : t1.y-t0.y;
        float alpha = (float)i/total_height;
        float beta  = (float)(i-(second_half ? t1.y-t0.y : 0))/segment_height; // be careful: with above conditions no division by zero here
        Vec2i A =               t0 + (t2-t0)*alpha;
        Vec2i B = second_half ? t1 + (t2-t1)*beta : t0 + (t1-t0)*beta;
        if (A.x>B.x) std::swap(A, B);
        // 填充方式为水平线段填充
        for (int j=A.x; j<=B.x; j++) {   // draw lines horizontally
            image.set(j, t0.y+i, color); // attention, due to int casts t0.y+i != A.y
        }
    }
}
*/

void triangle_2(Vec3f* pts, float* zbuffer, TGAImage& image, TGAColor color) {
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    
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
     
    Vec3f P;  // P is supposed to be inside triangle
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc_screen = bary_centric(pts[0], pts[1], pts[2], P);
            // judge if triangle contains P 
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) 
                continue;
            P.z = 0;
            for (int i=0; i<3; i++) 
                P.z += pts[i][2]*bc_screen[i];   // 这里没懂

            if (zbuffer[int(P.x+P.y*width)]<P.z) {
                zbuffer[int(P.x+P.y*width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}

Vec3f world2screen(Vec3f v) {
    return Vec3f( (v.x+1)*width/2, (v.y+1)*height/2, v.z );
}

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/head.obj");
    }

    // to storage the orfer
    float* zbuffer = new float[width * height];
    for (int i = 0; i < width * height; ++i) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }

    TGAImage image(width, height, TGAImage::RGB);
    Vec3f light_dir(0, 0, -1);
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec3f pts[3];  // point to screen
        for (int i = 0; i < 3; ++i)
            pts[i] = world2screen(model->vert(face[i]));
        
        // draw triangle
        triangle_2(pts, zbuffer, image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
    }
        // Vec2i screen_coords[3];
        // Vec3f world_coords[3];
        // for (int j=0; j<3; j++) {
            // Vec3f v = model->vert(face[j]);
            // 注意：.obj文件中的坐标是在[-1, 1]上 normalize 过了的
            // 这时候要把其转换为以左下角为原点的屏幕坐标系
            // 世界坐标系按照z轴向指向屏幕外
            // screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.);
            // world_coords[j]  = v;
        // }
        // in geometry.h, overload operator ^, it means cross product 
        // Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]);
        // n.normalize();
        // float intensity = n*light_dir;
        // if (intensity>0) {
        //     triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        // }
    

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}
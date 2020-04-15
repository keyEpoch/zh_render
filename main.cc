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

    model = new Model(argv[1]);
    light_dir.normalize();

    // render round one: obtain shadow_buffer
    {
        TGAImage depthimage(width, height, TGAImage::RGB);
        shadow_buffer = new float[width*height];

        for (int i = width*height; i--; ) 
            shadow_buffer[i] = -std::numeric_limits<float>::max();

        lookat(light_dir, center, up);
        viewport(width/8, height/8, width*3/4, height*3/4);
        projection(0);
        
        // std::cout << ModelView <<std::endl;
		// std::cout << Viewport << std::endl;
		// std::cout << Projection << std::endl;

        DepthShader depthshader;
        // screencorrds: after Viewport 
        Vec4f screencoords[3];

        for (int i = 0; i < model->nfaces(); ++i) {
            for (int j = 0; j < 3; ++j) 
                screencoords[j] = depthshader.vertex(i, j, model);
                
            triangle(screencoords, depthshader, depthimage, shadow_buffer, model);
        }
        
        depthimage.flip_vertically();
        depthimage.write_tga_file("depth.tga");
    }
    Matrix M = Viewport*Projection*ModelView;
    {
        TGAImage shadowimage(width, height, TGAImage::RGB);
        z_buffer = new float[width*height];
        // redefine transfer matrices
        lookat(eye, center, up);
        projection(-1.f/(eye - center).norm());
        viewport(width/8, height/8, width*3/4, height*3/4);

        ShadowShader shadowshader(ModelView, (Projection*ModelView).invert_transpose(), M*(Viewport*Projection*ModelView).invert());
        Vec4f screen_coords[3];
        
        for (int i = 0; i < model->nfaces(); i++) {
            for (int j = 0; j < 3; j++) 
                screen_coords[j] = shadowshader.vertex(i, j, model);
            
            triangle(screen_coords, shadowshader, shadowimage, z_buffer, model);
        }
        shadowimage.flip_vertically(); // to place the origin in the bottom left corner of the image
        shadowimage.write_tga_file("framebuffer.tga");
    }

    // render round two: obtain shadow shader


    return 0;
}
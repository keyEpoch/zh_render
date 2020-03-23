#pragma once 
#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
	std::vector<Vec3f> verts_;
    std::vector<Vec2f> uv_;
    std::vector<Vec3f> norms_;
	std::vector<std::vector<Vec3i> > faces_;
    void load_texture(std::string filename, TGAImage& img);
    TGAImage diffusemap_;
    TGAImage normalmap_;
    TGAImage specularmap_;
public:
	Model(const char *filename);
	~Model();

	int nverts();
	int nfaces();

	Vec3f vert(int i);
    Vec3f vert(int iface, int nthvert);

    Vec2i uv(int iface, int nthvert);   // return cordinates in real image

	std::vector<int> face(int idx);

    Vec3f normal(Vec2i uv);
    Vec3f normal(int iface, int nthvert);

    TGAColor diffuse(Vec2i uv);
    float specular(Vec2i uv);
};

/*
 * main.cpp
 *
 *  Created on: 30 сент. 2021 г.
 *      Author: Семен Беляев
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <cmath>

constexpr int IMG_WIDTH = 1920;
constexpr int IMG_HEIGHT = 1080;
constexpr uint32_t COL_BLACK = 0xff000000;
constexpr uint32_t COL_WHITE = 0xffffffff;
constexpr double XS=20.0;
constexpr double YS=20.0;
constexpr double ZS=300.0;
constexpr double Pi=acos(-1);
constexpr double XMIN=-15.0;
constexpr double YMIN=-15.0;
constexpr double XMAX=15.0;
constexpr double YMAX=15.0;
constexpr double BIGSTEP=0.25;
constexpr double SMALLSTEP=0.0001;



#pragma pack(push, 1)
struct TGAHeader {
	uint8_t id_len; // длина идентификатора
	uint8_t pal_type; //тип палитры
	uint8_t img_type;
	uint8_t pal_desc[5];
	uint16_t pos_x;
	uint16_t pos_y;
	uint16_t width;
	uint16_t height;
	uint8_t depth;
	uint8_t img_desc;

};
struct BMPFileHeader {
	uint16_t signature;
	uint32_t file_size;
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t image_offset;
};
struct BMPInfoHeader{

	uint32_t header_length;
	uint32_t width;
	uint32_t height;
	uint16_t planes;
	uint16_t depth;
	uint32_t compression;
	uint32_t image_length;
	uint32_t xppm;
	uint32_t yppm;
	uint32_t colors;
	uint32_t colors_important;
};

#pragma pack(pop)
struct Image{
	std::vector<uint32_t> pixels;
	std::vector<int> horizon;
	int width;
	int height;

};
double sinc(double x) {
	if (x == 0.0)
		return 1.0;
	return sin(x) / x;
}

double my_func(double x, double y) {
return sinc(hypot(x,y));
}

void fill_rect(std::vector<uint32_t> &image, int x0, int y0, int w, int h,
		uint32_t color) {
	for (int x = x0; x < x0 + w; ++x)
		for (int y = y0; y < y0 + h; ++y)
			image[y * IMG_WIDTH + x] = color;
}

void convert_isometry(double x, double y, double z, int &px, int &py ){
	px=int(IMG_WIDTH/2-XS*cos(Pi/6.0)*x+YS*cos(Pi/6.0)*y);
				py=int(IMG_HEIGHT/2+XS*sin(Pi/6.0)*x+YS*sin(Pi/6.0)*y-ZS*z);
}
void image_init(Image &image, int width, int height, uint32_t background){
	image.width=width;
	image.height=height;
	image.pixels.resize(width*height);
	image.horizon.resize(width);
	for (auto&& p: image.pixels) p = background;
}

void image_reset_horizon(Image &image) {
for (auto &&y : image.horizon)
	y = image.height;
}
void image_draw_pixel(Image &image, int x, int y, uint32_t color){
	if(x<0 or x>=image.width) return;
	if(y>=image.horizon[x]) return;
	image.horizon[x]=y;
	if (y<0 or y>=image.height) return;
	image.pixels[y*image.width+x]=color;
}

void image_save_tga( Image &image, const char *filename) {
	TGAHeader header { };
	header.img_type = 2;
	header.width = image.width;
	header.height = image.height;
	header.depth = 32;
	header.img_desc = 0b00101000;
	std::ofstream tga_file { filename, std::ios::binary };
	tga_file.write(reinterpret_cast<char*>(&header), sizeof(header));
	tga_file.write(reinterpret_cast<char*>(&image.pixels[0]),
			image.pixels.size() * sizeof(image.pixels[0]));
	tga_file.close();
}

void image_save_bmp( Image &image, const char *filename) {
	BMPFileHeader bmpf { };
	BMPInfoHeader bmpi { };
	std::ofstream bmp_file { filename, std::ios::binary };
	bmpf.signature = 0x0001 * 'B' + 0x0100 * 'M';
	bmpf.file_size = sizeof(bmpf) + sizeof(bmpi)
			+ image.pixels.size() * sizeof(image.pixels[0]);
	bmpf.image_offset = sizeof(bmpf)
			+ image.pixels.size() * sizeof(image.pixels[0]);
	bmpi.header_length = sizeof(bmpi);
	bmpi.width = image.width;
	bmpi.height = -image.height;
	bmpi.depth = 32;
	bmpi.image_length = image.pixels.size() * sizeof(image.pixels[0]);
	bmpi.planes = 1;
	bmp_file.write(reinterpret_cast<char*>(&bmpf), sizeof(bmpf));
	bmp_file.write(reinterpret_cast<char*>(&bmpi), sizeof(bmpi));
	bmp_file.write(reinterpret_cast<char*>(&image.pixels[0]),
			bmpi.image_length);
	bmp_file.close();
}

int main() {
	Image image;

	image_init(image, IMG_WIDTH, IMG_HEIGHT,COL_BLACK);

	double x,y,z;
	int px,py;
	image_reset_horizon(image);
	for(y=YMAX; y>=YMIN; y-=BIGSTEP)
		for(x=XMAX;x>=XMIN;x-=SMALLSTEP){
			z=my_func(x,y);
			convert_isometry(x,y,z,px,py);
			image_draw_pixel(image,px,py,COL_WHITE);
		}
	image_reset_horizon(image);
	for(y=YMAX; y>=YMIN; y-=SMALLSTEP)
			for(x=XMAX;x>=XMIN;x-=BIGSTEP){
				z=my_func(x,y);
				convert_isometry(x,y,z,px,py);
				image_draw_pixel(image,px,py,COL_WHITE);
			}
	image_save_tga(image,"output.tga");

	image_save_bmp(image,"output.bmp");
	return 0;

}


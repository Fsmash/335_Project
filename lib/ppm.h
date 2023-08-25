#ifndef _PPM_H_
#define _PPM_H_
//ppm.h
//Read and write PPM files. Plain format.
//
typedef struct t_ppmimage {
	int width;
	int height;
	void *data;
} Ppmimage;

Ppmimage *ppm1GetImage(char *filename);
void ppm1CleanupImage(Ppmimage *image);
Ppmimage *ppm1CreateImage(int width, int height);
void ppm1SaveImage(Ppmimage *image, char *filename);
void ppm1ClearImage(Ppmimage *image, unsigned char color);
void ppm1Setpixel(Ppmimage *image, int x, int y, unsigned char val);
//
Ppmimage *ppm3CreateImage(int width, int height);
Ppmimage *ppm3GetImage(char *filename);
void ppm3ClearImage(Ppmimage *image, unsigned char r, unsigned char g, unsigned char b);
void ppm3SaveImage(Ppmimage *image, char *filename);
void ppm3Setpixel(Ppmimage *image, int x, int y, int channel, unsigned char val);
void ppm3CleanupImage(Ppmimage *image);
//
Ppmimage *ppm6CreateImage(int width, int height);
Ppmimage *ppm6GetImage(char *filename);
void ppm6ClearImage(Ppmimage *image, unsigned char r, unsigned char g, unsigned char b);
void ppm6SaveImage(Ppmimage *image, char *filename);
void ppm6Setpixel(Ppmimage *image, int x, int y, int channel, unsigned char val);
void ppm6CleanupImage(Ppmimage *image);

#endif



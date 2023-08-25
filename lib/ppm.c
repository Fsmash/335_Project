//cs371
//program: ppm.c
//author:  Gordon Griesel
//date:    Fall 2013
//Read and write PPM files.
//   P1 = 2-color
//   P3 = Plain format (ASCII)
//   P6 = Packed format (binary)
//Files can be used to store images for viewing or texture maps.
/*

sample image file is below:
------------------------------------------
P1
#5x10 the number 1
5 10
0 0 0 0 0
0 0 1 0 0
0 1 1 0 0
0 0 1 0 0
0 0 1 0 0
0 0 1 0 0
0 0 1 0 0
0 0 1 0 0
0 1 1 1 0
0 0 0 0 0
------------------------------------------

sample image file is below:
------------------------------------------
P3
#4x2 yellow image
4 2
255
255 255 0 255 255 0 255 255 0 255 255 0
255 255 0 255 255 0 255 255 0 255 255 0
------------------------------------------

sample image file is below:
------------------------------------------
P6
#4x2 dim-yellow image
4 4
255
zzAzzAzzAzzAzzAzzAzzAzzAzzAzzAzzAzzAzzAzzAzzAzzA
------------------------------------------
*/
//
//format inventor: Jef Poskanzer
//When writing, we will follow his suggested limit of 70-character lines.
//When reading, we allow for much longer lines.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ppm.h"

static Ppmimage *allocPpmimage() {

  Ppmimage *image = (Ppmimage *)malloc(sizeof(Ppmimage));
  if (!image) {
    printf("ERROR: out of memory\n");
    exit(EXIT_FAILURE);
  }

  return image;

}

#define FORMAT_HEADER_MAX 4
static FILE *openPpmfile(const char *filename, const char *mode) {

  FILE *fp = fopen(filename, "r");
  if (!fp) {
    printf("ERROR: cannot open file **%s** for reading.\n", filename);
    exit(EXIT_FAILURE);
  }

  char ts[4096]; // 4096 for memory page alignment
	fgets(ts, FORMAT_HEADER_MAX, fp);
	if (strncmp(ts, mode, 2) != 0) {
		printf("ERROR: File is not ppm format.\n");
		exit(EXIT_FAILURE);
	}

  return fp;

}

static void removePpmComments(FILE *fp) {

  int c;
  while ((c = fgetc(fp)) != EOF && c == '#') {
    while ((c = fgetc(fp)) != EOF && c != '\n');
  }
  ungetc(c, fp);

}

static Ppmimage *ppmCreateImage(const int width, const int height)
{

	Ppmimage *image = allocPpmimage();
	image->data = (unsigned char *)malloc(width * height * 3);

	if (!image->data) {
		printf("ERROR: no memory for image data.\n");
		exit(EXIT_FAILURE);
	}

	image->width = width;
	image->height = height;
	return image;

}  

static void ppmCleanupImage(Ppmimage *image)
{
	if (image != NULL) {
		if (image->data != NULL)
			free(image->data);
		free(image);
	}
}

// P1 Begin
Ppmimage *ppm1GetImage(char *filename)
{

	FILE *fp = openPpmfile(filename, "P1");
	Ppmimage *image = allocPpmimage();
  removePpmComments(fp);

	fscanf(fp, "%u %u", &image->width, &image->height);
	image->data = (unsigned char *)malloc(image->width * image->height * 3);
	if (!image->data) {
		printf("ERROR: no memory for image data.\n");
		exit(EXIT_FAILURE);
	}

  unsigned int color;
	unsigned char *p = (unsigned char *)image->data;
	for (int i=0; i < image->height * image->width; i++) {
    fscanf(fp, "%u", &color);
    *p = color;
    p++;
	}

	fclose(fp);
	return image;

}

void ppm1CleanupImage(Ppmimage *image)
{
  return ppmCleanupImage(image);
}

Ppmimage *ppm1CreateImage(int width, int height)
{
  return ppmCreateImage(width, height);
}  

void ppm1SaveImage(Ppmimage *image, char *filename)
{

  FILE *fpo = openPpmfile(filename, "P1");
	unsigned char *p = (unsigned char *)image->data;

	fprintf(fpo, "P1\n%d %d\n", image->width, image->height);
	for (int i = 0; i < image->height; i++) {
		for (int j = 0; j < image->width; j++) {
			fprintf(fpo, "%u ",*p);
			p++;
		}
		fprintf(fpo, "\n");
	}
	fclose(fpo);

}  

void ppm1ClearImage(Ppmimage *image, unsigned char color)
{
	unsigned char *data = (unsigned char *)image->data;
	for (int i=0; i < image->height * image->width; i++) {
		*data++ = color;
	}
}

void ppm1Setpixel(Ppmimage *image, int x, int y, unsigned char val) {
	unsigned char *data = (unsigned char *)image->data + (y * image->width + x);
	*data = val;
}
// P1 End

// P3 Begin
Ppmimage *ppm3GetImage(char *filename)
{

	FILE *fp = openPpmfile(filename, "P3");
	Ppmimage *image = allocPpmimage();
  removePpmComments(fp);

  int maxval;
	fscanf(fp, "%u%u%u", &image->width, &image->height, &maxval);
	image->data = (unsigned char *)malloc(image->width * image->height * 3);
	if (!image->data) {
		printf("ERROR: no memory for image data.\n");
		exit(EXIT_FAILURE);
	}

	unsigned char *p = (unsigned char *)image->data;
	unsigned int rgb[3];
	for (int i = 0; i < image->height * image->width; i++) {
    fscanf(fp, "%u%u%u", &rgb[0], &rgb[1], &rgb[2]);
    *p = rgb[0]; p++;
    *p = rgb[1]; p++;
    *p = rgb[2]; p++;
	}

	fclose(fp);
	return image;

}

void ppm3CleanupImage(Ppmimage *image)
{
  ppmCleanupImage(image);
}

Ppmimage *ppm3CreateImage(int width, int height)
{
  return ppmCreateImage(width, height);
}  

void ppm3SaveImage(Ppmimage *image, char *filename)
{

  FILE *fpo = openPpmfile(filename, "P3");
	unsigned char *p = (unsigned char *)image->data;
	fprintf(fpo, "P3\n%d %d\n%d\n", image->width, image->height, 255);
	for (int i = 0; i < image->height; i++) {
		for (int j = 0; j < image->width; j++) {
			fprintf(fpo, "%u %u %u ", *p, *(p+1), *(p+2));
			p += 3;
		}
		fprintf(fpo, "\n");
	}
	fclose(fpo);

}  

void ppm3ClearImage(Ppmimage *image, unsigned char red, unsigned char green, unsigned char blue)
{
	unsigned char *data = (unsigned char *)image->data;
	for (int i = 0; i < image->height * image->width; i++) {
		*data++ = red;
		*data++ = green;
		*data++ = blue;
	}
}

void ppm3Setpixel(Ppmimage *image, int x, int y, int channel, unsigned char val) {
	unsigned char *data = (unsigned char *)image->data + ((y * image->width * 3) + (x * 3) + channel);
	*data = val;
}
// P3 End

//
//
//
//The routines below are for P6 format files.
//They store the colors in binary, rather than digital.
//Some characters will be unprintable in this format.
//Sample image file is below:
//------------------------------------------
//P6
//#4x2 red image
//4 2
//255
//a..a..a..a..a..a..a..a..
//------------------------------------------
//
//
//

Ppmimage *ppm6GetImage(char *filename)
{

	FILE *fp = openPpmfile(filename, "P6");
	Ppmimage *image = allocPpmimage();
  removePpmComments(fp);

	// //get past any newline or carrage-return
  int maxval;
	fscanf(fp, "%u%u%u", &image->width, &image->height, &maxval);

	//get past any newline or carrage-return
  int c;
  while ((c = fgetc(fp)) != EOF && (c == 10 || c ==13)) {}
	ungetc(c, fp);

	image->data = (unsigned char *)malloc(image->width * image->height * 3);
	if (!image->data) {
		printf("ERROR: no memory for image data.\n");
		exit(EXIT_FAILURE);
	}

	unsigned char *p = (unsigned char *)image->data;
	for (int i = 0; i < (image->height * image->width); i++) {
    *p = fgetc(fp); p++;
    *p = fgetc(fp); p++;
    *p = fgetc(fp); p++;
	}

	fclose(fp);
	return image;

}

void ppm6CleanupImage(Ppmimage *image)
{
  ppmCleanupImage(image);
}

Ppmimage *ppm6CreateImage(int width, int height)
{
  return ppmCreateImage(width, height);
}  

void ppm6SaveImage(Ppmimage *image, char *filename)
{

	unsigned char *p = (unsigned char *)image->data;
  FILE *fpo = openPpmfile(filename, "P6");
	fprintf(fpo, "P6\n%d %d\n%d\n", image->width, image->height, 255);
	for (int i = 0; i < image->height; i++) {
    fputc(*p++, fpo);
    fputc(*p++, fpo);
    fputc(*p++, fpo);
	}
	fclose(fpo);

}

void ppm6ClearImage(Ppmimage *image, unsigned char red, unsigned char green, unsigned char blue)
{

	unsigned char *data = (unsigned char *)image->data;
	for (int i = 0; i < image->height * image->width; i++) {
		*data++ = red;
		*data++ = green;
		*data++ = blue;
	}

}

void ppm6Setpixel(Ppmimage *image, int x, int y, int channel, unsigned char val) {
	unsigned char *data = (unsigned char *)image->data + ((y * image->width * 3) + (x * 3) + channel);
	*data = val;
}


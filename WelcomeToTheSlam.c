//cs371 Graphics Fall 2014

//program: WelcomeToTheSlam.c
//authors:  Jerome Zeito and Bryant Dinh
//date:    11/19/2014
//
//This program demonstrates a basketball colliding with a trampoline 
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include "lib/fonts.h"
#include "lib/ppm.h"

#define rnd() (float)rand() / (float)RAND_MAX
#define MAX_CONTROL_POINTS 10000
#define PI 3.14159265358979323846264338327950

typedef float Vec[3];

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

void initXWindows(void);
void init_opengl(void);
void init_textures(void);
void init_springs(void);
void init_sphere(void);
void cleanupXWindows(void);
void check_resize(XEvent *e);
void check_mouse(XEvent *e);
void check_keys(XEvent *e);
void maintain_springs(void);
void maintain_bball(void);
void physics(void);
void render(void);

int height = 50, width = 50;
int done=0;
int xres=640, yres=480;
float cubeRot[3]={2.0,0.0,0.0};
float cubeAng[3]={0.0,0.0,0.0};

GLfloat LightAmbient[]  = {  0.0f, 0.0f, 0.0f, 1.0f };
GLfloat LightDiffuse[]  = {  1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightSpecular[] = {  0.5f, 0.5f, 0.5f, 1.0f };
GLfloat LightPosition[] = { 100.0f, 40.0f, 40.0f, 1.0f };

float sphereCenter[3];
float sphereRadius;
float sphereRadius2;

float ballPos[3];
float ballForce[3];
float ballVel[3]={0.001,0.0,0.001};

int collide = 0;

Ppmimage *bballImage = NULL;
Ppmimage *trampolineImage = NULL;
Ppmimage *backboardImage = NULL;
GLuint bballTextureId;
GLuint trampolineTextureId;
GLuint backboardTextureId;

struct Mass {
  float mass;
  float oomass;
  float pos[3];
  float vel[3];
  float force[3];
} mass[MAX_CONTROL_POINTS];

int nmasses = 0;

struct Spring {
  int mass[2];
  float length;
  float stiffness;
} springs[MAX_CONTROL_POINTS * 4];

int nsprings = 0;

int main(void)
{
  initXWindows();
  init_opengl();
  glEnable(GL_TEXTURE_2D);
  init_textures();
  init_springs();
  init_sphere();
  while(!done) {
    while(XPending(dpy)) {
      XEvent e;
      XNextEvent(dpy, &e);
      check_resize(&e);
      check_mouse(&e);
      check_keys(&e);
    }
    physics();
    render();
    glXSwapBuffers(dpy, win);
  }
  cleanupXWindows();
  cleanup_fonts();
  return 0;
}

void init_springs(void) 
{
  int i, j, n;
  float x, xlen, z, zlen;
  xlen = .5;
  x = -10.0;
  zlen = .5;
  z = 0.0;
  //Create Masses
  for (i = 0, n = 0; i < height; i++) { 
    for (j = 0; j < width; j++, n++) {
      mass[n].mass = 1.0;
      mass[n].oomass = 1.0 / mass[n].mass;
      mass[j + i * height].pos[0] = x;
      mass[j + i * width].pos[1] = -1.031;
      mass[n].vel[0] = 0.0;
      mass[n].vel[1] = 0.0;
      mass[n].force[0] = 0.0;
      mass[n].force[1] = 0.0;
      mass[j + i * height].pos[2] = z;
      mass[n].vel[2] = 0.0;
      mass[n].force[2] = 0.0;
      nmasses++;
      z -= zlen;
    }
    x += xlen;
    z = 0;
  }

  printf(" nmasses:  %d", nmasses );
  nsprings = 0;

  int k;
  //Create horizontal single-spaced springs
  for (i = 0; i < height; i++) {
    for (j = 0; j < width - 1; j++) {
      k = j + i * height;
      springs[nsprings].mass[0] = k; 
      springs[nsprings].mass[1] = k + 1; 
      springs[nsprings].length = fabs(mass[k + 1].pos[2] - mass[k].pos[2]);
      springs[nsprings].stiffness = rnd() * 0.1;
      nsprings++;
    }
  }

  //Create horizontal double-spaced springs
  for (i = 0; i < height; i++) {
    for (j = 0; j < width - 2; j++) {
      k = j + i * height;
      springs[nsprings].mass[0] = k; 
      springs[nsprings].mass[1] = k + 2; 
      springs[nsprings].length = fabs(mass[k + 2].pos[2] - mass[k].pos[2]);
      springs[nsprings].stiffness = 0.07;
      nsprings++;
    }
  }
  //Create horizontal triple-spaced springs
  for (i = 0; i < height; i++) {
    for (j = 0; j < width - 3; j++) {
      k = j + i * height;
      springs[nsprings].mass[0] = k; 
      springs[nsprings].mass[1] = k + 3; 
      springs[nsprings].length = fabs(mass[k + 3].pos[2] - mass[k].pos[2]);
      springs[nsprings].stiffness = 0.07;
      nsprings++;
    }
  }

  //Create verticle single-spaced springs
  for (i = 0; i < height - 1; i++) {
    for (j = 0; j < width; j++) {
      k = j + i * height;
      springs[nsprings].mass[0] = k; 
      springs[nsprings].mass[1] = k + width;
      springs[nsprings].length = fabs(mass[k + width].pos[0] - mass[k].pos[0]);
      springs[nsprings].stiffness = rnd() * 0.1;
      nsprings++;
    }
  }

  //Create verticle double-spaced springs
  for (i = 0; i < height - 2; i++) {
    for (j = 0; j < width; j++) {
      k = j + i * width;
      springs[nsprings].mass[0] = k; 
      springs[nsprings].mass[1] = k + (width * 2);
      springs[nsprings].length = fabs(mass[k + (width * 2)].pos[0] - mass[k].pos[0]);
      springs[nsprings].stiffness = 0.07;
      nsprings++;
    }
  }

  //Create verticle triple-spaced springs
  for (i = 0; i < height - 3; i++) {
    for (j = 0; j < width; j++) {
      k = j + i * height;
      springs[nsprings].mass[0] = k; 
      springs[nsprings].mass[1] = k + (width * 3) ;
      springs[nsprings].length = fabs(mass[k + (width * 3)].pos[0] - mass[k].pos[0]);
      springs[nsprings].stiffness = 0.07;
      nsprings++;
    }
  }
  //Create top left to bottom right diagonal springs
  for (i = 0; i < height; i++) {
    for (j = 0; j < width - 1; j++) {
      k = j + i * height;
      springs[nsprings].mass[0] = k;
      springs[nsprings].mass[1] = k + width + 1;
      springs[nsprings].length = sqrt(xlen * xlen + zlen * zlen);
      springs[nsprings].stiffness = 0.07;
      nsprings++;
    }
  }

  //Create top right to bottom left diagonal springs
  for (i = 0; i < height; i++) {
    for (j = 0; j < width - 1; j++) {
      k = j + i * height;
      springs[nsprings].mass[0] = k + 1;
      springs[nsprings].mass[1] = k + width;
      springs[nsprings].length = sqrt(fabs(xlen * xlen + zlen * zlen));
      springs[nsprings].stiffness = 0.07;
      nsprings++;
    }
  }

  printf(" nsprings :  %d", nsprings );
}

void LightedCylinder()
{
  int i;
  glBegin(GL_QUAD_STRIP);
  for (i = 0; i <= 360; i++) {
    glColor3f(1.0,0.5,0.0);
    glVertex3f(10 + 2 * cos(i),5,-8 + 2 * sin(i));
    glVertex3f(10 + 2 * cos(i),4.8,-8 + 2 * sin(i));
  }
  glEnd();
  glColor3f(1.0,1.0,1.0);

  glBindTexture(GL_TEXTURE_2D, backboardTextureId);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(7.0, 9.0, -2.5);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(11.0, 9.0, -2.5);
  glTexCoord2f(1.0, 1.0);
  glVertex3f(11.0, 5.0, -2.5);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(7.0, 5.0, -2.5);
  glEnd();
}

void init_textures(void)
{
  //load the images file into a ppm structure.
  bballImage = ppm6GetImage("./images/bball.ppm");
  trampolineImage = ppm6GetImage("./images/trampoline.ppm");
  backboardImage = ppm6GetImage("./images/backboard.ppm"); 

  //create opengl texture elements for basketball
  glGenTextures(1, &bballTextureId);
  int w = bballImage->width;
  int h = bballImage->height;
  glBindTexture(GL_TEXTURE_2D, bballTextureId);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, bballImage->data);

  //create opengl texture elements for trampoline
  glGenTextures(1, &trampolineTextureId);
  w = trampolineImage->width;
  h = trampolineImage->height;

  glBindTexture(GL_TEXTURE_2D, trampolineTextureId);

  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, trampolineImage->data);

  //create opengl texture elements for backboard 
  glGenTextures(1, &backboardTextureId);
  w = backboardImage->width;
  h = backboardImage->height;

  glBindTexture(GL_TEXTURE_2D, backboardTextureId);

  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, backboardImage->data);
}

void init_sphere(void) {

  sphereCenter[0] = 0.0;
  sphereCenter[1] = 10.0;
  sphereCenter[2] = -10.0;
  sphereRadius = 1;
  printf("sphere : %f",sphereRadius); 
  sphereRadius2 = 30.0;
}

void cleanupXWindows(void)
{
  XDestroyWindow(dpy, win);
  XCloseDisplay(dpy);
}

void set_title(void)
{
  //Set the window title bar.
  XMapWindow(dpy, win);
  XStoreName(dpy, win, "CS371 - OpenGL Blank Animation Template Under XWindows");
}

void setup_screen_res(const int w, const int h)
{
  xres = w;
  yres = h;
}

void initXWindows(void)
{
  Window root;
  GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
  XVisualInfo *vi;
  Colormap cmap;
  XSetWindowAttributes swa;

  setup_screen_res(640, 480);
  dpy = XOpenDisplay(NULL);
  if(dpy == NULL) {
    printf("\n\tcannot connect to X server\n\n");
    exit(EXIT_FAILURE);
  }
  root = DefaultRootWindow(dpy);
  vi = glXChooseVisual(dpy, 0, att);
  if(vi == NULL) {
    printf("\n\tno appropriate visual found\n\n");
    exit(EXIT_FAILURE);
  } 

  cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
  swa.colormap = cmap;
  swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
    StructureNotifyMask | SubstructureNotifyMask;
  win = XCreateWindow(dpy, root, 0, 0, xres, yres, 0,
                      vi->depth, InputOutput, vi->visual,
                      CWColormap | CWEventMask, &swa);
  set_title();
  glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
  glXMakeCurrent(dpy, win, glc);

  srand(time(NULL));
}

void reshape_window(int width, int height)
{
  //window has been resized.
  setup_screen_res(width, height);

  glViewport(0, 0, (GLint)width, (GLint)height);
  glMatrixMode(GL_PROJECTION); glLoadIdentity();
  glMatrixMode(GL_MODELVIEW); glLoadIdentity();
  glOrtho(0, xres, 0, yres, -1, 1);
  set_title();
}

void init_opengl(void)
{
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);       // This Will Clear The Background Color To Black
  glClearDepth(1.0);              // Enables Clearing Of The Depth Buffer
  glDepthFunc(GL_LESS);                   // The Type Of Depth Test To Do
  glEnable(GL_DEPTH_TEST);                // Enables Depth Testing
  glShadeModel(GL_SMOOTH);            // Enables Smooth Color Shading
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();               // Reset The Projection Matrix
  gluPerspective(45.0f,(GLfloat)xres/(GLfloat)yres,0.1f,100.0f);
  glMatrixMode(GL_MODELVIEW);

  //Enable this so material colors are the same as vert colors.
  glEnable(GL_COLOR_MATERIAL);
  glEnable( GL_LIGHTING );
  glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);
  glLightfv(GL_LIGHT0, GL_POSITION,LightPosition);
  glEnable(GL_LIGHT0);

}

void check_resize(XEvent *e)
{
  //The ConfigureNotify is sent by the
  //server if the window is resized.
  if (e->type != ConfigureNotify)
    return;
  XConfigureEvent xce = e->xconfigure;
  if (xce.width != xres || xce.height != yres) {
    //Window size did change.
    reshape_window(xce.width, xce.height);
  }
}

void check_mouse(XEvent *e)
{
  //Did the mouse move?
  //Was a mouse button clicked?
  static int savex = 0;
  static int savey = 0;
  //
  if (e->type == ButtonRelease) {
    return;
  }
  if (e->type == ButtonPress) {
    if (e->xbutton.button==1) {
      //Left button is down
    }
    if (e->xbutton.button==3) {
      //Right button is down
    }
  }
  if (savex != e->xbutton.x || savey != e->xbutton.y) {
    //Mouse moved
    savex = e->xbutton.x;
    savey = e->xbutton.y;
  }
}

void check_keys(XEvent *e)
{
  if (e->type == KeyPress) {
    int key = XLookupKeysym(&e->xkey, 0);
    switch (key) {

      case XK_0: 
        ballVel[0] = 0.001;
        ballVel[1] = 0.0;
        ballVel[2] = 0.001;
        sphereCenter[0] = 0.0;
        sphereCenter[1] = 10.0;
        sphereCenter[2] = -10.0;
        break;

      case XK_1: 
        ballVel[0] = .1;
        ballVel[1] = 0.1;
        ballVel[2] = 0.001;
        sphereCenter[0] = -11.0;
        sphereCenter[1] = 10.0;
        sphereCenter[2] = -10.0;
        break;

      case XK_2: 
        ballVel[0] = 0.0;
        ballVel[1] = -0.1;
        ballVel[2] = 0.1;
        sphereCenter[0] = 0.0;
        sphereCenter[1] = 10.0;
        sphereCenter[2] = -20.0;
        break;

      case XK_3: 
        ballVel[0] = -0.1;
        ballVel[1] = -0.2;
        ballVel[2] = 0.001;
        sphereCenter[0] = 5.0;
        sphereCenter[1] = 7.0;
        sphereCenter[2] = -20.0;
        break;

      case XK_4: 
        ballVel[0] = .1;
        ballVel[1] = 0.0;
        ballVel[2] = -0.1;
        sphereCenter[0] = -5.0;
        sphereCenter[1] = 10.0;
        sphereCenter[2] = -10.0;
        break;

      case XK_5: 
        ballVel[0] = .1;
        ballVel[1] = 0.0;
        ballVel[2] = 0.001;
        sphereCenter[0] = -5.0;
        sphereCenter[1] = 10.0;
        sphereCenter[2] = -10.0;
        break;

      case XK_6: 
        ballVel[0] = -.1;
        ballVel[1] = 0.0;
        ballVel[2] = -0.001;
        sphereCenter[0] = -5.0;
        sphereCenter[1] = 10.0;
        sphereCenter[2] = -10.0;
        break;

      case XK_Escape:
        done=1;
        return;
    }
  }
}

void DrawBasketball()
{
  static int firsttime=1;
  //16 longitude lines.
  //8 latitude levels.
  //3 values each: x,y,z.
  int i, j, i2, j2, j3;
  static float verts[9][16][3];
  static float norms[9][16][3];
  static float    tx[9][17][2];
  if (firsttime) {
    //build basketball vertices here. only once!
    firsttime=0;
    float circle[16][2];
    float angle=0.0, inc = (PI * 2.0) / 16.0;
    for (i=0; i<16; i++) {
      circle[i][0] = cos(angle);
      circle[i][1] = sin(angle);
      angle += inc;
      printf("circle[%2i]: %f %f\n", i, circle[i][0], circle[i][1]);
    }
    //use the circle points to build all vertices.
    //8 levels of latitude...
    for (i=0; i<=8; i++) {
      for (j=0; j<16; j++) {
        verts[i][j][0] = circle[j][0] * circle[i][1]; 
        verts[i][j][2] = circle[j][1] * circle[i][1];
        verts[i][j][1] = circle[i][0];
        norms[i][j][0] = verts[i][j][0]; 
        norms[i][j][1] = verts[i][j][1];
        norms[i][j][2] = verts[i][j][2];
        tx[i][j][0] = (float)j / 16.0;
        tx[i][j][1] = (float)i / 8.0;
      }
      tx[i][j][0] = (float)j / 16.0;
      tx[i][j][1] = (float)i / 8.0;
    }
  }

  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glLightfv(GL_LIGHT0, GL_POSITION,LightPosition);
  glPushMatrix();
  glTranslatef(sphereCenter[0],sphereCenter[1],sphereCenter[2]);
  glRotatef(cubeAng[2],0.0f,0.0f,1.0f);

  //draw the ball, made out of quads...
  glColor3f(1.0, 1.0, 1.0);
  glBindTexture(GL_TEXTURE_2D, bballTextureId);
  glBegin(GL_QUADS);
  for (i=0; i<8; i++) {
    for (j=0; j<16; j++) {
      i2 = i+1;
      j2 = j+1;
      if (j2 >= 16) j2 -= 16;
      j3 = j+1;
      glNormal3fv(norms[i ][j ]); glTexCoord2fv(tx[i ][j ]); glVertex3fv(verts[i ][j ]);
      glNormal3fv(norms[i2][j ]); glTexCoord2fv(tx[i2][j ]); glVertex3fv(verts[i2][j ]);
      glNormal3fv(norms[i2][j2]); glTexCoord2fv(tx[i2][j3]); glVertex3fv(verts[i2][j2]);
      glNormal3fv(norms[i ][j2]); glTexCoord2fv(tx[i ][j3]); glVertex3fv(verts[i ][j2]);
    }
  }
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(sphereCenter[0], 0.0, sphereCenter[2]);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glColor4ub(255,255,255,80);

  glDisable (GL_BLEND);
  glPopMatrix();
  //draw trampoline 
  glColor3f(1.0,1.0,1.0);
  for (i = 0; i < height - 1; i++) {
    glBindTexture(GL_TEXTURE_2D, trampolineTextureId);
    glBegin(GL_TRIANGLE_STRIP);
    for (j = 0; j < width - 1; j++) {  
      glLoadIdentity();
      glNormal3f(0.0,1.0,0.0);
      glTexCoord2f((float)j/(float)width, (float)i/(float)height);
      glVertex3f(mass[j + i * height].pos[0], mass[j + i * height].pos[1],mass[j + i * height].pos[2]);
      glTexCoord2f((float)j/(float)width, (float)(i + 1)/(float)height);
      glVertex3f(mass[j + i * height + width].pos[0], mass[j + i * height + width].pos[1],mass[j + i * height + width].pos[2]);
    }
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);

  }

}

void physics(void)
{
  maintain_springs();
  maintain_bball();
}

void maintain_springs(void)
{
  float sForce[3];
  float sVector[3];
  float bpenalty;
  float mdist, oodist, factor;
  int m0, m1;
  int i;

  for (i = 0; i < nmasses; i++) {

    if ( (i > 0 && i < width - 1) || (i > height * width - width && i < height * width - 1) || (i % width == 0) || ((i+1) % width == 0 )) {

      mass[i].vel[0] = 0.0;
      mass[i].vel[1] = 0.0;
      mass[i].vel[2] = 0.0;
      mass[i].force[0] = 0.0;
      mass[i].force[1] = 0.0;
      mass[i].force[2] = 0.0;

    }

    else {

      mass[i].vel[0] += mass[i].force[0] * mass[i].oomass;
      mass[i].vel[1] += mass[i].force[1] * mass[i].oomass;
      mass[i].vel[2] += mass[i].force[2] * mass[i].oomass;

      mass[i].pos[0] += mass[i].vel[0];
      mass[i].pos[1] += mass[i].vel[1];
      mass[i].pos[2] += mass[i].vel[2];
      mass[i].force[0] = 0;
      mass[i].force[1] = 0;
      mass[i].force[2] = 0;

      //There is a collision sphere in the scene
      float d0,d1,d2,dist;
      float b0,b1,b2,bdist;
      float bdiff;
      d0 = mass[i].pos[0] - sphereCenter[0];
      d1 = mass[i].pos[1] - sphereCenter[1];
      d2 = mass[i].pos[2] - sphereCenter[2];
      dist = d0*d0+d1*d1+d2*d2;
      bdist = sqrt(dist);
      if (bdist < sphereRadius) {
        bdist = (bdist<1.0) ? 1.0 : bdist;
        b0 /= bdist;
        b1 /= bdist;
        b2 /= bdist;

        bdiff = sphereRadius - bdist;
        bpenalty = bdiff * 100.0; 

        collide = 1;
        ballForce[0] = b0 * bpenalty;  
        ballForce[1] = b1 * bpenalty;  
        ballForce[2] = b2 * bpenalty;  

        mass[i].force[0] = -b0 * bpenalty;
        mass[i].force[1] = -b1 * bpenalty;
        mass[i].force[2] = -b2 * bpenalty;

      } 
      if (dist <= sphereRadius) {
        //Ball has hit the trampoline!
        //Apply a penalty that pushes  trampoline away from the center of the bball.
        static const float penalty = 0.7;
        dist = 1.0 / sqrt(dist);
        d0 *= dist;
        d1 *= dist;
        d2 *= dist;
        mass[i].force[0] = d0 * penalty;
        mass[i].force[1] = d1 * penalty;
        mass[i].force[2] = d2 * penalty;
        //Apply some drag caused by the rough surface of the sphere
        //Stickiness
        mass[i].vel[0] *= 0.95;
        mass[i].vel[1] *= 0.95;
        mass[i].vel[2] *= 0.95;
      }

    }

    mass[i].vel[0] *= 0.92; 
    mass[i].vel[1] *= 0.92; 
    mass[i].vel[2] *= 0.92; 

  }

  for (i = 0; i < nsprings; i++) {
    m0 = springs[i].mass[0];
    m1 = springs[i].mass[1];

    sVector[0] = mass[m0].pos[0] - mass[m1].pos[0];
    sVector[1] = mass[m0].pos[1] - mass[m1].pos[1];
    sVector[2] = mass[m0].pos[2] - mass[m1].pos[2];

    mdist = sqrt(sVector[0] * sVector[0] + sVector[1] * sVector[1] + sVector[2] * sVector[2]);

    if (mdist == 0.0) mdist = 0.1;

    oodist = 1.0 / mdist;
    sVector[0] *= oodist;
    sVector[1] *= oodist;
    sVector[2] *= oodist;

    factor = -(mdist - springs[i].length) * springs[i].stiffness;
    sForce[0] = sVector[0] * factor;
    sForce[1] = sVector[1] * factor;
    sForce[2] = sVector[2] * factor;

    mass[m0].force[0] += sForce[0];
    mass[m0].force[1] += sForce[1];
    mass[m0].force[2] += sForce[2];

    mass[m1].force[0] -= sForce[0];
    mass[m1].force[1] -= sForce[1];
    mass[m1].force[2] -= sForce[2];

    sForce[0] = (mass[m1].vel[0] - mass[m0].vel[0]) * 0.002;
    sForce[1] = (mass[m1].vel[1] - mass[m0].vel[1]) * 0.002;
    sForce[2] = (mass[m1].vel[2] - mass[m0].vel[2]) * 0.002;

    mass[m0].force[0] += sForce[0];
    mass[m0].force[1] += sForce[1];
    mass[m0].force[2] += sForce[2];

    mass[m1].force[0] -= sForce[0];
    mass[m1].force[1] -= sForce[1];
    mass[m1].force[2] -= sForce[2];
  }
}

void maintain_bball(void) {

  //bounce ball...
  sphereCenter[0] += ballVel[0];
  sphereCenter[1] += ballVel[1];
  sphereCenter[2] += ballVel[2];
  ballVel[0] *= 0.999;
  ballVel[1] *= 0.999;
  ballVel[2] *= 0.999;

  int grav=1;
  if (collide && ballVel[1] < 0.0) {

    ballVel[0] += ballForce[0]; 
    ballVel[2] += ballForce[2];
    ballVel[1] = -ballVel[1];
    ballVel[1] += ballForce[1];
    grav=0;
    //rotate basketball
    cubeRot[2] = rnd() * ballVel[0] * -40.0;
    collide = 0; 
  }

  // zero out gravity when distance between sphere radius and distance between center and mass equals. 
  if (grav)
    ballVel[1] -= 0.01;

  int i;
  for (i=0; i<3; i++) {
    cubeAng[i] += cubeRot[i];
  }


}

void render(void)
{
  glClear(GL_DEPTH_BUFFER_BIT);
  glDepthFunc(GL_LESS);                   // The Type Of Depth Test To Do
  glEnable(GL_DEPTH_TEST);                // Enables Depth Testing
  glShadeModel(GL_SMOOTH);            // Enables Smooth Color Shading
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();               // Reset The Projection Matrix
  gluPerspective(45.0f,(GLfloat)xres/(GLfloat)yres,0.1f,100.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();               // Reset The Projection Matrix
  gluLookAt(-2.0,10.0,14.0,0.0,-1.031,-10.0,0.0,1.0,0.0); 
  DrawBasketball();
  LightedCylinder();
}

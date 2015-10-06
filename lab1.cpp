//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
/* Program: Homework 1
 * Author: Jeremy Martinez
 * Purpose: This program will display the waterfall model by showing each step of the process
 * and when either 'B' is pushed or the mouse is on screen then particles will fall on the
 * blocks and create a waterfall.
*/

#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <X11/Xutil.h>
extern "C" {
	#include "fonts.h"
}
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 25000
#define GRAVITY 0.1

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

struct Game {
	Shape box[5];
	Shape circle;
	Particle particle[MAX_PARTICLES];
	int n;
	int lastMousex, lastMousey;
	bool bubbler;
};


//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object
	Game game;
	game.n=0;
	game.bubbler = false;

	//declare box shapes
	for(int j = 0; j<5; j++){
	    game.box[j].width = 100;
	    game.box[j].height = 15;
	    game.box[j].center.x = 110 + j*120;
	    game.box[j].center.y = 500 - j*80;
   	 }

	game.circle.center.x = 800;
	game.circle.center.y = -15;
	game.circle.radius = 150;

	//start animation
	while(!done) {
		while(XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		movement(&game);
		render(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void) {
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void) {
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
							ButtonPress | ButtonReleaseMask |
							PointerMotionMask |
							StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
					InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}



void init_opengl(void)
{
        
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

#define rnd() (float)rand() /  (float)RAND_MAX

void makeParticle(Game *game, int x, int y) {
	if (game->n >= MAX_PARTICLES)
		return;
	std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y = rnd()*1.0 - 0.5;
	p->velocity.x =  1.0 + rnd()*0.1;
	game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;
	//static int n = 0;

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;
			for (int i=0; i<10; i++)
				makeParticle(game, e->xbutton.x, y);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		int y = WINDOW_HEIGHT - e->xbutton.y;
		for (int i=0; i<10; i++)
		    makeParticle(game, e->xbutton.x, y);
		//if (++n < 10)
		//	return;
		game->lastMousex = e->xbutton.x;
		game->lastMousey = y;
	}
}

int check_keys(XEvent *e, Game *game)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_Escape) {
			return 1;
		}
		//You may check other keys
        if (e->type == KeyPress){
            int key = XLookupKeysym(&e->xkey, 0);
            if(key == XK_b){
		game->bubbler ^= 1;
            }
        } 
	}
	return 0;
}

void movement(Game *game)
{
	Particle *p;

	if (game->n <= 0)
		return;

	for(int i=0; i<10; i++)
	    makeParticle(game, game->lastMousex, game->lastMousey);

	for(int i=0; i<game->n; i++) {
	    	p = &game->particle[i];
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;
		p->velocity.y -= GRAVITY;

		//check for collision with shapes...
		for(int j=0; j<5; j++){
		Shape *s = &game->box[j];
		 
		
		if(p->s.center.y < s->center.y + s->height && 
			p->s.center.y > s->center.y - s->height &&
			p->s.center.x >= s->center.x - s->width &&
			p->s.center.x <= s->center.x + s->width){
	    		p->velocity.y *= -0.25;
			p->s.center.y = s->center.y + s->height + 0.01;
		}

		float d0,d1,dist;
		d0 = p->s.center.x - game->circle.center.x;
		d1 = p->s.center.y - game->circle.center.y;
		dist = sqrt(d0*d0 + d1*d1);
		if(dist <= game->circle.radius){
		    //p->velocity.y = -p->velocity.y;
		    //p->velocity.x = -2 * (p->velocity.x);
		    d0 /= dist;
		    d1 /= dist;
		    d0 *= game->circle.radius * 1.01;
		    d1 *= game->circle.radius * 1.01;
		    p->s.center.x = game->circle.center.x + d0;
		    p->s.center.y = game->circle.center.y + d1;
	            p->velocity.x += d0 * 0.006;
		    p->velocity.y += d1 * 0.006;
		}

		}
		//check for offscreen
		if(p->s.center.y < 0.0 || p->s.center.y > WINDOW_HEIGHT){
		    memcpy(&game->particle[i], &game->particle[game->n-1], sizeof(Particle));
		//std::cout << "off screen" << std::endl;
	    	game->n--;
		}	

	}	
}

void render(Game *game)
{
	float w, h;
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...

	static int firsttime = 1;
	static int verts[60][2];
	static int n = 60;
	if(firsttime) {
	    float angle = 0.0;
	    float inc = (3.14159 * 2.0) / (float)n;
	    for(int i=0; i<n; i++){
	   	 verts[i][0] = cos(angle) * game->circle.radius + game->circle.center.x;
	   	 verts[i][1] = sin(angle) * game->circle.radius + game->circle.center.y;
		 angle += inc;
		 
	    }
	    
	    firsttime = 0;
	}
	
	glColor3ub(95,45,63);
	glPushMatrix();
	glBegin(GL_TRIANGLE_FAN);
		for(int i = 0; i<n; i++){
		    glVertex2i(verts[i][0], verts[i][1]);
		}

	glEnd();
	glPopMatrix();


	//draw box
	Shape *s;
	glColor3ub(0,47,167);
	
	for(int j=0; j<5; j++){
	s = &game->box[j];

	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	glBegin(GL_QUADS);
		glVertex2i(-w,-h);
		glVertex2i(-w, h);
		glVertex2i( w, h);
		glVertex2i( w,-h);
	glEnd();
	glPopMatrix();
	}
	
	glColor3ub(255,120,50);	
	Rect r[5];
	r[4].bot = game->box[4].center.y - 10;
	r[4].left = game->box[4].center.x;
	r[4].center = 1;
	ggprint16(&r[4],16, 0x00ff0000, "Requirements");
	r[3].bot = game->box[3].center.y - 10;
	r[3].left = game->box[3].center.x;
	r[3].center = 1;
	ggprint16(&r[3],16, 0x00ff0000, "Design");
	r[2].bot = game->box[2].center.y - 10;
	r[2].left = game->box[2].center.x;
	r[2].center = 1;
	ggprint16(&r[2],16, 0x00ff0000, "Coding");
	r[1].bot = game->box[1].center.y - 10;
	r[1].left = game->box[1].center.x;
	r[1].center = 1;
	ggprint16(&r[1],16, 0x00ff0000, "Testing");
	r[0].bot = game->box[0].center.y - 10;
	r[0].left = game->box[0].center.x;
	r[0].center = 1;
	ggprint16(&r[0],16, 0x00ff0000, "Maintenance");
	





	//draw all particles here
	for(int i=0; i<game->n; i++){
	    glPushMatrix();
		glColor3ub(150,160,220);
		Vec *c = &game->particle[i].s.center;
		w = 2;
		h = 2;
		glBegin(GL_QUADS);
		glVertex2i(c->x-w, c->y-h);
		glVertex2i(c->x-w, c->y+h);
		glVertex2i(c->x+w, c->y+h);
		glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}
	
	if(game->bubbler)
	{
	    for(int i = 0; i<10; i++)
		makeParticle(game,90,540);
	}
}





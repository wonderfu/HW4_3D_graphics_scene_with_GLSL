#ifndef _3DMAZE_H
#define _3DMAZE_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glut.h>

#define PI 3.14159265359
#define PI_180 0.01745329251
#define PI2 6.28318530718
#define MoveSpeed 1
#define Wall_H 10.0f
#define Wall_W 20.0f
#define Ceil_Floor 5.0f
#define N 0
#define E 1
#define S 2
#define W 3
#define Map_Wall 0
#define Map_Road 1
#define Map_Start 2
#define Map_End 3


/* Struct */
typedef struct Wall {
	bool way[4]; // N E S W
	int type; // maybe somthing will use it
} Wall;	

/* Function */
void Init(void);
void Display(void);
void Reshape(int, int);
void Idle(void);
void Keyboard(unsigned char, int, int);
void Mouse(int, int, int, int);
void Motion(int, int);
void LightSource(void);
void DrawWall(GLfloat, GLfloat);
void DrawCube(GLfloat, GLfloat);
void DrawTexture(void);
void Material(void);

#endif
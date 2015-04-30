#include "3dmaze.h"

/* Window */
int window_size[2] = { 800, 800 }; // W H

/* Camera */
GLdouble camera_eye[3] = { 0.0, 0.0, 20.0 };
GLdouble camera_center[3] = { 0.0, 0.0, 0.0 };
GLdouble camera_up[3] = { 0.0, 1.0, 0.0 };
GLfloat camera_angle = 0.0, camera_RL_angle = 0.0;
GLdouble camera_ray[3] = { 0.0, 0.0, -1.0 }; // eye & angle & ray decide center 

/* Light0 */
bool test_light = false;
GLfloat light0_ambient[] = { 0.3, 0.3, 0.3, 1.0 };
GLfloat light0_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light0_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light0_position[] = { 0.0, 200.0, 0.0, 1.0 };

/* Light1 */
bool flash_light = true;
GLfloat light1_ambient[] = { 0.3, 0.3, 0.3, 1.0 };
GLfloat light1_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light1_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light1_position[] = { 0.0, 0.0, 20.0, 1.0 };
GLfloat spot_direction[] = { 0.0, 0.0, -1.0 };

/* Mouse */
int old_mouse_pos[2] = { window_size[0] >> 1, window_size[1] >> 1 };
int bullet_dis = 0;
GLdouble radious = 0.2;
GLdouble bullet_ray[3] = { 0.0, 0.0, 0.0 };
GLdouble bullet_pos[3] = { 0.0, 0.0, 0.0 };
GLdouble gravity = -0.005;

/* Maze */
bool build_from_file = true;
int **map;
int map_w, map_h;

GLubyte mipmapImage32[32][32][4];
static GLuint texName;
GLfloat turn = 0.0;

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(200, 100);
	glutInitWindowSize(window_size[0], window_size[1]);
	glutCreateWindow("3D Maze");
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutPassiveMotionFunc(Motion);
	glutIdleFunc(Idle);
	glutReshapeFunc(Reshape);
	glutDisplayFunc(Display);
	
	Init();
	glutMainLoop();
	return 0;
}

void Init(void)
{
	// map file
	if (build_from_file)
	{
		FILE *stream;
		char mode;

		errno_t err = fopen_s(&stream, "Map.txt", "r");
		fseek(stream, 0L, SEEK_SET);

		fscanf_s(stream, "%d %d\r\n", &map_w, &map_h);
		map = (int**)malloc(sizeof(int*)*map_h);
		for (int i = 0; i < map_h; ++i)
			map[i] = (int*)malloc(sizeof(int)*map_w);
		
		for (int i = 0; i < map_h; ++i)
		{
			for (int j = 0; j < map_w; ++j)
			{
				fscanf_s(stream, " %c ", &mode);
				switch (mode)
				{
					case 'w':
						map[i][j] = Map_Wall;
						break;
					case 'r':
						map[i][j] = Map_Road;
						break;
					case 's':
						map[i][j] = Map_Start;
						camera_eye[0] = (map_h / 2 - i)*Wall_W;
						camera_eye[2] = (map_w / 2 - j)*Wall_W;
						break;
					case 'e':
						map[i][j] = Map_End;
						break;
				}
				printf("%c", mode);
			}
			printf("\n");
		}
		fclose(stream);
	}

	glClearColor(0.0, 0.0, 0.0, 1.0);
	// light
	glEnable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glShadeModel(GL_SMOOTH);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0);
	// mouse
	glutSetCursor(GLUT_CURSOR_NONE);
	// camera
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)window_size[0] / (GLfloat)window_size[1], 1.0, 10000.0);
	// texture
	DrawTexture();
	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, texName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, mipmapImage32);
}

void Display(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Camera */
	camera_center[0] = camera_eye[0] + camera_ray[0];
	//camera_center[1] = camera_eye[1] + camera_ray[1];
	camera_center[2] = camera_eye[2] + camera_ray[2];
	gluLookAt(camera_eye[0], camera_eye[1], camera_eye[2], camera_center[0], camera_center[1], camera_center[2], camera_up[0], camera_up[1], camera_up[2]);

	/* Light */
	LightSource();
	/* Material */
	//Material();
	/* Maze */
	GLfloat floor_half_w, floor_half_h;

	floor_half_w = map_w*Wall_W / 2;
	floor_half_h = map_h*Wall_W / 2;

	glPushMatrix();
		glColor3f(0.5f, 0.5f, 0.5f);
		glBegin(GL_QUADS);
			// ceil
			glNormal3f(0.0f, -1.0f, 0.0f);
			glVertex3f(floor_half_w + Wall_W / 2, Wall_H, floor_half_h + Wall_W / 2);
			glVertex3f(-floor_half_w + Wall_W / 2, Wall_H, floor_half_h + Wall_W / 2);
			glVertex3f(-floor_half_w + Wall_W / 2, Wall_H, -floor_half_h + Wall_W / 2);
			glVertex3f(floor_half_w + Wall_W / 2, Wall_H, -floor_half_h + Wall_W / 2);
			// floor
			glNormal3f(0.0f, 1.0f, 0.0f);
			glVertex3f(floor_half_w + Wall_W / 2, -Wall_H, floor_half_h + Wall_W / 2);
			glVertex3f(floor_half_w + Wall_W / 2, -Wall_H, -floor_half_h + Wall_W / 2);
			glVertex3f(-floor_half_w + Wall_W / 2, -Wall_H, -floor_half_h + Wall_W / 2);
			glVertex3f(-floor_half_w + Wall_W / 2, -Wall_H, floor_half_h + Wall_W / 2);
		glEnd();
	glPopMatrix();
	// wall
	int map_half_h = map_h / 2, map_half_w = map_w / 2;

	for (int i = 0; i < map_h; ++i)
	{
		for (int j = 0; j < map_w; ++j)
		{
			if (map[i][j] == Map_Wall)
				DrawWall((map_half_h - i)*Wall_W, (map_half_w - j)*Wall_W);
			if (map[i][j] == Map_End)
				DrawCube((map_half_h - i)*Wall_W, (map_half_w - j)*Wall_W);
		}
	}

	if (bullet_dis > 0)
	{
		bullet_pos[0] += bullet_ray[0] * 0.5;
		bullet_pos[1] += bullet_ray[1] * 0.5;
		bullet_pos[2] += bullet_ray[2] * 0.5;
		bullet_ray[1] += gravity;
		glPushMatrix();
			glTranslatef(bullet_pos[0], bullet_pos[1], bullet_pos[2]);
			glColor3f(0.0, 1.0, 0.0);
			glutSolidSphere(radious, 20, 20);
		glPopMatrix();
		--bullet_dis;
	}
	glFlush();
}

void Idle(void)
{
	glutSwapBuffers();
	glutPostRedisplay();
}

void Reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 1.0, 10000.0);
	glMatrixMode(GL_MODELVIEW);
	window_size[0] = w;
	window_size[1] = h;
}

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 'w':
		case 'W':
			camera_eye[0] += MoveSpeed*camera_ray[0];
			camera_eye[2] += MoveSpeed*camera_ray[2];
			break;
		case 's':
		case 'S':
			camera_eye[0] -= MoveSpeed*camera_ray[0];
			camera_eye[2] -= MoveSpeed*camera_ray[2];
			break;
		case 'a':
		case 'A':
			camera_eye[0] -= MoveSpeed*sin(camera_RL_angle);
			camera_eye[2] -= MoveSpeed*-cos(camera_RL_angle);
			break;
		case 'd':
		case 'D':
			camera_eye[0] += MoveSpeed*sin(camera_RL_angle);
			camera_eye[2] += MoveSpeed*-cos(camera_RL_angle);
			break;
		case 'l':
		case 'L':
			flash_light = !flash_light;
			if (flash_light)
				glEnable(GL_LIGHT1);
			else
				glDisable(GL_LIGHT1);
			break;
		case 't':
		case 'T':
			test_light = !test_light;
			if (test_light)
				glEnable(GL_LIGHT0);
			else
				glDisable(GL_LIGHT0);
			break;
		case 27: //ESC
			if (build_from_file)
			{
				for (int i = 0; i < map_h; ++i)
					free(map[i]);
				free(map);
			}
			exit(0);
			break;
	}
}

void Mouse(int button, int state, int x, int y)
{
	bullet_pos[0] = camera_eye[0];
	bullet_pos[1] = -1;
	bullet_pos[2] = camera_eye[2];
	bullet_ray[0] = camera_ray[0];
	bullet_ray[1] = +0.1;
	bullet_ray[2] = camera_ray[2];
	bullet_dis = 100;
}

void Motion(int x, int y)
{
	camera_RL_angle = ((GLdouble)x / window_size[0]) * PI2;
	camera_angle = camera_RL_angle - PI;
	camera_RL_angle -= PI/2;
	camera_ray[0] = sin(camera_angle);
	camera_ray[2] = -cos(camera_angle);
}

void LightSource(void)
{
	if (flash_light)
	{
		light1_position[0] = camera_eye[0];
		light1_position[1] = camera_eye[1];
		light1_position[2] = camera_eye[2];
		spot_direction[0] = camera_ray[0];
		//spot_direction[1] = camera_ray[1];
		spot_direction[2] = camera_ray[2];

		glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
		glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
		glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
		glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.5);
		glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.5);
		glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.2);

		glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
		glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);
		glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0);
	}
	if (test_light)
	{
		glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
		glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	}
}

void DrawTexture(void)
{
	int i, j;
	int half_line = 2;

	for (i = 0; i < 32; i++) 
	{
		for (j = 0; j < 32; j++) 
		{
			mipmapImage32[i][j][0] = 128;
			mipmapImage32[i][j][1] = 128;
			mipmapImage32[i][j][2] = 128;
			mipmapImage32[i][j][3] = 255;
		}
	}

	for (i = half_line; i < 16 - half_line; ++i)
	{
		for (j = half_line; j < 32 - half_line; ++j)
		{
			mipmapImage32[i][j][0] = 255;
			mipmapImage32[i][j][1] = 0;
			mipmapImage32[i][j][2] = 0;
		}
	}
	for (i = 16 + half_line; i < 32 - half_line; ++i)
	{
		for (j = 0; j < 16 - half_line; ++j)
		{
			mipmapImage32[i][j][0] = 255;
			mipmapImage32[i][j][1] = 0;
			mipmapImage32[i][j][2] = 0;
		}
		for (j = 16 + half_line; j < 32; ++j)
		{
			mipmapImage32[i][j][0] = 255;
			mipmapImage32[i][j][1] = 0;
			mipmapImage32[i][j][2] = 0;
		}
	}
}

void DrawWall(GLfloat x, GLfloat z)
{
	GLfloat width = Wall_W / 2;

	glPushMatrix();
		glTranslatef(x, 0, z);

		glBindTexture(GL_TEXTURE_2D, texName);
		glEnable(GL_TEXTURE_2D);

		glColor3f(0.8f, 0.0f, 0.0f);
		glBegin(GL_QUADS);
			glNormal3f(0.0f, 0.0f, 1.0f);
			glTexCoord2f(10.0, 0.0); glVertex3f(width, Wall_H, width);
			glTexCoord2f(0.0, 0.0); glVertex3f(-width, Wall_H, width);
			glTexCoord2f(0.0, 10.0); glVertex3f(-width, -Wall_H, width);
			glTexCoord2f(10.0, 10.0); glVertex3f(width, -Wall_H, width);

			glNormal3f(-1.0f, 0.0f, 0.0f);
			glTexCoord2f(10.0, 0.0); glVertex3f(-width, Wall_H, width);
			glTexCoord2f(0.0, 0.0); glVertex3f(-width, Wall_H, -width);
			glTexCoord2f(0.0, 10.0); glVertex3f(-width, -Wall_H, -width);
			glTexCoord2f(10.0, 10.0); glVertex3f(-width, -Wall_H, width);

			glNormal3f(0.0f, 0.0f, -1.0f);
			glTexCoord2f(10.0, 0.0); glVertex3f(-width, Wall_H, -width);
			glTexCoord2f(0.0, 0.0); glVertex3f(width, Wall_H, -width);
			glTexCoord2f(0.0, 10.0); glVertex3f(width, -Wall_H, -width);
			glTexCoord2f(10.0, 10.0); glVertex3f(-width, -Wall_H, -width);

			glNormal3f(1.0f, 0.0f, 0.0f);
			glTexCoord2f(10.0, 0.0); glVertex3f(width, Wall_H, -width);
			glTexCoord2f(0.0, 0.0); glVertex3f(width, Wall_H, width);
			glTexCoord2f(0.0, 10.0); glVertex3f(width, -Wall_H, width);
			glTexCoord2f(10.0, 10.0); glVertex3f(width, -Wall_H, -width);
		glEnd();
	glPopMatrix();
	return;
}

void DrawCube(GLfloat x, GLfloat z)
{
	glPushMatrix();	
		glTranslatef(x, 0, z);
		glRotatef(45, -1.0, 0.0, 0.0);
		glRotatef(45, 0.0, 0.0, 1.0);
		glRotatef(turn, 1.0, 1.0, 1.0);
		turn += 1;
	
		glBegin(GL_QUADS);
			glColor3f(0.0, 1.0, 0.0);	
			glVertex3f( 0.7, 0.7, -0.7);
			glVertex3f(-0.7, 0.7, -0.7);
			glVertex3f(-0.7, 0.7,  0.7);
			glVertex3f( 0.7, 0.7,  0.7);
	
			glColor3f(1.0, 0.0, 1.0);	
			glVertex3f( 0.7, -0.7,  0.7);
			glVertex3f(-0.7, -0.7,  0.7);
			glVertex3f(-0.7, -0.7, -0.7);
			glVertex3f( 0.7, -0.7, -0.7);
	
			glColor3f(0.0, 1.0, 1.0);	
			glVertex3f( 0.7,  0.7, 0.7);
			glVertex3f(-0.7,  0.7, 0.7);
			glVertex3f(-0.7, -0.7, 0.7);
			glVertex3f( 0.7, -0.7, 0.7);
	
			glColor3f(1.0, 0.0, 0.0);	
			glVertex3f( 0.7, -0.7, -0.7);
			glVertex3f(-0.7, -0.7, -0.7);
			glVertex3f(-0.7,  0.7, -0.7);
			glVertex3f( 0.7,  0.7, -0.7);
	
			glColor3f(1.0, 1.0, 0.0);	
			glVertex3f(-0.7,  0.7,  0.7);
			glVertex3f(-0.7,  0.7, -0.7);
			glVertex3f(-0.7, -0.7, -0.7);
			glVertex3f(-0.7, -0.7,  0.7);
	
			glColor3f(0.0, 0.0, 1.0);	
			glVertex3f(0.7,  0.7, -0.7);
			glVertex3f(0.7,  0.7,  0.7);
			glVertex3f(0.7, -0.7,  0.7);
			glVertex3f(0.7, -0.7, -0.7);
		glEnd();
	glPopMatrix();
}

void Material(void)
{
	GLfloat material_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat material_diffuse[] = { 0.4, 0.4, 0.4, 1.0 };
	GLfloat material_specular[] = { 0.7, 0.7, 0.7, 1.0 };

	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, 0.6 * 128.0);
}
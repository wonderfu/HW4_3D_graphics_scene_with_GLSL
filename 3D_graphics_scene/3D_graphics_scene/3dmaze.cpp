#include "3dmaze.h"
#include "shader.h"
#include "noise.h"

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
GLfloat light0_position[] = { 0.0, 150.0, 0.0, 1.0 };

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

/* Noise */
static GLuint noisetex;
double mynoise[NoiseWidth][NoiseHeight][NoiseDepth];

/* Maze */
bool build_from_file = true;
int **map;
int map_w, map_h;

GLfloat turn = 0.0;

/* Shader */
Shader brick_shader;
Shader inferno_shader;
Shader test1;
Shader test2;
float xtime = 0, ytime = 0, ztime = 0;

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
	glewInit();

	Init();
	glutMainLoop();
	return 0;
}

void Init(void)
{
	/* map file */
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
	
	/* light */
	glEnable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glShadeModel(GL_SMOOTH);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0);
	
	/* mouse */
	glutSetCursor(GLUT_CURSOR_NONE);
	
	/* camera */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)window_size[0] / (GLfloat)window_size[1], 1.0, 10000.0);
	
	// noise
	generateNoise();
	for (int i = 0; i < NoiseWidth; ++i)
	{
		for (int j = 0; j < NoiseHeight; ++j)
		{
			for (int k = 0; k < NoiseDepth; ++k)
			{
				mynoise[i][j][k] = turbulence(i, j, k, 32);
				//mynoise[i][j][k] = rand()%100/100.0;
			}
		}
	}

	// texture
	glGenTextures(1, &noisetex);
	glBindTexture(GL_TEXTURE_3D, noisetex);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, NoiseWidth, NoiseHeight, NoiseDepth, 0, GL_RGBA, GL_UNSIGNED_BYTE, mynoise);
	
	/* shader */
	brick_shader.init((char*)"./Shader Files/Brick.vert", (char*)"./Shader Files/Brick.frag");
	inferno_shader.init((char*)"./Shader Files/Inferno.vert", (char*)"./Shader Files/Inferno.frag");
	test1.init((char*)"./Shader Files/test1.vert", (char*)"./Shader Files/test1.frag");
	test2.init((char*)"./Shader Files/test2.vert", (char*)"./Shader Files/test2.frag");
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
			if (map[i][j] == Map_Start)
			{
				glPushMatrix();
					
					test2.bind();
					glUniform1f(glGetUniformLocation(test2.id(), "xtime"), xtime);
					glUniform1f(glGetUniformLocation(test2.id(), "ytime"), ytime);
					glUniform1f(glGetUniformLocation(test2.id(), "ztime"), ztime);
					glUniform1f(glGetUniformLocation(test2.id(), "size"), Wall_H);

					glTranslatef((map_half_h - i)*Wall_W, 0, (map_half_w - j)*Wall_W);

					glColor3f(0.0f, 0.0f, 1.0f);
					glBegin(GL_QUADS);
						glNormal3f(0.0f, 0.0f, 1.0f);
						glVertex3f(Wall_W / 2, Wall_H, Wall_W / 2);
						glVertex3f(-Wall_W / 2, Wall_H, Wall_W / 2);
						glVertex3f(-Wall_W / 2, -Wall_H, Wall_W / 2);
						glVertex3f(Wall_W / 2, -Wall_H, Wall_W / 2);

						glNormal3f(-1.0f, 0.0f, 0.0f);
						glVertex3f(-Wall_W / 2, Wall_H, Wall_W / 2);
						glVertex3f(-Wall_W / 2, Wall_H, -Wall_W / 2);
						glVertex3f(-Wall_W / 2, -Wall_H, -Wall_W / 2);
						glVertex3f(-Wall_W / 2, -Wall_H, Wall_W / 2);

						glNormal3f(0.0f, 0.0f, -1.0f);
						glVertex3f(-Wall_W / 2, Wall_H, -Wall_W / 2);
						glVertex3f(Wall_W / 2, Wall_H, -Wall_W / 2);
						glVertex3f(Wall_W / 2, -Wall_H, -Wall_W / 2);
						glVertex3f(-Wall_W / 2, -Wall_H, -Wall_W / 2);

						glNormal3f(1.0f, 0.0f, 0.0f);
						glVertex3f(Wall_W / 2, Wall_H, -Wall_W / 2);
						glVertex3f(Wall_W / 2, Wall_H, Wall_W / 2);
						glVertex3f(Wall_W / 2, -Wall_H, Wall_W / 2);
						glVertex3f(Wall_W / 2, -Wall_H, -Wall_W / 2);
					glEnd();

					test2.unbind();
					xtime += 0.01;
					ytime += 0.02;
					ztime += 0.03;
				glPopMatrix();
			}
		}
	}

	if (bullet_dis > 0)
	{
		bullet_pos[0] += bullet_ray[0] * 0.05;
		bullet_pos[1] += bullet_ray[1] * 0.05;
		bullet_pos[2] += bullet_ray[2] * 0.05;
		bullet_ray[1] += gravity*0.1;

		DrawBullet();
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
	bullet_dis = 1000;
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

void DrawWall(GLfloat x, GLfloat z)
{
	GLfloat width = Wall_W / 2;

	glPushMatrix();

		brick_shader.bind();
		glUniform3f(glGetUniformLocation(brick_shader.id(), "LightPosition"), light0_position[0], light0_position[1], light0_position[2]);
		glUniform3f(glGetUniformLocation(brick_shader.id(), "BrickColor"), 1, 0, 0);
		glUniform3f(glGetUniformLocation(brick_shader.id(), "MortarColor"), 0.6, 0.4, 0.2);
		glUniform2f(glGetUniformLocation(brick_shader.id(), "BrickSize"), 5, 2);
		glUniform2f(glGetUniformLocation(brick_shader.id(), "BrickPct"), 0.85, 0.85);

		glTranslatef(x, 0, z);

		//glBindTexture(GL_TEXTURE_2D, texName);
		//glEnable(GL_TEXTURE_2D);

		//glColor3f(0.8f, 0.0f, 0.0f);
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
		brick_shader.unbind();

	glPopMatrix();

	return;
}

void DrawCube(GLfloat x, GLfloat z)
{
	glPushMatrix();	
		test1.bind();

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

		test1.unbind();
	glPopMatrix();
}

void DrawBullet()
{
	glPushMatrix();
	
		inferno_shader.bind();
		glUniform1f(glGetUniformLocation(inferno_shader.id(), "Scale"), -1.0);
		glUniform1f(glGetUniformLocation(inferno_shader.id(), "Offset"), 0.0);
		glUniform3f(glGetUniformLocation(inferno_shader.id(), "FireColor1"), 0.8, 0.1, 0);
		glUniform3f(glGetUniformLocation(inferno_shader.id(), "FireColor2"), 0.1, 0.8, 0.0);
		glUniform1f(glGetUniformLocation(inferno_shader.id(), "Extent"), 10);
		glUniform1i(glGetUniformLocation(inferno_shader.id(), "sampler3d"), 0);
	
		glBindTexture(GL_TEXTURE_3D, noisetex);
		glEnable(GL_TEXTURE_3D);
	
		glTranslatef(bullet_pos[0], bullet_pos[1], bullet_pos[2]);
		glutSolidSphere(radious, 20, 20);
	
		inferno_shader.unbind();
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

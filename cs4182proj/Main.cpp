/* Link the libraries code-wise. */
#ifdef _MSC_VER
#	pragma comment(lib, "OpenGL32.lib")
#	pragma comment(lib, "GLu32.lib")

#	pragma comment(lib, "SDL.lib")
#	pragma comment(lib, "SDLmain.lib")
#	pragma comment(lib, "SDL_image.lib")
#endif //_MSC_VER

#include <string>
#include <cmath>

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_image.h>
#include <GL\glut.h>
#include <GL/GL.h>
#include <vector>
#include <windows.h>

using namespace std;

#define PI 3.141592653589793

unsigned Textures[3];
unsigned BoxList(0);					//Added!

										/* These will define the player's position and view angle. */
double X(0.0), Y(0.0), Z(0.0);
double ViewAngleHor(0.0), ViewAngleVer(0.0);

/*
* DegreeToRadian
*	Converts a specified amount of degrees to radians.
*/
inline double DegreeToRadian(double degrees)
{
	return (degrees / 180.f * PI);
}

/*
* GrabTexObjFromFile
*	This function will use SDL to load the specified image, create an OpenGL
*	texture object from it and return the texture object number.
*/
GLuint GrabTexObjFromFile(const std::string& fileName)
{
	/* Use SDL_image to load the PNG image. */
	SDL_Surface *Image = IMG_Load(fileName.c_str());

	/* Image doesn't exist or failed loading? Return 0. */
	if (!Image)
		return 0;

	unsigned Object(0);

	/* Generate one texture (we're creating only one). */
	glGenTextures(1, &Object);

	/* Set that texture as current. */
	glBindTexture(GL_TEXTURE_2D, Object);

	/* You can use these values to specify mipmaps if you want to, such as 'GL_LINEAR_MIPMAP_LINEAR'. */
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* We're setting textures to be repeated here. */
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //NEW!
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //NEW!

																  /* Create the actual texture object. */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Image->w, Image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, Image->pixels);

	/* Free the surface, we are finished with it. */
	SDL_FreeSurface(Image);

	return Object;
}

/*
*	CompileLists
*		Compiles the display lists used by our application.
*/
void CompileLists()
{
	/* Let's generate a display list for a box. */
	BoxList = glGenLists(1);
	glNewList(BoxList, GL_COMPILE);

	/*
	* Render everything as you usually would, without texture binding. We're rendering the box from the
	* '3D Objects' tutorial here.
	*/
	glBegin(GL_QUADS);
	/* Front */
	glTexCoord2d(0, 0); glVertex3d(400, 125, 0.4);
	glTexCoord2d(1, 0); glVertex3d(750, 125, 0.4);
	glTexCoord2d(1, 1); glVertex3d(750, 475, 0.4);
	glTexCoord2d(0, 1); glVertex3d(400, 475, 0.4);

	/* Left side */
	glTexCoord2d(0, 0); glVertex3d(400, 125, -0.4);
	glTexCoord2d(1, 0); glVertex3d(400, 125, 0.4);
	glTexCoord2d(1, 1); glVertex3d(400, 475, 0.4);
	glTexCoord2d(0, 1); glVertex3d(400, 475, -0.4);

	/* Back */
	glTexCoord2d(0, 0); glVertex3d(750, 125, -0.4);
	glTexCoord2d(1, 0); glVertex3d(400, 125, -0.4);
	glTexCoord2d(1, 1); glVertex3d(400, 475, -0.4);
	glTexCoord2d(0, 1); glVertex3d(750, 475, -0.4);

	/* Right side */
	glTexCoord2d(0, 0); glVertex3d(750, 125, 0.4);
	glTexCoord2d(1, 0); glVertex3d(750, 125, -0.4);
	glTexCoord2d(1, 1); glVertex3d(750, 475, -0.4);
	glTexCoord2d(0, 1); glVertex3d(750, 475, 0.4);

	/* Top */
	glTexCoord2d(0, 0); glVertex3d(400, 125, -0.4);
	glTexCoord2d(1, 0); glVertex3d(750, 125, -0.4);
	glTexCoord2d(1, 1); glVertex3d(750, 125, 0.4);
	glTexCoord2d(0, 1); glVertex3d(400, 125, 0.4);

	/* Bottom */
	glTexCoord2d(0, 0); glVertex3d(400, 475, -0.4);
	glTexCoord2d(1, 0); glVertex3d(750, 475, -0.4);
	glTexCoord2d(1, 1); glVertex3d(750, 475, 0.4);
	glTexCoord2d(0, 1); glVertex3d(400, 475, 0.4);
	glEnd();
	glEndList();
}

struct coordinate {
	float x, y, z;
	coordinate(float a, float b, float c) : x(a), y(b), z(c) {};
};

struct tex {
	float x, y;
	tex(float a, float b) : x(a), y(b) {};
};
//for faces, it can contain triangles and quads as well, the four variable contain which is that

struct quad {
	int quadNum;
	float normalVector[3];
	float vertexNumber[3];

};

int loadObject(const char* filename)
{
	std::vector<std::string*> coord;        //read every single line of the obj file as a string
	std::vector<coordinate*> vertex;
	//std::vector<face*> faces;
	std::vector<coordinate*> normals;
	std::vector<tex*> textures;
	std::vector< unsigned int > vertexIndicesQ, uvIndicesQ, normalIndicesQ, vertexIndicesT, uvIndicesT, normalIndicesT;

	//normal vectors for every face
	std::ifstream in(filename);     //open the .obj file
	if (!in.is_open())       //if not opened, exit with -1
	{
		std::cout << "Nor oepened" << std::endl;
		return -1;
	}
	char buf[256];
	//read in every line to coord
	while (!in.eof())
	{
		in.getline(buf, 256);
		coord.push_back(new std::string(buf));
	}
	//go through all of the elements of coord, and decide what kind of element is that
	for (int i = 0; i < coord.size(); i++)
	{
		if (coord[i]->c_str()[0] == '#')   //if it is a comment (the first character is #)
			continue;       //we don't care about that
		else if (coord[i]->c_str()[0] == 'v' && coord[i]->c_str()[1] == ' ') //if vector
		{
			float tmpx, tmpy, tmpz;
			sscanf_s(coord[i]->c_str(), "v %f %f %f", &tmpx, &tmpy, &tmpz);       //read in the 3 float coordinate to tmpx,tmpy,tmpz	

																				  // For each coordinate, apply a translation
			tmpz = tmpz - 1.0;
			tmpx = tmpx - 0.5;
			tmpy = tmpy - 0.5;

			vertex.push_back(new coordinate(tmpx, tmpy, tmpz));       //and then add it to the end of our vertex list
		}
		else if (coord[i]->c_str()[0] == 'v' && coord[i]->c_str()[1] == 'n')        //if normal vector
		{
			float tmpx, tmpy, tmpz;   //do the same thing
			sscanf_s(coord[i]->c_str(), "vn %f %f %f", &tmpx, &tmpy, &tmpz);
			normals.push_back(new coordinate(tmpx, tmpy, tmpz));
		}
		else if (coord[i]->c_str()[0] == 'v' && coord[i]->c_str()[1] == 't') // texture coordinate
		{
			float tmpx, tmpy;
			sscanf_s(coord[i]->c_str(), "vt %f %f", &tmpx, &tmpy);
			textures.push_back(new tex(tmpx, tmpy));
		}
		else if (coord[i]->c_str()[0] == 'f')     //if face (quad)
		{

			if (count(coord[i]->begin(), coord[i]->end(), ' ') == 3) {
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];

				int matches = sscanf_s(coord[i]->c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
					&vertexIndex[0], &uvIndex[0], &normalIndex[0],
					&vertexIndex[1], &uvIndex[1], &normalIndex[1],
					&vertexIndex[2], &uvIndex[2], &normalIndex[2]);

				vertexIndicesT.push_back(vertexIndex[0]);
				vertexIndicesT.push_back(vertexIndex[1]);
				vertexIndicesT.push_back(vertexIndex[2]);
				uvIndicesT.push_back(uvIndex[0]);
				uvIndicesT.push_back(uvIndex[1]);
				uvIndicesT.push_back(uvIndex[2]);
				normalIndicesT.push_back(normalIndex[0]);
				normalIndicesT.push_back(normalIndex[1]);
				normalIndicesT.push_back(normalIndex[2]);

			}
			else if (count(coord[i]->begin(), coord[i]->end(), ' ') == 4) {
				unsigned int vertexIndex[4], uvIndex[4], normalIndex[4];

				int matches = sscanf_s(coord[i]->c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
					&vertexIndex[0], &uvIndex[0], &normalIndex[0],
					&vertexIndex[1], &uvIndex[1], &normalIndex[1],
					&vertexIndex[2], &uvIndex[2], &normalIndex[2],
					&vertexIndex[3], &uvIndex[3], &normalIndex[3]);

				vertexIndicesQ.push_back(vertexIndex[0]);
				vertexIndicesQ.push_back(vertexIndex[1]);
				vertexIndicesQ.push_back(vertexIndex[2]);
				vertexIndicesQ.push_back(vertexIndex[3]);
				uvIndicesQ.push_back(uvIndex[0]);
				uvIndicesQ.push_back(uvIndex[1]);
				uvIndicesQ.push_back(uvIndex[2]);
				uvIndicesQ.push_back(uvIndex[3]);
				normalIndicesQ.push_back(normalIndex[0]);
				normalIndicesQ.push_back(normalIndex[1]);
				normalIndicesQ.push_back(normalIndex[2]);
				normalIndicesQ.push_back(normalIndex[3]);
			}
		}
		// At this point, have lists of normals, vertices, texture coordinates, and quad faces
		//raw
	}
	int num;        //the id for the list
	num = glGenLists(1);      //generate a uniqe
	glNewList(num, GL_COMPILE);      //and create it

	for (int idx = 1; idx < vertexIndicesQ.size(); idx = idx + 4) {
		glBegin(GL_QUADS);

		glNormal3f(normals[normalIndicesQ[idx] - 1]->x, normals[normalIndicesQ[idx - 1] - 1]->y, normals[normalIndicesQ[idx - 1] - 1]->z);
		glVertex3f(vertex[vertexIndicesQ[idx] - 1]->x, vertex[vertexIndicesQ[idx] - 1]->y, vertex[vertexIndicesQ[idx] - 1]->z);
		glVertex3f(vertex[vertexIndicesQ[idx + 0] - 1]->x, vertex[vertexIndicesQ[idx + 0] - 1]->y, vertex[vertexIndicesQ[idx + 0] - 1]->z);
		glVertex3f(vertex[vertexIndicesQ[idx + 1] - 1]->x, vertex[vertexIndicesQ[idx + 1] - 1]->y, vertex[vertexIndicesQ[idx + 1] - 1]->z);
		glVertex3f(vertex[vertexIndicesQ[idx + 2] - 1]->x, vertex[vertexIndicesQ[idx + 2] - 1]->y, vertex[vertexIndicesQ[idx + 2] - 1]->z);

		glEnd();

	}
	for (int idx = 0; idx < vertexIndicesT.size(); idx = idx + 3) {
		glBegin(GL_TRIANGLES);

		glNormal3f(normals[normalIndicesT[idx] - 1]->x, normals[normalIndicesT[idx] - 1]->y, normals[normalIndicesT[idx] - 1]->z);
		glVertex3f(vertex[vertexIndicesT[idx] - 1]->x, vertex[vertexIndicesT[idx] - 1]->y, vertex[vertexIndicesT[idx] - 1]->z);
		glVertex3f(vertex[vertexIndicesT[idx + 0] - 1]->x, vertex[vertexIndicesT[idx + 0] - 1]->y, vertex[vertexIndicesT[idx + 0] - 1]->z);
		glVertex3f(vertex[vertexIndicesT[idx + 1] - 1]->x, vertex[vertexIndicesT[idx + 1] - 1]->y, vertex[vertexIndicesT[idx + 1] - 1]->z);

		glEnd();
	}
	glEndList();
	//delete everything to avoid memory leaks
	for (int i = 0; i < coord.size(); i++)
		delete coord[i];
	for (int i = 0; i < normals.size(); i++)
		delete normals[i];
	for (int i = 0; i < vertex.size(); i++)
		delete vertex[i];
	for (int i = 0; i < textures.size(); i++)
		delete textures[i];

	return num;     //return with the id
}

double currLightPosX=4.0, currLightPosY = 2.0, currLightPosZ = 2.0;

double init = -0.8;
double xPos = init;
char direction = 'N';

bool r_drop = false;
bool r_rise = false;
double r_dropPos = 0;
double r_risePos = r_dropPos;
double r_time = 0;

bool b_drop = false;
bool b_rise = false;
double b_dropPos = 0;
double b_risePos = b_dropPos;
double b_time = 0;

bool y_drop = false;
bool y_rise = false;
double y_dropPos = 0;
double y_risePos = y_dropPos;
double y_time = 0;

bool g_drop = false;
bool g_rise = false;
double g_dropPos = 0;
double g_risePos = g_dropPos;
double g_time = 0;

bool p_drop = false;
bool p_rise = false;
double p_dropPos = 0;
double p_risePos = p_dropPos;
double p_time = 0;

double nextPos(double time) {
	return (1 * time + 0.5 * 15 * time * time);
}

/*
* DrawRoom
*	This will render the entire scene (in other words, draw the room).
*/
void DrawRoom()
{
	/* You also could do this at front by using the SDL surface's values or in an array. */
	static float WallTexWidth(0.f);
	static float WallTexHeight(0.f);

	static float FloorTexWidth(0.f);
	static float FloorTexHeight(0.f);

	static bool Once(false);
	static bool doggie(false);
	static int listNum = -1;
	/* Perform this check only once. */
	if (!Once)
	{
		/* Bind the wall texture. */
		glBindTexture(GL_TEXTURE_2D, Textures[0]);

		/* Retrieve the width and height of the current texture (can also be done up front with SDL and saved somewhere). */
		glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &WallTexWidth);
		glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &WallTexHeight);

		/* Bind the floor texture. */
		glBindTexture(GL_TEXTURE_2D, Textures[1]);

		/* Retrieve the width and height of the current texture (can also be done up front with SDL and saved somewhere). */
		glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &FloorTexWidth);
		glGetTexLevelParameterfv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &FloorTexHeight);

		

		glEnable(GL_LIGHTING);
		

		Once = true;
	}

	glPushMatrix();

	/* Move the world and rotate the view. */
	glRotated(ViewAngleVer, 1, 0, 0);
	glRotated(ViewAngleHor, 0, 1, 0);

	glTranslated(-X, -Y, -Z);

	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	glShadeModel(GL_SMOOTH);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);
	glEnable(GL_COLOR_MATERIAL);
	//glClear(GL_COLOR_BUFFER_BIT);
	GLfloat normal[] = { 1, 1, 1, 1 };
	GLfloat ballSpecular[] = { 1, 1, 1, 1 };
	GLfloat ballEmission[] = { 0, 0, 0, 1 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ballSpecular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, ballEmission);

	GLfloat redballAmbient[] = { 0.9725, 0.3137, 0.2706, 1 };
	GLfloat redballDiffuse[] = { 0.9725, 0.3137, 0.2706, 0.5 };
	GLfloat redballDiffuse2[] = { 0.9725, 0.3137, 0.2706, 0.2 };
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, redballAmbient);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, redballDiffuse);
	glTranslated(xPos, 0, -1.2);
	//std::cout << xPos << " " << (xPos <= -0.7501) << endl;
	if (xPos <= init-0.2) direction = 'P';
	else if (xPos >= init+0.2) direction = 'N';
	if (direction == 'N') xPos -= 0.0001;
	else xPos += 0.0001;
	GLfloat red_light_position[] = { 0,0,0,1 };
	//std::cout << currLightPosY << endl;
	//glLightfv(GL_LIGHT1, GL_POSITION, red_light_position);
	//glLightfv(GL_LIGHT1, GL_AMBIENT, redballAmbient);
	//glLightfv(GL_LIGHT1, GL_DIFFUSE, redballDiffuse);
	//glLightfv(GL_LIGHT1, GL_SPECULAR, ballSpecular);
	//
	////glEnable(GL_LIGHT1);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, redballAmbient);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, redballAmbient);
	glPushMatrix();
	if (r_drop) {
		glTranslated(0, r_dropPos, 0);
		r_time += 0.0001;
		r_dropPos = r_dropPos - nextPos(r_time);
		r_dropPos = r_dropPos <= -0.5 ? -0.5 : r_dropPos;
		r_risePos = r_dropPos;
	}
	if (r_rise) {
		glTranslated(0, r_risePos, 0);
		r_risePos = r_risePos + 0.001;
		if (r_risePos >= 0) {
			r_risePos = 0;
			r_rise = false;

		}
			
		r_dropPos = r_risePos;
	}
		glColor3f(0.9725, 0.3137, 0.2706);
		glutSolidSphere(0.1, 20, 20);
		glPushMatrix();
		glScalef(1.1, 1.1, 1.1);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, redballDiffuse);
		glutSolidSphere(0.1, 20, 20);
		glPopMatrix();
	glPopMatrix();

	glTranslated(0.5, 0, 0);
	GLfloat blue_light_position[] = { 0,0,0,1 };
	GLfloat blueballAmbient[] = { 0.27451, 0.41961, 0.72549, 1 };
	GLfloat blueballDiffuse[] = { 0.27451, 0.41961, 0.72549, 0.5 };
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, blueballAmbient)
	//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, blueballDiffuse);
	glLightfv(GL_LIGHT2, GL_POSITION, blue_light_position);
	glLightfv(GL_LIGHT2, GL_AMBIENT, blueballAmbient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, blueballDiffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, ballSpecular);
	//glEnable(GL_LIGHT2);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, blueballAmbient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, blueballAmbient);
	glPushMatrix();
	if (b_drop) {
		glTranslated(0, b_dropPos, 0);
		b_time += 0.0001;
		b_dropPos = b_dropPos - nextPos(b_time);
		b_dropPos = b_dropPos <= -0.5 ? -0.5 : b_dropPos;
		b_risePos = b_dropPos;
	}
	if (b_rise) {
		glTranslated(0, b_risePos, 0);
		b_risePos = b_risePos + 0.001;
		if (b_risePos >= 0) {
			b_risePos = 0;
			b_rise = false;

		}

		b_dropPos = b_risePos;
	}
		glColor3f(0.27451, 0.41961, 0.72549);
		glutSolidSphere(0.1, 20, 20);
		glPushMatrix();
		glScalef(1.1, 1.1, 1.1);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, blueballDiffuse);
		glutSolidSphere(0.1, 20, 20);
		glPopMatrix();
	glPopMatrix();

	GLfloat yellowballAmbient[] = { 0.9922, 0.9647, 0.3569, 1 };
	GLfloat yellowballDiffuse[] = { 0.9922, 0.9647, 0.3569, 0.5 };
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, yellowballAmbient);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, yellowballDiffuse);
	glTranslated(0.5, 0, 0);
	glLightfv(GL_LIGHT3, GL_POSITION, red_light_position);
	glLightfv(GL_LIGHT3, GL_AMBIENT, yellowballAmbient);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, yellowballDiffuse);
	glLightfv(GL_LIGHT3, GL_SPECULAR, ballSpecular);
	//glEnable(GL_LIGHT3);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, yellowballAmbient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, yellowballAmbient);
	glPushMatrix();
		if (y_drop) {
			glTranslated(0, y_dropPos, 0);
			y_time += 0.0001;
			y_dropPos = y_dropPos - nextPos(y_time);
			y_dropPos = y_dropPos <= -0.5 ? -0.5 : y_dropPos;
			y_risePos = y_dropPos;
		}
		if (y_rise) {
			glTranslated(0, y_risePos, 0);
			y_risePos = y_risePos + 0.001;
			if (y_risePos >= 0) {
				y_risePos = 0;
				y_rise = false;

			}

			y_dropPos = y_risePos;
		}
		glColor3f(0.9922, 0.9647, 0.3569);
		glutSolidSphere(0.1, 20, 20);
		glColor3f(0.9922, 0.9647, 0.3569);
		glutSolidSphere(0.1, 20, 20);
		glPushMatrix();
		glScalef(1.1, 1.1, 1.1);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, yellowballDiffuse);
		glutSolidSphere(0.1, 20, 20);
		glPopMatrix();
	glPopMatrix();

	GLfloat greenballAmbient[] = { 0.4392, 0.9373, 0.502, 1 };
	GLfloat greenballDiffuse[] = { 0.4392, 0.9373, 0.502, 0.5 };
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, greenballAmbient);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, greenballDiffuse);
	glTranslated(0.5, 0, 0);
	glLightfv(GL_LIGHT4, GL_POSITION, red_light_position);
	glLightfv(GL_LIGHT4, GL_AMBIENT, greenballAmbient);
	glLightfv(GL_LIGHT4, GL_DIFFUSE, greenballDiffuse);
	glLightfv(GL_LIGHT4, GL_SPECULAR, ballSpecular);
	//glEnable(GL_LIGHT4);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, greenballAmbient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, greenballAmbient);
	glPushMatrix();
		if (g_drop) {
			glTranslated(0, g_dropPos, 0);
			g_time += 0.0001;
			g_dropPos = g_dropPos - nextPos(g_time);
			g_dropPos = g_dropPos <= -0.5 ? -0.5 : g_dropPos;
			g_risePos = g_dropPos;
		}
		if (g_rise) {
			glTranslated(0, g_risePos, 0);
			g_risePos = g_risePos + 0.001;
			if (g_risePos >= 0) {
				g_risePos = 0;
				g_rise = false;

			}

			g_dropPos = g_risePos;
		}
		glColor4fv(greenballAmbient);
		glutSolidSphere(0.1, 20, 20);
		glPushMatrix();
		glScalef(1.1, 1.1, 1.1);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, greenballDiffuse);
		glutSolidSphere(0.1, 20, 20);
		glPopMatrix();
	glPopMatrix();

	GLfloat purple_light_position[] = { 0,0,0,1 };
	GLfloat purpleballAmbient[] = { 0.6863, 0.4353, 0.7725, 1 };
	GLfloat purpleballDiffuse[] = { 0.6863, 0.4353, 0.7725, 0.5 };
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, purpleballAmbient);
	//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, purpleballDiffuse);
	glTranslated(0.5, 0, 0);
	glLightfv(GL_LIGHT5, GL_POSITION, purple_light_position);
	glLightfv(GL_LIGHT5, GL_AMBIENT, purpleballAmbient);
	glLightfv(GL_LIGHT5, GL_DIFFUSE, purpleballDiffuse);
	glLightfv(GL_LIGHT5, GL_SPECULAR, ballSpecular);
	
	//glEnable(GL_LIGHT5);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, purpleballAmbient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, purpleballAmbient);
	glPushMatrix();
		if (p_drop) {
			glTranslated(0, p_dropPos, 0);
			p_time += 0.0001;
			p_dropPos = p_dropPos - nextPos(p_time);
			p_dropPos = p_dropPos <= -0.5 ? -0.5 : p_dropPos;
			p_risePos = p_dropPos;
		}
		if (p_rise) {
			glTranslated(0, p_risePos, 0);
			p_risePos = p_risePos + 0.001;
			if (p_risePos >= 0) {
				p_risePos = 0;
				p_rise = false;

			}

			p_dropPos = p_risePos;
		}
		glColor3f(0.6863, 0.4353, 0.7725);
		glutSolidSphere(0.1, 20, 20);
		glColor3f(1, 1, 1);
		glPushMatrix();
		glScalef(1.1, 1.1, 1.1);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, purpleballDiffuse);
		glutSolidSphere(0.1, 20, 20);
		glPopMatrix();
	glPopMatrix();
	
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, normal);
	//glFlush();
	glPopAttrib();
	glPopMatrix();

	glPushMatrix();
	GLfloat light_position[] = { -2, 0, -1,1 };
	GLfloat light_2_position[] = { 0,0,-6,0 };
	//std::cout << currLightPosY << endl;
	GLfloat ambient[] = { 0,0,0,1 };
	GLfloat diffuse[] = { 1,1,1,1 };
	GLfloat specular[] = { 0,0,0,1 };
	GLfloat glLight[] = { 1, 1, 1, 1 };

	//glClear(GL_COLOR_BUFFER_BIT);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightModelfv(GL_FRONT_AND_BACK, glLight);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT6, GL_POSITION, light_2_position);
	glLightfv(GL_LIGHT6, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT6, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT6, GL_SPECULAR, specular);
	glLightModelfv(GL_FRONT_AND_BACK, glLight);
	glEnable(GL_LIGHT6);
	glPopMatrix();

	/* Set the coordinate system. */
	glOrtho(0, 800, 600, 0, -1, 1);
	
	
	


	/* Draw walls. */
	glBindTexture(GL_TEXTURE_2D, Textures[0]);

	glBegin(GL_QUADS);
	/* Wall in front of you when the app starts. */
	glTexCoord2f(0, 0);
	glVertex3d(-200, 0, 4.0);

	glTexCoord2f(1200.f / WallTexWidth, 0);
	glVertex3d(1000, 0, 4.0);

	glTexCoord2f(1200.f / WallTexWidth, 400.f / WallTexHeight);
	glVertex3d(1000, 500, 4.0);

	glTexCoord2f(0, 400.f / WallTexHeight);
	glVertex3d(-200, 500, 4.0);

	/* Wall left of you. */
	glTexCoord2f(0, 0);
	glVertex3d(-200, 0, -4.0);

	glTexCoord2f(1200.f / WallTexWidth, 0);
	glVertex3d(-200, 0, 4.0);

	glTexCoord2f(1200.f / WallTexWidth, 400.f / WallTexHeight);
	glVertex3d(-200, 500, 4.0);

	glTexCoord2f(0, 400.f / WallTexHeight);
	glVertex3d(-200, 500, -4.0);

	/* Wall right of you. */
	glTexCoord2f(0, 0);
	glVertex3d(1000, 0, 4.0);

	glTexCoord2f(1200.f / WallTexWidth, 0);
	glVertex3d(1000, 0, -4.0);

	glTexCoord2f(1200.f / WallTexWidth, 400.f / WallTexHeight);
	glVertex3d(1000, 500, -4.0);

	glTexCoord2f(0, 400.f / WallTexHeight);
	glVertex3d(1000, 500, 4.0);

	/* Wall behind you (you won't be able to see this just yet, but you will later). */
	glTexCoord2f(0, 0);
	glVertex3d(1000, 0, -4.0);

	glTexCoord2f(1200.f / WallTexWidth, 0);
	glVertex3d(-200, 0, -4.0);

	glTexCoord2f(1200.f / WallTexWidth, 400.f / WallTexHeight);
	glVertex3d(-200, 500, -4.0);

	glTexCoord2f(0, 400.f / WallTexHeight);
	glVertex3d(1000, 500, -4.0);
	glEnd();


	/* Draw the floor and the ceiling, this is done separatly because glBindTexture isn't allowed inside glBegin. */
	glBindTexture(GL_TEXTURE_2D, Textures[1]);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3d(-200, 500, 4.0);

	glTexCoord2f(1200.f / FloorTexWidth, 0);
	glVertex3d(1000, 500, 4.0);

	glTexCoord2f(1200.f / FloorTexWidth, (8.f / 2.f * 600.f) / FloorTexHeight);
	glVertex3d(1000, 500, -4.0);

	glTexCoord2f(0, (8.f / 2.f * 600.f) / FloorTexHeight);
	glVertex3d(-200, 500, -4.0);

	/* Ceiling. */
	glTexCoord2f(0, 0);
	glVertex3d(-200, 0, 4.0);

	glTexCoord2f(1200.f / FloorTexWidth, 0);
	glVertex3d(1000, 0, 4.0);

	glTexCoord2f(1200.f / FloorTexWidth, (8.f / 2.f * 600.f) / FloorTexHeight);
	glVertex3d(1000, 0, -4.0);

	glTexCoord2f(0, (8.f / 2.f * 600.f) / FloorTexHeight);
	glVertex3d(-200, 0, -4.0);
	glEnd();

	

	/* Now we're going to render some boxes using display lists. */
	glPushMatrix();
	/* Let's make it a bit smaller... */
	glScaled(0.5, 0.4, 0.5);

	

	/* Can't bind textures while generating a display list, but we can give it texture coordinates and bind it now. */
	glBindTexture(GL_TEXTURE_2D, Textures[2]);

	/*
	* Because display lists have preset coordinates, we'll need to translate it to move it around. Note that we're
	* moving the small version of the cube around, not the big version (because we scaled *before* translating).
	*/
	glTranslated(-700, 750, 6);

	/*
	* Let's draw a whole lot of boxes. Note that because we're not pushing and popping matrices, translations
	* and changes will 'accumulate' and add to the previous translation.
	*/
	for (short i(0); i < 12; ++i)
	{
		glTranslated(350, 0, 0);

		/* These make sure that every once in a while, a new row is started. */
		if (i == 5)		glTranslated(-1575, -350, 0);
		if (i == 9)		glTranslated(-1225, -350, 0);

		/*
		* glCallList is all that is really needed to execute the display list. Remember to try the 'K' button
		* to turn on wireframe mode, with these extra polygons, it looks pretty neat!
		*/
		glCallList(BoxList);

		if (!doggie) {
			listNum = loadObject("Wolf_obj.obj");
			doggie = true;
		}
	}

	

	glPopMatrix();

	glPopMatrix();
	glCallList(listNum);
	//glBindTexture(GL_TEXTURE_2D, Textures[0]);
	

}

void menu() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
	glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer (background)

										  // Draw a Red 1x1 Square centered at origin
	glBegin(GL_QUADS);              // Each set of 4 vertices form a quad
	glColor3f(1.0f, 0.0f, 0.0f); // Red
	glVertex2f(-0.5f, -0.5f);    // x, y
	glVertex2f(0.5f, -0.5f);
	glVertex2f(0.5f, 0.5f);
	glVertex2f(-0.5f, 0.5f);
	glEnd();

	glFlush();
}

void theMenu(int val) {

	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	/* Initialize SDL and set up a window. */
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_WM_SetCaption("OpenGL - Display Lists", 0);
	SDL_WM_GrabInput(SDL_GRAB_ON);

	SDL_ShowCursor(SDL_DISABLE);

	SDL_SetVideoMode(800, 600, 32, SDL_OPENGL);

	/* Basic OpenGL initialization, handled in 'The Screen'. */
	glShadeModel(GL_SMOOTH);
	glClearColor(0, 0, 0, 1);

	glViewport(0, 0, 800, 600);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(80.0, 800.0 / 600.0, 0.1, 100.0);
	
	/* We now switch to the modelview matrix. */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);

	glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	/* Enable 2D texturing. */
	glEnable(GL_TEXTURE_2D);

	/* Set up alpha blending. */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4d(1, 1, 1, 1);

	Textures[0] = GrabTexObjFromFile("Data/star.png");
	Textures[1] = GrabTexObjFromFile("Data/star.png");
	Textures[2] = GrabTexObjFromFile("Data/Box.png");			//Added!

																//Replaced this with a loop that immediately checks the entire array.
																//sizeof(Textures) is the size of the entire array in bytes (unsigned int = 4 bytes)
																//so sizeof(Textures) would give 3 * 4 = 12 bytes, divide this by 4 bytes and you
																//have 3.
	for (unsigned i(0); i < sizeof(Textures) / sizeof(unsigned); ++i)
	{
		if (Textures[i] == 0)
		{
#ifdef _WIN32
			MessageBoxA(0, "Something went seriously wrong!", "Fatal Error!", MB_OK | MB_ICONERROR);
#endif //_WIN32

			return 1;
		}
	}

	/* Compile the display lists. */
	CompileLists();

	SDL_Event event;

	bool Menu(false);

	int RelX(0), RelY(0);
	int MovementDelay(SDL_GetTicks());

	bool Wireframe(false);
	bool Keys[4] =
	{
		false, /* Up arrow down? */
		false, /* Down arrow down? */
		false, /* Left arrow down? */
		false  /* Right arrow down? */
	};

	//glutCreateWindow("version 3");
	glutCreateMenu(theMenu);
	glutAddMenuEntry("hello", 1);
	glutAddMenuEntry("hello", 2);
	glutAddMenuEntry("hello", 3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	//glutEnterGameMode();

	/* Application loop. */
	for (;;)
	{
		/* Handle events with SDL. */
		if (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				break;

			/* Mouse events? */
			else if (event.type == SDL_MOUSEMOTION && !Menu)
			{
				/* Get the relative mouse movement of the mouse (based on CurMouseCoord - PrevMouseCoord). */
				SDL_GetRelativeMouseState(&RelX, &RelY);

				ViewAngleHor += RelX / 4;
				ViewAngleVer += RelY / 4;

				/* Prevent the horizontal angle from going over 360 degrees or below 0 degrees. */
				if (ViewAngleHor >= 360.0)		ViewAngleHor = 0.0;
				else if (ViewAngleHor < 0.0)		ViewAngleHor = 360.0;

				/* Prevent the vertical view from moving too far (comment this out to get a funny effect). */
				if (ViewAngleVer > 60.0)			ViewAngleVer = 60.0; /* 60 degrees is when you're looking down. */
				else if (ViewAngleVer < -60.0)	ViewAngleVer = -60.0; /* This is when you're looking up. */

																	  /* This delay might seem strange, but it helps smoothing out the mouse if you're experiencing jittering. */
				SDL_Delay(5);
			}

			else if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE)
					if (Menu) { 
						Menu = false;
						glEnable(GL_DEPTH_TEST);
						SDL_ShowCursor(SDL_DISABLE);
					}
					else break;
				else if (event.key.keysym.sym == SDLK_m) {
					Menu = true;
					glDisable(GL_DEPTH_TEST);
					SDL_ShowCursor(SDL_ENABLE);
				}
				else if (!Menu) {
					if (event.key.keysym.sym == SDLK_k)
						glPolygonMode(GL_FRONT_AND_BACK, ((Wireframe = !Wireframe) ? GL_LINE : GL_FILL));

					if (event.key.keysym.sym == SDLK_b) {
						PlaySound(TEXT("Data/bark.wav"), NULL, SND_FILENAME || SND_ASYNC);
					}

					if (event.key.keysym.sym == SDLK_a) {
						PlaySound(TEXT("Data/c.wav"), NULL, SND_FILENAME || SND_ASYNC);
						r_drop = true;
						r_rise = false;
							
					}

					if (event.key.keysym.sym == SDLK_s) {
						PlaySound(TEXT("Data/d.wav"), NULL, SND_FILENAME || SND_ASYNC);
						b_drop = true;
						b_rise = false;
					}

					if (event.key.keysym.sym == SDLK_d) {
						PlaySound(TEXT("Data/e.wav"), NULL, SND_FILENAME || SND_ASYNC);
						y_drop = true;
						y_rise = false;
					}

					if (event.key.keysym.sym == SDLK_f) {
						PlaySound(TEXT("Data/f.wav"), NULL, SND_FILENAME || SND_ASYNC);
						g_drop = true;
						g_rise = false;
					}

					if (event.key.keysym.sym == SDLK_g) {
						PlaySound(TEXT("Data/g.wav"), NULL, SND_FILENAME || SND_ASYNC);
						p_drop = true;
						p_rise = false;
					}


					if (event.key.keysym.sym == SDLK_UP)		Keys[0] = true;
					if (event.key.keysym.sym == SDLK_DOWN)		Keys[1] = true;
					if (event.key.keysym.sym == SDLK_LEFT)		Keys[2] = true;
					if (event.key.keysym.sym == SDLK_RIGHT)		Keys[3] = true;
				}

				
			}

			else if (event.type == SDL_KEYUP && !Menu)
			{
				
				if (event.key.keysym.sym == SDLK_a) {
					r_rise = true;
					r_drop = false;
					r_time = 0;
				}
				if (event.key.keysym.sym == SDLK_s) {
					b_rise = true;
					b_drop = false;
					b_time = 0;
				}
				if (event.key.keysym.sym == SDLK_d) {
					y_rise = true;
					y_drop = false;
					y_time = 0;
				}
				if (event.key.keysym.sym == SDLK_f) {
					g_rise = true;
					g_drop = false;
					g_time = 0;
				}
				if (event.key.keysym.sym == SDLK_g) {
					p_rise = true;
					p_drop = false;
					p_time = 0;
				}
				if (event.key.keysym.sym == SDLK_UP)		Keys[0] = false;
				if (event.key.keysym.sym == SDLK_DOWN)		Keys[1] = false;
				if (event.key.keysym.sym == SDLK_LEFT)		Keys[2] = false;
				if (event.key.keysym.sym == SDLK_RIGHT)		Keys[3] = false;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glPushMatrix();
		if (!Menu) {
			
			DrawRoom();
			
		}
		else {
			menu();
		}
		glPopMatrix();
		
		
		

		/* Move if the keys are pressed, this is explained in the tutorial. */
		if (Keys[0])
		{
			X -= cos(DegreeToRadian(ViewAngleHor + 90.0)) * 0.025;
			Z -= sin(DegreeToRadian(ViewAngleHor + 90.0)) * 0.025;
		}

		if (Keys[1])
		{
			X += cos(DegreeToRadian(ViewAngleHor + 90.0)) * 0.025;
			Z += sin(DegreeToRadian(ViewAngleHor + 90.0)) * 0.025;
		}

		if (Keys[2])
		{
			X += cos(DegreeToRadian(ViewAngleHor + 180.0)) * 0.025;
			Z += sin(DegreeToRadian(ViewAngleHor + 180.0)) * 0.025;
		}

		if (Keys[3])
		{
			X -= cos(DegreeToRadian(ViewAngleHor + 180.0)) * 0.025;
			Z -= sin(DegreeToRadian(ViewAngleHor + 180.0)) * 0.025;
		}

		/* Swap the display buffers. */
		SDL_GL_SwapBuffers();
	}

	/* Delete the created textures. */
	glDeleteTextures(3, Textures);		//Changed to 3.
	glDeleteLists(BoxList, 1);

	/* Clean up. */
	SDL_Quit();

	return 0;
}
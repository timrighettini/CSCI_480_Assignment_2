// assign2.cpp : Defines the entry point for the console application.
//

/*
	CSCI 480 Computer Graphics
	Assignment 2: Simulating a Roller Coaster
	C++ starter code
*/

#include "stdafx.h"
#include <pic.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <GL/glu.h>
#include <GL/glut.h>

// Include other libraries
#include <iostream>
#include <string>
#include <sstream> 

/* represents one control point along the spline */
struct point {
	double x;
	double y;
	double z;
};

/* spline struct which contains how many control points, and an array of control points */
struct spline {
	int numControlPoints;
	struct point *points;
};

/* the spline array */
struct spline *g_Splines;

/* total number of splines */
int g_iNumOfSplines;

/*Template functions and variables from the first assignment*/

int g_iMenuId; // Will be used for the side menu

// Animation values
bool saveScreenShotOn = false; // Will determine whether a screenshot is to be saved out or not
int frameNum = 0; // Number of frames created for the animation
char* fName; // Char array to be used for the correct handling of fileNames

/* Assignment #1 Callback vars*/
int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;

CONTROLSTATE g_ControlState = ROTATE;

/* state of the world */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};

/***** Values for scaling mouse enabled rotation/translation/scaling to make mouse movement easier*****/
float translateMultDPI = 0.5;
float rotationMultDPI = 1;
float scaleMultDPI = 1;
/***** *****/
/* END */

/* New values for the second assignment */
// These values will hold the vertex on a spline based on control points
float xVal;
float yVal;
float zVal;
float s = 0.5; // This will contain the s parameter in the C-R formula
float MAX_LINE_LEN = 0.075; // Will hold how big a line segment should be within a spline
// These values will hold copies of the control points to be used in spline calculations
point p0; // Pi-1
point p1; // Pi
point p2; // Pi+1
point p3; // Pi+2

// These values will be used for the textures
Pic* groundTexture; // Will hold the image to be used for the ground plane texture
GLuint groundTextureID; // This value will hold the texture ID for the ground plane

Pic* skyTexture; // Will hold the image to be used for the sky plane texture 
GLuint skyTextureID; // This value will hold the texture ID for the sky plane

// These values will hold the what the base left/right/top/bottom values a texture plane will be
float rectangleRight = 0.5;
float rectangleLeft = -0.5;

float rectangleUp = -0.5;
float rectangleDown = 0.5;
/*These values will have to be multipled by -1 when being represented in the z-axis because of the right handed coordinate system*/

/* Write a screenshot to the specified filename */ 
void saveScreenshot (char *filename)
{
  int i;//, j;
  Pic *in = NULL;

  if (filename == NULL)
    return;

  /* Allocate a picture buffer */
  in = pic_alloc(640, 480, 3, NULL);

  printf("File to save to: %s\n", filename);

  for (i=479; i>=0; i--) {
    glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                 &in->pix[i*in->nx*in->bpp]);
  }

  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);
}

void positionCamera() { // This method will set the camera to point and be in the appropriate places
	gluLookAt(
		0.0, 0.0, 3.0, // Where camera is placed
		0.0, 0.0, 0.0,  // Where the center of the scene is
		0.0, 1.0, 0.0   // The "up" vector, which in my case, is the Unit Y Vector
	);
}

void myinit() {	/* setup gl view here */
	glClearColor(0.0, 0.0, 0.0, 0.0);

	/* Set up the shading type*/
	glShadeModel(GL_SMOOTH);

	// Enable the Depth Test and Mask so that z-buffering is enabled correctly
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	// Enable polygon offset
	glEnable(GL_POLYGON_OFFSET_LINE);

	// Enable GL_POINTS properties
	glPointSize(10);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);

	// Enable GL_LINES properties
	glLineWidth(5);
}

void myinitTexture() {
	// Load my texture images into memory
	groundTexture = jpeg_read("testGround.jpg", NULL); // This will return a Pic struct with information about the image relating to a texture
	skyTexture = jpeg_read("test.jpg", NULL); // This will return a Pic struct with information about the image relating to a texture

	if (!groundTexture || !skyTexture) {
		std::cout << "Ground or Sky Image NOT LOADED into the program, exiting..." << std::endl;
		exit(1);
	}
	else { // Show the BPP for the images
		std::cout << "BPP groundTexture: " << groundTexture->bpp << std::endl;
		std::cout << "BPP skyTexture: " << skyTexture->bpp << std::endl;
	}

	// The images have now been loaded into memory, now let's actually set them up as textures

	// Set up the global texture reference IDs
	groundTextureID = 0; // Reference variable for the ground texture ID
	skyTextureID = 1; // Reference variable for the sky texture ID

	// Tell OpenGL to make room for the new textures
	glGenTextures(1, &groundTextureID);
	glGenTextures(1, &skyTextureID);

	// I'll do the initial binds and other parameter settings to the ground texture first
	glBindTexture(GL_TEXTURE_2D, groundTextureID);

	// Set up the texture wrapping parimeters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repeat the S texture coordinate
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repeat the T texture coordinate

	// Set up minmapping (it's got to look good at a distance, right?)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// I will load in the image data at the start of the display function, since I have two textures to use at a time

	// To test for now, load in the ground plane image to use for a texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, groundTexture->nx, groundTexture->ny, 0, GL_RGB, GL_UNSIGNED_BYTE, groundTexture->pix);
}


/* Assignment 1 callbacks */
/* converts mouse drags into information about 
rotation/translation/scaling */
void mousedrag(int x, int y)
{
  int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};
  
  switch (g_ControlState)
  {
    case TRANSLATE:  
      if (g_iLeftMouseButton)
      {
        g_vLandTranslate[0] += vMouseDelta[0]*0.01;
        g_vLandTranslate[1] -= vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandTranslate[2] += vMouseDelta[1]*0.01;
      }
      break;
    case ROTATE:
      if (g_iLeftMouseButton)
      {
        g_vLandRotate[0] += vMouseDelta[1];
        g_vLandRotate[1] += vMouseDelta[0];
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandRotate[2] += vMouseDelta[1];
      }
      break;
    case SCALE:
      if (g_iLeftMouseButton)
      {
        g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
        g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
      }
      break;
  }
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{

  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      g_iLeftMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_MIDDLE_BUTTON:
      g_iMiddleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      g_iRightMouseButton = (state==GLUT_DOWN);
      break;
  }
 
  switch(glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      g_ControlState = TRANSLATE;
      break;
    case GLUT_ACTIVE_SHIFT:
      g_ControlState = SCALE;
      break;
    default:
      g_ControlState = ROTATE;
      break;
  }

  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}
/* END */

/* Assignment 2 Functions */
void drawControlPoints() {
	glBegin(GL_POINTS);
	for (int i = 0; i < g_iNumOfSplines; i++) {
		for (int j = 0; j < g_Splines[i].numControlPoints; j++) { // Draw the control points, and see if the spline goes through
			glColor3f(1.0, 0.0, 0.0); 
			glVertex3f(g_Splines[i].points[j].x, g_Splines[i].points[j].y, g_Splines[i].points[j].z);			
		}
	}
	glEnd();
}

void drawLine(point v0, point v1) {
	// Draw the first point	
	glColor3f(1.0, 1.0, 1.0);
	glVertex3f(v0.x, v0.y, v0.z);
	
	// Draw the second point
	glColor3f(1.0, 1.0, 1.0);
	glVertex3f(v1.x, v1.y, v1.z);
}


point getCoordinateXYZ(float u) {
	point p; // Instantiate the point

	// Use a derived version of the C-R matrix formula
	p.x = ( p0.x * ( -(pow(u, 3) * s)       + (2 * pow(u, 2) * s)         - (u * s) )  ) + 
		  ( p1.x * (  (pow(u, 3) * (2 - s)) + (pow(u, 2) * ((s - 3)))     + (  1  ) )  ) + 
		  ( p2.x * (  (pow(u, 3) * (s - 2)) + (pow(u, 2) * (3 - (2 * s))) + (u * s) )  ) + 
		  ( p3.x * (  (pow(u, 3) * s)       - (pow(u, 2) * s) )  );

	p.y = ( p0.y * ( -(pow(u, 3) * s)       + (2 * pow(u, 2) * s)         - (u * s) )  ) + 
		  ( p1.y * (  (pow(u, 3) * (2 - s)) + (pow(u, 2) * ((s - 3)))     + (  1  ) )  ) + 
		  ( p2.y * (  (pow(u, 3) * (s - 2)) + (pow(u, 2) * (3 - (2 * s))) + (u * s) )  ) + 
		  ( p3.y * (  (pow(u, 3) * s)       - (pow(u, 2) * s) )  );

	p.z = ( p0.z * ( -(pow(u, 3) * s)       + (2 * pow(u, 2) * s)         - (u * s) )  ) + 
		  ( p1.z * (  (pow(u, 3) * (2 - s)) + (pow(u, 2) * ((s - 3)))     + (  1  ) )  ) + 
		  ( p2.z * (  (pow(u, 3) * (s - 2)) + (pow(u, 2) * (3 - (2 * s))) + (u * s) )  ) + 
		  ( p3.z * (  (pow(u, 3) * s)       - (pow(u, 2) * s) )  );
	
	return p;
}

// Spline Functions for assignment #2
void drawSpline(float u0, float u1, float maxLineLengthSquared) {
	float uMidPoint = (u0 + u1) / (float)2; // Get the midpoint between these two values
	point c0 = getCoordinateXYZ(u0); // Get the coordindate for u0
	point c1 = getCoordinateXYZ(u1); // Get the coordindate for u1

	// Get the distance between these two coordinates
	point distance;
	distance.x = c1.x - c0.x;
	distance.y = c1.y - c0.y;
	distance.z = c1.z - c0.z;

	// Compare the magnitude SQUARED of this vector to maxLineLength squared -- saves on calculating a sqrt()
	float magnitudeSquared = pow(distance.x, 2) + pow(distance.y, 2) + pow(distance.z, 2);

	// If mag^2 > maxLL^2 -- Do the recursive call with the midpoint
	if (magnitudeSquared > maxLineLengthSquared) {
		drawSpline(u0, uMidPoint, maxLineLengthSquared);
		drawSpline(uMidPoint, u1, maxLineLengthSquared);
	}	
	// Else -- Draw the line
	else {
		drawLine(c0, c1);
	}
}

void drawAllSplines() {
	for (int i = 0; i < g_iNumOfSplines; i++) {
		glBegin(GL_LINES);
		int loopCondition = 0; // This value will be used to determine how many segments are drawn for any given spline
		for (int j = 1; j <= g_Splines[i].numControlPoints - 3; j++) {
			// Start at the second point, because the first point doesn't help attach to the curve at the beginning
			// End at the third to last point, because the last of the points doesn't help attach to the curve either at the end
			/* Set the four control points */
			p0 = g_Splines[i].points[j-1]; // Pi-1
			p1 = g_Splines[i].points[j]; // Pi
			p2 = g_Splines[i].points[j+1]; // Pi+1
			p3 = g_Splines[i].points[j+2]; // Pi+2
			
			/* Go into the spline drawing function */
			drawSpline(0, 1, pow(MAX_LINE_LEN, 2));
		}
		glEnd();
	}
}

/* represents one control point along the spline
struct point {
	double x;
	double y;
	double z;
};

spline struct which contains how many control points, and an array of control points 
struct spline {
	int numControlPoints;
	struct point *points;
};
*/

void drawSkyBox(float groundPlaneSize) {
	// Enable OpenGL texturing
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // Use texture color only -- no lighting

	// Turn on the texturing
	glEnable(GL_TEXTURE_2D);

	// Do all of the skybox rendering here!

	// Load in the ground texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, groundTexture->nx, groundTexture->ny, 0, GL_RGB, GL_UNSIGNED_BYTE, groundTexture->pix);

	// Draw the ground plane first: xz plane with the lower bound of the sky: xz axis with -y/2 height from center
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(rectangleLeft  * groundPlaneSize, -groundPlaneSize/2, rectangleUp   * groundPlaneSize);
		glTexCoord2f(1.0, 0.0); glVertex3f(rectangleRight * groundPlaneSize, -groundPlaneSize/2, rectangleUp   * groundPlaneSize);
		glTexCoord2f(1.0, 1.0); glVertex3f(rectangleRight * groundPlaneSize, -groundPlaneSize/2, rectangleDown * groundPlaneSize);
		glTexCoord2f(0.0, 1.0); glVertex3f(rectangleLeft  * groundPlaneSize, -groundPlaneSize/2, rectangleDown * groundPlaneSize);
	glEnd();

	// Load in the sky texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, groundTexture->nx, groundTexture->ny, 0, GL_RGB, GL_UNSIGNED_BYTE, skyTexture->pix);

	// Draw the sky plane: xy axis with y/2 height from center
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(rectangleLeft  * groundPlaneSize, groundPlaneSize/2, rectangleUp   * groundPlaneSize);
		glTexCoord2f(1.0, 0.0); glVertex3f(rectangleRight * groundPlaneSize, groundPlaneSize/2, rectangleUp   * groundPlaneSize);
		glTexCoord2f(1.0, 1.0); glVertex3f(rectangleRight * groundPlaneSize, groundPlaneSize/2, rectangleDown * groundPlaneSize);
		glTexCoord2f(0.0, 1.0); glVertex3f(rectangleLeft  * groundPlaneSize, groundPlaneSize/2, rectangleDown * groundPlaneSize);
	glEnd();

	// Draw the sky plane: xy axis with -z/2 height from center
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(rectangleLeft  * groundPlaneSize, rectangleDown * groundPlaneSize, -groundPlaneSize/2);
		glTexCoord2f(1.0, 0.0); glVertex3f(rectangleRight * groundPlaneSize, rectangleDown * groundPlaneSize, -groundPlaneSize/2);
		glTexCoord2f(1.0, 1.0); glVertex3f(rectangleRight * groundPlaneSize, rectangleUp   * groundPlaneSize, -groundPlaneSize/2);
		glTexCoord2f(0.0, 1.0); glVertex3f(rectangleLeft  * groundPlaneSize, rectangleUp   * groundPlaneSize, -groundPlaneSize/2);
	glEnd();

	// Draw the sky plane: xy axis with z/2 height from center
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(rectangleLeft  * groundPlaneSize, rectangleDown * groundPlaneSize, groundPlaneSize/2);
		glTexCoord2f(1.0, 0.0); glVertex3f(rectangleRight * groundPlaneSize, rectangleDown * groundPlaneSize, groundPlaneSize/2);
		glTexCoord2f(1.0, 1.0); glVertex3f(rectangleRight * groundPlaneSize, rectangleUp   * groundPlaneSize, groundPlaneSize/2);
		glTexCoord2f(0.0, 1.0); glVertex3f(rectangleLeft  * groundPlaneSize, rectangleUp   * groundPlaneSize, groundPlaneSize/2);
	glEnd();

	// Draw the sky plane: yz axis with -x/2 width from center
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(-groundPlaneSize/2, rectangleDown * groundPlaneSize, rectangleLeft  * groundPlaneSize);
		glTexCoord2f(1.0, 0.0); glVertex3f(-groundPlaneSize/2, rectangleDown * groundPlaneSize, rectangleRight * groundPlaneSize);
		glTexCoord2f(1.0, 1.0); glVertex3f(-groundPlaneSize/2, rectangleUp   * groundPlaneSize, rectangleRight * groundPlaneSize);
		glTexCoord2f(0.0, 1.0); glVertex3f(-groundPlaneSize/2, rectangleUp   * groundPlaneSize, rectangleLeft  * groundPlaneSize);
	glEnd();

	// Draw the sky plane: yz axis with x/2 width from center
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(groundPlaneSize/2, rectangleDown * groundPlaneSize, rectangleLeft  * groundPlaneSize);
		glTexCoord2f(1.0, 0.0); glVertex3f(groundPlaneSize/2, rectangleDown * groundPlaneSize, rectangleRight * groundPlaneSize);
		glTexCoord2f(1.0, 1.0); glVertex3f(groundPlaneSize/2, rectangleUp   * groundPlaneSize, rectangleRight * groundPlaneSize);
		glTexCoord2f(0.0, 1.0); glVertex3f(groundPlaneSize/2, rectangleUp   * groundPlaneSize, rectangleLeft  * groundPlaneSize);
	glEnd();

	// Disable the texturing
	glDisable(GL_TEXTURE_2D);
}

/* Callback functions for Assignment #2 */
void reshape(int w, int h) { // This function will project the contents of the program correctly
	glViewport(0, 0, (GLsizei)w, (GLsizei)h); // Set the clipping area of the window to be correct

	// Set the matrix mode to projection, so that this will actually work
	glMatrixMode(GL_PROJECTION);

	// Now begin the actual reshaping
	glLoadIdentity(); // Reset the matrix

	// Set up the perspective projection matrix
	gluPerspective(60.0, (double)w/h, 0.01, 1000.0);
	
	positionCamera(); // Sets the camera position

	/*
		Viewing Angle: 60
		Aspect Ratio: 1.333 (4:3)
		Near Clipping Plane: 0.01
		Far  "            ": 1000.0
	*/
	// Set the matrix mode back to modelView, so things do not get messed up
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // Reset the matrix
}

char* createFileName() {
	std::string name = "";
	/*Append the appropriate number of 0's in front of the string*/
	if (frameNum < 10) {
		name.append("00");		
	}
	else if (frameNum < 100) {
		name.append("0");
	}

	std::stringstream s; // Use this to convert the number into a string
	s << frameNum; // Get the number in string format
	name.append(s.str()); // Put that formatted number into a string	
	name.append(".jpg"); // Need the image file type

	std::cout << name << std::endl;

	fName = new char[name.length() +1]; // Will be used to hold char* version of string
	strcpy(fName, name.c_str()); // Copy it in 

	return fName; // Return the fileName
}

void idle() {
  /* do some stuff... */
	if (saveScreenShotOn == true) {
		saveScreenshot(createFileName());
		frameNum++;
		delete[] fName; // Make sure to give the memory back
	}
  /* make the screen update */
  glutPostRedisplay();
}

void display() {
  /* draw 1x1 cube about origin */
  /* replace this code with your height field implementation */
  /* you may also want to precede it with your 
rotation/translation/scaling */

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Obviously, clear these values, or KABOOM!!!

	glMatrixMode(GL_MODELVIEW); // Set mode to ModelView Here, just to be sure...

	glLoadIdentity(); // Reset the Matrix

	/* Test Code for the spline */
	glPushMatrix(); // Push on the new transformations that are about to be done
	
	glTranslatef(
		(translateMultDPI * -g_vLandTranslate[0]), // Inverting this value (multiplying by -1) made it so that the shape followed the mouse for a-axis translations
		(translateMultDPI * g_vLandTranslate[1]),
		(translateMultDPI * -g_vLandTranslate[2])
	); // Translate the matrix

	glRotatef(g_vLandRotate[0], 1, 0, 0); // Rotate along the x-axis - This value was inverted (multiplied by -1) because it made more sense to me to invert the X-axis rotation; it's what I am used to. (Autodesk Maya usage)
	glRotatef(g_vLandRotate[1], 0, 1, 0); // Rotate along the y-axis - This value was inverted (multiplied by -1) because it made more sense to me to invert the Y-axis rotation; it's what I am used to. (Autodesk Maya usage)
	glRotatef(g_vLandRotate[2], 0, 0, 1); // Rotate along the z-axis

	glScalef(
		(scaleMultDPI * g_vLandScale[0]),
		(scaleMultDPI * g_vLandScale[1]),
		(scaleMultDPI * g_vLandScale[2])
	); // Scale the Matrix

	// Draw the textured skybox
	drawSkyBox(10.0);

	// Draw the control points
	drawControlPoints();

	// Draw out the splines
	drawAllSplines();

	glPopMatrix(); // Remove the transformation matrix

	glutSwapBuffers();
}

void keyPressed(unsigned char key, int x, int y) {	
	// Turns on/off the animation
	if (key == ' ') { // Space Pressed, turn screenshot saving on/off
		if (saveScreenShotOn == false) { 
			saveScreenShotOn = true;
			frameNum = 0;
		}
		else { 
			saveScreenShotOn = false;
			frameNum = 0;
		}		
	}
	// Reset Button for Assignment 1 Callbacks
	if (key == 27) { // Reset the position of the heightfield -- ESC key
		for (int i = 0; i < 3; i++) {
			g_vLandRotate[i] = 0.0;
			g_vLandTranslate[i] = 0.0;
			g_vLandScale[i] = 1.0;
		}
	}
}

void menuFunction(int value) {
  switch (value) {
    case 0:
      exit(0);
      break;
  }
}


int loadSplines(char *argv) {
	char *cName = (char *)malloc(128 * sizeof(char));
	FILE *fileList;
	FILE *fileSpline;
	int iType, i = 0, j, iLength;

	/* load the track file */
	fileList = fopen(argv, "r");
	if (fileList == NULL) {
		printf ("can't open file\n");
		exit(1);
	}
  
	/* stores the number of splines in a global variable */
	fscanf(fileList, "%d", &g_iNumOfSplines);

	g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

	/* reads through the spline files */
	for (j = 0; j < g_iNumOfSplines; j++) {
		i = 0;
		fscanf(fileList, "%s", cName);
		fileSpline = fopen(cName, "r");

		if (fileSpline == NULL) {
			printf ("can't open file\n");
			exit(1);
		}

		/* gets length for spline file */
		fscanf(fileSpline, "%d %d", &iLength, &iType);

		/* allocate memory for all the points */
		g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
		g_Splines[j].numControlPoints = iLength;

		/* saves the data to the struct */
		while (fscanf(fileSpline, "%lf %lf %lf", 
			&g_Splines[j].points[i].x, 
			&g_Splines[j].points[i].y, 
			&g_Splines[j].points[i].z) != EOF) {
			i++;
		}
	}

	free(cName);

	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// I've set the argv[1] to track.txt.
	// To change it, on the "Solution Explorer",
	// right click "assign1", choose "Properties",
	// go to "Configuration Properties", click "Debugging",
	// then type your track file name for the "Command Arguments"
	if (argc<2)
	{  
		printf ("usage: %s <trackfile>\n", argv[0]);
		exit(0);
	}

	loadSplines(argv[1]);

	// Do all of the OpenGL initialization stuff
	glutInit(&argc,argv);

	// Set up the window
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(640,480);
	glutInitWindowPosition(200,200);
	glutCreateWindow(argv[0]);

	/* tells glut to use a particular display function to redraw */
	glutDisplayFunc(display);

	// Add in the reshape function, so that the camera is aligned to the proper aspect ratio, and that the window draws correctly
	glutReshapeFunc(reshape);
  
	/* allow the user to quit using the right mouse button menu */
	g_iMenuId = glutCreateMenu(menuFunction);
	glutSetMenu(g_iMenuId);
	glutAddMenuEntry("Quit",0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
  
	/* replace with any animate code */
	glutIdleFunc(idle);

	// Callback for keyboard function
	glutKeyboardFunc(keyPressed);

	/* Assignment 1 Callbacks -- Used to test for spline accuracy*/	
	/* callback for mouse drags */
	glutMotionFunc(mousedrag);
	/* callback for idle mouse movement */
	glutPassiveMotionFunc(mouseidle);
	/* callback for mouse button changes */
	glutMouseFunc(mousebutton);
	/* END */

	/* do initializations */
	myinit(); // Do all of the Basic OpenGL initializations
	myinitTexture(); // Do all of the texture related initializations separately

	glutMainLoop();

	return 0;
}
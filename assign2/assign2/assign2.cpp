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
		0.0, 0.0, -3.0, // Where camera should point
		0.0, 0.0, 0.0,  // Camera Placement
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

	/* do initialization */
	myinit();

	glutMainLoop();

	return 0;
}
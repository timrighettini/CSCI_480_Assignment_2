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
#include <vector>

void calculateInitialVectors(); // Prototype for this function
void loadAnimations();

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

/*For animations*/
Pic* groundAnimationFrames[121]; // Will hold all of the frames for the ground animation
Pic* skyAnimationFrames[241]; // Will hold all of the frames for the sky animation
bool animationOn = true; // Will determine if animation will be on or not for the simulation (decreases loading time if off, but not nearly as cool)

// Animation counters
int skyFrameCounter = 0; // What frame is currently being displayed for the sky animation
int groundFrameCounter = 0; // What frame is currently being displayed for the ground animation
int animationSwitch = 0; // When this value = a certain number, the animations will advance to the next frame

// These values will hold the what the base left/right/top/bottom values a texture plane will be
float rectangleRight = 0.5;
float rectangleLeft = -0.5;

float rectangleUp = -0.5;
float rectangleDown = 0.5;
/*These values will have to be multipled by -1 when being represented in the z-axis because of the right handed coordinate system*/

// These values will hold the lowest point of a spline and the farthest distance the spline ever reaches from the center -- this will help size and orient the skyBox correctly
float maxDistSquared = 0; // This value will hold how far the furthest spline point is away from the center, squared
float maxDist = 0; // This value will hold how far the furthest spline point is away from the center
float lowestPoint = 0; // The lowest value of any spline coordinate, in terms of y

float FLOOR_SUB = 0; // How far the ground plane will be from the floor relative to the lowest point of the splines
float MAX_DIST_MULT = 4.75; // How far the skybox will extend out from the farthest point in the spline(s)

// These values will hold the display list for everything to be drawn
GLuint splineTrackDisplayList; // This value will hold the display list reference

// Will be used for traveling through a spline, iteratively, and non realistically
int controlPointNum = 1; // This is which control point/spline segment the camera is currently on
int currentSplineNum = 0; // This value will increase if there are multiple splines, otherwise, it will most likely stay at zero
float distanceIteratorNum = 0.0000; // This value will go from 0 to 1, when it equals 1, it will reset back to zero and the number above will increment++ or to 1
float INCREMENTOR = 0.0175; // Will decide how fast the roller coaster should go

// Window Height Values
int windowX = 640;  
int windowY = 480; 

// Values for holding tangents, norms, and biNorms
point tangent_prev;
point norm_prev;
point biNorm_prev;

point tangent_current;
point norm_current;
point biNorm_current;

// Arbitrary vector to be used for initial Calculation
point arbitrary;

// Store all Norms and BiNorms at control points -- or else bad things happen because of global -- initialize these arrays when the size number of splines/control points is found
// These vectors will hold vectors that correspond to certain (u) values throughout any given spline segment
std::vector<std::vector<point>> cpNormsLeft;
std::vector<std::vector<point>> cpBiNormsLeft;
std::vector<std::vector<point>> cpTangentsLeft;
std::vector<std::vector<point>> cpPositionsLeft;

std::vector<std::vector<point>> cpNormsRight;
std::vector<std::vector<point>> cpBiNormsRight;
std::vector<std::vector<point>> cpTangentsRight;
std::vector<std::vector<point>> cpPositionsRight;

bool drawDouble = true; // Will draw two pairs of rails assuming the 

float trackDiameter = 0.025; // The radius of the track.  Used so that the camera can move above it instead of through it.

point camPosition;
point cameraOriginPosition;

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

void positionCamera(double eye_x, double eye_y, double eye_z, double center_x, double center_y, double center_z, double up_x, double up_y, double up_z) { // This method will set the camera to point and be in the appropriate places
	gluLookAt(
		eye_x, eye_y, eye_z, // Where camera is placed
		center_x, center_y, center_z,  // Where the center of the scene is
		up_x, up_y, up_z   // The "up" vector
	);
}

point getUnitVector(point a) {
	// Get the magnitude
	float aDist = pow(a.x, 2) + pow(a.y, 2) + pow(a.z, 2);

	if (aDist == 0) {
		return a; // Cannot divide by zero
	}

	aDist = sqrt(aDist);

	// Normalize the vector
	a.x /= aDist;
	a.y /= aDist;
	a.z /= aDist;
		
	return a;
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

point getTagentXYZ(float u) {
	point p; // Instantiate the point

	// Use a derived version of the C-R matrix formula
	p.x = ( p0.x * ( -(3 * pow(u, 2) * s)       + (4 * u * s)         - (s) )  ) + 
		  ( p1.x * (  (3 * pow(u, 2) * (2 - s)) + (2 * u * ((s - 3)))     + (  0  ) )  ) + 
		  ( p2.x * (  (3 * pow(u, 2) * (s - 2)) + (2 * u * (3 - (2 * s))) + (s) )  ) + 
		  ( p3.x * (  (3 * pow(u, 2) * s)       - (2 * u * s) )  );

	p.y = ( p0.y * ( -(3 * pow(u, 2) * s)       + (4 * u * s)         - (s) )  ) + 
		  ( p1.y * (  (3 * pow(u, 2) * (2 - s)) + (2 * u * ((s - 3)))     + (  0  ) )  ) + 
		  ( p2.y * (  (3 * pow(u, 2) * (s - 2)) + (2 * u * (3 - (2 * s))) + (s) )  ) + 
		  ( p3.y * (  (3 * pow(u, 2) * s)       - (2 * u * s) )  );

	p.z = ( p0.z * ( -(3 * pow(u, 2) * s)       + (4 * u * s)         - (s) )  ) + 
		  ( p1.z * (  (3 * pow(u, 2) * (2 - s)) + (2 * u * ((s - 3)))     + (  0  ) )  ) + 
		  ( p2.z * (  (3 * pow(u, 2) * (s - 2)) + (2 * u * (3 - (2 * s))) + (s) )  ) + 
		  ( p3.z * (  (3 * pow(u, 2) * s)       - (2 * u * s) )  );
	
	return p;
}

point getCrossProduct(point a, point b) {
	point c; // The result of the cross product

	c.x = (a.y * b.z) - (a.z * b.y);
	c.y = (a.z * b.x) - (a.x * b.z);
	c.z = (a.x * b.y) - (a.y * b.x);

	return c;
}

void setCameraPlacement() {

	// Get p0 to p3 to attain the proper point p for the camera placement
	p0 = g_Splines[currentSplineNum].points[controlPointNum-1]; // Pi-1
	p1 = g_Splines[currentSplineNum].points[controlPointNum]; // Pi
	p2 = g_Splines[currentSplineNum].points[controlPointNum+1]; // Pi+1
	p3 = g_Splines[currentSplineNum].points[controlPointNum+2]; // Pi+2

	if (currentSplineNum == 0 && controlPointNum == 1 && distanceIteratorNum == 0.000) { // Then set up the initial coordinate system
		calculateInitialVectors();
	}
	else { // Get the next coordinate system based upon the previous one
		// First, get the point that the camera should be looking at (it will be on the spline segment ahead of the one currently being traversed)
		// Get the tangent for the current coordinate
		tangent_current = getUnitVector(getTagentXYZ(distanceIteratorNum)); // Will be the point that the  camera points to as it is traveling along the roller coaster

		// Next, get the correct Norm/Bi-Norm vectors	
		// Calculate the norm/biNorm of the current frame based upon values from the previous/current frame
		norm_current = getUnitVector(getCrossProduct(getUnitVector(biNorm_prev), getUnitVector(tangent_current)));
		biNorm_current = getUnitVector(getCrossProduct(getUnitVector(tangent_current), getUnitVector(norm_current)));

	/*	std::cout << (norm_current.x * tangent_current.x) + (norm_current.y * tangent_current.y) + (norm_current.z * tangent_current.z) << std::endl;
		std::cout << (norm_current.x *  biNorm_current.x) + (norm_current.y *  biNorm_current.y) + (norm_current.z *  biNorm_current.z) << std::endl;
		std::cout << (biNorm_current.x * tangent_current.x) + (biNorm_current.y * tangent_current.y) + (biNorm_current.z * tangent_current.z) << std::endl;
		std::cout << (biNorm_current.x * norm_current.x * tangent_current.x) + (biNorm_current.y * norm_current.y * tangent_current.y) + (biNorm_current.z * norm_current.z * tangent_current.z) << std::endl;*/
	}

	// Now get the point for the camera placement
	camPosition = getCoordinateXYZ(distanceIteratorNum);
	//camPosition.y -= trackDiameter * 7.5; // Up is negative in this coordinate system

	// Then, get where the camera should be pointing to
	cameraOriginPosition = tangent_current; // Set the tangent to where the camera should be looking towards

	cameraOriginPosition.x *= 100;
	cameraOriginPosition.y *= 100;
	cameraOriginPosition.z *= 100;

	// Finally, set the camera's position

	//positionCamera(
	//	camPosition.x         , camPosition.y         , camPosition.z, 
	//	cameraOriginPosition.x, cameraOriginPosition.y, cameraOriginPosition.z, 
	//	biNorm_current.x, biNorm_current.y, biNorm_current.z//0.0, 1.0, 0.0
	//); // Sets the camera position

	/*
		Viewing Angle: 60
		Aspect Ratio: 1.333 (4:3)
		Near Clipping Plane: 0.01
		Far  "            ": 1000.0
	*/
	// Set the matrix mode back to modelView, so things do not get messed up


	distanceIteratorNum += INCREMENTOR;

	// Set the current norm/biNorm to be the previous ones now
	norm_prev = norm_current;
	biNorm_prev = biNorm_current;

	//std::cout << distanceIteratorNum << std::endl;
	//std::cout << camPosition.x << std::endl;
	//std::cout << camPosition.y << std::endl;
	//std::cout << camPosition.z << std::endl;
	//std::cout << controlPointNum << std::endl;

	if (distanceIteratorNum >= 1) { // Reset u and increment the control point num ++
		distanceIteratorNum = 0.000;
		controlPointNum++;
		if (controlPointNum == g_Splines[currentSplineNum].numControlPoints - 2) { // If we have reached the end of the spline, reset the control point value and increment the spline currently being traversed
			controlPointNum = 1;
			currentSplineNum++;
			if (currentSplineNum == g_iNumOfSplines) { // Reset currentSplineNum value back to 0 if we have passed the last spline
				currentSplineNum = 0;
			}
			calculateInitialVectors();
		}
	}
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
	groundTexture = jpeg_read("Images/Earth_Stars_1024.jpg", NULL); // This will return a Pic struct with information about the image relating to a texture
	skyTexture = jpeg_read("Images/Stars_1024.jpg", NULL); // This will return a Pic struct with information about the image relating to a texture

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

	// Tell OpenGL to make room for the new textures
	glGenTextures(1, &groundTextureID);

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

	loadAnimations(); // Load in all of the animation frames from the Images directory
}

void calculateLowFarPointsSplines() {
	// Loop through all of the spline values and find the lowest point and the furthest distance away from the center
	for (int i = 0; i < g_iNumOfSplines; i++) {
		for (int j = 0; j < g_Splines[i].numControlPoints; j++) {
			// Get the distance squared for this point
			float tempDistSquared = (pow(g_Splines[i].points[j].x, 2) + pow(g_Splines[i].points[j].y, 2) + pow(g_Splines[i].points[j].z, 2)); // Get the dot product of the vector
			if (tempDistSquared > maxDistSquared) {
				maxDistSquared = tempDistSquared;
			}
			if (g_Splines[i].points[j].y < lowestPoint || (i == 0 && j == 0)) {
				lowestPoint = g_Splines[i].points[j].y;
			}
		}	
	}
	// Finally, get the raw maximum distance
	maxDist = sqrt(maxDistSquared);

	FLOOR_SUB = -maxDist/8; // This will keep the skybox reasonably below the lowest point of the coaster

	// Do some print to make sure that the values are right
	std::cout << "Max Distance Squared: " << maxDistSquared <<  std::endl;
	std::cout << "Max Distance: " << maxDist <<  std::endl;
	std::cout << "Lowest Point: " << lowestPoint <<  std::endl;
	std::cout << "Bottom Skybox Point: " << FLOOR_SUB <<  std::endl;
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
	glPointSize(10);
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

/*
int controlPointNum = 1; // This is which control point/spline segment the camera is currently pon
float distanceIteratorNum = 0.0000; // This value will go from 0 to 1, when it equals 1, it will reset back to zero and the number above will increment++ or to 1
*/

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

void drawRailSection(int splineNumber, int controlPointNumber, bool drawingLeft) {
	float railRadius = trackDiameter/2; // How far the unitized vectors will be scaled for attaining the proper sized rail

	glLineWidth(100 * trackDiameter);
	
	for (int i = 0; i < cpNormsLeft[controlPointNumber - 1].size() - 1; i++) {

		// To Test, let's just draw some points to the screen
		glColor3f(1.0, 0.0, 1.0); // Color for the lines

		point norm; //= cpNormsLeft[cpNormsLeft.size() - controlPointNumber];
		point biNorm; //= cpBiNormsLeft[cpBiNormsLeft.size() - controlPointNumber];
		point tangent; //= cpTangentsLeft[cpTangentsLeft.size() - controlPointNumber];
		point position;

		point norm2; //= cpNormsLeft[cpNormsLeft.size() - controlPointNumber - 1];
		point biNorm2; //= cpBiNormsLeft[cpBiNormsLeft.size() - controlPointNumber - 1];
		point tangent2; //= cpTangentsLeft[cpTangentsLeft.size() - controlPointNumber - 1];
		point position2;

		if (drawingLeft == true) {
			norm = cpNormsLeft[controlPointNumber - 1][i];
			biNorm = cpBiNormsLeft[controlPointNumber - 1][i];
			tangent = cpTangentsLeft[controlPointNumber - 1][i];
			position = cpPositionsLeft[controlPointNumber - 1][i];

			norm2 = cpNormsLeft[controlPointNumber - 1][i+1];
			biNorm2 = cpBiNormsLeft[controlPointNumber - 1][i+1];
			tangent2 = cpTangentsLeft[controlPointNumber - 1][i+1];
			position2 = cpPositionsLeft[controlPointNumber - 1][i+1];
		}
		else {
			norm = cpNormsRight[controlPointNumber - 1][i];
			biNorm = cpBiNormsRight[controlPointNumber - 1][i];
			tangent = cpTangentsRight[controlPointNumber - 1][i];
			position = cpPositionsRight[controlPointNumber - 1][i];

			norm2 = cpNormsRight[controlPointNumber - 1][i+1];
			biNorm2 = cpBiNormsRight[controlPointNumber - 1][i+1];
			tangent2 = cpTangentsRight[controlPointNumber - 1][i+1];
			position2 = cpPositionsRight[controlPointNumber - 1][i+1];
		}

		glBegin(GL_LINES);

			glVertex3f( // v0
				position.x + railRadius * (norm.x - biNorm.x), 
				position.y + railRadius * (norm.y - biNorm.y), 
				position.z + railRadius * (norm.z - biNorm.z)
			);

			glVertex3f( // v0 + 1
				position2.x + railRadius * (norm2.x - biNorm2.x), 
				position2.y + railRadius * (norm2.y - biNorm2.y), 
				position2.z + railRadius * (norm2.z - biNorm2.z)
			);

		glEnd();

		glBegin(GL_LINES);

			glVertex3f( // v1
				position.x + railRadius * (norm.x + biNorm.x), 
				position.y + railRadius * (norm.y + biNorm.y), 
				position.z + railRadius * (norm.z + biNorm.z)
			);

			glVertex3f( // v1 + 1
				position2.x + railRadius * (norm2.x + biNorm2.x), 
				position2.y + railRadius * (norm2.y + biNorm2.y), 
				position2.z + railRadius * (norm2.z + biNorm2.z)
			);

		glEnd();

		glBegin(GL_LINES);

			glVertex3f( // v2
				position.x + railRadius * (-norm.x + biNorm.x), 
				position.y + railRadius * (-norm.y + biNorm.y), 
				position.z + railRadius * (-norm.z + biNorm.z)
			);

			glVertex3f( // v2 + 1
				position2.x + railRadius * (-norm2.x + biNorm2.x), 
				position2.y + railRadius * (-norm2.y + biNorm2.y), 
				position2.z + railRadius * (-norm2.z + biNorm2.z)
			);

		glEnd();

		glBegin(GL_LINES);

			glVertex3f( // v3
				position.x + railRadius * (-norm.x - biNorm.x), 
				position.y + railRadius * (-norm.y - biNorm.y), 
				position.z + railRadius * (-norm.z - biNorm.z)
			);

			glVertex3f( // v3 + 1
				position2.x + railRadius * (-norm2.x - biNorm2.x), 
				position2.y + railRadius * (-norm2.y - biNorm2.y), 
				position2.z + railRadius * (-norm2.z - biNorm2.z)
			);

		glEnd();

		// Draw the six rectangles for the cross section, using the method described in Level 5 on the website

		// Draw the front rectangle
		if (i == 0 && splineNumber == 1) { // If this is the first norm section, draw the front quad
			glColor3f(1.0, 1.0, 0.0); 
			glBegin(GL_QUADS);

				glVertex3f( // v0
					position.x + railRadius * (norm.x - biNorm.x), 
					position.y + railRadius * (norm.y - biNorm.y), 
					position.z + railRadius * (norm.z - biNorm.z)
				);

				glVertex3f( // v1
					position.x + railRadius * (norm.x + biNorm.x), 
					position.y + railRadius * (norm.y + biNorm.y), 
					position.z + railRadius * (norm.z + biNorm.z)
				);

				glVertex3f( // v2
					position.x + railRadius * (-norm.x + biNorm.x), 
					position.y + railRadius * (-norm.y + biNorm.y), 
					position.z + railRadius * (-norm.z + biNorm.z)
				);

				glVertex3f( // v3
					position.x + railRadius * (-norm.x - biNorm.x), 
					position.y + railRadius * (-norm.y - biNorm.y), 
					position.z + railRadius * (-norm.z - biNorm.z)
				);
			
			glEnd();
		}

		// Draw the back rectangle
		if (i == cpNormsLeft[controlPointNumber - 1].size() - 2 || i == cpNormsRight[controlPointNumber - 1].size() - 2) { // If this is the last section of norms, draw the back quad
			glColor3f(1.0, 1.0, 0.0); 
			glBegin(GL_QUADS);

				glVertex3f( // v0 + 1
					position2.x + railRadius * (norm2.x - biNorm2.x), 
					position2.y + railRadius * (norm2.y - biNorm2.y), 
					position2.z + railRadius * (norm2.z - biNorm2.z)
				);

				glVertex3f( // v1 + 1
					position2.x + railRadius * (norm2.x + biNorm2.x), 
					position2.y + railRadius * (norm2.y + biNorm2.y), 
					position2.z + railRadius * (norm2.z + biNorm2.z)
				);

				glVertex3f( // v2 + 1
					position2.x + railRadius * (-norm2.x + biNorm2.x), 
					position2.y + railRadius * (-norm2.y + biNorm2.y), 
					position2.z + railRadius * (-norm2.z + biNorm2.z)
				);

				glVertex3f( // v3 + 1
					position2.x + railRadius * (-norm2.x - biNorm2.x), 
					position2.y + railRadius * (-norm2.y - biNorm2.y), 
					position2.z + railRadius * (-norm2.z - biNorm2.z)
				);
			
			glEnd();
		}

		// Draw the right rectangle
		glBegin(GL_QUADS);
			glColor3f(1.0, 1.0, 0.0); 
			glVertex3f( // v0
				position.x + railRadius * (norm.x - biNorm.x), 
				position.y + railRadius * (norm.y - biNorm.y), 
				position.z + railRadius * (norm.z - biNorm.z)
			);

			glVertex3f( // v0 + 1
				position2.x + railRadius * (norm2.x - biNorm2.x), 
				position2.y + railRadius * (norm2.y - biNorm2.y), 
				position2.z + railRadius * (norm2.z - biNorm2.z)
			);

			glVertex3f( // v1 + 1
				position2.x + railRadius * (norm2.x + biNorm2.x), 
				position2.y + railRadius * (norm2.y + biNorm2.y), 
				position2.z + railRadius * (norm2.z + biNorm2.z)
			);

			glVertex3f( // v1
				position.x + railRadius * (norm.x + biNorm.x), 
				position.y + railRadius * (norm.y + biNorm.y), 
				position.z + railRadius * (norm.z + biNorm.z)
			);

		glEnd();

		// Draw the top rectangle
		glBegin(GL_QUADS);
			glColor3f(0.8, 0.8, 0.8); // Make the top a special color	
			glVertex3f( // v1
				position.x + railRadius * (norm.x + biNorm.x), 
				position.y + railRadius * (norm.y + biNorm.y), 
				position.z + railRadius * (norm.z + biNorm.z)
			);

			glVertex3f( // v1 + 1
				position2.x + railRadius * (norm2.x + biNorm2.x), 
				position2.y + railRadius * (norm2.y + biNorm2.y), 
				position2.z + railRadius * (norm2.z + biNorm2.z)
			);

			glVertex3f( // v2 + 1
				position2.x + railRadius * (-norm2.x + biNorm2.x), 
				position2.y + railRadius * (-norm2.y + biNorm2.y), 
				position2.z + railRadius * (-norm2.z + biNorm2.z)
			);

			glVertex3f( // v2
				position.x + railRadius * (-norm.x + biNorm.x), 
				position.y + railRadius * (-norm.y + biNorm.y), 
				position.z + railRadius * (-norm.z + biNorm.z)
			);

		glEnd();

		// Draw the left rectangle
		glBegin(GL_QUADS);
			glColor3f(1.0, 1.0, 0.0); 
			glVertex3f( // v3
				position.x + railRadius * (-norm.x - biNorm.x), 
				position.y + railRadius * (-norm.y - biNorm.y), 
				position.z + railRadius * (-norm.z - biNorm.z)
			);

			glVertex3f( // v3 + 1
				position2.x + railRadius * (-norm2.x - biNorm2.x), 
				position2.y + railRadius * (-norm2.y - biNorm2.y), 
				position2.z + railRadius * (-norm2.z - biNorm2.z)
			);
	
			glVertex3f( // v2 + 1
				position2.x + railRadius * (-norm2.x + biNorm2.x), 
				position2.y + railRadius * (-norm2.y + biNorm2.y), 
				position2.z + railRadius * (-norm2.z + biNorm2.z)
			);

			glVertex3f( // v2
				position.x + railRadius * (-norm.x + biNorm.x), 
				position.y + railRadius * (-norm.y + biNorm.y), 
				position.z + railRadius * (-norm.z + biNorm.z)
			);		

			
		glEnd();

		// Draw the bottom rectangle
		glBegin(GL_QUADS);
			glColor3f(0.8, 0.8, 0.8); // Make the top a special color				
			glVertex3f( // v0
				position.x + railRadius * (norm.x - biNorm.x), 
				position.y + railRadius * (norm.y - biNorm.y), 
				position.z + railRadius * (norm.z - biNorm.z)
			);

			glVertex3f( // v0 + 1
				position2.x + railRadius * (norm2.x - biNorm2.x), 
				position2.y + railRadius * (norm2.y - biNorm2.y), 
				position2.z + railRadius * (norm2.z - biNorm2.z)
			);

			glVertex3f( // v3 + 1
				position2.x + railRadius * (-norm2.x - biNorm2.x), 
				position2.y + railRadius * (-norm2.y - biNorm2.y), 
				position2.z + railRadius * (-norm2.z - biNorm2.z)
			);

			glVertex3f( // v3
				position.x + railRadius * (-norm.x - biNorm.x), 
				position.y + railRadius * (-norm.y - biNorm.y), 
				position.z + railRadius * (-norm.z - biNorm.z)
			);

		glEnd();
	}
}

void getNormals(int splineNumber, int controlPointNumber, float offset) {
	// Get p0 to p3 to attain the proper point p for the point placement
	p0 = g_Splines[splineNumber].points[controlPointNumber-1]; // Pi-1
	p1 = g_Splines[splineNumber].points[controlPointNumber  ]; // Pi
	p2 = g_Splines[splineNumber].points[controlPointNumber+1]; // Pi+1
	p3 = g_Splines[splineNumber].points[controlPointNumber+2]; // Pi+2

	// Push back new vectors into my lists of vectors for the norms
	std::vector<point> norms;
	std::vector<point> binorms;
	std::vector<point> tangents;
	std::vector<point> points;
	if (offset == -1) {
		cpTangentsLeft.push_back(tangents);
		cpNormsLeft.push_back(norms);
		cpBiNormsLeft.push_back(binorms);
		cpPositionsLeft.push_back(points);
	}
	else {
		cpTangentsRight.push_back(tangents);
		cpNormsRight.push_back(norms);
		cpBiNormsRight.push_back(binorms);
		cpPositionsRight.push_back(points);	
	}
	
	// Calculate the vectors
	while(distanceIteratorNum < 1) {
		if (splineNumber == 0 && controlPointNumber == 1 && distanceIteratorNum == 0.000) { // Then calculate the initial vectors for drawing the cross sections
			calculateInitialVectors();
		}
		else { // Update according to the current control point system
			// First, get the point that the camera should be looking at (it will be on the spline segment ahead of the one currently being traversed)
			// Get the tangent for the current coordinate
			tangent_current = getUnitVector(getTagentXYZ(distanceIteratorNum)); // Will be the point that the  camera points to as it is traveling along the roller coaster

			// Next, get the correct Norm/Bi-Norm vectors	
			// Calculate the norm/biNorm of the current frame based upon values from the previous/current frame
			norm_current = getUnitVector(getCrossProduct(getUnitVector(biNorm_prev), getUnitVector(tangent_current)));
			biNorm_current = getUnitVector(getCrossProduct(getUnitVector(tangent_current), getUnitVector(norm_current)));
		}		

		// Then store the current tangent, norm, and biNorm within array indices -- will have to reverse iterate through this list to match up with the control points
		if (offset == -1) {
			cpTangentsLeft[controlPointNumber-1].push_back(tangent_current);
			cpNormsLeft[controlPointNumber-1].push_back(norm_current);
			cpBiNormsLeft[controlPointNumber-1].push_back(biNorm_current);
			cpPositionsLeft[controlPointNumber-1].push_back(getCoordinateXYZ(distanceIteratorNum));
			cpPositionsLeft[controlPointNumber-1][cpPositionsLeft[controlPointNumber-1].size() - 1].x += offset * (maxDist * 0.015);
			if (!(arbitrary.x >= 1)) { // This means tha that the y-up arbitrary vector worked, go with regular case for track drawing
				cpPositionsLeft[controlPointNumber-1][cpPositionsLeft[controlPointNumber-1].size() - 1].y += offset * (maxDist * 0.015);	
			}
			else { // Draw the track along the z instead of the y axis so it does not get in the way
				cpPositionsLeft[controlPointNumber-1][cpPositionsLeft[controlPointNumber-1].size() - 1].z += offset * (maxDist * 0.015);
			}						
		}

		else {
			cpTangentsRight[controlPointNumber-1].push_back(tangent_current);
			cpNormsRight[controlPointNumber-1].push_back(norm_current);
			cpBiNormsRight[controlPointNumber-1].push_back(biNorm_current);
			cpPositionsRight[controlPointNumber-1].push_back(getCoordinateXYZ(distanceIteratorNum));
			cpPositionsRight[controlPointNumber-1][cpPositionsRight[controlPointNumber-1].size() - 1].x += offset * (maxDist * 0.015);						
			if (!(arbitrary.x >= 1)) { // This means tha that the y-up arbitrary vector worked, go with regular case for track drawing
				cpPositionsRight[controlPointNumber-1][cpPositionsRight[controlPointNumber-1].size() - 1].y += offset * (maxDist * 0.015);
			}
			else { // Draw the track along the z instead of the y axis so it does not get in the way
				cpPositionsRight[controlPointNumber-1][cpPositionsRight[controlPointNumber-1].size() - 1].z += offset * (maxDist * 0.015);
			}
		}

		// Set the current norm/biNorm to be the previous ones now
		norm_prev = norm_current;
		biNorm_prev = biNorm_current;

		distanceIteratorNum += INCREMENTOR;
	}

	// Add one last set of values to the end of each vector
	// Update according to the current control point system
	// First, get the point that the camera should be looking at (it will be on the spline segment ahead of the one currently being traversed)
	// Get the tangent for the current coordinate
	tangent_current = getUnitVector(getTagentXYZ(distanceIteratorNum)); // Will be the point that the  camera points to as it is traveling along the roller coaster
	
	// Next, get the correct Norm/Bi-Norm vectors	
	// Calculate the norm/biNorm of the current frame based upon values from the previous/current frame
	norm_current = getUnitVector(getCrossProduct(getUnitVector(biNorm_prev), getUnitVector(tangent_current)));
	biNorm_current = getUnitVector(getCrossProduct(getUnitVector(tangent_current), getUnitVector(norm_current)));
	
	if (offset == -1) {
		cpTangentsLeft[controlPointNumber-1].push_back(tangent_current);
		cpNormsLeft[controlPointNumber-1].push_back(norm_current);
		cpBiNormsLeft[controlPointNumber-1].push_back(biNorm_current);
		cpPositionsLeft[controlPointNumber-1].push_back(getCoordinateXYZ(distanceIteratorNum));
		cpPositionsLeft[controlPointNumber-1][cpPositionsLeft[controlPointNumber-1].size() - 1].x += offset * (maxDist * 0.015);
		if (!(arbitrary.x >= 1)) { // This means tha that the y-up arbitrary vector worked, go with regular case for track drawing
			cpPositionsLeft[controlPointNumber-1][cpPositionsLeft[controlPointNumber-1].size() - 1].y += offset * (maxDist * 0.015);	
		}
		else { // Draw the track along the z instead of the y axis so it does not get in the way
			cpPositionsLeft[controlPointNumber-1][cpPositionsLeft[controlPointNumber-1].size() - 1].z += offset * (maxDist * 0.015);
		}	
	}
	else {
		cpTangentsRight[controlPointNumber-1].push_back(tangent_current);
		cpNormsRight[controlPointNumber-1].push_back(norm_current);
		cpBiNormsRight[controlPointNumber-1].push_back(biNorm_current);
		cpPositionsRight[controlPointNumber-1].push_back(getCoordinateXYZ(distanceIteratorNum));
		cpPositionsRight[controlPointNumber-1][cpPositionsRight[controlPointNumber-1].size() - 1].x += offset * (maxDist * 0.015);						
		if (!(arbitrary.x >= 1)) { // This means tha that the y-up arbitrary vector worked, go with regular case for track drawing
			cpPositionsRight[controlPointNumber-1][cpPositionsRight[controlPointNumber-1].size() - 1].y += offset * (maxDist * 0.015);
		}
		else { // Draw the track along the z instead of the y axis so it does not get in the way
			cpPositionsRight[controlPointNumber-1][cpPositionsRight[controlPointNumber-1].size() - 1].z += offset * (maxDist * 0.015);
		}
	}

	// Set the current norm/biNorm to be the previous ones now
	norm_prev = norm_current;
	biNorm_prev = biNorm_current;

	distanceIteratorNum = 0.000;
}

void drawCrossSections() { // Draw the cross sections for the coaster
	// Loop through all of the control points
	// Get all of the norms for the cross sections
	for (int i = 0; i < g_iNumOfSplines; i++) {
		for (int j = 1; j < g_Splines[i].numControlPoints - 2; j++) { // Draw the control points, and see if the spline goes through
			getNormals(i, j, -1);
		}
	}

	for (int i = 0; i < g_iNumOfSplines; i++) {
		for (int j = 1; j < g_Splines[i].numControlPoints - 2; j++) { // Draw the control points, and see if the spline goes through
			getNormals(i, j, 1);
		}
	}


	// Draw them all out here
	for (int i = 0; i < g_iNumOfSplines; i++) {
		for (int j = 1; j < g_Splines[i].numControlPoints - 2; j++) { // Draw the control points, and see if the spline goes through
			drawRailSection(i, j, true);
		}
	}

	for (int i = 0; i < g_iNumOfSplines; i++) {
		for (int j = 1; j < g_Splines[i].numControlPoints - 2; j++) { // Draw the control points, and see if the spline goes through
			drawRailSection(i, j, false);
		}
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

void drawSkyBox(float groundPlaneSize, float groundOffset) {
	// Enable OpenGL texturing
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // Use texture color only -- no lighting

	// Turn on the texturing
	glEnable(GL_TEXTURE_2D);

	// Do all of the skybox rendering here!
	float halfGroundPlaneSize = groundPlaneSize/2; // Saves on FP divisions

	// Load in the ground texture
	// Swap Animation Frames
	if (animationOn == true)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, groundAnimationFrames[groundFrameCounter]->nx,  groundAnimationFrames[groundFrameCounter]->ny, 0, GL_RGB, GL_UNSIGNED_BYTE,  groundAnimationFrames[groundFrameCounter]->pix);
	else 
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, groundTexture->nx,  groundTexture->ny, 0, GL_RGB, GL_UNSIGNED_BYTE, groundTexture->pix);
	if (animationSwitch == 1) {
		groundFrameCounter++;
	}
	if (groundFrameCounter > 120) {
		groundFrameCounter = 0;
	}

	// Draw the ground plane first: xz plane with the lower bound of the sky: xz axis with -y/2 height from center
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(rectangleLeft  * groundPlaneSize, groundOffset, rectangleUp   * groundPlaneSize);
		glTexCoord2f(1.0, 0.0); glVertex3f(rectangleRight * groundPlaneSize, groundOffset, rectangleUp   * groundPlaneSize);
		glTexCoord2f(1.0, 1.0); glVertex3f(rectangleRight * groundPlaneSize, groundOffset, rectangleDown * groundPlaneSize);
		glTexCoord2f(0.0, 1.0); glVertex3f(rectangleLeft  * groundPlaneSize, groundOffset, rectangleDown * groundPlaneSize);
	glEnd();

	// Load in the sky texture
	// Swap Animation Frames
	if (animationOn == true)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, skyAnimationFrames[skyFrameCounter]->nx,  skyAnimationFrames[skyFrameCounter]->ny, 0, GL_RGB, GL_UNSIGNED_BYTE,  skyAnimationFrames[skyFrameCounter]->pix);
	else 
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, skyTexture->nx,  skyTexture->ny, 0, GL_RGB, GL_UNSIGNED_BYTE, skyTexture->pix); 

	if (animationSwitch == 1) {
		skyFrameCounter++;
		animationSwitch = 0;
	}
	else {
		animationSwitch++;
	}
	if (skyFrameCounter > 240) {
		skyFrameCounter = 0;
	}

	// Draw the sky plane: xy axis with y/2 height from center
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(rectangleLeft  * groundPlaneSize, groundPlaneSize + groundOffset, rectangleUp   * groundPlaneSize);
		glTexCoord2f(1.0, 0.0); glVertex3f(rectangleRight * groundPlaneSize, groundPlaneSize + groundOffset, rectangleUp   * groundPlaneSize);
		glTexCoord2f(1.0, 1.0); glVertex3f(rectangleRight * groundPlaneSize, groundPlaneSize + groundOffset, rectangleDown * groundPlaneSize);
		glTexCoord2f(0.0, 1.0); glVertex3f(rectangleLeft  * groundPlaneSize, groundPlaneSize + groundOffset, rectangleDown * groundPlaneSize);
	glEnd();

	
	// Draw the sky plane: xy axis with -z/2 height from center
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(rectangleLeft  * groundPlaneSize, (rectangleDown * groundPlaneSize) + (halfGroundPlaneSize) + groundOffset, -(halfGroundPlaneSize));
		glTexCoord2f(1.0, 0.0); glVertex3f(rectangleRight * groundPlaneSize, (rectangleDown * groundPlaneSize) + (halfGroundPlaneSize) + groundOffset, -(halfGroundPlaneSize));
		glTexCoord2f(1.0, 1.0); glVertex3f(rectangleRight * groundPlaneSize, (rectangleUp   * groundPlaneSize) + (halfGroundPlaneSize) + groundOffset, -(halfGroundPlaneSize));
		glTexCoord2f(0.0, 1.0); glVertex3f(rectangleLeft  * groundPlaneSize, (rectangleUp   * groundPlaneSize) + (halfGroundPlaneSize) + groundOffset, -(halfGroundPlaneSize));
	glEnd();

	// Draw the sky plane: xy axis with z/2 height from center
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(rectangleLeft  * groundPlaneSize, (rectangleDown * groundPlaneSize) + (halfGroundPlaneSize) + groundOffset, (halfGroundPlaneSize));
		glTexCoord2f(1.0, 0.0); glVertex3f(rectangleRight * groundPlaneSize, (rectangleDown * groundPlaneSize) + (halfGroundPlaneSize) + groundOffset, (halfGroundPlaneSize));
		glTexCoord2f(1.0, 1.0); glVertex3f(rectangleRight * groundPlaneSize, (rectangleUp   * groundPlaneSize) + (halfGroundPlaneSize) + groundOffset, (halfGroundPlaneSize));
		glTexCoord2f(0.0, 1.0); glVertex3f(rectangleLeft  * groundPlaneSize, (rectangleUp   * groundPlaneSize) + (halfGroundPlaneSize) + groundOffset, (halfGroundPlaneSize));
	glEnd();

	// Swap Animation Frames
	if (animationOn == true)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, skyAnimationFrames[240-skyFrameCounter]->nx,  skyAnimationFrames[240-skyFrameCounter]->ny, 0, GL_RGB, GL_UNSIGNED_BYTE,  skyAnimationFrames[240-skyFrameCounter]->pix);	
	else 
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, skyTexture->nx,  skyTexture->ny, 0, GL_RGB, GL_UNSIGNED_BYTE, skyTexture->pix); 

	// Draw the sky plane: yz axis with -x/2 width from center
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(-(halfGroundPlaneSize), (rectangleDown * groundPlaneSize) + (halfGroundPlaneSize) + groundOffset, rectangleLeft  * groundPlaneSize);
		glTexCoord2f(1.0, 0.0); glVertex3f(-(halfGroundPlaneSize), (rectangleDown * groundPlaneSize) + (halfGroundPlaneSize) + groundOffset, rectangleRight * groundPlaneSize);
		glTexCoord2f(1.0, 1.0); glVertex3f(-(halfGroundPlaneSize), (rectangleUp   * groundPlaneSize) + (halfGroundPlaneSize) + groundOffset, rectangleRight * groundPlaneSize);
		glTexCoord2f(0.0, 1.0); glVertex3f(-(halfGroundPlaneSize), (rectangleUp   * groundPlaneSize) + (halfGroundPlaneSize) + groundOffset, rectangleLeft  * groundPlaneSize);
	glEnd();

	// Draw the sky plane: yz axis with x/2 width from center
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f((halfGroundPlaneSize), (rectangleDown * groundPlaneSize) + (halfGroundPlaneSize) + groundOffset, rectangleLeft  * groundPlaneSize);
		glTexCoord2f(1.0, 0.0); glVertex3f((halfGroundPlaneSize), (rectangleDown * groundPlaneSize) + (halfGroundPlaneSize) + groundOffset, rectangleRight * groundPlaneSize);
		glTexCoord2f(1.0, 1.0); glVertex3f((halfGroundPlaneSize), (rectangleUp   * groundPlaneSize) + (halfGroundPlaneSize) + groundOffset, rectangleRight * groundPlaneSize);
		glTexCoord2f(0.0, 1.0); glVertex3f((halfGroundPlaneSize), (rectangleUp   * groundPlaneSize) + (halfGroundPlaneSize) + groundOffset, rectangleLeft  * groundPlaneSize);
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
	
	positionCamera(
		0.0, 0.0, 0.0, 
		0.0, 0.0, 1.0, 
		0.0, 1.0, 0.0
	); // Sets the camera position

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
	//glutTimerFunc(500, setCameraPlacement, 1);	

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
	glLoadIdentity(); // Reset the Matrix

	glMatrixMode(GL_MODELVIEW); // Set mode to ModelView Here, just to be sure...

	setCameraPlacement(); // Set up the camera to point into the correct place

	positionCamera(
		camPosition.x         , camPosition.y         , camPosition.z, 
		-cameraOriginPosition.x, -cameraOriginPosition.y, -cameraOriginPosition.z, 
		biNorm_current.x, biNorm_current.y, biNorm_current.z//0.0, 1.0, 0.0
	); // Sets the camera position
	

	/*glRotatef(biNorm_current.x, 1, 0, 0);
	glRotatef(biNorm_current.y, 0, 1, 0);
	glRotatef(biNorm_current.z, 0, 0, 1);*/
	//glTranslatef(-camPosition.x*2, -camPosition.y*2, -camPosition.z*2);

	/* Test Code for the spline */
	//glPushMatrix(); // push on the new transformations that are about to be done
	
	/*
	glTranslatef(
		(translateMultDPI * -g_vLandTranslate[0]), // inverting this value (multiplying by -1) made it so that the shape followed the mouse for a-axis translations
		(translateMultDPI * g_vLandTranslate[1]),
		(translateMultDPI * -g_vLandTranslate[2])
	); // translate the matrix

	glRotatef(-g_vLandRotate[0], 1, 0, 0); // rotate along the x-axis - this value was inverted (multiplied by -1) because it made more sense to me to invert the x-axis rotation; it's what i am used to. (autodesk maya usage)
	glRotatef(-g_vLandRotate[1], 0, 1, 0); // rotate along the y-axis - this value was inverted (multiplied by -1) because it made more sense to me to invert the y-axis rotation; it's what i am used to. (autodesk maya usage)
	glRotatef(g_vLandRotate[2], 0, 0, 1); // rotate along the z-axis

	glScalef(
		(scaleMultDPI * g_vLandScale[0]),
		(scaleMultDPI * g_vLandScale[1]),
		(scaleMultDPI * g_vLandScale[2])
	); // Scale the Matrix
	//*/

	// Draw the textured skybox -- I cannot put this in the displayList because of the animation
	drawSkyBox(maxDist * MAX_DIST_MULT, lowestPoint + FLOOR_SUB);

	// Call the display List
	glCallList(splineTrackDisplayList);

	//glPopMatrix(); // Remove the transformation matrix

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

void calculateInitialVectors() {
	point arb;
	arb.x = 0;
	arb.y = 1;
	arb.z = 0;
	while (true) {
		// Initialize the beginning tangents
		tangent_prev = getUnitVector(getTagentXYZ(distanceIteratorNum)); // Initialize this value to the first tangent	
		tangent_current = tangent_prev;

		// Initialize the norms
		norm_prev = getUnitVector(getCrossProduct(getUnitVector(tangent_prev), getUnitVector(arb))); // Get the unit vector for T0 X (1,1,1)
		norm_current = norm_prev;

		// Initialize the biNorms
		biNorm_prev = getUnitVector(getCrossProduct(getUnitVector(tangent_prev), getUnitVector(norm_prev))); // Get the unit vector for T0 X N0
		biNorm_current = biNorm_prev;

		if (biNorm_current.y < 0) { // If norm is pointing upside down, flip it
			biNorm_current.y *= -1;
			biNorm_prev.y *= -1;
		}	
		if ((norm_current.x == 0 && norm_current.y == 0 && norm_current.z == 0)
			|| (biNorm_current.x == 0 && biNorm_current.y == 0 && biNorm_current.z == 0)) { // If vectors are zero, change the arbitrary vector and cross again
			arb.x = 1;
			arb.y = 0;
			arb.z = 0;
			continue;
		}
		else { // Break
			break;
		}
	}

	arbitrary = arb;

	// Test to see if the values are prependicular to each other
	std::cout << (norm_prev.x * tangent_prev.x) + (norm_prev.y * tangent_prev.y) + (norm_prev.z * tangent_prev.z) << std::endl;
	std::cout << (norm_prev.x *  biNorm_prev.x) + (norm_prev.y *  biNorm_prev.y) + (norm_prev.z *  biNorm_prev.z) << std::endl;
	std::cout << (biNorm_prev.x * tangent_prev.x) + (biNorm_prev.y * tangent_prev.y) + (biNorm_prev.z * tangent_prev.z) << std::endl;
	std::cout << (biNorm_prev.x * norm_prev.x * tangent_prev.x) + (biNorm_prev.y * norm_prev.y * tangent_prev.y) + (biNorm_prev.z * norm_prev.z * tangent_prev.z) << std::endl;
}

void compileDisplayList() { // This function will compile the display list before the program starts
	splineTrackDisplayList = glGenLists(1); // This value will hold the display list reference
	
	glNewList(splineTrackDisplayList, GL_COMPILE);
		
		// The first argument controls HOW BIG the skybox actually is
		// The second argument, assuming that the skybox's center is at 0, 0, shifts the skybox up or down by x amount

		// Draw out the rails/crossSections
		drawCrossSections();

		// Draw the control points
		drawControlPoints();

		// Draw out the splines
		drawAllSplines();

	glEndList();
}

void loadAnimations() {
	std::cout << "Loading Ground animations..." << std::endl;
	for (int i = 0; i < 121; i++) { // There are 121 frames to load in for this animation

		std::string name = "Images/Earth_Stars/Earth_Stars_1024";
		/*Append the appropriate number of 0's in front of the string*/

		// Load in the first set of animation frames -- for the ground plane
		if (i < 10) {
			name.append("00");		
		}
		else if (i < 100) {
			name.append("0");
		}

		std::stringstream s; // Use this to convert the number into a string
		s << i; // Get the number in string format
		name.append(s.str()); // Put that formatted number into a string	
		name.append(".jpg"); // Need the image file type

		//std::cout << name << std::endl;

		char * newName = new char[name.length() +1]; // Will be used to hold char* version of string
		strcpy(newName, name.c_str()); // Copy it in 

		// Load in the image
		groundAnimationFrames[i] = jpeg_read(newName, NULL);

		delete[] newName;

		if (!groundAnimationFrames[i]) {
			std::cerr << "Image not loaded for Ground Plane, exiting" << std::endl;
			exit(1);
		}
		if (!animationOn)
			break; // Only load in the first frame
	}

	std::cout << "Loading Sky animations..." << std::endl;
	// Load in the Sky Plane animation
	for (int i = 0; i < 241; i++) { // There are 121 frames to load in for this animation

		std::string name = "Images/Stars_Sky/Stars_Move_1024";
		/*Append the appropriate number of 0's in front of the string*/

		// Load in the first set of animation frames -- for the ground plane
		if (i < 10) {
			name.append("00");		
		}
		else if (i < 100) {
			name.append("0");
		}

		std::stringstream s; // Use this to convert the number into a string
		s << i; // Get the number in string format
		name.append(s.str()); // Put that formatted number into a string	
		name.append(".jpg"); // Need the image file type

		//std::cout << name << std::endl;

		char * newName = new char[name.length() +1]; // Will be used to hold char* version of string
		strcpy(newName, name.c_str()); // Copy it in 

		// Load in the image
		skyAnimationFrames[i] = jpeg_read(newName, NULL);

		delete[] newName;

		if (!skyAnimationFrames[i]) {
			std::cerr << "Image not loaded for Sky Plane, exiting" << std::endl;
			exit(1);
		}
		if (!animationOn)
			break; // Only load in the first frame
	}
}

int main(int argc, char* argv[])
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

	if (argc > 2) { // Load in the animation agrument
		if (argv[2][0] == 'f' || argv[2][0] == 'F') { // Turns OFF the animation
			animationOn = false;
		}
		else { // Leave the animation ON
			animationOn = true; 
		}
	}

	// Do all of the OpenGL initialization stuff
	glutInit(&argc,argv);

	// Set up the window
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(windowX,windowY);
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
	calculateLowFarPointsSplines(); // Get the low points and the max distances away from the center in terms of the spline control points

	// Initialize the camera traversal globals
	// Will be used for traveling through a spline, iteratively, and non realistically
	controlPointNum = 1; // This is which control point/spline segment the camera is currently on
	currentSplineNum = 0; // This value will increase if there are multiple splines, otherwise, it will most likely stay at zero
	distanceIteratorNum = 0.000; // This value will go from 0 to 1, when it equals 1, it will reset back to zero and the number above will increment++ or to 1

	compileDisplayList(); // Compile the display list

	glutMainLoop();

	return 0;
}
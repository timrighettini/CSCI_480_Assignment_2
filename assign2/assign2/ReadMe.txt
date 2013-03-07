======================
==========
Tim Righttini's Assignment #2 Readme: CSCI_480 Computer Graphics (CG)
==========
======================

------
Hello:
------

This OpenGL program is a simulation that renders a roller coaster based upon a spline file read in from the program.   After the coaster itself is rendered, the camera follows the spline that is the path of the roller coaster at a proportional rate (u).  

The program completes the following tasks, EXCLUDING extra credit:
1.  Renders a Catmull-Rom Spline to represent the center path of the track.
2.  Renders a ground plane, which is the earth.
3.  Renders a sky plane (x5), which are the stars of the sky
4.  Renders rail cross sections
5.  Moves the camera at (I hope) a reasonable speed that is definitely continuous in path and orientation, based upon the Sloan Method.
6.  Renders the coaster in an interesting manner (since it's rendered in space, I tried to make it as visible and realistic as I could).
7.  Runs at >15fps at the 640x480 window size
8.  Has an included 1000 frame animation.  The animation frames are stored in the Animation folder, in the same directory level as the README for this project.

Extra Credit Includes the following:
1.  Double Rails was implemented in the program.  I tried to do so the best I could without any criss-crossing.
2.  I made my own track called timTrack.sp.  You should load it in and check it out.  It's a fast paced coaster with numerous sharp turns and speed zones.
3.  The background is ANIMATED.  The texture frames change as the coaster moves along. 
4.  The Skybox size scales and moves depending on the size of the coaster.  The Skybox scales based upon the point the farthest away from the center, and its ground plane moves to slightly lower than the lowest point of the spline.  Refer to the functions calculateLowFarPointsSplines() and drawSkyBox().
5.  The spline is drawn through the recursive method.  This seemed easier to implement for some reason versus the iterative method.
6.  I did a lot of image editing to get the most realistic, square space textures from the base earth and stars images that I found from the internet.  My work can be seen within the Images folder, particulary with the PSD files.
7.  The Size of the rails, and the distance between them, changes as the track gets bigger and smaller so that the rails do not seem disproportionate to each other as the track size changes.  Refer to the getNormals() function to see how this works.

---------
Controls:
---------

None: Just watch the camera roll!

-------------
Special Notes
-------------

*Note 1:  Unfortunately, loading in all of the animated images leads to very LONG load times for the program.  Fortunately, I accounted for this: all you have to do is pass in 'f' as your second VS2010 command line argument and you will only load in the static backgrounds for the ground and sky planes.

*Note 2:  If you look down at the Earth from a high viewing angle, you'll get the illusion that it is 3D.  I really tried hard to make this effect, and I hope you like it!

*Note 3:  The Bi-Normal is used as the up vector for the camera, and it is always positive based upon my calculateInitialVectors() function.  

*Note 4:  For the rails, the gray sections are perpendicular to the Bi-Normal, and the Yellow Sections are perpendicular to the Normal at any given (u).  Also, purple lines are used as borders for the rails.  This is both an extra credit attempt and a special note, since the coloring of the rail actually gives you some information about how Sloan's Method is being calculated.  Can you guess what the color scheme relates to?

*Note 5:  My program will only work with ONE SPLINE.  If you try to load in more than one spline, well, who knows what will happen...

Besides for the notes and what I have mentioned above, this is everything you need to know about the program.

Thanks! 

-------
The End
-------

========================================================================
    CONSOLE APPLICATION : assign2 Project Overview
========================================================================

AppWizard has created this assign2 application for you.

This file contains a summary of what you will find in each of the files that
make up your assign2 application.


assign2.vcxproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

assign2.vcxproj.filters
    This is the filters file for VC++ projects generated using an Application Wizard. 
    It contains information about the association between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).

assign2.cpp
    This is the main application source file.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named assign2.pch and a precompiled types file named StdAfx.obj.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" comments to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <ctime>

#include "Shapes.h"

using namespace std;

// viewport variables
unsigned char* pixels;
int width, height;

// debug buffers for u and v
unsigned char* xComp;
unsigned char* yComp;

// camera info
Vector3 eye; Vector3 focus; Vector3 up;
float fovy;
float nearz, nearx, neary;

// scene objects
Shape* objects[206];
Sphere* spheres[204];
Triangle* triangles[2];
Light* lights[2];
int numObjects;
int numLights;

int DEBUG_INDEX;
int DEBUG;
int doOnce;

#define PI 3.14159265

/**
 * Writes a new PPM file
 */
void writePPM(const char* filename, unsigned char* pixels, int width, int height)
{
    // create a buffer to save
    unsigned char buffer[width*height*3];

    // init buffer as upside-down so it will save properly
    int index, reverseindex;
    for(int i = 0; i<height; i++)
    {
        for(int j = 0; j<3*width; j++)
        {
              index = j + 3*width*i;
              reverseindex = j + 3*width*(height-i);
              buffer[index] = pixels[reverseindex];
        }
    }

    // create file
    FILE* file;
    file = fopen(filename, "wb");

    // write header, contents
    fprintf(file, "P6\n%d %d\n255\n", width, height);
    fwrite(buffer, 1, 3*width*height, file);

    // clean up
    fclose(file);
}

//////////////////////////////////////////////////////////////////////////////////
// Draws to the OpenGL window
//////////////////////////////////////////////////////////////////////////////////
// find closest object that intersects with ray
float findClosestIntersect(Vector3 rayDir, Vector3 raySrc, int &object)
{
    float t = 1000, current;
    for(int i=0; i<numObjects; i++)
    {
        current = objects[i]->findIntersect(rayDir, raySrc, NEAR_SIDE);
        if(current > .01 && current < t)
        {
            // get return value and object index
            t = current;
            object = i;
        }
    }
    if(t>=999) return -1;
    return t;
}
// Shading based on Blinn-Phong lighting model, as described in the book and in class
//		src is the ray source, p is the point in question, ints are the indexes of object and light
//		Returns the amount that the color vector should be incremented
Vector3 calcShading(Vector3 p, Vector3 src, Shape* object, Light* light)
{
	Vector3 out = Vector3(0, 0, 0);

	// Start by calculating a bunch of useful normalized vectors
	Vector3 n = object->getNormal(p); n /= n.length(); // normal at p
	Vector3 v = src - p; v /= v.length(); // ray P-->Eye
	Vector3 l = light->getPoint() - p; l /= l.length(); // ray P-->Light Source

	// Make sure normal faces forward
	if(v.dot(n)<0) n = -n;

	// Calculate "perfect reflection" vector and some cosines
	float nl = n.dot(l); nl = (nl>0) ? nl : 0;
	Vector3 r = n*nl*2 - l; r /= r.length();
	float rv = r.dot(v);

	// If material is not diffuse, zero out diffuse component
	if(object->getMaterial()!=MAT_DIFFUSE) nl = 0;

	// If there's nothing between p and the light, add shading
	int o;
	if(findClosestIntersect((p-light->getPoint()), light->getPoint(), o)>.99)
	{
		Vector3 blend = object->getColor() * light->getColor();
		// Increment output color. Assumes Kd and Ks are both 1
		out += /* Diffuse: */	blend*nl
			+  /* Specular:*/	blend*pow(rv,10);
	}
	return out;
}
// Reflection & Refraction
Vector3 reflectRefract(Vector3 p, Vector3 rayDir, Vector3 raySrc, Shape* object, int depth)
{
	// if past max depth, color dark grey
	if(depth>9) return Vector3(0.2, 0.2, 0.2);

	Vector3 reflectColor = Vector3(0, 0, 0);
	Vector3 refractColor = Vector3(0, 0, 0);
	Vector3 nextP, t;
	Shape* nextObj;
	float dist;
	int i, o;

	// REFLECTION:
	// find reflected ray
	Vector3 n = object->getNormal(p); n /= n.length();
	if(n.dot(rayDir)<0) n = -n;
	rayDir /= rayDir.length();
	float dn = n.dot(rayDir);
	Vector3 r = rayDir-n*dn*2; r /= r.length();
	// find closest object along reflected ray
	dist = findClosestIntersect(r, p, o);
	if(dist>0)
	{
		// if there is an intersect, get that point
		nextP = p + r*dist;
		nextObj = objects[o];
		// if the object is diffuse, calc shading for all lights
		if(nextObj->getMaterial()==MAT_DIFFUSE)
			{for(i=0; i<numLights; i++) reflectColor += calcShading(nextP, p, nextObj, lights[i]);}
		// otherwise, reflect/refract again!
		else reflectColor += reflectRefract(nextP, r, p, nextObj, depth+1);
	}

	// REFRACTION:
	float mixCoeff, root;
	float nr = 1 / 1.5;
	float R0 = ((nr-1)/(nr+1))*((nr-1)/(nr+1));
	rayDir = p - raySrc; rayDir /= rayDir.length();
	n = object->getNormal(p); n /= n.length();
	dn = n.dot(rayDir);

	if(object->getMaterial()==MAT_REFRACT && dn<0)
	{
		// get refracted ray
		dn = -dn;
        root = sqrt(1 - (nr*nr)*(1 - (dn*dn)));
		if(root>0)
		{
            t = rayDir*nr - n*(nr*dn + root); t /= t.length();
			// now find other side of object
			dist = object->findIntersect(t, p, FAR_SIDE);
			nextP = p + t*dist;
			refractColor += reflectRefract(nextP, t, p, object, depth+1);
		}
		// calculate mixing coefficient for fresnel effects
		mixCoeff = R0 + (1-R0)*(1-dn)*(1-dn)*(1-dn)*(1-dn)*(1-dn);
	}else if(object->getMaterial()==MAT_REFRACT && dn>0)
	{
		// get refracted ray
        nr = 1/nr; n = -n;// reverse quantities since we're exiting
        root = sqrt(1 - (nr*nr)*(1 - (dn*dn)));
        if(root>0)
        {
            t = rayDir*nr - n*(nr*dn + root); t /= t.length();
			// now shoot t out of p to find next object
			dist = findClosestIntersect(t, p, o);
			if(dist>0)
			{
				nextP = p + t*dist;
				nextObj = objects[o];
				// if the object is diffuse, calc shading for all lights
				if(nextObj->getMaterial()==MAT_DIFFUSE)
					{for(int i=0; i<numLights; i++) refractColor += calcShading(nextP, p, nextObj, lights[i]);}
				// otherwise, reflect/refract again!
				else refractColor += reflectRefract(nextP, rayDir, p, nextObj, depth+1);
			}
		}
		// calculate mixing coefficient for fresnel effects
		mixCoeff = R0 + (1-R0)*(1-dn)*(1-dn)*(1-dn)*(1-dn)*(1-dn);
	} else mixCoeff = 1;

	// blend reflect color and refract color, plus the normal shading for each light
	Vector3 out = Vector3(0, 0, 0);
	Vector3 shading = Vector3(0, 0, 0);
	for(i=0; i<numLights; i++) shading += calcShading(p, raySrc, object, lights[i]);
	out = shading + (reflectColor*(mixCoeff) + refractColor*(1-mixCoeff));

	return out;
}
// primary rendering function
void display()
{
	if(doOnce) return; // only need to bother rendering once
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int index;
	int object;

	Vector3 rayDir, color, p, n, tmp;

	float dist;

	float r = nearx, l = -nearx;
	float t = neary, b = -neary;
	float u, v;

	for(int x=0; x<width; x++)
	{
		for(int y=0; y<height; y++)
		{
			index = 3*width*y + 3*x;
			if(width*y + x==DEBUG_INDEX)
			{
				DEBUG = 1;
				cout<<"Debug Log:"<<x<<", "<<y<<endl;
			}

			// calculate ray direction
			u = l + (r-l)*(x+0.5)/width;
			v = b + (t-b)*(y+0.5)/height;
			rayDir[0] = u; rayDir[1] = v; rayDir[2] = focus[2];

			// find closest object intersect point
			dist = findClosestIntersect(rayDir, eye, object);

			// if intersect exists, determine pixel color from lighting
			if(dist>=1)
			{
				// calculate coords of collision, normal vector
				p = eye + rayDir * dist;
				n = objects[object]->getNormal(p);

				// coloring computations
				color[0] = 0; color[1] = 0; color[2] = 0; // zero out pixel color
				// if there is ambient light, it goes in this line

				// if the object is diffuse, calc shading for all lights
				if(objects[object]->getMaterial()==MAT_DIFFUSE)
					{for(int i=0; i<numLights; i++) color += calcShading(p, eye, objects[object], lights[i]);}
				// otherwise, reflect/refract!
				else color += reflectRefract(p, rayDir, eye, objects[object], 0);

				// prevent color components from going out of bounds
				color[0] = (color[0]>1) ? 1 : color[0];
				color[1] = (color[1]>1) ? 1 : color[1];
				color[2] = (color[2]>1) ? 1 : color[2];

				// place pixel colors in pixel buffer
				pixels[index+0]=(int)(color[0]*255);
				pixels[index+1]=(int)(color[1]*255);
				pixels[index+2]=(int)(color[2]*255);

			} else {pixels[index+0]=0; pixels[index+1]=0; pixels[index+2]=0;} // if no intersect, color black

			if(DEBUG==1)
			{
				pixels[index+0]=(int)(0);
				pixels[index+1]=(int)(255);
				pixels[index+2]=(int)(0);
			}
			DEBUG = 0;
			//cout<<"Finished rendering "<<x<<", "<<y<<endl;
		}
	}

	// Render picture
	glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	glutSwapBuffers();

	doOnce = 1;
}

//////////////////////////////////////////////////////////////////////////////////
// Handles keyboard events
//////////////////////////////////////////////////////////////////////////////////
void keyboard(unsigned char k, int x, int y)
{
	switch (k)
	{
		// the escape key and 'q' quit the program
		case 27:
		case 'q':
			exit(0);
			break;
		case 'w':
			writePPM("output.ppm", pixels, width, height);
		break;
	}
	//glutPostRedisplay(); // no need for re-rendering
}

//////////////////////////////////////////////////////////////////////////////////
// Called occasionally to see if anything's happening
//////////////////////////////////////////////////////////////////////////////////
void idle()
{
	//glutPostRedisplay(); // no need for re-rendering
}


//////////////////////////////////////////////////////////////////////////////////
// Read in a raw PPM file of the "P6" style.
//
// Input: "filename" is the name of the file you want to read in
// Output: "pixels" will point to an array of pixel values
//         "width" will be the width of the image
//         "height" will be the height of the image
//
// The PPM file format is:
//
//   P6
//   <image width> <image height>
//   255
//   <raw, 8-bit binary stream of RGB values>
//
// Open one in a text editor to see for yourself.
//
//////////////////////////////////////////////////////////////////////////////////
void readPPM(const char* filename, unsigned char*& pixels, int& width, int& height)
{
	// try to open the file
	FILE* file;
	file = fopen(filename, "rb");
	if (file == NULL)
	{
		cout << " Couldn't open file " << filename << "! " << endl;
		exit(1);
	}

	// read in the image dimensions
	fscanf(file, "P6\n%d %d\n255\n", &width, &height);
	int totalPixels = width * height;

	// allocate three times as many pixels since there are R,G, and B channels
	pixels = new unsigned char[3 * totalPixels];
	fread(pixels, 1, 3 * totalPixels, file);
	fclose(file);

	// reverse y coord of pixels so they display right side up
	int index, reverseindex;
	unsigned char buf;
	for(int i = 0; i<height/2; i++)
	{
		for(int j = 0; j<3*width; j++)
		{
			index = j + 3*width*i;
			reverseindex = j + 3*width*(height-i);
			buf = pixels[index];
			pixels[index] = pixels[reverseindex];
			pixels[reverseindex] = buf;
		}
	}

	// output some success information
	cout << " Successfully read in " << filename << " with dimensions: "
	<< width << " " << height << endl;
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    clock_t t=clock();

	// init frame dimensions and buffer
	width = 800;
	height = 600;
	pixels = new unsigned char[3*width*height];

	DEBUG_INDEX = -1;
	DEBUG = 0;
	doOnce = 0;

	// init camera
	eye = Vector3(0, 0, 0);
	focus = Vector3(0, 0, 1);
	up = Vector3(0, 1, 0);
	fovy = 65;
	// calculate near plane
	nearz = 1;
	neary = focus[2] * tan(fovy/2 * (PI/180));
	nearx = neary * ((float)width/(float)height);

	// init scene objects
	numObjects = 0;

	// primary spheres
	spheres[0] = new Sphere(Vector3(0, -1000, 10), 997, Vector3(0.5, 0.5, 0.5), MAT_DIFFUSE);
	spheres[1] = new Sphere(Vector3(3.5, 0, 10), 3, Vector3(1, 1, 1), MAT_REFRACT);
	//spheres[2] = new Sphere(Vector3(6, -1, 20), 2, Vector3(0, 1, 1), MAT_DIFFUSE);
	//spheres[3] = new Sphere(Vector3(0, 0, 5), 1.5, Vector3(1, 1, 1), MAT_REFLECT);
	objects[numObjects++] = spheres[0];
	objects[numObjects++] = spheres[1];
	//objects[numObjects++] = spheres[2];
	//objects[numObjects++] = spheres[3];

	//triangles
	Vector3 a = Vector3(-3.5-sin(PI/4)*3,  3, 10-cos(PI/4)*3);
	Vector3 b = Vector3(-3.5-sin(PI/4)*3, -3, 10-cos(PI/4)*3);
	Vector3 c = Vector3(-3.5+sin(PI/4)*3,  3, 10+cos(PI/4)*3);
	Vector3 d = Vector3(-3.5+sin(PI/4)*3, -3, 10+cos(PI/4)*3);
	triangles[0] = new Triangle(a, b, c, Vector3(1, 1, 1), MAT_REFLECT);
	triangles[1] = new Triangle(b, d, c, Vector3(1, 1, 1), MAT_REFLECT);
	objects[numObjects++] = triangles[0]; objects[numObjects++] = triangles[1];

	// background spheres
	float x, y;
	for(int i=0; i<200; i++)
	{
		x = -20 + 2*(i%20);
		y = -2  + 2*(i/20);
		spheres[numObjects] = new Sphere(Vector3(x, y, 20), 1, Vector3(1, 1, 1), MAT_DIFFUSE);
		objects[numObjects] = spheres[numObjects];
		numObjects++;
	}

    lights[0] = new Light(Vector3(-10, 3, 5), Vector3(1, 1, 1));
    lights[1] = new Light(Vector3(10, 3, 7.5), Vector3(0.5, 0.25, 0.25));
	numLights = 2;

	/* debug u and v ray components
	xComp = new unsigned char[3*width*height];
	yComp = new unsigned char[3*width*height];
	float r = nearx, l = -nearx;
	float t = neary, b = -neary;
	float u, v;
	for(int x=0; x<width; x++)
	{
		for(int y=0; y<height; y++)
			{
				int index = 3*width*y + 3*x;
				u = abs(l + (r-l)*(x+0.5)/width)  * 255;
				v = abs(b + (t-b)*(y+0.5)/height) * 255;
				xComp[index+0]=u; xComp[index+1]=0; xComp[index+2]=0;
				yComp[index+0]=0; yComp[index+1]=v; yComp[index+2]=0;
			}
	}
	writePPM("xComp.ppm", xComp, width, height);
	writePPM("yComp.ppm", yComp, width, height);
	delete xComp; delete yComp;*/

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("CMPSC 180, Homework 4");

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);

    float dt = ((float)(clock()-t))/((float)CLOCKS_PER_SEC);
    cout<<dt<<endl;

	glutMainLoop();

	return 0;
}

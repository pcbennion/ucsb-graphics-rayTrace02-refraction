#include "Shapes.h"

using namespace std;

//==========================
// SPHERE CLASS
//==========================
// constructor
Sphere::Sphere(Vector3 center, float radius, Vector3 color, int material)
	{this->center = center; this->r = radius; this->color = color; this->material = material;}

// Ray Intersection
//		solves quadratic equation derived from placing ray components in the sphere equation
//		returns -1 if no intersect
//		returns # of ray lengths to closest intersect otherwise
float Sphere::findIntersect(Vector3 rayDir, Vector3 raySrc, int param)
{
	// Find vector representing center of sphere-->ray origin
	Vector3 omc = raySrc - this->center;

	// Compute three parts of quadratic equation
	float a = rayDir.dot(rayDir);
	float b = 2 * rayDir.dot(omc);
	float c = omc.dot(omc) - r*r;

	// Quadratic formula!
	//	calculate discriminant. If <0, not intersect possible
	float disc = b*b - 4*a*c;
	if(disc<0) return -1;

	// find the two solutions
	float t1 = (-b + sqrt(disc))/(2*a);
	float t2 = (-b - sqrt(disc))/(2*a);

	if(param == NEAR_SIDE)
	{
		// return smaller t
		if(t2>0 && (t2<t1 || t1<0)) return t2;
		return t1;
	} else{
		// return larger t
		if(t1>=t2) return t1;
		return t2;
	}
}

// accessors
Vector3 Sphere::getNormal(Vector3 p)
	{return Vector3(p[0] - center[0], p[1] - center[1], p[2] - center[2]);}
Vector3 Sphere::getCenter()
	{return center;}
Vector3 Sphere::getColor()
	{return color;}
int Sphere::getMaterial()
	{return material;}

//==========================
// TRIANGLE CLASS
//==========================
// constructor
Triangle::Triangle(Vector3 a, Vector3 b, Vector3 c, Vector3 color, int material)
{
	// save coords, color, material
	vert[0] = a;
	vert[1] = b;
	vert[2] = c;
	this->color = color;
	this->material = material;

	// precompute normal
	Vector3 bma(b - a);
	Vector3 cma(c - a);
	normal = bma.cross(cma);
}

// Ray Intersection
//		Uses method found at http://geomalgorithms.com/a06-_intersect-2.html
//			to solve a simplified barycentric model
//		Returns -1 if no intersect
//		Returns # ray lengths to triangle otherwise
float Triangle::findIntersect(Vector3 rayDir, Vector3 raySrc, int param)
{
	// Calculate vectors to represent two edges of triangle
	Vector3 u = vert[1] - vert[0];
	Vector3 v = vert[2] - vert[0];

	// Find distance until intersect with triangle plane
	// 		ray-plane intersection: raySrc + r*rayDir = p, where p is a point on the plane
	Vector3 w = raySrc - vert[0]; // vector of Vert 1 --> Ray Origin
	float nw = - normal.dot(w);
	float ndir = normal.dot(rayDir);
	if(nw<0){nw = -nw; ndir = -ndir;} // ensure the normal is forward-facing
	if(abs(ndir<0.00000001)) return -1; // ray is near parallel with plane; return no intersect
	float r = nw/ndir; // distance to intersect: here's our return value
	if(r<0) return -1; // no intersect; plane is behind ray

	// Calculate useful numbers for barycentric test
	w = (raySrc + rayDir * r) - vert[0]; // Key Vertex --> Intersect Point
	float uu, uv, vv, wu, wv, denom; // dot products and denominator
	uu = u.dot(u); uv = u.dot(v); vv = v.dot(v);
	wu = w.dot(u); wv = w.dot(v);
	denom = uv*uv - uu*vv;

	// Mathematically reduced barycentric test: calculate 'edge coords' along our u and v vectors
	//		w is within triangle if s,  t, and s+t are between 0 and 1
	float s, t;
	s = (uv*wv - vv*wu)/denom;
	if(s<0||s>1) return -1;
	t = (uv*wu - uu*wv)/denom;
	if(t<0||(s+t)>1) return -1;

	// If we've made it this far, intersect is within triangle! Return the calculated distance.
	return r;
}

// accessors
Vector3 Triangle::getNormal(Vector3 p)
	{return normal;}
Vector3 Triangle::getCoord(int index)
	{return vert[index];}
Vector3 Triangle::getColor()
	{return color;}
int Triangle::getMaterial()
	{return material;}

//==========================
// LIGHT CLASS
//==========================
// constructor
Light::Light(Vector3 point, Vector3 color)
	{this->point = point; this->color = color;}

// accessors
Vector3 Light::getPoint()
	{return point;}
Vector3 Light::getColor()
	{return color;}

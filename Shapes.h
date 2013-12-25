#ifndef SHAPES_H_
#define SHAPES_H_

#include "Vector3.h"

// material values
static const int MAT_DIFFUSE = 1;
static const int MAT_REFLECT = 2;
static const int MAT_REFRACT = 3;

// parameter for intersect function
static const int NEAR_SIDE = 0;
static const int FAR_SIDE = -1;

/**
 * Class definitions for Shapes and Lights
 */

class Shape // interface for shape functions
{
public:
    virtual float findIntersect(Vector3 rayDir, Vector3 rayOrig, int param) = 0;
    virtual Vector3 getNormal(Vector3 p) = 0;
    virtual Vector3 getColor() = 0;
    virtual int getMaterial() = 0;
protected:
private:
};

class Sphere : public Shape
{
public:
    Sphere(Vector3 center, float radius, Vector3 color, int material);
    virtual float findIntersect(Vector3 rayDir, Vector3 rayOrig, int param);
    virtual Vector3 getNormal(Vector3 p);
    virtual Vector3 getColor();
    virtual int getMaterial();
    Vector3 getCenter();
protected:
private:
    Vector3 center;
    Vector3 color;
    float r;
    int material;
};

class Triangle : public Shape
{
public:
    Triangle(Vector3 a, Vector3 b, Vector3 c, Vector3 color, int material);
    virtual float findIntersect(Vector3 rayDir, Vector3 rayOrig, int param);
    virtual Vector3 getNormal(Vector3 p);
    virtual Vector3 getColor();
    virtual int getMaterial();
    Vector3 getCoord(int index);
protected:
private:
    Vector3 vert[3];
    Vector3 normal;
    Vector3 color;
    int material;
};

class Light
{
public:
    Light(Vector3 point, Vector3 color);
    Vector3 getPoint();
    Vector3 getColor();
protected:
private:
    Vector3 point;
    Vector3 color;
};

#endif /* SHAPES_H_ */

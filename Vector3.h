#ifndef VECTOR3_H
#define VECTOR3_H

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <cassert>

class Vector3
{
public:
	// Vector elements
	float elems[3];

	// Constructors
	Vector3()
		{elems[0]=0; elems[1]=0; elems[2]=0;};
	Vector3(float coords[3])
		{elems[0]=coords[0]; elems[1]=coords[1]; elems[2]=coords[2];};
	Vector3(float x, float y, float z)
		{elems[0]=x; elems[1]=y; elems[2]=z;};
	Vector3(const Vector3& v)
		{elems[0]=v.elems[0]; elems[1]=v.elems[1]; elems[2]=v.elems[2];};

	// Vector operations:
	//		vector length, dot product, cross product
	void print();
	float length()
		{return sqrt(elems[0]*elems[0] + elems[1]*elems[1] + elems[2]*elems[2]);};
	float dot(Vector3 v)
		{return (elems[0]*v[0] + elems[1]*v[1] + elems[2]*v[2]);};
	Vector3 cross(Vector3 v)
	{
		return Vector3(	elems[1]*v[2] - elems[2]*v[1],
						elems[2]*v[0] - elems[0]*v[2],
						elems[0]*v[1] - elems[1]*v[0]);
	};

	// Subscript operator
	float& operator[] (const int i)
		{assert(i<3&&i>=0); return elems[i];};

	// Assignment operators
	Vector3 operator=(Vector3 rhs)
	{
		this->elems[0] = rhs[0]; this->elems[1] = rhs[1]; this->elems[2] = rhs[2];
		return *this;
	}
	Vector3 operator=(float* rhs)
	{
		this->elems[0] = rhs[0]; this->elems[1] = rhs[1]; this->elems[2] = rhs[2];
		return *this;
	}

	// Arithmetic operators:
	//		vector addition, subtraction, negation, and multiplying across elements
	//		scalar multiplication, division.
	Vector3 operator+=(Vector3 rhs)
		{elems[0]+=rhs[0]; elems[1]+=rhs[1]; elems[2]+=rhs[2]; return *this;};
	Vector3 operator+(Vector3 rhs)
		{return Vector3(elems[0] + rhs[0], elems[1] + rhs[1], elems[2] + rhs[2]);};
	Vector3 operator-()
		{elems[0]=-elems[0]; elems[1]=-elems[1]; elems[2]=-elems[2];return *this;};
	Vector3 operator-=(Vector3 rhs)
		{elems[0]-=rhs[0]; elems[1]-=rhs[1]; elems[2]-=rhs[2]; return *this;};
	Vector3 operator-(Vector3 rhs)
		{return Vector3(elems[0] - rhs[0], elems[1] - rhs[1], elems[2] - rhs[2]);};
	Vector3 operator*=(float rhs)
		{elems[0]*=rhs; elems[1]*=rhs; elems[2]*=rhs; return *this;};
	Vector3 operator*(float rhs)
		{return Vector3(elems[0] * rhs, elems[1] * rhs, elems[2] * rhs);};
	Vector3 operator*(Vector3 rhs)
		{return Vector3(elems[0] * rhs[0], elems[1] * rhs[1], elems[2] * rhs[2]);};
	Vector3 operator/=(float rhs)
		{elems[0]/=rhs; elems[1]/=rhs; elems[2]/=rhs; return *this;};
	Vector3 operator/(float rhs)
		{return Vector3(elems[0] / rhs, elems[1] / rhs, elems[2] / rhs);};

	// Comparison operators
	bool operator==(Vector3 rhs)
	{
		for(int i=0; i<3; i++){if(elems[i]!=rhs[i]) return false;}
		return true;
	};
	bool operator!=(Vector3 rhs)
	{
		for(int i=0; i<3; i++){if(elems[i]!=rhs[i]) return true;}
		return false;
	};
};

// Output operator for easy printing
static std::ostream& operator<<(std::ostream& os, Vector3 v)
{
	std::cout<<"<";
	for(int i=0; i<3; i++) {std::cout<<v[i]; if(i<2) std::cout<<", ";}
	std::cout<<">";
	return os;
}

#endif /* VECTOR3_H_ */

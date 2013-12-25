Peter Bennion
CS180: Computer Graphics
Project 5: Reflective and Refractive Raytracing

Submission is designed to be run on the computers at
UCSB's CSIL tech lab. Makefile and includes are for a
Linux Fedora distribution, with GLU and GLUT installed.

Project builds upon my initial raytracer. Common effects
that "bounce" a light ray have been implemented. The
refraction model in this submission is flawed, causing
too much lensing around the edges.

Added features:
- Triangle shapes.
- Ray reflection, reflective objects.
- Ray refraction, "glass" objects.
- Fresnel effects on glass objects. (Partial reflections)
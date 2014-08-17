#include <iostream>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include "objLoader.h"
#include "bitmap_image.hpp"
#include <math.h>

#define window_width  640
#define window_height 480

float const PI = 3.14159265;
float const RAD_PER_DEGREE = PI / 180;

static float theta = -90.0f;
static float aspect = 1.0f;
static float x = 0.0f;
static float y = 0.0f;
static Mesh surface;

int meshWidth;
int meshHeight;

std::ostream & operator <<(std::ostream & is, vec3 v) {
	is << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return is;
}

float length(vec3 &a) {
	return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

vec3 cross(vec3 &a, vec3 &b) {
	vec3 c;
	c.x = a.y * b.z - a.z * b.y;
	c.y = a.z * b.z - a.x * b.z;
	c.z = a.x * b.y - a.y * b.x;
	return c;
}

void normalize(vec3 &a) {
	float d = length(a);
	a.x /= d;
	a.y /= d;
	a.z /= d;
}

vec3 CalcNormal(vec3 &v1, vec3 &v2, vec3 &v3) {
	vec3 a, b;
	a.x = v2.x - v1.x;
	a.y = v2.y - v1.y;
	a.z = v2.z - v1.z;
	b.x = v3.x - v1.x;
	b.y = v3.y - v1.y;
	b.z = v3.z - v1.z;
	normalize(a);
	normalize(b);
	return cross(a, b);
}

vec3 VectorAdd(vec3 &a, vec3 &b) {
	vec3 res;
	res.x = a.x + b.x;
	res.y = a.y + b.y;
	res.z = a.z + b.z;

	return res;
}

void onResize(int w, int h) {
	glViewport(0, 0, w, h);
	aspect = float(w) / float(h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, aspect, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);

}

void CalcMeshNormals(Mesh &mesh) {
	vec3 v1, v2, v3;
	for (size_t i = 0; i < mesh.indices.size() / 3; ++i) {
		v1 = mesh.vertices[mesh.indices[i * 3]];
		v2 = mesh.vertices[mesh.indices[i * 3 + 1]];
		v3 = mesh.vertices[mesh.indices[i * 3 + 2]];
		vec3 n = CalcNormal(v1, v2, v3);
		// n.x *= -1.0f; n.y *= -1.0f; n.z *= -1.0f;
		mesh.normals[mesh.indices[i * 3]] = VectorAdd(
				mesh.normals[mesh.indices[i * 3]], n);
		mesh.normals[mesh.indices[i * 3 + 1]] = VectorAdd(
				mesh.normals[mesh.indices[i * 3 + 1]], n);
		mesh.normals[mesh.indices[i * 3 + 2]] = VectorAdd(
				mesh.normals[mesh.indices[i * 3 + 2]], n);
	}
	for (size_t i = 0; i < mesh.normals.size(); ++i) {
		normalize(mesh.normals[i]);
	}
}

void GenMesh(float w, float h, float tilesX, float tilesY) {
	float stepX = w / tilesX;
	float stepY = h / tilesY;

	float cx = w / 2.0f;
	float cy = h / 2.0f;

	uint32_t line_size = 0;
	vec3 zero;
	zero.x = 0.0f;
	zero.y = 0.0f;
	zero.z = 0.0f;

	for (size_t i = 0; i <= int(tilesY); ++i) {
		line_size = 0;
		for (size_t j = 0; j <= int(tilesX); ++j) {
			float s = float(j) * stepX - cx;
			float t = float(i) * stepY - cy;
			float r = t * t + s * s;
			vec3 v;
			v.x = s;
			v.z = t;
			v.y = sinf(r / 10.0f) / (1.0f + r / 10.0f);
			surface.vertices.push_back(v);
			surface.normals.push_back(zero);
			++line_size;
		}
	}

	uint32_t pos = 0;

	for (uint32_t y = 0; y < int(tilesY) - 1; ++y) {
		for (uint32_t x = 0; x < int(tilesX) - 1; ++x) {
			surface.indices.push_back(pos);
			surface.indices.push_back(pos + line_size);
			surface.indices.push_back(pos + line_size + 1);
			surface.indices.push_back(pos);
			surface.indices.push_back(pos + line_size + 1);
			surface.indices.push_back(pos + 1);
			++pos;
		}
		pos += 2;
	}
}

void getMeshFromBitmap() {
	using namespace std;
	bitmap_image image("surfacegs.bmp");

	if (!image) {
		cout << "Error - Failed to open: input.bmp\n";
	}

	unsigned char red;
	unsigned char green;
	unsigned char blue;

	const unsigned int height = image.height();
	const unsigned int width = image.width();
	meshHeight = height;
	meshWidth = width;
	vec3 zero;
	zero.x = 0.0f;
	zero.y = 0.0f;
	zero.z = 0.0f;

	for (unsigned int y = 0; y < height; ++y) {
		for (unsigned int x = 0; x < width; ++x) {
			image.get_pixel(x, y, red, green, blue);
			float avarage = float(red) / float(2);
			vec3 v;
			v.x = float(x);
			v.z = float(y);
			v.y = avarage;
			surface.vertices.push_back(v);
			surface.normals.push_back(zero);
		}
	}
	unsigned long pos = 0;
	for (unsigned int y = 0; y < (height - 1); ++y) {
		for (unsigned int x = 0; x < (width - 1); ++x) {
			surface.indices.push_back(pos);
			surface.indices.push_back(pos + width);
			surface.indices.push_back(pos + width + 1);
			surface.indices.push_back(pos);
			surface.indices.push_back(pos + width + 1);
			surface.indices.push_back(pos + 1);
			++pos;
		}
		pos++;
	}
}

void getMeshFromBitmap1(char* filename) {
	int i;
	FILE* f = fopen(filename, "rb");
	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

	// extract image height and width from header
	const unsigned int width = *(int*) &info[18];
	const unsigned int height = *(int*) &info[22];
	int size = 3 * width * height;
	unsigned char* data = new unsigned char[size]; // allocate 3 bytes per pixel
	fread(data, sizeof(unsigned char), size, f); // read the rest of the data at once
	fclose(f);

	meshHeight = height;
	meshWidth = width;
	vec3 zero;
	zero.x = 0.0f;
	zero.y = 0.0f;
	zero.z = 0.0f;

	for (i = 0; i < size; i += 3) {
		vec3 v;
		int pixel = i / 3;
		v.x = pixel % width;
		v.z = pixel / width;
		v.y = float(data[i] + data[i + 1] + data[i + 2]) / float(4);
		surface.vertices.push_back(v);
		surface.normals.push_back(zero);
	}

	unsigned long pos = 0;
	for (unsigned int y = 0; y < (height - 1); ++y) {
		for (unsigned int x = 0; x < (width - 1); ++x) {
			surface.indices.push_back(pos);
			surface.indices.push_back(pos + width);
			surface.indices.push_back(pos + width + 1);
			surface.indices.push_back(pos);
			surface.indices.push_back(pos + width + 1);
			surface.indices.push_back(pos + 1);
			++pos;
		}
		++pos;
	}

}

void InitScene() {
//	GenMesh(30.0f, 30.0f, 50.0f, 50.0f);
//	getMeshFromBitmap1("surfacegs.bmp");
	getMeshFromBitmap();
	CalcMeshNormals(surface);
}

// Main loop
void main_loop_function() {
	// And depth (used internally to block obstructed objects)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Load identity matrix
	glLoadIdentity();
	glPushMatrix();
	glRotatef(-90, 0.0f, 1.0f, 0.0f);
//	glRotatef(theta, 0.0f, 0.0f, 1.0f);
	glRotatef(theta, 0.0f, 1.0f, 0.0f);

//	surface.vertices[vertIndex].y;
	int vertexPositio = -(int(x) + int(y) * meshWidth);
	vec3 vertex = surface.vertices[vertexPositio];
	glTranslatef(x, -5 - vertex.y, y);

	vec4 pos;
	pos.x = 0.0f;
	pos.y = 20.0f;
	pos.z = -99.0f;
	pos.w = 0.0f;
	glLightfv(GL_LIGHT0, GL_POSITION, &pos.x);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, &surface.normals.front());
	glVertexPointer(3, GL_FLOAT, 0, &surface.vertices.front());
	glDrawElements(GL_TRIANGLES, surface.indices.size(), GL_UNSIGNED_INT,
			&surface.indices.front());
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glPopMatrix();
	glutSwapBuffers();
}

void GL_Setup(int width, int height) {
	InitScene();
	float green_mat[] = { 0.2, 0.1, 0.1, 0.0 };
	float green_mat_d[] = { 1.0, 1.0f, 0.5, 0.0 };
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(true);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, green_mat);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, green_mat_d);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, green_mat_d);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &shines);
	glClearColor(0.0f, 0.7f, 0.7f, 0.0f);
	glShadeModel(GLU_SMOOTH);
	glMatrixMode(GL_PROJECTION);
	glViewport(0, 0, width, height);
	gluPerspective(70.0f, (float) width / height, .1, 500);
	glMatrixMode(GL_MODELVIEW);
}

int step = 1;

void onKeyEvent(unsigned char key, int xMouse, int yMouse) {
	switch (key) {
	case 'w':
		x += step * cosf(theta * RAD_PER_DEGREE);
		y += step * sinf(theta * RAD_PER_DEGREE);
		break;
	case 's':
		x -= step * cosf(theta * RAD_PER_DEGREE);
		y -= step * sinf(theta * RAD_PER_DEGREE);
		break;
	case 'a':
		theta -= 3;
		break;
	case 'd':
		theta += 3;
		break;
	}
	if (y > 0) {
		y = 0;
	} else if (y < -meshHeight) {
		y = -meshHeight;
	}
	if (x > 0) {
		x = 0;
	} else if (x < -meshWidth) {
		x = -meshWidth;
	}
}

// Initialize GLUT and start main loop
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitWindowSize(window_width, window_height);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Hello GL");
	glutIdleFunc(main_loop_function);
	GL_Setup(window_width, window_height);
	glutKeyboardFunc(onKeyEvent);
	glutMainLoop();
}

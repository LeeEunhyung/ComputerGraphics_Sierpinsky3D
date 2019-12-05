#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "InitShader.h"
#include "mat.h"
#include "vec.h"

#define BUFFER_OFFSET( offset )	((GLvoid*) (offset))

unsigned int width = 512;
unsigned int height = 512;

GLuint vao;
GLuint buffer;
GLuint simpleProgram;
GLuint vPosition;
GLuint vColor;

int NumSubdivides;
int NumTriangles;
int NumVertices;

int Counting = 3;

vec3* points;
vec3* colors;
int index = 0;

mat4 rotating;
mat4 scaling;

float theta = 0.01f;
float scale = 0.5f;
float scaleUnit = 0.01f;

int NumClickLeft = 0;
int NumClickRight = 0;

vec3 baseVertices[] =
{
	vec3(0.0f,					0.0f,						1.0f),
	vec3(0.0f,					0.942809f,				-0.333333f),
	vec3(-0.816497f,		-0.471405f,			-0.333333f),
	vec3(0.816497f,			-0.471405f,			-0.333333f)
};

vec3 baseColors[] =
{
	vec3(1.0f, 0.0f, 0.0f), // 빨
	vec3(0.0f, 1.0f, 0.0f), // 초
	vec3(0.0f, 0.0f, 1.0f), // 파
	vec3(0.0f, 0.0f, 0.0f), // 검
	vec3(0.5f, 0.5f, 0.5f)  // 회
};

void init();
void display();
void idle();

void divide_triangle(vec3 a, vec3 b, vec3 c, unsigned int cnt, unsigned int coli);
void triangle(vec3 a, vec3 b, vec3 c, unsigned int coli);
void draw3DGasket(unsigned int col1, unsigned int col2, unsigned int col3, unsigned int col4);

void gasketKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void gasketMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

void setGasketVertices(unsigned int col1, unsigned int col2, unsigned int col3, unsigned int col4);

void devide();

int main()
{
	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	window = glfwCreateWindow(width, height, "HW#2", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) std::cout << "Error\n";

	init();

	glfwSetKeyCallback(window, gasketKeyCallback);
	glfwSetMouseButtonCallback(window, gasketMouseButtonCallback);

	while (!glfwWindowShouldClose(window))
	{
		display();
		idle();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	free(points);
	free(colors);

	return 0;
}

void init()
{
	NumSubdivides = Counting;
	NumTriangles = 4 * pow(3.0f, NumSubdivides);
	NumVertices = 6 * NumTriangles;

	points = (vec3*)malloc(sizeof(vec3) * NumVertices);
	colors = (vec3*)malloc(sizeof(vec3) * NumVertices);

	draw3DGasket(0, 1, 2, 3);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(vec3) * NumVertices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * NumVertices, points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec3) * NumVertices, sizeof(vec3) * NumVertices, colors);

	simpleProgram = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(simpleProgram);

	vPosition = glGetAttribLocation(simpleProgram, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	vColor = glGetAttribLocation(simpleProgram, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vec3) * NumVertices));

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glUseProgram(simpleProgram);

	if (NumClickRight % 4 == 0)
	{
		rotating = 1; 
	}
	else if (NumClickRight % 4 == 1)
	{
		rotating = RotateX(theta);
	}
	else if (NumClickRight % 4 == 2)
	{
		rotating = RotateY(theta);
	}
	else if (NumClickRight % 4 == 3)
	{
		rotating = RotateZ(theta);
	}
	scaling = Scale(scale);

	GLuint uMat = glGetUniformLocation(simpleProgram, "uMat");
	glUniformMatrix4fv(uMat, 1, GL_FALSE, rotating * scaling); //mat4는 shader 안에서는 mat 또는 배열로 취급 x -> count = 1 / 행과 열을 바꿀건지 바꾸지 않을건지

	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}

void idle()
{
	theta += 0.1f;
}

void divide_triangle(vec3 a, vec3 b, vec3 c, unsigned int cnt, unsigned int coli)
{
	if (cnt > 0)
	{
		vec3 ab = (a + b) / 2.0f;
		vec3 ac = (a + c) / 2.0f;
		vec3 bc = (b + c) / 2.0f;

		triangle(ab, bc, ac, 4);

		divide_triangle(a, ab, ac, cnt - 1, coli);
		divide_triangle(c, ac, bc, cnt - 1, coli);
		divide_triangle(b, bc, ab, cnt - 1, coli);
	}
	else
	{
		triangle(a, b, c, coli);
	}
}

void triangle(vec3 a, vec3 b, vec3 c, unsigned int coli)
{
	points[index] = a;
	colors[index++] = baseColors[coli];

	points[index] = b;
	colors[index++] = baseColors[coli];

	points[index] = c;
	colors[index++] = baseColors[coli];
}

void draw3DGasket(unsigned int col1, unsigned int col2, unsigned int col3, unsigned int col4)
{
	divide_triangle(baseVertices[0], baseVertices[1], baseVertices[2], NumSubdivides, col1);
	divide_triangle(baseVertices[3], baseVertices[2], baseVertices[1], NumSubdivides, col2);
	divide_triangle(baseVertices[0], baseVertices[3], baseVertices[1], NumSubdivides, col3);
	divide_triangle(baseVertices[0], baseVertices[2], baseVertices[3], NumSubdivides, col4);
}

void gasketKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_Q:
			exit(0);
			break;
		
		case GLFW_KEY_1:
			setGasketVertices(0, 1, 2, 3);
			NumClickLeft = 0;
			break;

		case GLFW_KEY_2:
			setGasketVertices(1, 2, 3, 0);
			NumClickLeft = 1;
			break;

		case GLFW_KEY_3:
			setGasketVertices(2, 3, 0, 1);
			NumClickLeft = 2;
			break;

		case GLFW_KEY_4:
			setGasketVertices(3, 0, 1, 2);
			NumClickLeft = 3;
			break;

		case GLFW_KEY_U:
			Counting++;
			devide();
			break;

		case GLFW_KEY_D:
			Counting--;
			if (Counting <= 0) Counting = 1;
			devide();
			break;

		case GLFW_KEY_EQUAL:
			scale += scaleUnit;
			break;

		case GLFW_KEY_MINUS:
			scale -= scaleUnit;
			if (scale <= 0.1f) scale = 0.1f;
			break;

		default:
			break;
		}
	}
}

void gasketMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		NumClickLeft++;
		if (NumClickLeft % 4 == 0)
		{
			setGasketVertices(0, 1, 2, 3);
		}
		else if (NumClickLeft % 4 == 1)
		{
			setGasketVertices(1, 2, 3, 0);
		}
		else if (NumClickLeft % 4 == 2)
		{
			setGasketVertices(2, 3, 0, 1);
		}
		else if (NumClickLeft % 4 == 3)
		{
			setGasketVertices(3, 0, 1, 2);
		}
		if (NumClickLeft > 3)
			NumClickLeft = 0;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) 
	{
		NumClickRight++;
	}
}

void setGasketVertices(unsigned int col1, unsigned int col2, unsigned int col3, unsigned int col4)
{
	index = 0;

	draw3DGasket(col1, col2, col3, col4);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * NumVertices, points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec3) * NumVertices, sizeof(vec3) * NumVertices, colors);
}

void devide()
{
	index = 0;

	NumSubdivides = Counting;
	NumTriangles = 4 * pow(3.0f, NumSubdivides);
	NumVertices = 6 * NumTriangles;

	points = (vec3*)realloc(points, sizeof(vec3) * NumVertices);
	colors = (vec3*)realloc(colors, sizeof(vec3) * NumVertices);

	draw3DGasket(0, 1, 2, 3);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(vec3) * NumVertices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * NumVertices, points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec3) * NumVertices, sizeof(vec3) * NumVertices, colors);

	vPosition = glGetAttribLocation(simpleProgram, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	vColor = glGetAttribLocation(simpleProgram, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vec3) * NumVertices));

	simpleProgram = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(simpleProgram);

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

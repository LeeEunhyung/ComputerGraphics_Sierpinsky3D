#version 330

in vec4 vPosition;
in vec4 vColor;

uniform mat4 uMat;

out vec4 fColor;

void main()
{
	gl_Position = uMat * vPosition;
	fColor = vColor;
}
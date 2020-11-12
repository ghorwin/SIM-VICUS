#version 330 core

// vertex shader

// input:  attribute named 'position' with 3 floats per vertex
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;

// test: 'flat' in front of fragColor disables color interpolation
out vec4 fragColor;

uniform mat4 worldToView;            // parameter: the camera matrix

void main() {
  gl_Position = worldToView * vec4(position, 1.0);
  fragColor = color;
}


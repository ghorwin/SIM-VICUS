#version 330

// GLSL version 3.3

// Vertex shader with only position and a transformation matrix.

layout(location = 0) in vec3 position; // input:  attribute with index '0' with 3 floats (x, y, z coords) per vertex

uniform mat4 worldToView;              // parameter: world to view transformation matrix
uniform mat4 modelToWorld;             // parameter: model to world transformation matrix

void main() {
  // Mind matrix multiplication order
  gl_Position = worldToView * modelToWorld * vec4(position, 1.0);
}


#version 330

// GLSL version 3.3
// Vertex shader with only a world2view matrix.

layout(location = 0) in vec3 position; // input:  attribute with index '0' with 3 floats (x, y, z coords) per vertex

flat out vec3 startPos;
out vec3 vertPos;

uniform mat4 worldToView;              // parameter: world to view transformation matrix

void main() {
  // Mind matrix multiplication order
  vec4 pos = worldToView * vec4(position, 1.0);

  gl_Position = pos;

  vertPos     = pos.xyz / pos.w;
  startPos    = vertPos;
}


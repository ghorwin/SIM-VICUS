#version 330

// GLSL version 3.3
// Vertex shader for simple lines without color vertex attribute and
// a transformation matrix.
// Color of fragments is controlled by fragment shader.

layout(location = 0) in vec3 position; // input:  attribute with index '0' with 3 floats (x, y, z coords) per vertex

out vec4 fragColor;                    // output: fragment color

uniform mat4 worldToView;              // parameter: world to view transformation matrix
uniform mat4 modelToWorld;             // parameter: model to world transformation matrix

void main() {
  // Mind matrix multiplication order
  gl_Position = worldToView * modelToWorld * vec4(position, 1.0);
  fragColor = vec4(0.9f, 0.9f, 0.9f, 1.0f);
}


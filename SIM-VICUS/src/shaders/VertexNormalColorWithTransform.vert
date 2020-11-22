#version 330

// GLSL version 3.3
// vertex shader

layout(location = 0) in vec3 position; // input:  attribute with index '0' with 3 elements per vertex
layout(location = 1) in vec3 normal;   // input:  attribute with index '1' with 3 elements per vertex
layout(location = 2) in vec4 color;    // input:  attribute with index '2' with 4 elements (=rgba) per vertex

out vec4 fragColor;                    // output: fragment color
out vec3 fragNormal;                   // output: fragment normal vector
out vec3 fragPos;                      // output: fragment position in world coords

uniform mat4 worldToView;              // parameter: world to view transformation matrix
uniform mat4 modelToWorld;             // parameter: model to world transformation matrix

void main() {
  // Mind multiplication order for matrixes
  gl_Position = worldToView * modelToWorld * vec4(position, 1.0);
  fragPos = position;
  fragColor = color;
  fragNormal = normal; 
  // Note: do not rotate normals - light position is also given in world coordinates
}

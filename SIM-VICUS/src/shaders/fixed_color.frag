#version 330 core

// fragment shader

uniform vec3 col;                // parameter: grid color as rgb triple

out vec4 finalColor;             // output:    final color value as rgba-value

void main() {
  finalColor = vec4(col, 1.0);
}

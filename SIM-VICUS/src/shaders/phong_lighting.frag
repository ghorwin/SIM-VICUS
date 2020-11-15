#version 330 core

// fragment shader with phong lighting (diffuse and specular lighting)

in vec3 fragPos;           // input: fragment position (world coords)
in vec4 fragColor;         // input: interpolated color as rgba-value
in vec3 fragNormal;        // input: normal vector of fragment

out vec4 finalColor;       // output: final color value as rgba-value

uniform vec3 lightPos;     // parameter: light position as vec3 (world coords)
uniform vec3 lightColor;   // parameter: light color as rgb
uniform vec3 viewPos;      // parameter: view position as vec3 (world coords)

void main() {
  // ambient
  float ambientStrength = 0.1;
  vec3 ambient = ambientStrength * lightColor;

  // diffuse
  vec3 norm = normalize(fragNormal);
  vec3 lightDir = normalize(lightPos - fragPos);
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  // specular
  float specularStrength = 0.4;
  vec3 viewDir = normalize(viewPos - fragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = specularStrength * spec * lightColor;

  vec3 result = (ambient + diffuse + specular) * fragColor.xyz;
  finalColor = vec4(result, 1.0);
}

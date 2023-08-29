/*	SIM-VICUS - Building and District Energy Simulation Tool.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Dirk Weiss  <dirk.weiss -[at]- tu-dresden.de>
	  Stephan Hirth  <stephan.hirth -[at]- tu-dresden.de>
	  Hauke Hirsch  <hauke.hirsch -[at]- tu-dresden.de>

	  ... all the others from the SIM-VICUS team ... :-)

	This program is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#ifndef Vic3DConstantsH
#define Vic3DConstantsH

namespace Vic3D {

/*! Definition of shader program IDs. */
enum Shaders {
	/*! grid.vert:
		in vec2 position  (x,y)
		uniform mat4 worldToView

		grid.frag:
		uniform vec3 gridColor;                // parameter: grid color as rgb triple
		uniform vec3 backColor;                // parameter: background color as rgb triple
	*/
	SHADER_GRID,

	/*! Vertex.vert:
		in vec3 position  (x,y,z)
		uniform mat4 worldToView

		fixed_color.frag:
		uniform vec3 fixedColor;               // parameter: grid color as rgb triple
	*/
	SHADER_LINES,

	/*! dashedLines.vert:
		layout(location = 0) in vec3 position; // input:  attribute with index '0' with 3 floats (x, y, z coords) per vertex

		flat out vec3 startPos;                // output: start position of line with no (flat) interpolation -> start positon on the line in NDC-Coordinates
		out vec3 vertPos;                      // output: start position of line with default interpolation   -> actual positon on the line in NDC-Coordinates
											   // --> startPos, vertPos
		uniform mat4 worldToView;              // parameter: world to view transformation matrix

		dashedLines.frag:
		uniform vec4 fixedColor;               // parameter: grid color as rgba-value
		uniform vec2  resolution;              // parameter: resolution of viewport (width, height)
		uniform float dashSize;                // parameter: dash line size
		uniform float gapSize;                 // parameter: gap size
	*/
	SHADER_DASHED_LINES,

	/*! VertexNormalColor.vert:
		layout(location = 0) in vec3 position; // input:  attribute with index '0' with 3 elements per vertex (coordinates)
		layout(location = 1) in vec3 normal;   // input:  attribute with index '1' with 3 elements per vertex (normal)
		layout(location = 2) in vec4 color;    // input:  attribute with index '2' with 4 elements (=rgba) per vertex
		uniform mat4 worldToView;              // parameter: the world-to-view matrix

		phong_lighting.frag:
		uniform vec3 lightPos;                 // parameter: light position as vec3 (world coords)
		uniform vec3 lightColor;               // parameter: light color as rgb
		uniform vec3 viewPos;                  // parameter: view position as vec3 (world coords)
	*/
	SHADER_OPAQUE_GEOMETRY,

	/*! VertexWithTransform.vert:
		layout(location = 0) in vec3 position; // input:  attribute with index '0' with 3 floats (x, y, z coords) per vertex
		uniform mat4 worldToView;              // parameter: world to view transformation matrix
		uniform mat4 modelToWorld;             // parameter: model to world transformation matrix

		fixed_color.frag:
		uniform vec3 fixedColor;               // parameter: grid color as rgb triple

		Note: Used to draw selected geometry with wireframe.
	*/
	SHADER_WIREFRAME,

	/*! VertexNormalColorWithTransform.vert:
		layout(location = 0) in vec3 position; // input:  attribute with index '0' with 3 elements per vertex
		layout(location = 1) in vec3 normal;   // input:  attribute with index '1' with 3 elements per vertex
		layout(location = 2) in vec4 color;    // input:  attribute with index '2' with 4 elements (=rgba) per vertex
		uniform mat4 worldToView;              // parameter: world to view transformation matrix
		uniform mat4 modelToWorld;             // parameter: model to world transformation matrix

		phong_lighting.frag:
		uniform vec3 lightPos;                 // parameter: light position as vec3 (world coords)
		uniform vec3 lightColor;               // parameter: light color as rgb
		uniform vec3 viewPos;                  // parameter: view position as vec3 (world coords)
	*/
	SHADER_COORDINATE_SYSTEM,

	/*!	VertexNormalColor.vert:
		layout(location = 0) in vec3 position; // input:  attribute with index '0' with 3 elements per vertex (coordinates)
		layout(location = 1) in vec3 normal;   // input:  attribute with index '1' with 3 elements per vertex (normal)
		layout(location = 2) in vec4 color;    // input:  attribute with index '2' with 4 elements (=rgba) per vertex
		uniform mat4 worldToView;              // parameter: the world-to-view matrix

		simple.frag:
		no uniforms
	*/
	SHADER_TRANSPARENT_GEOMETRY,
	NUM_SHADER_PROGRAMS
};

// default line width
const double DEFAULT_LINE_WEIGHT			= 0.05;
// multiplier to apply to width of entities
const double DEFAULT_LINE_WEIGHT_SCALING	= 0.005;
// Multiplyer for different layers and their heights
const double Z_MULTIPLYER					= 0.00005;

// in m, should be enough, right?
const float SNAP_DISTANCES_THRESHHOLD		= 0.5;

const unsigned int SEGMENT_COUNT_ARC		= 5;
const unsigned int SEGMENT_COUNT_CIRCLE		= 10;
const unsigned int SEGMENT_COUNT_ELLIPSE	= 5;


#define VIC3D_STRIP_STOP_INDEX 0xFFFFFFFF

#define INVALID_POINT QVector3D(-1e19f,-1e19f,-1e19f)

} // namespace Vic3D

#endif // Vic3DConstantsH

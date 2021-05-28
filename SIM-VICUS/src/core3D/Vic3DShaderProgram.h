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

#ifndef Vic3DShaderProgramH
#define Vic3DShaderProgramH

#include <QString>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QOpenGLShaderProgram;
QT_END_NAMESPACE

namespace Vic3D {

/*! A small wrapper class around QOpenGLShaderProgram to encapsulate
	shader code compilation and linking and error handling.

	It is meant to be used with shader programs in files, for example from
	qrc files.

	The embedded shader programm is not destroyed automatically upon destruction.
	You must call destroy() to end the lifetime of the allocated OpenGL resources.
*/
class ShaderProgram {
public:
	ShaderProgram() {}
	ShaderProgram(const QString & vertexShaderFilePath, const QString & fragmentShaderFilePath);

	/*! Creates shader program, compiles and links the programs. */
	void create();
	/*! Destroys OpenGL resources, OpenGL context must be made current before this function is callded! */
	void destroy();

	/*! Access to the native shader program. */
	QOpenGLShaderProgram * shaderProgram() { return m_program; }

	/*! Just forwards to embedded QShaderProgram. */
	void bind();
	/*! Just forwards to embedded QShaderProgram. */
	void release();

	/*! Path to vertex shader program, used in create(). */
	QString		m_vertexShaderFilePath;
	/*! Path to fragment shader program, used in create(). */
	QString		m_fragmentShaderFilePath;


	// Note: Uniform-Handling is pretty simple, probably better to wrap that somehow.

	/*! List of uniform values to be resolved. Values is used in create(). */
	QStringList	m_uniformNames;

	/*! Holds uniform Ids to be used in conjunction with setUniformValue(). */
	QList<int>	m_uniformIDs;

private:
	/*! The wrapped native QOpenGLShaderProgram. */
	QOpenGLShaderProgram	*m_program;
};

} // namespace Vic3D

#endif // Vic3DShaderProgramH

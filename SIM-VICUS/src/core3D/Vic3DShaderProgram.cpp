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

#include "Vic3DShaderProgram.h"

#include <QOpenGLShaderProgram>
#include <QDebug>

#include "Vic3DOpenGLException.h"

namespace Vic3D {


ShaderProgram::ShaderProgram(const QString & vertexShaderFilePath, const QString & fragmentShaderFilePath) :
	m_vertexShaderFilePath(vertexShaderFilePath),
	m_fragmentShaderFilePath(fragmentShaderFilePath),
	m_program(nullptr)
{
}


void ShaderProgram::create() {
	FUNCID(ShaderProgram::create);
	Q_ASSERT(m_program == nullptr);

	// build and compile our shader program
	// ------------------------------------

	m_program = new QOpenGLShaderProgram();

	// read the shader programs from the resource
	if (!m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, m_vertexShaderFilePath))
		throw OpenGLException(QString("Error compiling vertex shader %1:\n%2").arg(m_vertexShaderFilePath).arg(m_program->log()), FUNC_ID);

	if (!m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, m_fragmentShaderFilePath))
		throw OpenGLException(QString("Error compiling fragment shader %1:\n%2").arg(m_fragmentShaderFilePath).arg(m_program->log()), FUNC_ID);

	if (!m_program->link())
		throw OpenGLException(QString("Shader linker error:\n%2").arg(m_program->log()), FUNC_ID);

	m_uniformIDs.clear();
	for (const QString & uniformName : m_uniformNames)
		m_uniformIDs.append( m_program->uniformLocation(uniformName));
}


void ShaderProgram::destroy() {
	delete m_program;
	m_program = nullptr;
}


void ShaderProgram::bind() {
	m_program->bind();
}

void ShaderProgram::release() {
	m_program->release();
}

} // namespace Vic3D

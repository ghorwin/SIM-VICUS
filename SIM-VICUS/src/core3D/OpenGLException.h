#ifndef OPENGLEXCEPTION_H
#define OPENGLEXCEPTION_H

#include <QString>
#include <IBK_Exception.h>

/*! A wrapper around IBK::Exception, that accepts QString arguments. */
class OpenGLException : public IBK::Exception {
public:
	OpenGLException(const QString & msg) :
		IBK::Exception(msg.toStdString(), "")
	{
	}

	OpenGLException(const QString & msg, const QString & where) :
		IBK::Exception(msg.toStdString(), where.toStdString())
	{
	}

	OpenGLException(OpenGLException & previous, const QString & msg) :
		IBK::Exception(previous, msg.toStdString(), "")
	{
	}

	OpenGLException(OpenGLException & previous, const QString & msg, const QString & where)  :
		IBK::Exception(previous, msg.toStdString(), where.toStdString())
	{
	}
};

#endif // OPENGLEXCEPTION_H

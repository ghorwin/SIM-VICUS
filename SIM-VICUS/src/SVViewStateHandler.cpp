#include "SVViewStateHandler.h"

SVViewStateHandler * SVViewStateHandler::m_self = nullptr;

SVViewStateHandler & SVViewStateHandler::instance() {
	Q_ASSERT_X(m_self != nullptr, "[SVViewStateHandler::instance]", "You must create an instance of "
		"SVViewStateHandler before accessing SVViewStateHandler::instance()!");
	return *m_self;
}

SVViewStateHandler::SVViewStateHandler(QObject * parent) :
	QObject(parent)
{
	// singleton check
	Q_ASSERT_X(m_self == nullptr, "[SVViewStateHandler::SVViewStateHandler]", "You must not create multiple instances of "
		"SVViewStateHandler!");
	m_self = this;
}


SVViewStateHandler::~SVViewStateHandler() {
	m_self = nullptr;
}


void SVViewStateHandler::setViewState(const SVViewState & newViewState) {
	m_previousViewState = m_viewState;
	m_viewState = newViewState;
	emit viewStateChanged();
}


void SVViewStateHandler::restoreLastViewState() {
	setViewState(SVViewState(m_previousViewState)); // Mind: temporary object is needed here!
}


void SVViewStateHandler::refreshColors() {
	emit colorRefreshNeeded();
}

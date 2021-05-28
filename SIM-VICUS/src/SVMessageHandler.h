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

#ifndef SVMessageHandlerH
#define SVMessageHandlerH

#include <QObject>
#include <IBK_MessageHandler.h>

class SVMessageHandler : public QObject, public IBK::MessageHandler {
	Q_OBJECT
public:
	explicit SVMessageHandler(QObject *parent = nullptr);
	virtual ~SVMessageHandler();


	/*! Overloaded to received msg info. */
	virtual void msg(const std::string& msg,
		IBK::msg_type_t t = IBK::MSG_PROGRESS,
		const char * func_id = nullptr,
		int verbose_level = -1);

signals:
	/*! Emitted whenever a message was received.
		Shall be connected to the log window to display the message.
	*/
	void msgReceived(int type, QString msg);

};

#endif // SVMessageHandlerH

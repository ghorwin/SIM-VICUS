/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
*/

#ifndef QtExt_SleeperH
#define QtExt_SleeperH

#include <QThread>

namespace QtExt {

/*! \brief Contains some function for sleeping. Subclassed from QThread in order to make these functions public.*/
class Sleeper : public QThread
{
public:
	/*! Forces the current thread to sleep for usecs microseconds.*/
	static void usleep(unsigned long usecs) {
		QThread::usleep(usecs);
	}

	/*! Forces the current thread to sleep for msecs milliseconds.*/
	static void msleep(unsigned long msecs) {
		QThread::msleep(msecs);
	}

	/*! Forces the current thread to sleep for secs seconds.*/
	static void sleep(unsigned long secs){
		QThread::sleep(secs);
	}
};

} // end namespace QtExt

/*! \file QtExt_Sleeper.h
	\brief Contains the class Sleeper.
*/

#endif // QtExt_SleeperH

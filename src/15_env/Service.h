
/** service class header.
    @file service.h
 
    This file belongs to the SYNTHESE project (public transportation specialized software)
    Copyright (C) 2002 Hugues Romain - RCS <contact@reseaux-conseil.com>
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef SYNTHESE_ENV_SERVICE_H
#define SYNTHESE_ENV_SERVICE_H

#include "Complyer.h"
#include "ServicePointer.h"
#include "Types.h"

#include <string>

#include "UId.h"

#include "Schedule.h"

namespace synthese
{
	namespace time
	{
		class Date;
		class DateTime;
	}

	namespace env
	{
		class Path;

		/** Service abstract base class.
			TRIDENT = VehicleJourney

			A service represents the ability to follow a path
			at certain days and hours.

			The days when the service is provided are stored in a Calendar object.
			Even if a Service intrinsically corresponds to a sequence of 
			(arrival schedule - departure schedule) couples, these schedules are not
			stored in Service objects but per Edge (Line/Road). However, this is how 
			Service objects are persisted.

			It is completely independent from the "vehicle" : this ability 
			can be provided by an external entity (bus, train...), 
			but also self-provided by the traveller himself 
			(walking, cycling...).

			@ingroup m35
		*/
		class Service : public Complyer
		{
		private:
			std::string	_serviceNumber;
			uid			_pathId;
			Path*		_path;

		public:

			Service(
				const std::string& serviceNumber,
				Path* path
			);

			Service();
			~Service ();


			//! @name Getters
			//@{
				const Path*			getPath () const;
				Path*				getPath ();
				uid					getPathId()			const;
				const std::string&	getServiceNumber()	const;


				/** Gets a departure schedule for this service.
					@param rank Rank of the stop where to get the departure schedule
					@return see the implementations of the method.
				*/
				virtual time::Schedule getDepartureSchedule (int rank = 0) const = 0;
			//@}

			//! @name Setters
			//@{
				void setPath(Path* path);
				void setPathId(uid id);
				void setServiceNumber(std::string serviceNumber);
			//@}



			//! @name Query methods
			//@{
				virtual bool isContinuous () const = 0;

				/** Accessor to the key of the service, provided by the implementation subclass.
				@return id of the service in the database
				*/
				virtual uid		getId()	const = 0;

				virtual std::string getTeam() const;

				/** Is this service providen a given day ?
					@param originDate Departure date of the service from its origin (warning: do not test the customer departure date which can be one or more days later; use getOriginDateTime to compute the origin date)
					@return true if the service runs at the specified date according to its Calendar
				*/
				virtual bool isProvided(const time::Date& originDate) const = 0;

				virtual time::Schedule getDepartureBeginScheduleToIndex(int rankInPath) const = 0;
				virtual time::Schedule getDepartureEndScheduleToIndex(int rankInPath) const = 0;
				virtual time::Schedule getArrivalBeginScheduleToIndex(int rankInPath) const = 0;
				virtual time::Schedule getArrivalEndScheduleToIndex(int rankInPath) const = 0;



				/** Tests if the service could be inserted in the same line than the current one, according to the line theory.
					@param other service to test
					@return bool true if the two services are compatible
					@author Hugues Romain
					@date 2008
					@warning the method must be used only at a time where the service knows its path, so use the method on the currently registered service, taking the new one as parameter
				*/
				bool respectsLineTheoryWith(const Service& other) const;



				/** Generation of the next departure of a service according to a schedule and a presence date time, in the day of the presence time only, according to the compliances.
					@param method Search departure or arrival :
						- ServicePointer::DEPARTURE_TO_ARRIVAL
						- ServicePointer::ARRIVAL_TO_DEPARTURE
					@param edge Edge
					@param presenceDateTime Goal  time
					@param computingTime Time of the computing
					@param controlIfTheServiceIsReachable service selection method :
						- true : the result is a usable service : its departure time must be in the future, and the reservation rules must be followed
						- false : the result is a runnable service : if the reservation on it is compulsory, then there must bu at least one reservation for the service
					@param inverted : indicates if the range computing must follow the same rules as method says (false) or the inverted ones (true)
					@return A full ServicePointer to the service. If the service cannot be used at the specified date/time, then the ServicePointer points to a NULL service.
					@author Hugues Romain
					@date 2007
					@warning The service index is unknown in the generated ServicePointer.					
				*/
				virtual ServicePointer getFromPresenceTime(
					AccessDirection method
					, const Edge* edge
					, const time::DateTime& presenceDateTime
					, const time::DateTime& computingTime
					, bool controlIfTheServiceIsReachable
					, bool inverted
				) const = 0;

				virtual time::DateTime getLeaveTime(
					const ServicePointer& servicePointer
					, const Edge* edge
				) const = 0;

				/** Date of the departure from the origin of the service.
				@param departureDate Date of use of the service at the scheduled point
				@param departureTime Known schedule of departure in the service journey
				@return time::DateTime Date of the departure from the origin of the service.
				@author Hugues Romain
				@date 2007

				*/
				time::DateTime getOriginDateTime(const time::Date& departureDate, const time::Schedule& departureTime) const;
			//@}
		};
	}
}

#endif

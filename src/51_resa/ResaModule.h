
/** ResaModule class header.
	@file ResaModule.h

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

#ifndef SYNTHESE_ResaModule_h__
#define SYNTHESE_ResaModule_h__

#include "ResaTypes.h"

#include "ModuleClassTemplate.hpp"
#include "Registry.h"

#include <map>
#include <ostream>

#include <boost/thread/recursive_mutex.hpp>

namespace synthese
{
	namespace pt
	{
		class ReservationContact;
	}

	namespace util
	{
		class Env;
	}

	namespace dblog
	{
		class DBLogEntry;
	}

	namespace security
	{
		class Profile;
	}

	namespace html
	{
		class HTMLTable;
	}

	namespace server
	{
		class Session;
	}

	/**	@defgroup m51Actions 51 Actions
		@ingroup m51

		@defgroup m51Pages 51 Pages
		@ingroup m51

		@defgroup m51Functions 51 Functions
		@ingroup m51

		@defgroup m51Exceptions 51 Exceptions
		@ingroup m51

		@defgroup m51Alarm 51 Messages recipient
		@ingroup m51

		@defgroup m51LS 51 Table synchronizers
		@ingroup m51

		@defgroup m51Admin 51 Administration pages
		@ingroup m51

		@defgroup m51Rights 51 Rights
		@ingroup m51

		@defgroup m51Logs 51 DB Logs
		@ingroup m51

		@defgroup m51 51 Reservation
		@ingroup m5

		The reservation module provides the ability to book seats on demand responsive transport lines.

		The features of the reservation module are :
			- BoolingScreenFunction : booking screen, available directly in a route planner journey roadmap
			- BoolingConfirmationFunction : booking confirmation
			
		A logged standard user uses the administration panel to access to the following features :
			- edit personal informations (in security module)
			- edit favorites journeys (in pt_journeyplanner module)
			- display reservations history
				- cancel a reservation
			- display an integrated route planner (in pt_journeyplanner module)

		A logged operator uses the administration panel to access to the following features :
			- personal informations (security module)
			- search a customer (security module)
				- edit customer personal informations (security module)
				- display customer reservations history
					- cancel a reservation
			- display the commercial lines with reservations
				- display the reservation list of the line
					- ServiceReservationsRoadMapFunction : display the detailed reservation list of a service / course (pop up : not in the admin panel, optimized for printing)
			- display an integrated route planner (pt_journeyplanner module)
			
			Move the following features in a call center module :
			- display the call center planning
			- display the calls list
				- display a call log

		A logged driver uses the administration panel to access to the following features :
			- display the commercial lines
				- display the reservation list of the line
					- display the detailed reservation list of a service / course

	@{
	*/

	//////////////////////////////////////////////////////////////////////////
	/// 51 Reservation Module namespace.
	///	@author Hugues Romain
	///	@date 2008
	//////////////////////////////////////////////////////////////////////////
	namespace resa
	{
		class ReservationTransaction;
		class CancelReservationAction;
		class OnlineReservationRule;

		/** 51 Reservation module class.
		*/
		class ResaModule:
			public server::ModuleClassTemplate<ResaModule>
		{
			friend class server::ModuleClassTemplate<ResaModule>;
			
		private:
			typedef std::map<const server::Session*, util::RegistryKeyType> _SessionsCallIdMap;
			static _SessionsCallIdMap _sessionsCallIds;
			static boost::recursive_mutex _sessionsCallIdsMutex;

			static const std::string _BASIC_PROFILE_NAME;
			static const std::string _AUTORESA_PROFILE_NAME;
			static const std::string _ADMIN_PROFILE_NAME;
			static const std::string _RESERVATION_CONTACT_PARAMETER;

			static boost::shared_ptr<security::Profile>	_basicProfile;
			static boost::shared_ptr<security::Profile>	_autoresaProfile;
			static boost::shared_ptr<security::Profile>	_adminProfile;
			static boost::shared_ptr<OnlineReservationRule> _reservationContact;

		public:
			static boost::shared_ptr<security::Profile> GetBasicResaCustomerProfile();
			static boost::shared_ptr<security::Profile> GetAutoResaResaCustomerProfile();

			/** Called whenever a parameter registered by this module is changed
			*/
			static void ParameterCallback(
				const std::string& name,
				const std::string& value
			);

			static void DisplayReservations(
				std::ostream& stream,
				const ReservationTransaction& reservation
			);

			static void CallOpen(const server::Session* session);
			static void CallClose(const server::Session* session);
			static util::RegistryKeyType GetCurrentCallId(const server::Session* session);

			static std::string GetStatusIcon(ReservationStatus status);
			static std::string GetStatusText(ReservationStatus status);

			static OnlineReservationRule* GetReservationContact();
		};
	}
	/** @} */
}

#endif // SYNTHESE_ResaModule_h__

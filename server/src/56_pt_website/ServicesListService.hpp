
//////////////////////////////////////////////////////////////////////////////////////////
///	ServicesListService class header.
///	@file ServicesListService.hpp
///	@author Hugues Romain
///	@date 2012
///
///	This file belongs to the SYNTHESE project (public transportation specialized software)
///	Copyright (C) 2002 Hugues Romain - RCSmobility <contact@rcsmobility.com>
///
///	This program is free software; you can redistribute it and/or
///	modify it under the terms of the GNU General Public License
///	as published by the Free Software Foundation; either version 2
///	of the License, or (at your option) any later version.
///
///	This program is distributed in the hope that it will be useful,
///	but WITHOUT ANY WARRANTY; without even the implied warranty of
///	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///	GNU General Public License for more details.
///
///	You should have received a copy of the GNU General Public License
///	along with this program; if not, write to the Free Software
///	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef SYNTHESE_ServicesListService_H__
#define SYNTHESE_ServicesListService_H__

#include "FactorableTemplate.h"
#include "FunctionWithSite.h"
#include "JourneyPattern.hpp"

namespace synthese
{
	namespace calendar
	{
		class CalendarTemplate;
	}

	namespace pt
	{
		class CommercialLine;
		class ScheduledService;
		class SchedulesBasedService;
		class StopArea;
	}

	namespace pt_operation
	{
		class OperationUnit;
		class VehicleService;
	}

	namespace pt_website
	{
		//////////////////////////////////////////////////////////////////////////
		///	35.15 Function : ServicesListService.
		/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Services_List
		//////////////////////////////////////////////////////////////////////////
		///	@ingroup m35Functions refFunctions
		///	@author Hugues Romain
		///	@date 2012
		/// @since 3.4.0
		class ServicesListService:
			public util::FactorableTemplate<server::Function, ServicesListService>
		{
		public:
			static const std::string PARAMETER_WAYBACK;
			static const std::string PARAMETER_DISPLAY_DATE;
			static const std::string PARAMETER_BASE_CALENDAR_ID;
			static const std::string PARAMETER_DATE_FILTER;
			static const std::string PARAMETER_MIN_DEPARTURE_TIME;
			static const std::string PARAMETER_MAX_DEPARTURE_TIME;
			static const std::string PARAMETER_DEPARTURE_PLACE;
			static const std::string PARAMETER_MIN_DELAY_BETWEEN_DEPARTURE_AND_CALL; // TODO replace it by a per-line value
			
			static const std::string DATA_ID;
			static const std::string DATA_DEPARTURE_SCHEDULE;
			static const std::string DATA_DEPARTURE_PLACE_NAME;
			static const std::string DATA_ARRIVAL_SCHEDULE;
			static const std::string DATA_ARRIVAL_PLACE_NAME;
			static const std::string DATA_RUNS_AT_DATE;
			static const std::string DATA_SERVICE;
			static const std::string ATTR_NUMBER;
			static const std::string ATTR_PATH_ID;
			static const std::string TAG_SERVICES;
			static const std::string ATTR_IS_RESERVABLE;

			static const std::string TAG_CALENDAR;
			static const std::string TAG_RESERVATION_DELIVERY_TIME;
			static const std::string ATTR_TIME;

		protected:
			//! \name Page parameters
			//@{
				boost::logic::tribool _wayBack;
				boost::shared_ptr<const pt::ScheduledService> _service;
				boost::shared_ptr<const pt::CommercialLine> _line;
				boost::shared_ptr<const pt_operation::OperationUnit> _operationUnit;
				boost::shared_ptr<const pt_operation::VehicleService> _vehicleService;
				boost::gregorian::date _displayDate;
				boost::optional<calendar::Calendar> _baseCalendar;
				boost::optional<boost::posix_time::time_duration> _minDepartureTime;
				boost::optional<boost::posix_time::time_duration> _maxDepartureTime;
				boost::optional<util::RegistryKeyType> _departurePlaceId;
				boost::posix_time::time_duration _minDelayBetweenDepartureAndCall;
			//@}


			//////////////////////////////////////////////////////////////////////////
			/// Conversion from attributes to generic parameter maps.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Services_List#Request
			//////////////////////////////////////////////////////////////////////////
			///	@return Generated parameters map
			/// @author Hugues Romain
			/// @date 2012
			/// @since 3.4.0
			util::ParametersMap _getParametersMap() const;



			//////////////////////////////////////////////////////////////////////////
			/// Conversion from generic parameters map to attributes.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Services_List#Request
			//////////////////////////////////////////////////////////////////////////
			///	@param map Parameters map to interpret
			/// @author Hugues Romain
			/// @date 2012
			/// @since 3.4.0
			virtual void _setFromParametersMap(
				const util::ParametersMap& map
			);

			void _addServiceIfCompliant(
				graph::ServiceSet& result,
				const pt::SchedulesBasedService& service
			) const;


			void _addServices(
				graph::ServiceSet& result,
				const pt::CommercialLine& line
			) const;

		public:
			ServicesListService();


			//! @name Setters
			//@{
			//@}



			//////////////////////////////////////////////////////////////////////////
			/// Display of the content generated by the function.
			/// @param stream Stream to display the content on.
			/// @param request the current request
			/// @author Hugues Romain
			/// @date 2012
			virtual util::ParametersMap run(std::ostream& stream, const server::Request& request) const;



			//////////////////////////////////////////////////////////////////////////
			/// Gets if the function can be run according to the user of the session.
			/// @param session the current session
			/// @return true if the function can be run
			/// @author Hugues Romain
			/// @date 2012
			virtual bool isAuthorized(const server::Session* session) const;



			//////////////////////////////////////////////////////////////////////////
			/// Gets the Mime type of the content generated by the function.
			/// @return the Mime type of the content generated by the function
			/// @author Hugues Romain
			/// @date 2012
			virtual std::string getOutputMimeType() const;
		};
}	}

#endif // SYNTHESE_ServicesListService_H__


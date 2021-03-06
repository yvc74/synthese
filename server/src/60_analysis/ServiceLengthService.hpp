
//////////////////////////////////////////////////////////////////////////////////////////
///	ServiceLengthService class header.
///	@file ServiceLengthService.hpp
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

#ifndef SYNTHESE_ServiceLengthService_H__
#define SYNTHESE_ServiceLengthService_H__

#include "FactorableTemplate.h"
#include "Function.h"

#include "GraphTypes.h"

namespace synthese
{
	namespace pt
	{
		class ScheduledService;
		class StopArea;
	}

	namespace resa
	{
		class Reservation;
	}

	namespace analysis
	{
		//////////////////////////////////////////////////////////////////////////
		///	60.15 Function : ServiceLengthService.
		/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Service_length
		//////////////////////////////////////////////////////////////////////////
		///	@ingroup m60Functions refFunctions
		///	@author Hugues Romain
		///	@date 2012
		/// @since 3.4.0
		class ServiceLengthService:
			public util::FactorableTemplate<server::Function,ServiceLengthService>
		{
		public:
			static const std::string PARAMETER_DATE;
			
		protected:
			static const std::string ATTR_PLANNED_DISTANCE;
			static const std::string ATTR_REAL_DISTANCE;
			static const std::string ATTR_EMPTY_DISTANCE;

			static const std::string TAG_LEG;
			static const std::string ATTR_DEPARTURE_STOP_ID;
			static const std::string ATTR_DEPARTURE_STOP_NAME;
			static const std::string ATTR_ARRIVAL_STOP_ID;
			static const std::string ATTR_ARRIVAL_STOP_NAME;
			static const std::string ATTR_LENGTH;
			static const std::string ATTR_PASSENGERS;

			//! \name Page parameters
			//@{
				boost::shared_ptr<const pt::ScheduledService> _service;
				boost::gregorian::date _date;
			//@}
			
			struct Leg
			{
				const pt::StopArea* startStop;
				const pt::StopArea* endStop;
				size_t passengers;
				graph::MetricOffset distance;
			};
			typedef std::vector<Leg> Legs;

			typedef std::map<
					const resa::Reservation*,
					bool // True = departure
				> ReservationPoint;

			typedef std::map<const pt::StopArea*, ReservationPoint> ReservationPoints;

			static void AddReservation(
				ReservationPoints& points,
				const resa::Reservation& resa,
				bool departure
			);

			static size_t GetPassengers(
				const ReservationPoints& points,
				const pt::StopArea& stopArea,
				bool departure
			);
			
			//////////////////////////////////////////////////////////////////////////
			/// Conversion from attributes to generic parameter maps.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Service_length#Request
			//////////////////////////////////////////////////////////////////////////
			///	@return Generated parameters map
			/// @author Hugues Romain
			/// @date 2012
			/// @since 3.4.0
			util::ParametersMap _getParametersMap() const;
			
			
			
			//////////////////////////////////////////////////////////////////////////
			/// Conversion from generic parameters map to attributes.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Service_length#Request
			//////////////////////////////////////////////////////////////////////////
			///	@param map Parameters map to interpret
			/// @author Hugues Romain
			/// @date 2012
			/// @since 3.4.0
			virtual void _setFromParametersMap(
				const util::ParametersMap& map
			);
			
			
		public:
			//! @name Setters
			//@{
			//	void setObject(boost::shared_ptr<const Object> value) { _object = value; }
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

#endif // SYNTHESE_ServiceLengthService_H__



//////////////////////////////////////////////////////////////////////////////////////////
/// GetCompositionsService class header.
///	@file GetCompositionsService.hpp
///	@author Hugues Romain
///	@date 2011
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

#ifndef SYNTHESE_GetCompositionsService_H__
#define SYNTHESE_GetCompositionsService_H__

#include "FactorableTemplate.h"
#include "Function.h"

#include <boost/date_time/gregorian/greg_date.hpp>

namespace synthese
{
	namespace pt
	{
		class StopPoint;
		class ScheduledService;
	}

	namespace cms
	{
		class Webpage;
	}

	namespace vehicle
	{
		class Composition;
		class Vehicle;

		//////////////////////////////////////////////////////////////////////////
		///	37.15 Function : GetCompositionsService.
		/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Compositions list
		//////////////////////////////////////////////////////////////////////////
		///	@ingroup m37Functions refFunctions
		///	@author Hugues Romain
		///	@date 2011
		/// @since 3.2.1
		class GetCompositionsService:
			public util::FactorableTemplate<server::Function,GetCompositionsService>
		{
		public:
			static const std::string PARAMETER_SERVICE_ID;
			static const std::string PARAMETER_DATE;
			static const std::string PARAMETER_COMPOSITION_PAGE_ID;
			static const std::string PARAMETER_VEHICLE_PAGE_ID;
			static const std::string PARAMETER_QUAY_PAGE_ID;

			static const std::string DATA_VEHICLES;
			static const std::string DATA_QUAYS;
			static const std::string DATA_RANK;
			static const std::string DATA_NUMBER;
			static const std::string DATA_NAME;
			static const std::string DATA_PICTURE;
			static const std::string DATA_SEATS;
			static const std::string DATA_COMPOSITION_ID;
			static const std::string DATA_VEHICLE_ID;
			static const std::string DATA_QUAY_ID;

		protected:
			//! \name Page parameters
			//@{
				boost::gregorian::date _date;
				boost::shared_ptr<const pt::ScheduledService> _service;
				boost::shared_ptr<const cms::Webpage> _compositionPage;
				boost::shared_ptr<const cms::Webpage> _vehiclePage;
				boost::shared_ptr<const cms::Webpage> _quayPage;
			//@}


			//////////////////////////////////////////////////////////////////////////
			/// Conversion from attributes to generic parameter maps.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Compositions list#Request
			//////////////////////////////////////////////////////////////////////////
			///	@return Generated parameters map
			/// @author Hugues Romain
			/// @date 2011
			/// @since 3.2.1
			util::ParametersMap _getParametersMap() const;



			//////////////////////////////////////////////////////////////////////////
			/// Conversion from generic parameters map to attributes.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Compositions list#Request
			//////////////////////////////////////////////////////////////////////////
			///	@param map Parameters map to interpret
			/// @author Hugues Romain
			/// @date 2011
			/// @since 3.2.1
			virtual void _setFromParametersMap(
				const util::ParametersMap& map
			);


			void displayComposition(
				std::ostream& stream,
				const server::Request& request,
				const Composition& composition,
				std::size_t rank
			) const;

			void displayVehicle(
				std::ostream& stream,
				const server::Request& request,
				const Vehicle& vehicle,
				const std::string& number,
				std::size_t rank
			) const;

			void displayQuay(
				std::ostream& stream,
				const server::Request& request,
				const pt::StopPoint& quay,
				std::size_t rank
			) const;

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
			/// @date 2011
			virtual util::ParametersMap run(std::ostream& stream, const server::Request& request) const;



			//////////////////////////////////////////////////////////////////////////
			/// Gets if the function can be run according to the user of the session.
			/// @param session the current session
			/// @return true if the function can be run
			/// @author Hugues Romain
			/// @date 2011
			virtual bool isAuthorized(const server::Session* session) const;



			//////////////////////////////////////////////////////////////////////////
			/// Gets the Mime type of the content generated by the function.
			/// @return the Mime type of the content generated by the function
			/// @author Hugues Romain
			/// @date 2011
			virtual std::string getOutputMimeType() const;
		};
	}
}

#endif // SYNTHESE_GetCompositionsService_H__

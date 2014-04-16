
//////////////////////////////////////////////////////////////////////////////////////////
///	VehicleServicesListService class header.
///	@file VehicleServicesListService.hpp
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

#ifndef SYNTHESE_VehicleServicesListService_H__
#define SYNTHESE_VehicleServicesListService_H__

#include "FactorableTemplate.h"
#include "FunctionWithSite.h"

#include <boost/date_time/date.hpp>

namespace synthese
{
	namespace pt
	{
		class ScheduledService;
	}

	namespace pt_operation
	{
		class OperationUnit;
		class VehicleService;

		//////////////////////////////////////////////////////////////////////////
		///	37.15 Function : VehicleServicesListService.
		/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Vehicle services list
		//////////////////////////////////////////////////////////////////////////
		///	@ingroup m37Functions refFunctions
		///	@author Hugues Romain
		///	@date 2012
		/// @since 3.4.0
		class VehicleServicesListService:
			public util::FactorableTemplate<cms::FunctionWithSite<false>,VehicleServicesListService>
		{
		public:
			static const std::string PARAMETER_PAGE;
			static const std::string PARAMETER_WITH_DETAIL;
			static const std::string PARAMETER_SERVICE; // id of a scheduled service

			static const std::string TAG_VEHICLE_SERVICE;
			static const std::string TAG_VEHICLE_SERVICES;

		protected:
			//! \name Page parameters
			//@{
				Date::Type _date;
				Name::Type _name;
				const cms::Webpage* _page;
				boost::optional<const OperationUnit&> _operationUnit;
				boost::shared_ptr<const VehicleService> _service;
				const pt::ScheduledService* _scheduledService;
				bool _withDetail;
			//@}


			//////////////////////////////////////////////////////////////////////////
			/// Conversion from attributes to generic parameter maps.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Vehicle services list#Request
			//////////////////////////////////////////////////////////////////////////
			///	@return Generated parameters map
			/// @author Hugues Romain
			/// @date 2012
			/// @since 3.4.0
			util::ParametersMap _getParametersMap() const;



			//////////////////////////////////////////////////////////////////////////
			/// Conversion from generic parameters map to attributes.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Vehicle services list#Request
			//////////////////////////////////////////////////////////////////////////
			///	@param map Parameters map to interpret
			/// @author Hugues Romain
			/// @date 2012
			/// @since 3.4.0
			virtual void _setFromParametersMap(
				const util::ParametersMap& map
			);


			void _exportService(
				const VehicleService& vs,
				util::ParametersMap& map
			) const;
				

		public:
			VehicleServicesListService();


			//! @name Setters
			//@{
				void setDate(boost::gregorian::date value) { _date = value; }
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

#endif // SYNTHESE_VehicleServicesListService_H__

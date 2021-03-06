
//////////////////////////////////////////////////////////////////////////////////////////
/// ServiceDensityService class header.
/// @file ServiceDensityService.hpp
/// @author Xavier Raffin
/// @date 2013
///
/// This file belongs to the SYNTHESE project (public transportation specialized software)
/// Copyright (C) 2002 Hugues Romain - RCSmobility <contact@rcsmobility.com>
///
/// This program is free software; you can redistribute it and/or
/// modify it under the terms of the GNU General Public License
/// as published by the Free Software Foundation; either version 2
/// of the License, or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef SYNTHESE_PTServiceDensityService_H__
#define SYNTHESE_PTServiceDensityService_H__

#include "UtilTypes.h"
#include "FactorableTemplate.h"
#include "Function.h"
#include "AccessParameters.h"

#include <boost/optional.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <set>

namespace geos
{
	namespace geom
	{
		class Point;
	}
}

namespace synthese
{
	class CoordinatesSystem;

	namespace cms
	{
		class Webpage;
	}

	namespace pt
	{
		class StopPoint;
	}

	namespace analysis
	{
		//////////////////////////////////////////////////////////////////////////
		/// 60.15 Function : DensityService calculation Service.
		/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Density_Service
		//////////////////////////////////////////////////////////////////////////
		/// @ingroup m35Functions refFunctions
		/// @author Xavier Raffin, Xavier Raffin
		/// @date 2013
		/// @since 3.1.18
		class ServiceDensityService:
			public util::FactorableTemplate<server::Function,ServiceDensityService>
		{
		public:
			static const std::string PARAMETER_START_DATE;
			static const std::string PARAMETER_END_DATE;
			static const std::string PARAMETER_SRID;
			static const std::string PARAMETER_DENSITY_AREA_CENTER_POINT;
			static const std::string PARAMETER_PAGE_ID;
			static const std::string PARAMETER_SERVICE_NUMBER;
			static const std::string PARAMETER_ROLLING_STOCK_LIST;
			static const std::string PARAMETER_NETWORK_LIST;
			static const std::string PARAMETER_DISPLAY_SERVICE_REACHED;
			static const std::string PARAMETER_ONLY_RELAY_PARKS;
		protected:
			static const std::string MAX_DISTANCE_TO_CENTER_POINT;
			static const std::string SERVICE_NUMBER_REACHED;
			static const std::string IS_SERVICE_NUMBER_REACHED;

			static const std::string DATA_SERVICE;
			static const std::string DATA_SERVICE_ID;
			static const std::string DATA_SERVICE_DEPARTURE_SCHEDULE;
			static const std::string DATA_COMMERCIAL_LINE_NAME;
			static const std::string DATA_LINE_NAME;
			static const std::string DATA_SERVICE_NETWORK;
			static const std::string DATA_SERVICE_ROLLING_STOCK;
			static const std::string DATA_SERVICE_RANK;

			static const std::string DATA_STOP;
			static const std::string DATA_STOP_POINT_ID;
			static const std::string DATA_STOP_POINT_NAME;
			static const std::string DATA_STOP_DISTANCE;
			static const std::string DATA_STOP_DATASOURCE;
			static const std::string DATA_STOP_RANK; 

			//! \name Page parameters
			//@{
				boost::posix_time::ptime _startDate;
				boost::posix_time::ptime _endDate;
				boost::shared_ptr<const cms::Webpage> _page;
				const CoordinatesSystem* _coordinatesSystem;
				boost::shared_ptr<geos::geom::Point> _centerPoint;
				std::size_t _serviceNumberToReach;
				graph::AccessParameters _accessParameters;
				bool _displayServices;
				bool _onlyRelayParks;
			//@}


			//////////////////////////////////////////////////////////////////////////
			/// Conversion from attributes to generic parameter maps.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Density_Service#Request
			//////////////////////////////////////////////////////////////////////////
			/// @return Generated parameters map
			/// @author Xavier Raffin
			/// @date 2013
			util::ParametersMap _getParametersMap() const;



			//////////////////////////////////////////////////////////////////////////
			/// Conversion from generic parameters map to attributes.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Density_Service#Request
			//////////////////////////////////////////////////////////////////////////
			/// @param map Parameters map to interpret
			/// @author Xavier Raffin
			/// @date 2013
			virtual void _setFromParametersMap(
				const util::ParametersMap& map
			);



		public:

			//////////////////////////////////////////////////////////////////////////
			/// Display of the content generated by the function.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Density_Service#Response
			//////////////////////////////////////////////////////////////////////////
			/// @param stream Stream to display the content on.
			/// @param request the current request
			/// @author Xavier Raffin
			/// @date 2013
			virtual util::ParametersMap run(std::ostream& stream, const server::Request& request) const;


			//////////////////////////////////////////////////////////////////////////
			/// Gets if the function can be run according to the user of the session.
			/// @param session the current session
			/// @return true if the function can be run
			/// @author Xavier Raffin
			/// @date 2013
			virtual bool isAuthorized(const server::Session* session) const;


			//////////////////////////////////////////////////////////////////////////
			/// Gets the Mime type of the content generated by the function.
			/// @return the Mime type of the content generated by the function
			/// @author Xavier Raffin
			/// @date 2013
			virtual std::string getOutputMimeType() const;


		private:

			util::RegistryKeyType _key;
			
			class SortableStopPoint
			{
			private:
				const pt::StopPoint * _sp;
				int _distanceToCenter;
				std::string _opCode;
			public:
				SortableStopPoint(const pt::StopPoint * sp, int distanceToCenter);
				bool operator<(SortableStopPoint const &otherStopPoint) const;
				const pt::StopPoint * getStopPoint() const;
				int getDistanceToCenter() const;
				std::string getOpCode() const;
			};

			typedef std::set<SortableStopPoint> StopPointSetType;

			//////////////////////////////////////////////////////////////////////////
			/// Add the StopPoint to the stopPoint set
			/// @author Xavier Raffin
			/// @date 2013
			void addStop(
				StopPointSetType & stopPointSet,
				const pt::StopPoint & sp,
				const boost::posix_time::ptime & startDateTime,
				const boost::posix_time::ptime & endDateTime
			) const;

			int CalcDistanceToCenter (const pt::StopPoint & stopPoint) const;
		};
	}
}

#endif // SYNTHESE_PTServiceDensityService_H__

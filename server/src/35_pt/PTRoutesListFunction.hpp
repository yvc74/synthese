
//////////////////////////////////////////////////////////////////////////////////////////
/// PTRoutesListFunction class header.
///	@file PTRoutesListFunction.hpp
///	@author Hugues Romain
///	@date 2010
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

#ifndef SYNTHESE_PTRoutesListFunction_H__
#define SYNTHESE_PTRoutesListFunction_H__

#include "FactorableTemplate.h"
#include "Function.h"

#include <boost/optional.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

namespace synthese
{
	namespace cms
	{
		class Webpage;
	}

	namespace calendar
	{
		class CalendarTemplate;
	}

	namespace pt
	{
		class StopPoint;
		class CommercialLine;

		//////////////////////////////////////////////////////////////////////////
		/// 35.15 Function : Lists the routes of a line.
		/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Routes_list
		//////////////////////////////////////////////////////////////////////////
		///	@author Hugues Romain
		///	@date 2010
		///	@since 3.1.16
		///	@ingroup m35Functions refFunctions
		class PTRoutesListFunction:
			public util::FactorableTemplate<server::Function,PTRoutesListFunction>
		{
		public:
			static const std::string PARAMETER_MERGE_SAME_ROUTES;
			static const std::string PARAMETER_MERGE_INCLUDING_ROUTES;
			static const std::string PARAMETER_PAGE_ID;
			static const std::string PARAMETER_DATE;
			static const std::string PARAMETER_CALENDAR_ID;
			static const std::string PARAMETER_FILTER_MAIN_ROUTES;
			static const std::string PARAMETER_STOP_ID;

			static const std::string DATA_NAME;
			static const std::string DATA_LENGTH;
			static const std::string DATA_STOPS_NUMBER;
			static const std::string DATA_DIRECTION;
			static const std::string DATA_ORIGIN_CITY_NAME;
			static const std::string DATA_ORIGIN_STOP_NAME;
			static const std::string DATA_DESTINATION_CITY_NAME;
			static const std::string DATA_DESTINATION_STOP_NAME;
			static const std::string DATA_RANK;
			static const std::string DATA_RANK_IS_ODD;
			static const std::string DATA_IS_MAIN;
			static const std::string DATA_ROUTES;
			static const std::string DATA_WAYBACK;


		protected:
			//! \name Page parameters
			//@{
				boost::shared_ptr<const pt::CommercialLine> _line;
				boost::shared_ptr<const cms::Webpage> _page;
				boost::optional<boost::posix_time::ptime> _date;
				boost::shared_ptr<const calendar::CalendarTemplate> _calendar;
				boost::shared_ptr<const pt::StopPoint> _stop;
				bool _mergeSameRoutes;
				bool _mergeIncludingRoutes;
				boost::optional<bool> _filterMainRoutes;
			//@}


			//////////////////////////////////////////////////////////////////////////
			/// Conversion from attributes to generic parameter maps.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Routes_list
			//////////////////////////////////////////////////////////////////////////
			///	@return Generated parameters map
			/// @author Hugues Romain
			/// @date 2010
			util::ParametersMap _getParametersMap() const;



			//////////////////////////////////////////////////////////////////////////
			/// Conversion from generic parameters map to attributes.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Routes_list
			//////////////////////////////////////////////////////////////////////////
			///	@param map Parameters map to interpret
			/// @author Hugues Romain
			/// @date 2010
			virtual void _setFromParametersMap(
				const util::ParametersMap& map
			);


		public:
			//! @name Setters
			//@{
				void setLine(boost::shared_ptr<const pt::CommercialLine> value) { _line = value; }
			//@}



			//////////////////////////////////////////////////////////////////////////
			/// Display of the content generated by the function.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Routes_list
			//////////////////////////////////////////////////////////////////////////
			/// @param stream Stream to display the content on.
			/// @param request the current request
			/// @author Hugues Romain
			/// @date 2010
			virtual util::ParametersMap run(std::ostream& stream, const server::Request& request) const;



			//////////////////////////////////////////////////////////////////////////
			/// Gets if the function can be run according to the user of the session.
			/// @param session the current session
			/// @return true if the function can be run
			/// @author Hugues Romain
			/// @date 2010
			virtual bool isAuthorized(const server::Session* session) const;



			//////////////////////////////////////////////////////////////////////////
			/// Gets the Mime type of the content generated by the function.
			/// @return the Mime type of the content generated by the function
			/// @author Hugues Romain
			/// @date 2010
			virtual std::string getOutputMimeType() const;
		};
	}
}

#endif // SYNTHESE_PTRoutesListFunction_H__

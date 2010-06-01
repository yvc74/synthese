
//////////////////////////////////////////////////////////////////////////
/// LineStopUpdateAction class header.
///	@file LineStopUpdateAction.hpp
///	@author Hugues Romain
///	@date 2010
///
///	This file belongs to the SYNTHESE project (public transportation specialized software)
///	Copyright (C) 2002 Hugues Romain - RCS <contact@reseaux-conseil.com>
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

#ifndef SYNTHESE_LineStopUpdateAction_H__
#define SYNTHESE_LineStopUpdateAction_H__

#include "Action.h"
#include "FactorableTemplate.h"

namespace synthese
{
	namespace pt
	{
		class LineStop;
		class PhysicalStop;

		//////////////////////////////////////////////////////////////////////////
		/// 35.15 Action : LineStopUpdateAction.
		/// @ingroup m35Actions refActions
		///	@author Hugues Romain
		///	@date 2010
		/// @since 3.1.18
		//////////////////////////////////////////////////////////////////////////
		/// Key : LineStopUpdateAction
		///
		/// Parameters :
		///	<ul>
		///		<li>actionParamid : id of the object to update</li>
		///		<li>actionParamps (optional) : id of the physical stop to use. No change if undefined</li>
		///		<li>actionParamad (optional) : departure is allowed. No change if undefined</li>
		///		<li>actionParamaa (optional) : arrival is allowed. No change if undefined</li>
		///	</ul>
		class LineStopUpdateAction:
			public util::FactorableTemplate<server::Action, LineStopUpdateAction>
		{
		public:
			static const std::string PARAMETER_LINE_STOP_ID;
			static const std::string PARAMETER_PHYSICAL_STOP_ID;
			static const std::string PARAMETER_ALLOWED_DEPARTURE;
			static const std::string PARAMETER_ALLOWED_ARRIVAL;

		private:
			boost::shared_ptr<LineStop> _lineStop;
			boost::shared_ptr<PhysicalStop> _physicalStop;
			boost::optional<bool> _allowedDeparture;
			boost::optional<bool> _allowedArrival;

		protected:
			//////////////////////////////////////////////////////////////////////////
			/// Generates a generic parameters map from the action parameters.
			/// @return The generated parameters map
			server::ParametersMap getParametersMap() const;



			//////////////////////////////////////////////////////////////////////////
			/// Reads the parameters of the action on a generic parameters map.
			/// @param map Parameters map to interpret
			/// @exception ActionException Occurs when some parameters are missing or incorrect.
			void _setFromParametersMap(const server::ParametersMap& map);

		public:
			//////////////////////////////////////////////////////////////////////////
			/// The action execution code.
			/// @param request the request which has launched the action
			void run(server::Request& request);
			


			//////////////////////////////////////////////////////////////////////////
			/// Tests if the action can be launched in the current session.
			/// @param session the current session
			/// @return true if the action can be launched in the current session
			virtual bool isAuthorized(const server::Session* session) const;



			//! @name Setters
			//@{
				void setLineStop(boost::shared_ptr<LineStop> value) { _lineStop = value; }
			//@}
		};
	}
}

#endif // SYNTHESE_LineStopUpdateAction_H__

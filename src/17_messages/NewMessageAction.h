////////////////////////////////////////////////////////////////////////////////
/// NewMessageAction class header.
///	@file NewMessageAction.h
///	@author Hugues Romain
///
///	This file belongs to the SYNTHESE project (public transportation specialized
///	software)
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
///	along with this program; if not, write to the Free Software Foundation,
///	Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
////////////////////////////////////////////////////////////////////////////////

#ifndef SYNTHESE_NewMessageAction_H__
#define SYNTHESE_NewMessageAction_H__

#include "Action.h"
#include "FactorableTemplate.h"
#include "UId.h"

namespace synthese
{
	namespace messages
	{
		class Scenario;
		class SentScenario;
		class ScenarioTemplate;
		class Alarm;

		/** Alarm creation action class.
			@ingroup m17Actions refActions
		*/
		class NewMessageAction : public util::FactorableTemplate<server::Action, NewMessageAction>
		{
		public:
			static const std::string PARAMETER_IS_TEMPLATE;
			static const std::string PARAMETER_SCENARIO_ID;
			static const std::string PARAMETER_MESSAGE_TEMPLATE;

		private:
			boost::shared_ptr<const SentScenario>		_sentScenario;
			boost::shared_ptr<const ScenarioTemplate>	_scenarioTemplate;
			bool										_isTemplate;
			boost::shared_ptr<const Alarm>				_messageTemplate;

		protected:
			/** Conversion from attributes to generic parameter maps.
			*/
			server::ParametersMap getParametersMap() const;

			/** Conversion from generic parameters map to attributes.
				Removes the used parameters from the map.
				@exception ActionException Occurs when some parameters are missing or incorrect.
			*/
			void _setFromParametersMap(const server::ParametersMap& map) throw(server::ActionException);

		public:
			void setScenarioId(uid scenarioId);
			void setIsTemplate(bool value);

			/** Action to run, defined by each subclass.
			*/
			void run() throw(server::ActionException);

			virtual bool _isAuthorized() const;
		};
	}
}

#endif // SYNTHESE_NewMessageAction_H__

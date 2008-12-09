
/** AddUserFavoriteJourneyAction class implementation.
	@file AddUserFavoriteJourneyAction.cpp
	@author Hugues Romain
	@date 2007

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

#include "UserFavoriteJourney.h"
#include "UserFavoriteJourneyTableSync.h"

#include "ActionException.h"
#include "ParametersMap.h"

#include "UserTableSync.h"
#include "User.h"

#include "ObjectNotFoundException.h"

#include "AddUserFavoriteJourneyAction.h"

using namespace std;

namespace synthese
{
	using namespace server;
	using namespace security;
	using namespace util;
	using namespace db;
	
	namespace util
	{
		template<> const string FactorableTemplate<Action, routeplanner::AddUserFavoriteJourneyAction>::FACTORY_KEY("add_favorite_journey");
	}

	namespace routeplanner
	{
		const string AddUserFavoriteJourneyAction::PARAMETER_USER_ID = Action_PARAMETER_PREFIX + "uid";
		const string AddUserFavoriteJourneyAction::PARAMETER_ORIGIN_CITY_NAME = Action_PARAMETER_PREFIX + "ocn";
		const string AddUserFavoriteJourneyAction::PARAMETER_ORIGIN_PLACE_NAME = Action_PARAMETER_PREFIX + "opn";
		const string AddUserFavoriteJourneyAction::PARAMETER_DESTINATION_CITY_NAME = Action_PARAMETER_PREFIX + "dcn";
		const string AddUserFavoriteJourneyAction::PARAMETER_DESTINATION_PLACE_NAME = Action_PARAMETER_PREFIX + "dpn";
		
		
		AddUserFavoriteJourneyAction::AddUserFavoriteJourneyAction()
			: util::FactorableTemplate<Action, AddUserFavoriteJourneyAction>()
		{
		}
		
		
		
		ParametersMap AddUserFavoriteJourneyAction::getParametersMap() const
		{
			ParametersMap map;
			//map.insert(make_pair(PARAMETER_xxx, _xxx));
			return map;
		}
		
		
		
		void AddUserFavoriteJourneyAction::_setFromParametersMap(const ParametersMap& map)
		{
			uid id(map.getUid(PARAMETER_USER_ID, true, FACTORY_KEY));
			try
			{
				_user = UserTableSync::Get(id);
			}
			catch (ObjectNotFoundException<User>& e)
			{
				throw ActionException(e.getMessage());
			}
			_originCityName = map.getString(PARAMETER_ORIGIN_CITY_NAME, true, FACTORY_KEY);
			_originPlaceName = map.getString(PARAMETER_ORIGIN_PLACE_NAME, true, FACTORY_KEY);
			_destinationCityName = map.getString(PARAMETER_DESTINATION_CITY_NAME, true, FACTORY_KEY);
			_destinationPlaceName = map.getString(PARAMETER_DESTINATION_PLACE_NAME, true, FACTORY_KEY);
		}
		
		
		
		void AddUserFavoriteJourneyAction::run()
		{
			UserFavoriteJourney f;
			f.setUser(_user.get());
			f.setOriginPlaceName(_originPlaceName);
			f.setOriginCityName(_originCityName);
			f.setDestinationCityName(_destinationCityName);
			f.setDestinationPlaceName(_destinationPlaceName);
			UserFavoriteJourneyTableSync::Save(&f);
		}
	}
}

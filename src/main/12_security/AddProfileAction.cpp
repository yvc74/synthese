
/** AddProfileAction class implementation.
	@file AddProfileAction.cpp

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

#include "01_util/Conversion.h"

#include "12_security/Profile.h"
#include "12_security/ProfileTableSync.h"
#include "12_security/AddProfileAction.h"
#include "12_security/SecurityModule.h"

#include "30_server/ActionException.h"

using namespace std;

namespace synthese
{
	using namespace server;
	using namespace util;
	
	namespace security
	{
		const string AddProfileAction::PARAMETER_NAME = Action_PARAMETER_PREFIX + "apan";
		const string AddProfileAction::PARAMETER_TEMPLATE_ID = Action_PARAMETER_PREFIX + "apat";


		Request::ParametersMap AddProfileAction::getParametersMap() const
		{
			Request::ParametersMap map;
			map.insert(make_pair(PARAMETER_NAME, _name));
			if (_templateProfile != NULL)
				map.insert(make_pair(PARAMETER_TEMPLATE_ID, Conversion::ToString(_templateProfile->getKey())));
			return map;
		}

		void AddProfileAction::setFromParametersMap(Request::ParametersMap& map)
		{
			Request::ParametersMap::iterator it;

			it = map.find(PARAMETER_NAME);
			if (it == map.end())
				throw ActionException("Name not specified");
			_name = it->second;
			map.erase(it);

			it = map.find(PARAMETER_TEMPLATE_ID);
			if (it != map.end())
			{
				try
				{
					_templateProfile = SecurityModule::getProfiles().get(Conversion::ToLongLong(it->second));
				}
				catch (Profile::RegistryKeyException e)
				{
					throw ActionException("Template profile not found");
				}
				map.erase(it);
			}
		}

		void AddProfileAction::run()
		{
			Profile* profile = new Profile;
			profile->setName(_name);
			if (_templateProfile)
			{
				profile->setParent(_templateProfile->getKey());
				profile->setRights(_templateProfile->getRightsString());
			}
			ProfileTableSync::save(profile);
			delete profile;
		}

		AddProfileAction::AddProfileAction()
			: Action()
			, _templateProfile(NULL)
		{
		}
	}
}
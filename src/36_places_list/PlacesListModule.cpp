
/** PlacesListModule class implementation.
	@file PlacesListModule.cpp

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

#include "PlacesListModule.h"
#include "GraphConstants.h"

using namespace std;

namespace synthese
{
	using namespace graph;
	using namespace server;
	using namespace transportwebsite;
	
	template<> const std::string util::FactorableTemplate<ModuleClass,PlacesListModule>::FACTORY_KEY("36_places_list");
	
	namespace server
	{
		template<> const string ModuleClassTemplate<PlacesListModule>::NAME("Site web transport public");
		
		template<> void ModuleClassTemplate<PlacesListModule>::PreInit()
		{
		}
		
		template<> void ModuleClassTemplate<PlacesListModule>::Init()
		{
		}
		
		template<> void ModuleClassTemplate<PlacesListModule>::End()
		{
		}
	}

	namespace transportwebsite
	{
		PlacesListModule::UserClassNames PlacesListModule::GetAccessibilityNames()
		{
			UserClassNames result;
			result.push_back(make_pair(USER_PEDESTRIAN, "Pi�ton"));
			result.push_back(make_pair(USER_HANDICAPPED, "PMR"));
			result.push_back(make_pair(USER_BIKE, "V�lo"));
			return result;
		}
	}
}

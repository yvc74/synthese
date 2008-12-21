
/** NonPredefinedInterfacePage class implementation.
	@file NonPredefinedInterfacePage.cpp

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

#include "NonPredefinedInterfacePage.h"

using namespace std;

namespace synthese
{
	template<> const std::string util::FactorableTemplate<interfaces::InterfacePage,interfaces::NonPredefinedInterfacePage>::FACTORY_KEY("*");

	namespace interfaces
	{



		NonPredefinedInterfacePage::NonPredefinedInterfacePage()
			: Registrable(UNKNOWN_VALUE)
		{

		}
	}
}

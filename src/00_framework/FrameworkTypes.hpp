
/** FrameworkTypes class header.
	@file FrameworkTypes.hpp

	This file belongs to the SYNTHESE project (public transportation specialized software)
	Copyright (C) 2002 Hugues Romain - RCSmobility <contact@rcsmobility.com>

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

#ifndef SYNTHESE__FrameworkTypes_hpp__
#define SYNTHESE__FrameworkTypes_hpp__

#include "UtilTypes.h" // TODO : merge this file in the current one

#include <vector>

namespace synthese
{
	typedef enum
	{
		FORMAT_INTERNAL,
		FORMAT_SQL,
		FORMAT_XML
	} SerializationFormat;

	typedef std::vector<util::RegistryKeyType> LinkedObjectsIds;

}

#endif // SYNTHESE__FrameworkTypes_hpp__


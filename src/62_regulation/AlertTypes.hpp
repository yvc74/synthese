
/** AlertTypes class header.
	@file AlertTypes.hpp

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

#ifndef SYNTHESE_alert_AlertTypes_hpp__
#define SYNTHESE_alert_AlertTypes_hpp__


namespace synthese
{
	namespace regulation
	{

		typedef enum {
			ALERT_TYPE_UNKNOWN = -1,
			ALERT_TYPE_CALLBACKREQUEST
		} AlertType;

}	}

#endif // SYNTHESE_alert_AlertTypes_hpp__

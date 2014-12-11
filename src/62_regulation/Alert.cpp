﻿/** Alert class implementation.
	@file Alert.cpp

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

#include "Alert.hpp"


using namespace boost;
using namespace std;
using namespace boost::posix_time;

namespace synthese
{

    using namespace regulation;
	using namespace util;

	CLASS_DEFINITION(Alert, "t120_alerts", 120)
    /*
	FIELD_DEFINITION_OF_TYPE(Service, "service_id", SQL_INTEGER)
	FIELD_DEFINITION_OF_TYPE(Stop, "stop_id", SQL_INTEGER)
	FIELD_DEFINITION_OF_TYPE(ActivationTime, "activation_time", SQL_DATETIME)
	FIELD_DEFINITION_OF_TYPE(ActivationUser, "activation_user_id", SQL_INTEGER)
	FIELD_DEFINITION_OF_TYPE(CancellationTime, "cancellation_time", SQL_DATETIME)
	FIELD_DEFINITION_OF_TYPE(CancellationUser, "cancellation_user_id", SQL_INTEGER)
    */

	namespace regulation
	{

		Alert::Alert(
			RegistryKeyType id
		):	Registrable(id),
			Object<Alert, AlertSchema>(
				Schema(
					FIELD_VALUE_CONSTRUCTOR(Key, id)
/*					FIELD_DEFAULT_CONSTRUCTOR(Service),
					FIELD_DEFAULT_CONSTRUCTOR(Stop),
					FIELD_DEFAULT_CONSTRUCTOR(Date),
					FIELD_DEFAULT_CONSTRUCTOR(ActivationTime),
					FIELD_DEFAULT_CONSTRUCTOR(CancellationTime),
					FIELD_DEFAULT_CONSTRUCTOR(ActivationUser),
					FIELD_DEFAULT_CONSTRUCTOR(CancellationUser)
*/
                    )
                )
		{}



        /*
		void Alert::addAdditionalParameters(
			util::ParametersMap& map,
			std::string prefix
		) const	{

			// Service detail
			boost::shared_ptr<ParametersMap> serviceMap(new ParametersMap);
			get<Service>()->toParametersMap(*serviceMap, false);
			map.insert(prefix + TAG_SERVICE, serviceMap);

			// Stop detail
			boost::shared_ptr<ParametersMap> stopMap(new ParametersMap);
			get<Stop>()->toParametersMap(*stopMap, false);
			map.insert(prefix + TAG_STOP_POINT, stopMap);
		}
        */


		Alert::~Alert()
		{

		}
}	}

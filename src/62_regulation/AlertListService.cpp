
//////////////////////////////////////////////////////////////////////////////////////////
///	AlertListService class implementation.
///	@file AlertListService.cpp
///	@author mjambert
///	@date 2014
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

#include "AlertListService.hpp"

#include "DBModule.h"
#include "ObjectBase.hpp"
#include "RequestException.h"
#include "Request.h"

using namespace boost;
using namespace std;

namespace synthese
{
	using namespace util;
	using namespace server;
	using namespace security;

	template<>
	const string FactorableTemplate<Function,db::AlertListService>::FACTORY_KEY = "object";
	
	namespace db
	{
		const string AlertListService::PARAMETER_ADDITIONAL_PARAMETERS = "additional_parameters";
		
		const string AlertListService::DATA_OBJECT("object");
		


		ParametersMap AlertListService::_getParametersMap() const
		{
			ParametersMap map;
			return map;
		}



		void AlertListService::_setFromParametersMap(const ParametersMap& map)
		{
			Function::setOutputFormatFromMap(map, string());
			
			// Object
			try
			{
				_object = DBModule::GetObject(
					map.get<RegistryKeyType>(Request::PARAMETER_OBJECT_ID),
					Env::GetOfficialEnv()
				);
			}
			catch(ObjectNotFoundException<ObjectBase>& e)
			{
				throw RequestException("No such object : "+ e.getMessage());
			}

			// Additional parameters
			_additionalParameters = map.getDefault<bool>(PARAMETER_ADDITIONAL_PARAMETERS, false);
		}



		ParametersMap AlertListService::run(
			std::ostream& stream,
			const Request& request
		) const {
			ParametersMap map;
			_object->toParametersMap(map, _additionalParameters);
			
			if (_outputFormat == MimeTypes::JSON)
			{
				map.outputJSON(stream, DATA_OBJECT);
			}
			
			return map;
		}
		
		
		
		bool AlertListService::isAuthorized(
			const Session* session
		) const {
			return true;
		}



		std::string AlertListService::getOutputMimeType() const
		{
			if (_outputFormat == MimeTypes::JSON)
			{
				return "application/json";
			}
			return "text/html";
		}



		AlertListService::AlertListService():
			_additionalParameters(false)
		{}
}	}

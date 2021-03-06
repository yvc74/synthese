
/** GetVehiclesService class implementation.
	@file GetVehiclesService.cpp
	@author Hugues Romain
	@date 2011

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

#include "RequestException.h"
#include "Request.h"
#include "GetVehiclesService.hpp"
#include "Webpage.h"
#include "CommercialLine.h"
#include "VehicleTableSync.hpp"

using namespace boost;
using namespace std;

namespace synthese
{
	using namespace cms;
	using namespace pt;
	using namespace security;
	using namespace server;
	using namespace util;
	using namespace vehicle;
	
	template<>
	const string FactorableTemplate<Function, GetVehiclesService>::FACTORY_KEY("GetVehiclesService");

	namespace vehicle
	{
		const string GetVehiclesService::PARAMETER_VEHICLE_PAGE_ID("vp");
		const string GetVehiclesService::PARAMETER_LINE_ID("li");

		const string GetVehiclesService::DATA_RANK = "rank";
		const string GetVehiclesService::TAG_VEHICLE = "vehicle";

		ParametersMap GetVehiclesService::_getParametersMap() const
		{
			ParametersMap map;

			if(_vehiclePage.get())
			{
				map.insert(PARAMETER_VEHICLE_PAGE_ID, _vehiclePage->getKey());
			}

			if(_line.get())
			{
				map.insert(PARAMETER_LINE_ID, _line->getKey());
			}

			if(_vehicle.get())
			{
				map.insert(Request::PARAMETER_OBJECT_ID, _vehicle->getKey());
			}

			return map;
		}

		void GetVehiclesService::_setFromParametersMap(const ParametersMap& map)
		{
			// Vehicle display page
			try
			{
				RegistryKeyType id(map.getDefault<RegistryKeyType>(PARAMETER_VEHICLE_PAGE_ID, 0));
				if(id > 0)
				{
					_vehiclePage = Env::GetOfficialEnv().getEditable<Webpage>(id);
				}
			}
			catch(ObjectNotFoundException<Webpage>&)
			{
				throw RequestException("No such vehicle page");
			}

			// Vehicle search
			if(map.getDefault<RegistryKeyType>(Request::PARAMETER_OBJECT_ID, 0)) try
			{
				_vehicle = VehicleTableSync::Get(map.get<RegistryKeyType>(Request::PARAMETER_OBJECT_ID), *_env);
			}
			catch (ObjectNotFoundException<Vehicle>&)
			{
				throw RequestException("No such vehicle");
			}
			else if(map.getDefault<RegistryKeyType>(PARAMETER_LINE_ID, 0)) try
			{
				_line = Env::GetOfficialEnv().get<CommercialLine>(map.get<RegistryKeyType>(PARAMETER_LINE_ID));
			}
			catch(ObjectNotFoundException<CommercialLine>&)
			{
				throw RequestException("No such line");
			}
		}



		bool GetVehiclesService::_selectVehicle( const Vehicle& object ) const
		{
			// Line filter
			if(_line.get())
			{
				AllowedLines::Type::const_iterator it(
					object.get<AllowedLines>().find(
						const_cast<CommercialLine*>(_line.get())
				)	);
				if(it == object.get<AllowedLines>().end())
				{
					return false;
				}
			}

			return true;
		}



		util::ParametersMap GetVehiclesService::run(
			std::ostream& stream,
			const Request& request
		) const {

			Vehicle::Registry::ConstVector vehicles;

			if(_vehicle.get())
			{
				vehicles.push_back(_vehicle);
			}
			else
			{
				const Vehicle::Registry& registry(Env::GetOfficialEnv().getRegistry<Vehicle>());
				vehicles = registry.getConstVector(boost::bind(&GetVehiclesService::_selectVehicle, this, _1));
			}

			size_t rank(0);
			ParametersMap pm;
			BOOST_FOREACH(const Vehicle::Registry::ConstVector::value_type& vehicle, vehicles)
			{
				if(_vehiclePage.get())
				{
					_displayVehicle(
						stream,
						request,
						*vehicle,
						rank++
					);
				}
				else
				{
					boost::shared_ptr<ParametersMap> vPM(new ParametersMap);
					vehicle->toParametersMap(*vPM, true);
					pm.insert(TAG_VEHICLE, vPM);
				}
			}

			return pm;
		}



		bool GetVehiclesService::isAuthorized(
			const Session* session
		) const {
			return true;
		}



		std::string GetVehiclesService::getOutputMimeType() const
		{
			return _vehiclePage.get() ? _vehiclePage->getMimeType() : "text/html";
		}



		void GetVehiclesService::_displayVehicle(
			std::ostream& stream,
			const server::Request& request,
			const Vehicle& vehicle,
			std::size_t rank
		) const {

			ParametersMap pm(getTemplateParameters());

			vehicle.toParametersMap(pm, true);
			pm.insert(DATA_RANK, rank);

			// Launch of the display
			_vehiclePage->display(stream, request, pm);
		}
}	}

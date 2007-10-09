
/** ScheduleSheetReservationRowInterfaceElement class implementation.
	@file ScheduleSheetReservationRowInterfaceElement.cpp

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

#include "ScheduleSheetReservationRowInterfaceElement.h"

#include "33_route_planner/Types.h"

#include "15_env/ReservationRuleInterfacePage.h"

#include "11_interfaces/ValueElementList.h"
#include "11_interfaces/Interface.h"


using namespace std;
using namespace boost;

namespace synthese
{
	using namespace interfaces;
	using namespace env;

	template<> const string util::FactorableTemplate<LibraryInterfaceElement,routeplanner::ScheduleSheetReservationRowInterfaceElement>::FACTORY_KEY("schedules_reservation");

	namespace routeplanner
	{
		void ScheduleSheetReservationRowInterfaceElement::storeParameters(ValueElementList& vel)
		{
			_cellHeader = vel.front();
			_cellFooter = vel.front();
		}

		string ScheduleSheetReservationRowInterfaceElement::display(
			std::ostream& stream
			, const interfaces::ParametersVector& parameters
			, VariablesMap& variables
			, const void* object /*= NULL*/
			, const server::Request* request /*= NULL*/
		) const {
			const JourneyBoardJourneys* jv = static_cast<const JourneyBoardJourneys*>(object);
			const ReservationRuleInterfacePage* resaInterfacePage = _page->getInterface()->getPage<ReservationRuleInterfacePage>();

			for (JourneyBoardJourneys::const_iterator it = jv->begin(); it != jv->end(); ++it)
			{
				stream << _cellHeader->getValue(parameters, variables, object, request);
				resaInterfacePage->display(stream, variables, **it, request);
				stream << _cellFooter->getValue(parameters, variables, object, request);
			}

			return string();
		}

		ScheduleSheetReservationRowInterfaceElement::~ScheduleSheetReservationRowInterfaceElement()
		{
		}
	}
}


/** JourneyBoardStopCellInterfacePage class implementation.
	@file JourneyBoardStopCellInterfacePage.cpp

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

#include "JourneyBoardStopCellInterfacePage.h"
#include "SentAlarm.h"
#include "AddressablePlace.h"
#include "NamedPlace.h"
#include "PhysicalStop.h"
#include "Projection.h"

#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

namespace synthese
{
	using namespace messages;
	using namespace interfaces;
	using namespace util;
	using namespace geography;
	using namespace time;
	using namespace env;
	using namespace road;
	using namespace routeplanner;

	template<> const string util::FactorableTemplate<InterfacePage, JourneyBoardStopCellInterfacePage>::FACTORY_KEY(
		"journey_board_stop_cell"
	);

	namespace routeplanner
	{
		const string JourneyBoardStopCellInterfacePage::DATA_IS_ARRIVAL("is_arrival");
		const string JourneyBoardStopCellInterfacePage::DATA_AlARM_LEVEL("alarm_level");
		const string JourneyBoardStopCellInterfacePage::DATA_ALARM_MESSAGE("alarm_message");
		const string JourneyBoardStopCellInterfacePage::DATA_IS_TERMINUS("is_terminus");
		const string JourneyBoardStopCellInterfacePage::DATA_STOP_NAME("stop_name");
		const string JourneyBoardStopCellInterfacePage::DATA_ODD_ROW("is_odd_row");
		const string JourneyBoardStopCellInterfacePage::DATA_FIRST_TIME("first_time");
		const string JourneyBoardStopCellInterfacePage::DATA_LAST_TIME("last_time");
		const string JourneyBoardStopCellInterfacePage::DATA_LONGITUDE("longitude");
		const string JourneyBoardStopCellInterfacePage::DATA_LATITUDE("latitude");

		void JourneyBoardStopCellInterfacePage::display(
			std::ostream& stream
			, bool isItArrival
			, const SentAlarm* alarm
			, bool isItTerminus
			, const PhysicalStop& physicalStop
			, bool color
			, const DateTime& time
			, int continuousServiceRange
			, const server::Request* request /*= NULL */
		) const	{

			DateTime endRangeTime(time);
			if (continuousServiceRange > 0)
				endRangeTime += continuousServiceRange;

			GeoPoint point(WGS84FromLambert(physicalStop));

			ParametersVector pv;
			pv.push_back( Conversion::ToString( isItArrival ));
			pv.push_back( alarm ? alarm->getLongMessage() : string() );
			pv.push_back( alarm ? Conversion::ToString(alarm->getLevel()) : string() );
			pv.push_back( Conversion::ToString( isItTerminus ));
			pv.push_back( dynamic_cast<const NamedPlace*>(physicalStop.getHub())->getFullName() );
			pv.push_back( Conversion::ToString( color ));
			pv.push_back( time.isUnknown() ? string() : time.getHour().toString() );
			pv.push_back( (continuousServiceRange > 0) ? endRangeTime.getHour().toString() : string() );
			pv.push_back( lexical_cast<string>(point.getLongitude()) );
			pv.push_back( lexical_cast<string>(point.getLatitude()) );

			VariablesMap vm;
			InterfacePage::_display( stream, pv, vm, NULL, request );
		}



		JourneyBoardStopCellInterfacePage::JourneyBoardStopCellInterfacePage()
			: Registrable(UNKNOWN_VALUE)
		{

		}
	}
}

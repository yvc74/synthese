
/** PTOperationStatisticsTableSync class implementation.
	@file PTOperationStatisticsTableSync.cpp

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

#include "PTOperationStatisticsTableSync.hpp"
#include "VehiclePositionTableSync.hpp"
#include "ScheduledServiceTableSync.h"
#include "JourneyPatternTableSync.hpp"
#include "CommercialLineTableSync.h"
#include "VehicleTableSync.hpp"

using namespace std;
using namespace boost;

namespace synthese
{
	using namespace db;
	using namespace pt;
	using namespace vehicle;

	namespace pt_operation
	{
		PTOperationStatisticsTableSync::CountSearchResult PTOperationStatisticsTableSync::Count(
			boost::gregorian::date_period period,
			Step rowStep,
			Step colStep,
			What what
		){
			CountSearchResult r;
			stringstream s;
			bool hasRowStep(rowStep != NO_STEP);
			bool hasColStep(colStep != NO_STEP);
			DB* db = DBModule::GetDB();

			s << "SELECT " <<
				"SUM(" << GetSQLWhat(what) << ") AS result";
				if(hasRowStep) s << "," << GetSQLColumnOrGroupBy(db, rowStep, true) << " AS row";
				if(hasColStep) s << "," << GetSQLColumnOrGroupBy(db, colStep, true) << " AS col";
			s << " FROM " << VehiclePositionTableSync::TABLE.NAME << " AS r " <<
				" WHERE " <<
				Time::FIELD.name << ">='" << gregorian::to_iso_extended_string(period.begin())  << " 00:00:00' AND " <<
				Time::FIELD.name << "<'" << gregorian::to_iso_extended_string(period.end()) << " 00:00:00'";
			if(hasRowStep || hasColStep)
			{
				s << " GROUP BY ";
				if(hasRowStep) s << GetSQLColumnOrGroupBy(db, rowStep, false);
				if(hasRowStep && hasColStep) s << ",";
				if(hasColStep) s << GetSQLColumnOrGroupBy(db, colStep, false);
			}
			if(hasRowStep || hasColStep)
			{
				s << " ORDER BY ";
				if(hasRowStep) s << GetSQLColumnOrGroupBy(db, rowStep, true);
				if(hasRowStep && hasColStep) s << ",";
				if(hasColStep) s << GetSQLColumnOrGroupBy(db, colStep, true);
			}


			DBResultSPtr rows = DBModule::GetDB()->execQuery(s.str());
			while(rows->next ())
			{
				const string row(hasRowStep ? rows->getText("row") : "row");
				const string col(hasColStep ? rows->getText("col") : "col");

				CountSearchResult::iterator itrow(r.find(row));
				if(itrow == r.end())
				{
					itrow = r.insert(make_pair(row, CountSearchResult::mapped_type())).first;
				}
				itrow->second.insert(make_pair(col, rows->getInt("result")));
			}
			return r;
		}



		string PTOperationStatisticsTableSync::GetSQLColumnOrGroupBy(DB* db, Step step, bool column)
		{
			if(step == SERVICE_STEP) return VehiclePositionService::FIELD.name;
			if(step == DATE_STEP) return db->getSQLDateFormat("%Y-%m-%d", Time::FIELD.name);
			if(step == HOUR_STEP) return db->getSQLDateFormat("%H", Time::FIELD.name);
			if(step == WEEK_DAY_STEP) return db->getSQLDateFormat("%w", Time::FIELD.name);
			if(step == WEEK_STEP) return db->getSQLDateFormat("%W", Time::FIELD.name);
			if(step == MONTH_STEP) return db->getSQLDateFormat("%Y-%m", Time::FIELD.name);
			if(step == YEAR_STEP) return db->getSQLDateFormat("%Y", Time::FIELD.name);
			if(column)
			{
				if(step == LINE_STEP)
					return "(SELECT c."+ ShortName::FIELD.name +
						" FROM "+ CommercialLineTableSync::TABLE.NAME +" AS c " +
						"INNER JOIN "+ JourneyPatternTableSync::TABLE.NAME +" AS jp ON "+
							"jp."+ JourneyPatternCommercialLine::FIELD.name +"=c."+ TABLE_COL_ID +
						" INNER JOIN "+ ScheduledServiceTableSync::TABLE.NAME +" AS s ON "
							"s."+ ServicePath::FIELD.name +"=jp."+ TABLE_COL_ID +
						" WHERE s."+ TABLE_COL_ID +"=r."+ VehiclePositionService::FIELD.name +")";
				if(step == VEHICLE_STEP)
					return "(SELECT v."+ Name::FIELD.name +" FROM "+ VehicleTableSync::TABLE.NAME +" AS v "+
						"WHERE v."+ TABLE_COL_ID +"=r."+ VehiclePositionVehicle::FIELD.name +")";
			}
			else
			{
				if(step == LINE_STEP)
					return "(SELECT jp."+ JourneyPatternCommercialLine::FIELD.name +
						" FROM "+ JourneyPatternTableSync::TABLE.NAME +" AS jp "+
						"INNER JOIN "+ ScheduledServiceTableSync::TABLE.NAME +" AS s ON "+
							"s."+ ServicePath::FIELD.name +"=jp."+ TABLE_COL_ID +
						" WHERE s."+ TABLE_COL_ID +"=r."+ VehiclePositionService::FIELD.name +")";
				if(step == VEHICLE_STEP)
					return "r."+ VehiclePositionVehicle::FIELD.name;
			}
			return string();
		}



		std::string PTOperationStatisticsTableSync::GetSQLWhat( What what )
		{
			stringstream s;
			s << "(SELECT p2." << MeterOffset::FIELD.name << " FROM " << VehiclePositionTableSync::TABLE.NAME << " AS p2 WHERE p2." <<
				Time::FIELD.name << ">r." << Time::FIELD.name <<
				" AND p2." << MeterOffset::FIELD.name << ">0" <<
				" AND p2." << VehiclePositionVehicle::FIELD.name << "=r." << VehiclePositionVehicle::FIELD.name <<
				" ORDER BY p2." << Time::FIELD.name << " LIMIT 1) - " << MeterOffset::FIELD.name;
			return s.str();
		}
	}
}

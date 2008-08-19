

/** ReservationRuleTableSync class implementation.
	@file ReservationRuleTableSync.cpp

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

#include <sstream>

#include "ReservationRuleTableSync.h"

#include "Conversion.h"

#include "DBModule.h"
#include "SQLiteResult.h"
#include "SQLite.h"
#include "SQLiteException.h"

using namespace std;
using namespace boost;

namespace synthese
{
	using namespace db;
	using namespace util;
	using namespace env;

	template<> const string util::FactorableTemplate<SQLiteTableSync,ReservationRuleTableSync>::FACTORY_KEY("15.10.06 Reservation rules");

	namespace db
	{
		template<> const std::string SQLiteTableSyncTemplate<ReservationRuleTableSync>::TABLE_NAME = "t021_reservation_rules";
		template<> const int SQLiteTableSyncTemplate<ReservationRuleTableSync>::TABLE_ID = 21;
		template<> const bool SQLiteTableSyncTemplate<ReservationRuleTableSync>::HAS_AUTO_INCREMENT = true;

		template<> void SQLiteDirectTableSyncTemplate<ReservationRuleTableSync,ReservationRule>::load(ReservationRule* rr, const db::SQLiteResultSPtr& rows )
		{
		    rr->setKey (rows->getLongLong (TABLE_COL_ID));
		    
		    bool online (rows->getBool (ReservationRuleTableSync::COL_ONLINE));

		    bool originIsReference (rows->getBool (ReservationRuleTableSync::COL_ORIGINISREFERENCE));
		    
		    int minDelayMinutes = rows->getInt (ReservationRuleTableSync::COL_MINDELAYMINUTES);
		    int minDelayDays = rows->getInt (ReservationRuleTableSync::COL_MINDELAYDAYS);
		    int maxDelayDays = rows->getInt (ReservationRuleTableSync::COL_MAXDELAYDAYS);
		    
		    synthese::time::Hour hourDeadline = 
			synthese::time::Hour::FromSQLTime (rows->getText (ReservationRuleTableSync::COL_HOURDEADLINE));
		    
		    std::string phoneExchangeNumber (
			rows->getText (ReservationRuleTableSync::COL_PHONEEXCHANGENUMBER));

		    std::string phoneExchangeOpeningHours (
			rows->getText (ReservationRuleTableSync::COL_PHONEEXCHANGEOPENINGHOURS));

		    std::string description (
			rows->getText (ReservationRuleTableSync::COL_DESCRIPTION));

		    std::string webSiteUrl (
			rows->getText (ReservationRuleTableSync::COL_WEBSITEURL));

		    rr->setCompliant (rows->getTribool (ReservationRuleTableSync::COL_TYPE));
		    rr->setOnline (online);
		    rr->setOriginIsReference (originIsReference);
		    rr->setMinDelayMinutes (minDelayMinutes);
		    rr->setMinDelayDays (minDelayDays);
		    rr->setMaxDelayDays (maxDelayDays);
		    rr->setHourDeadLine (hourDeadline);
		    rr->setPhoneExchangeNumber (phoneExchangeNumber);
		    rr->setPhoneExchangeOpeningHours (phoneExchangeOpeningHours);
		    rr->setDescription (description);
		    rr->setWebSiteUrl (webSiteUrl);
		}


		template<> void SQLiteDirectTableSyncTemplate<ReservationRuleTableSync,ReservationRule>::save(ReservationRule* object)
		{
			SQLite* sqlite = DBModule::GetSQLite();
			stringstream query;
			if (object->getKey() > 0)
			{
				query
					<< "UPDATE " << TABLE_NAME << " SET "
					/// @todo fill fields [,]FIELD=VALUE
					<< " WHERE " << TABLE_COL_ID << "=" << Conversion::ToString(object->getKey());
			}
			else
			{
				object->setKey(getId());
                query
					<< " INSERT INTO " << TABLE_NAME << " VALUES("
					<< Conversion::ToString(object->getKey())
					/// @todo fill other fields separated by ,
					<< ")";
			}
			sqlite->execUpdate(query.str());
		}

		template<> void SQLiteDirectTableSyncTemplate<ReservationRuleTableSync,ReservationRule>::_link(ReservationRule* obj, const SQLiteResultSPtr& rows, GetSource temporary)
		{

		}

		template<> void SQLiteDirectTableSyncTemplate<ReservationRuleTableSync,ReservationRule>::_unlink(ReservationRule* obj)
		{

		}

	}

	namespace env
	{
		const std::string ReservationRuleTableSync::COL_TYPE ("reservation_type");
		const std::string ReservationRuleTableSync::COL_ONLINE ("online");
		const std::string ReservationRuleTableSync::COL_ORIGINISREFERENCE ("origin_is_reference");
		const std::string ReservationRuleTableSync::COL_MINDELAYMINUTES ("min_delay_minutes");
		const std::string ReservationRuleTableSync::COL_MINDELAYDAYS ("min_delay_days");
		const std::string ReservationRuleTableSync::COL_MAXDELAYDAYS ("max_delay_days");
		const std::string ReservationRuleTableSync::COL_HOURDEADLINE ("hour_deadline");
		const std::string ReservationRuleTableSync::COL_PHONEEXCHANGENUMBER ("phone_exchange_number");
		const std::string ReservationRuleTableSync::COL_PHONEEXCHANGEOPENINGHOURS ("phone_exchange_opening_hours");
		const std::string ReservationRuleTableSync::COL_DESCRIPTION ("description");
		const std::string ReservationRuleTableSync::COL_WEBSITEURL ("web_site_url");

		ReservationRuleTableSync::ReservationRuleTableSync()
			: SQLiteDirectTableSyncTemplate<ReservationRuleTableSync,ReservationRule>()
		{
			addTableColumn(TABLE_COL_ID, "INTEGER", false);
			addTableColumn (COL_TYPE, "INTEGER", true);
			addTableColumn (COL_ONLINE, "BOOLEAN", true);
			addTableColumn (COL_ORIGINISREFERENCE, "BOOLEAN", true);
			addTableColumn (COL_MINDELAYMINUTES, "INTEGER", true);
			addTableColumn (COL_MINDELAYDAYS, "INTEGER", true);
			addTableColumn (COL_MAXDELAYDAYS, "INTEGER", true);
			addTableColumn (COL_HOURDEADLINE, "TIME", true);
			addTableColumn (COL_PHONEEXCHANGENUMBER, "TEXT", true);
			addTableColumn (COL_PHONEEXCHANGEOPENINGHOURS, "TEXT", true);
			addTableColumn (COL_DESCRIPTION, "TEXT", true);
			addTableColumn (COL_WEBSITEURL, "TEXT", true);
		}

		void ReservationRuleTableSync::rowsAdded(db::SQLite* sqlite,  db::SQLiteSync* sync, const db::SQLiteResultSPtr& rows, bool isFirstSync)
		{
			while (rows->next ())
			{
				ReservationRule* object(new ReservationRule());
				load(object, rows);
				object->store();
			}
		}

		void ReservationRuleTableSync::rowsUpdated(db::SQLite* sqlite,  db::SQLiteSync* sync, const db::SQLiteResultSPtr& rows)
		{
			while (rows->next ())
			{
			    uid id = rows->getLongLong (TABLE_COL_ID);
			    if (ReservationRule::Contains(id))
			    {
				shared_ptr<ReservationRule> object = ReservationRule::GetUpdateable(id);
				load(object.get(), rows);
			    }
			}
		}

		void ReservationRuleTableSync::rowsRemoved( db::SQLite* sqlite,  db::SQLiteSync* sync, const db::SQLiteResultSPtr& rows )
		{
			while (rows->next ())
			{
				uid id = rows->getLongLong (TABLE_COL_ID);
				if (ReservationRule::Contains(id))
				{
					ReservationRule::Remove(id);
				}
			}
		}

		vector<shared_ptr<ReservationRule> > ReservationRuleTableSync::Search(
			int first /*= 0*/
			, int number /*= 0*/
		){
			SQLite* sqlite = DBModule::GetSQLite();
			stringstream query;
			query
				<< " SELECT *"
				<< " FROM " << TABLE_NAME
				<< " WHERE 1 " 
				/// @todo Fill Where criteria
				// eg << TABLE_COL_NAME << " LIKE '%" << Conversion::ToSQLiteString(name, false) << "%'"
				;
			if (number > 0)
				query << " LIMIT " << Conversion::ToString(number + 1);
			if (first > 0)
				query << " OFFSET " << Conversion::ToString(first);

			try
			{
				SQLiteResultSPtr rows = sqlite->execQuery(query.str());
				vector<shared_ptr<ReservationRule> > objects;
				while (rows->next ())
				{
					shared_ptr<ReservationRule> object(new ReservationRule);
					load(object.get(), rows);
					link(object.get(), rows, GET_AUTO);
					objects.push_back(object);
				}
				return objects;
			}
			catch(SQLiteException& e)
			{
				throw Exception(e.getMessage());
			}
		}
	}
}

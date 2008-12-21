/** AlarmTableSync class implementation.
	@file AlarmTableSync.cpp

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

#include "AlarmTableSync.h"

#include "AlarmTemplate.h"
#include "ScenarioSentAlarm.h"
#include "SingleSentAlarm.h"
#include "AlarmTemplate.h"
#include "SentScenario.h"
#include "SentScenarioInheritedTableSync.h"
#include "ScenarioTemplate.h"
#include "ScenarioTemplateInheritedTableSync.h"
#include "17_messages/Types.h"
#include "AlarmObjectLinkTableSync.h"
#include "AlarmTemplateInheritedTableSync.h"
#include "SingleSentAlarmInheritedTableSync.h"
#include "ScenarioSentAlarmInheritedTableSync.h"

#include "Conversion.h"

#include "SQLiteResult.h"
#include "SQLite.h"

#include "DateTime.h"

using namespace std;
using namespace boost;

namespace synthese
{
	using namespace db;
	using namespace messages;
	using namespace time;
	using namespace util;

	namespace util
	{
		template<> const string FactorableTemplate<SQLiteTableSync,AlarmTableSync>::FACTORY_KEY("17.10.01 Alarms");
	}

	namespace messages
	{
		const string AlarmTableSync::_COL_CONFLICT_LEVEL = "conflict_level";
		const string AlarmTableSync::_COL_RECIPIENTS_NUMBER = "recipients";

		const string AlarmTableSync::COL_IS_TEMPLATE = "is_template";
		const string AlarmTableSync::COL_ENABLED = "is_enabled";
		const string AlarmTableSync::COL_LEVEL = "level";
		const string AlarmTableSync::COL_SHORT_MESSAGE = "short_message"; 
		const string AlarmTableSync::COL_LONG_MESSAGE = "long_message";
		const string AlarmTableSync::COL_PERIODSTART = "period_start";
		const string AlarmTableSync::COL_PERIODEND = "period_end"; 
		const string AlarmTableSync::COL_SCENARIO_ID = "scenario_id"; 
	}

	namespace db
	{
		template<> const SQLiteTableFormat SQLiteTableSyncTemplate<AlarmTableSync>::TABLE(
			AlarmTableSync::CreateFormat(
				"t003_alarms",
				SQLiteTableFormat::CreateFields(
					SQLiteTableFormat::Field(AlarmTableSync::COL_IS_TEMPLATE, INTEGER),
					SQLiteTableFormat::Field(AlarmTableSync::COL_ENABLED, INTEGER),
					SQLiteTableFormat::Field(AlarmTableSync::COL_LEVEL, INTEGER),
					SQLiteTableFormat::Field(AlarmTableSync::COL_SHORT_MESSAGE, TEXT),
					SQLiteTableFormat::Field(AlarmTableSync::COL_LONG_MESSAGE, TEXT),
					SQLiteTableFormat::Field(AlarmTableSync::COL_PERIODSTART, TIMESTAMP),
					SQLiteTableFormat::Field(AlarmTableSync::COL_PERIODEND, TIMESTAMP),
					SQLiteTableFormat::Field(AlarmTableSync::COL_SCENARIO_ID, INTEGER),
					SQLiteTableFormat::Field()
				), SQLiteTableFormat::CreateIndexes(
					SQLiteTableFormat::Index(
						"scenariostart",
						SQLiteTableFormat::Index::CreateFieldsList(
							AlarmTableSync::COL_SCENARIO_ID,
							AlarmTableSync::COL_PERIODSTART,
							string()
					)	),
					SQLiteTableFormat::Index()
		)	)	);
	    
		template<>
		string SQLiteInheritanceTableSyncTemplate<AlarmTableSync,Alarm>::_GetSubClassKey(const SQLiteResultSPtr& row)
		{
			return row->getBool(AlarmTableSync::COL_IS_TEMPLATE)
				? AlarmTemplateInheritedTableSync::FACTORY_KEY
				: (row->getLongLong(AlarmTableSync::COL_SCENARIO_ID) > 0) 
				? ScenarioSentAlarmInheritedTableSync::FACTORY_KEY
				: SingleSentAlarmInheritedTableSync::FACTORY_KEY
				;
		}


		template<>
		string SQLiteInheritanceTableSyncTemplate<AlarmTableSync,Alarm>::_GetSubClassKey(const Alarm* obj)
		{
			return	(dynamic_cast<const SingleSentAlarm*>(obj) != NULL)
				?	SingleSentAlarmInheritedTableSync::FACTORY_KEY
				:	(dynamic_cast<const ScenarioSentAlarm*>(obj) != NULL)
				?	ScenarioSentAlarmInheritedTableSync::FACTORY_KEY
				:	AlarmTemplateInheritedTableSync::FACTORY_KEY
				;
		}

		template<> void SQLiteInheritanceTableSyncTemplate<AlarmTableSync,Alarm>::_CommonLoad(
			Alarm* alarm
			, const SQLiteResultSPtr& rows,
			Env* env,
			LinkLevel linkLevel
		){
			alarm->setLevel (static_cast<AlarmLevel>(rows->getInt ( AlarmTableSync::COL_LEVEL)));
			alarm->setShortMessage (rows->getText (AlarmTableSync::COL_SHORT_MESSAGE));
			alarm->setLongMessage (rows->getText (AlarmTableSync::COL_LONG_MESSAGE));
		}
	}


	namespace messages
	{
		AlarmTableSync::AlarmTableSync ()
		: SQLiteInheritanceTableSyncTemplate<AlarmTableSync,Alarm>()
		{
		}


		AlarmTableSync::~AlarmTableSync ()
		{
		}
	}
}

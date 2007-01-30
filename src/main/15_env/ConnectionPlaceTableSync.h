
/** ConnectionPlaceTableSync class header.
	@file ConnectionPlaceTableSync.h

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

#ifndef SYNTHESE_ENVLSSQL_CONNECTIONPLACETABLESYNC_H
#define SYNTHESE_ENVLSSQL_CONNECTIONPLACETABLESYNC_H


#include <string>
#include <iostream>

#include "02_db/SQLiteTableSyncTemplate.h"

#include "15_env/City.h"

namespace synthese
{
	namespace env
	{
		/** ConnectionPlace SQLite table synchronizer.
			@ingroup m15

			Connection places table :
				- on insert : 
				- on update : 
				- on delete : X
		*/
		class ConnectionPlaceTableSync : public db::SQLiteTableSyncTemplate<ConnectionPlace>
		{
		public:
			static const std::string TABLE_COL_NAME;
			static const std::string TABLE_COL_CITYID;
			static const std::string TABLE_COL_CONNECTIONTYPE;
			static const std::string TABLE_COL_ISCITYMAINCONNECTION;
			static const std::string TABLE_COL_DEFAULTTRANSFERDELAY;
			static const std::string TABLE_COL_TRANSFERDELAYS;
			static const std::string TABLE_COL_ALARMID;

			ConnectionPlaceTableSync ();
			~ConnectionPlaceTableSync ();

			void rowsAdded (const synthese::db::SQLiteQueueThreadExec* sqlite, 
				synthese::db::SQLiteSync* sync,
				const synthese::db::SQLiteResult& rows);

			void rowsUpdated (const synthese::db::SQLiteQueueThreadExec* sqlite, 
				synthese::db::SQLiteSync* sync,
				const synthese::db::SQLiteResult& rows);

			void rowsRemoved (const synthese::db::SQLiteQueueThreadExec* sqlite, 
				synthese::db::SQLiteSync* sync,
				const synthese::db::SQLiteResult& rows);
		};
	}
}
#endif


/** SQLiteSync class header.
	@file SQLiteSync.h

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

#ifndef SYNTHESE_DB_SQLITESYNC_H
#define SYNTHESE_DB_SQLITESYNC_H


#include "02_db/SQLiteUpdateHook.h"
#include "02_db/Constants.h"

#include <map>
#include <string>
#include <iostream>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/shared_ptr.hpp>


namespace synthese
{
namespace db
{

    class SQLiteQueueThreadExec;
    class SQLiteTableSync;



/** SQLite synchronizer class.
    Manages a set of table synchronizers which synchronize 
    db data with live object model.


@ingroup m02
*/

class SQLiteSync : public synthese::db::SQLiteUpdateHook
{
 private:

    bool _isRegistered;

	std::map<std::string, boost::shared_ptr<SQLiteTableSync> > _tableSynchronizers;
	std::map<std::string, boost::shared_ptr<SQLiteTableSync> > _rankedTableSynchronizers;
    mutable boost::recursive_mutex _tableSynchronizersMutex; 

 public:

    SQLiteSync ();
    ~SQLiteSync ();

    void registerCallback (SQLiteQueueThreadExec* emitter);
    
    void eventCallback (SQLiteQueueThreadExec* emitter,
			const SQLiteEvent& event);

    /** Adds a new table synchronizer to this global synchronizer update hook.
	This method is thread-safe.
		@param rank Rank of the synchroniser in the load procedure
		@param synchroniser The synchronizer singleton to store
    */
	void addTableSynchronizer (const std::string& rank, boost::shared_ptr<SQLiteTableSync> synchronizer);

    bool hasTableSynchronizer (const std::string& tableName) const;
	boost::shared_ptr<SQLiteTableSync> getTableSynchronizer (const std::string& tableName) const;
	std::map<std::string, boost::shared_ptr<SQLiteTableSync> > getTableSynchronizers () const;


};




}

}
#endif


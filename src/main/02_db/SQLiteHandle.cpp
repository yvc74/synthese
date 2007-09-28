#include "02_db/SQLiteHandle.h"

#include "02_db/SQLiteException.h"
#include "02_db/SQLiteLazyResult.h"
#include "02_db/SQLiteCachedResult.h"

#include "01_util/Conversion.h"
#include "01_util/Log.h"


using namespace synthese::util;
using namespace std;



namespace synthese
{

    namespace db
    {
	
	
	void cleanupHandle (sqlite3* hdl)
	{
	    int retc = sqlite3_close (hdl);
	    if (retc != SQLITE_OK)
	    {
		throw SQLiteException ("Cannot close SQLite handle (error=" + Conversion::ToString (retc) + ")");
	    }
	    // TODO clean update hook struct.
	}
	
	
	
	int sqliteBusyHandler (void* arg, int nbCalls)
	{
	    // std::cerr << "busy handleer !! " << std::endl;
	    // Return a non-zero value so that a retry is made, waiting for SQLite not ot be busy anymore...
	    return 1;
	    
	}
	

	
	void sqliteUpdateHook (void* userData, int opType, const char* dbName, const char* tbName, sqlite_int64 rowId)
	{
	    // WARNING : the update hook is invoked only when working with the connection
	    // created inside the body of this thread (initialize).
	    UpdateHookStruct* uhs = (UpdateHookStruct*) userData;
	    
	    SQLiteEvent event;
	    event.opType = opType;
	    event.dbName = dbName;
	    event.tbName = tbName;
	    event.rowId = rowId;
	    
	    uhs->events.push_back (event);
	}
	
	

	SQLiteHandle::SQLiteHandle (const boost::filesystem::path& databaseFile)
	    : _databaseFile (databaseFile)
	    , _handle (&cleanupHandle)
	    , _hooksMutex (new boost::mutex ())

	{
	}
	
	
	
	SQLiteHandle::~SQLiteHandle ()
	{
	}
	
	
	void 
	SQLiteHandle::registerUpdateHook (SQLiteUpdateHook* hook)
	{
	    boost::mutex::scoped_lock hooksLock (*_hooksMutex);
		    
	    _hooks.push_back (hook);
	    hook->registerCallback (this);
	}
	


	UpdateHookStruct* 
	SQLiteHandle::getUpdateHookStruct ()
	{
	    if (_updateHookStruct.get () == 0)
	    {
		_updateHookStruct.reset (new UpdateHookStruct ());
	    }
	    return _updateHookStruct.get ();
	}
	
	


	SQLiteResultSPtr 
	SQLiteHandle::execQuery (const SQLiteStatementSPtr& statement, bool lazy)
	{
	    // lazy = false;
	    SQLiteResultSPtr result (new SQLiteLazyResult (statement));
	    if (lazy)
	    {
		return result;
	    }
	    else
	    {
		SQLiteCachedResult* cachedResult = new SQLiteCachedResult (result);
		return SQLiteResultSPtr (cachedResult);
	    }
	}




	sqlite3* 
	SQLiteHandle::getHandle () 
	{
	    if (_handle.get() == 0)
	    {
		
		sqlite3* handle;
		int retc = sqlite3_open (_databaseFile.string ().c_str (), &handle);
		if (retc != SQLITE_OK)
		{
		    throw SQLiteException ("Cannot open SQLite handle to " + 
					   _databaseFile.string () + "(error=" + Conversion::ToString (retc) + ")");
		}
		    
		// int 
		sqlite3_busy_handler(handle, &sqliteBusyHandler, 0);
		
		// std::cerr << " New handle ! " << handle << std::endl;
		sqlite3_update_hook (handle, &sqliteUpdateHook, getUpdateHookStruct ());
		_handle.reset (handle);
	    }
	    return _handle.get ();
	    
	}


	void 
	SQLiteHandle::execUpdate (const SQLiteStatementSPtr& statement)
	{
	    UpdateHookStruct* uhs = getUpdateHookStruct ();
	    
	    uhs->events.clear ();

	    int retc = SQLITE_ROW;
	    while (retc == SQLITE_ROW)
	    {
		retc = sqlite3_step (statement->getStatement ());
	    }
	    if (retc != SQLITE_DONE)
	    {
		throw SQLiteException ("Error executing precompiled statement (error=" + Conversion::ToString (retc) + ")" + 
				       Conversion::ToTruncatedString (statement->getSQL ()));
	    }

	    // Call hooks!
	    const std::vector<SQLiteEvent>& events = uhs->events;
	    for (std::vector<SQLiteEvent>::const_iterator it = events.begin ();
		 it != events.end (); ++it)
	    {
		for (std::vector<SQLiteUpdateHook*>::const_iterator ith = _hooks.begin ();
		     ith != _hooks.end (); ++ith)
		{
		    (*ith)->eventCallback (this, *it);
		}
	    }
	    
	}




	    
	SQLiteStatementSPtr 
	SQLiteHandle::compileStatement (const SQLData& sql)
	{
	    return SQLiteStatementSPtr (new SQLiteStatement (*this, sql));
	}



    }
}


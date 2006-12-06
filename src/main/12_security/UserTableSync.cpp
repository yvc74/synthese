
#include <sstream>

#include "01_util/Conversion.h"

#include "02_db/SQLiteResult.h"
#include "02_db/SQLiteThreadExec.h"
#include "02_db/SQLiteException.h"

#include "12_security/SecurityModule.h"
#include "12_security/UserTableSync.h"
#include "12_security/User.h"
#include "12_security/UserTableSyncException.h"

using namespace std;

namespace synthese
{
	using namespace db;
	using namespace util;
	using namespace security;

	namespace db
	{
		const std::string SQLiteTableSyncTemplate<User>::TABLE_NAME = "t026_users";
		const int SQLiteTableSyncTemplate<User>::TABLE_ID = 26;
		const bool SQLiteTableSyncTemplate<User>::HAS_AUTO_INCREMENT = true;
	}
	namespace security
	{
		const std::string UserTableSync::TABLE_COL_ID = "id";
		const std::string UserTableSync::TABLE_COL_NAME = "name";
		const std::string UserTableSync::TABLE_COL_SURNAME = "surname";
		const std::string UserTableSync::TABLE_COL_LOGIN = "login";
		const std::string UserTableSync::TABLE_COL_PASSWORD = "password";
		const std::string UserTableSync::TABLE_COL_PROFILE_ID = "profile_id";

		UserTableSync::UserTableSync()
			: db::SQLiteTableSyncTemplate<User> ( TABLE_NAME, true, true, TRIGGERS_ENABLED_CLAUSE)
		{
			addTableColumn(TABLE_COL_ID, "INTEGER", false);
			addTableColumn(TABLE_COL_NAME, "TEXT", true);
			addTableColumn(TABLE_COL_SURNAME, "TEXT", true);
			addTableColumn(TABLE_COL_LOGIN, "TEXT", true);
			addTableColumn(TABLE_COL_PASSWORD, "TEXT", true);
			addTableColumn(TABLE_COL_PROFILE_ID, "INTEGER", true);
		}


		void UserTableSync::rowsUpdated( const SQLiteThreadExec* sqlite,  SQLiteSync* sync, const SQLiteResult& rows )
		{
			for (int i=0; i<rows.getNbRows(); ++i)
			{
				/// @todo implementation
			}
		}


		void UserTableSync::rowsAdded( const SQLiteThreadExec* sqlite,  SQLiteSync* sync, const SQLiteResult& rows )
		{
		}


		void UserTableSync::rowsRemoved( const SQLiteThreadExec* sqlite,  SQLiteSync* sync, const SQLiteResult& rows )
		{
			/// @todo implementation
		}


		User* UserTableSync::getUser( const db::SQLiteThreadExec* sqlite, uid id )
		{
			std::stringstream query;
			query
				<< "SELECT * "
				<< "FROM " << TABLE_NAME
				<< "WHERE " << TABLE_COL_ID << "=" << Conversion::ToString(id);
			try
			{
				db::SQLiteResult rows = sqlite->execQuery(query.str());
				if (rows.getNbRows() <= 0)
					throw UserTableSyncException("User "+ Conversion::ToString(id) + " not found in database.");
				User* user = new User;
				loadUser(user, rows);
				return user;
			}
			catch (SQLiteException e)
			{
				throw UserTableSyncException(e.getMessage());
			}
		}

		User* UserTableSync::getUser( const db::SQLiteThreadExec* sqlite, const std::string& login )
		{
			std::stringstream query;
			query
				<< "SELECT *"
				<< " FROM " << TABLE_NAME
				<< " WHERE " << TABLE_COL_LOGIN << "=" << Conversion::ToSQLiteString(login);
			try
			{
				db::SQLiteResult rows = sqlite->execQuery(query.str());
				if (rows.getNbRows() <= 0)
					throw UserTableSyncException("User "+ login + " not found in database.");
				User* user = new User;
				loadUser(user, rows);
				return user;
			}
			catch (SQLiteException e)
			{
				throw UserTableSyncException(e.getMessage());
			}
		}

		void UserTableSync::loadUser(User* user, const db::SQLiteResult& rows, int rowId)
		{
			user->setKey(Conversion::ToLongLong(rows.getColumn(rowId, TABLE_COL_ID)));
			user->setPassword(rows.getColumn(rowId, TABLE_COL_PASSWORD));
			user->setName(rows.getColumn(rowId, TABLE_COL_NAME));
			user->setSurname(rows.getColumn(rowId, TABLE_COL_SURNAME));
			user->setLogin(rows.getColumn(rowId, TABLE_COL_LOGIN));
			try
			{
				user->setProfile(SecurityModule::getProfiles().get(Conversion::ToLongLong(rows.getColumn(rowId, TABLE_COL_PROFILE_ID))));
			}
			catch (Profile::RegistryKeyException e)
			{
				throw UserTableSyncException("Bad profile "+ rows.getColumn(rowId, TABLE_COL_PROFILE_ID));
			}
		}

		void UserTableSync::saveUser( const db::SQLiteThreadExec* sqlite, User* user )
		{
			try
			{
				if (user->getKey() != 0)
				{
					// UPDATE
				}
				else // INSERT
				{
					/// @todo Implement control of the fields
					user->setKey(getId(1,1));	/// @todo handle grid id
					stringstream query;
					query
						<< "INSERT INTO " << TABLE_NAME
						<< " VALUES(" 
						<< Conversion::ToString(user->getKey())
						<< "," << Conversion::ToSQLiteString(user->getName())
						<< "," << Conversion::ToSQLiteString(user->getSurname())
						<< "," << Conversion::ToSQLiteString(user->getLogin())
						<< "," << Conversion::ToSQLiteString(user->getPassword())
						<< "," << Conversion::ToString(user->getProfile()->getKey())
						<< ")";
					sqlite->execUpdate(query.str());
				}
			}
			catch (SQLiteException e)
			{
				throw UserTableSyncException("Insert/Update error " + e.getMessage());
			}
			catch (...)
			{
				throw UserTableSyncException("Unknown Insert/Update error");
			}
		}

		std::vector<User*> UserTableSync::searchUsers( const db::SQLiteThreadExec* sqlite, const std::string& login, const std::string name, uid profileId, int first /*= 0*/, int number /*= 0*/ )
		{
			stringstream query;
			query
				<< " SELECT *"
				<< " FROM " << TABLE_NAME
				<< " WHERE " << TABLE_COL_LOGIN << " LIKE '%" << Conversion::ToSQLiteString(login, false) << "%'"
				<< " AND " << TABLE_COL_NAME << " LIKE '%" << Conversion::ToSQLiteString(name, false) << "%'";
			if (profileId > 0)
				query << " AND " << TABLE_COL_PROFILE_ID << "=" << Conversion::ToString(profileId);
			if (number > 0)
				query << " LIMIT " << Conversion::ToString(number + 1);
			if (first > 0)
				query << " OFFSET " << Conversion::ToString(first);
			
			try
			{
				SQLiteResult result = sqlite->execQuery(query.str());
				vector<User*> users;
				for (int i = 0; i < result.getNbRows(); ++i)
				{
					User* user = new User;
					loadUser(user, result, i);
					users.push_back(user);
				}
				return users;
			}
			catch(SQLiteException& e)
			{
				throw UserTableSyncException(e.getMessage());
			}
		}
	}
}



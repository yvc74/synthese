////////////////////////////////////////////////////////////////////////////////
/// SQLiteTableSyncTemplate class header.
///	@file SQLiteTableSyncTemplate.h
///	@author Marc Jambert
///
///	This file belongs to the SYNTHESE project (public transportation specialized
///	software)
///	Copyright (C) 2002 Hugues Romain - RCS <contact@reseaux-conseil.com>
///
///	This program is free software; you can redistribute it and/or
///	modify it under the terms of the GNU General Public License
///	as published by the Free Software Foundation; either version 2
///	of the License, or (at your option) any later version.
///
///	This program is distributed in the hope that it will be useful,
///	but WITHOUT ANY WARRANTY; without even the implied warranty of
///	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///	GNU General Public License for more details.
///
///	You should have received a copy of the GNU General Public License
///	along with this program; if not, write to the Free Software Foundation,
///	Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
////////////////////////////////////////////////////////////////////////////////

#ifndef SYNTHESE_SQLiteTableSyncTemplate_H__
#define SYNTHESE_SQLiteTableSyncTemplate_H__

#include "DBModule.h"
#include "SQLite.h"
#include "SQLiteTableSync.h"
#include "SQLiteResult.h"
#include "DBEmptyResultException.h"
#include "SQLiteException.h"
#include "FactorableTemplate.h"
#include "UtilTypes.h"
#include "Log.h"
#include "CoordinatesSystem.hpp"

#include <sstream>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

namespace synthese
{
	namespace db
	{
		////////////////////////////////////////////////////////////////////
		/// Table synchronizer template.
		//////////////////////////////////////////////////////////////////////////
		///	@ingroup m10
		/// @warning Geometry columns must always be at the end. If non geometry
		/// columns are present after any geometry column, they will be ignored.
		template <class K>
		class SQLiteTableSyncTemplate:
			public util::FactorableTemplate<SQLiteTableSync, K>
		{
		public:
			////////////////////////////////////////////////////////////////////
			/// Format of the table
			/// The access to TABLE is public to allow the use of it in SQL 
			/// queries everywhere
			static const SQLiteTableSync::Format TABLE;


			////////////////////////////////////////////////////////////////////
			/// Fields of the table.
			/// To allow to loop on the table without knowing its size,
			/// the last element of the _FIELDS array must be an empty Field
			/// object.
			/// The first field must be the primary key of the table.
			static const SQLiteTableSync::Field _FIELDS[];



			////////////////////////////////////////////////////////////////////
			/// Indexes of the table.
			/// To allow to loop on the table without knowing its size,
			/// the last element of the _INDEXES array must be an empty Index
			/// object.
			static const SQLiteTableSync::Index _INDEXES[];



			//////////////////////////////////////////////////////////////////////////
			/// Replacement of * fields getter to ensure conversion of geometry
			/// columns into WKB format
			/// @author Hugues Romain
			/// @since 3.2.0
			/// @date 2010
			static std::string _fieldsGetter;
			
		private:
			////////////////////////////////////////////////////////////////////
			///	Gets the SQL schema of the table.
			///	@return the SQL schema of the table
			///	@author Marc Jambert, Hugues Romain
			/// @warning this method considers that the first field is always
			/// the primary key of the table. To avoid this behavior, do not
			/// use the SQLiteTableSyncTemplate class.
			//////////////////////////////////////////////////////////////////////////
			/// This method builds the same text as present in the the sql field of
			/// the SQL_MASTER virtual table in the record corresponding to the current
			/// table. A difference between the two texts seems that the schema is
			/// different.
			/// 
			/// Note : The name of the geometry columns created by SpatiaLite are double-
			/// quoted.
			static std::string _GetSQLFieldsSchema();



			//////////////////////////////////////////////////////////////////////////
			/// Gets the SQL code to execute to create the table in the database.
			/// Note : This method is different to _GetSQLFieldsSchema because the
			/// creation of geometry columns is done by AddGeometryColumn and not only
			/// by specifying the schema.
			//////////////////////////////////////////////////////////////////////////
			/// @return the SQL command which creates the table in the database
			/// @author Hugues Romain
			/// @date 2010
			/// @since 3.2.0
			static std::string _GetTableCreationSQL();



			////////////////////////////////////////////////////////////////////
			/// Creates the SQL statement to create an index in the database given a certain format.
			///	@param index The index to create
			///	@return std::string The SQL statement
			///	@author Hugues Romain
			///	@date 2007
			static std::string _GetSQLIndexSchema(const SQLiteTableSync::Index& index);

			
			
			////////////////////////////////////////////////////////////////////
			///	Creates the SQL schema of the table related triggers.
			///	@return SQL statement
			///	@author Marc Jambert
			static std::string _GetSQLTriggerNoUpdate();



			////////////////////////////////////////////////////////////////////
			///	Tests if the table has a field of the specified name.
			///	@param name Name of the searched field
			///	@return bool true if the table has such a field
			static bool _HasField(const std::string& name);

			
			
			////////////////////////////////////////////////////////////////////
			///	Creates the database wide name of an index.
			///	@param index index
			///	@return name of the index
			///	@author Hugues Romain
			static std::string _GetIndexName(const SQLiteTableSync::Index& index);



			////////////////////////////////////////////////////////////////////
			/// Adapts table in SQLite db to conform to this class table format.
			/// Right now, only new column addition/insertion is supported.
			/// Any other change to table schema is not supported yet.
			/// @param sqlite SQLite access.
			/// @author Marc Jambert
			/// @date 2007
			static void _UpdateSchema(
				SQLite* sqlite
			);



		protected:

			//! @name Table run variables
			//@{
				static util::RegistryObjectType _autoIncrementValue;						//!< Value of the last created object
				static boost::shared_ptr<boost::mutex> _idMutex;	//!< Mutex
			//@}


			//! @name Table run variables handlers
			//@{
				static util::RegistryKeyType encodeUId(
					util::RegistryObjectType objectId
				){
					static int nodeId = boost::lexical_cast<util::RegistryNodeType>(
						server::ModuleClass::GetParameter("dbring_node_id", "1")
					);
					return util::encodeUId(
						K::TABLE.ID,
						nodeId,
						objectId
					);
				}



				static void _InitAutoIncrement()
				{
					if (!K::TABLE.HAS_AUTO_INCREMENT)
						return;

					try
					{
						SQLite* sqlite = DBModule::GetSQLite();
						std::stringstream query;
						query
							<< "SELECT " << 0x00000000FFFFFFFFLL
							<< " & " << TABLE_COL_ID << " AS maxid FROM " << K::TABLE.NAME
							<< " WHERE " << TABLE_COL_ID << ">=" << encodeUId(0) << " AND "
							<< TABLE_COL_ID << "<=" << encodeUId(0xFFFFFFFF) <<
							" ORDER BY " << TABLE_COL_ID << " DESC LIMIT 1"
						;

						SQLiteResultSPtr result (sqlite->execQuery(query.str()));
						if (result->next ())
						{
							util::RegistryKeyType maxid = result->getLongLong("maxid");
							if (maxid > 0)
							{
								_autoIncrementValue = util::decodeObjectId(maxid) + 1;
								util::Log::GetInstance().debug("Auto-increment of table "+ K::TABLE.NAME +" initialized at "+ boost::lexical_cast<std::string>(_autoIncrementValue));
							}
						}

					}
					catch (SQLiteException& e)
					{
						util::Log::GetInstance().debug("Table "+ K::TABLE.NAME +" without preceding id.", e);

					}
					catch (...)
					{
						//					Log::GetInstance().debug("Table "+ getTableName() +" without preceding id.", e);
					}
				}
			//@}



			//! @name Data access
			//@{
				/** Gets a result row in the database.
					@param key key of the row to get (corresponds to the id field)
					@return SQLiteResultSPtr The found result row
					@throw DBEmptyResultException if the key was not found in the table
					@author Hugues Romain
					@date 2007				
				*/
				static SQLiteResultSPtr _GetRow(util::RegistryKeyType key )
				{
					SQLite* sqlite = DBModule::GetSQLite();
					std::stringstream query;
					query
						<< "SELECT " << _fieldsGetter
						<< " FROM " << K::TABLE.NAME
						<< " WHERE " << TABLE_COL_ID << "=" << key
						<< " LIMIT 1";
					SQLiteResultSPtr rows (sqlite->execQuery (query.str()));
					if (rows->next() == false) throw DBEmptyResultException<K>(key);
					return rows;
				}
			//@}


			//! @name Virtual access to instantiated static methods
			//@{
				virtual void initAutoIncrement() const { _InitAutoIncrement(); }
				virtual void firstSync(SQLite* sqlite, SQLiteSync* sync) const { _FirstSync(sqlite, sync); }
				virtual void updateSchema(SQLite* sqlite) const { _UpdateSchema(sqlite); }
				virtual const SQLiteTableSync::Format& getFormat() const { return K::TABLE; }
			//@}



			/** Utility method to get a row by id.
			This uses a precompiled statement for performance 
			*/
			virtual SQLiteResultSPtr getRowById(SQLite* sqlite, util::RegistryKeyType id) const
			{
				std::stringstream ss;
				ss << "SELECT " << _fieldsGetter << " FROM " << K::TABLE.NAME << " WHERE ROWID=" << id << " LIMIT 1";

				return sqlite->execQuery (ss.str (), false); 
			}

			
			////////////////////////////////////////////////////////////////////
			///	Launch all "data insertion" handlers (loads all the data).
			///	@param sqlite
			///	@param sync
			///	@author Hugues Romain
			///	@date 2008
			////////////////////////////////////////////////////////////////////
			static void _FirstSync(
				SQLite* sqlite, 
				SQLiteSync* sync
			){
				// Callbacks according to what already exists in the table.
				if (!K::TABLE.IGNORE_CALLBACKS_ON_FIRST_SYNC)
				{
					std::stringstream ss;
					ss << "SELECT " << _fieldsGetter << " FROM " << K::TABLE.NAME;
					SQLiteResultSPtr result = sqlite->execQuery (ss.str (), true);
					K().rowsAdded (sqlite, sync, result);
				}
			}

		public:
			/** Unique ID generator for autoincremented tables.
			*/
			static util::RegistryKeyType getId()
			{			
				boost::mutex::scoped_lock mutex(*_idMutex);

				util::RegistryObjectType retval = _autoIncrementValue++;

				return encodeUId(retval);
			}



			static std::string GetFieldValue(
				util::RegistryKeyType id,
				const std::string& field
			){
				std::stringstream query;
				query
					<< "SELECT " << field << " FROM " << K::TABLE.NAME
					<< " WHERE " << TABLE_COL_ID << "=" << id;
				SQLiteResultSPtr rows = DBModule::GetSQLite()->execQuery(query.str());
				if (!rows->next())
					throw DBEmptyResultException<K>(id);
				else
					return rows->getText(field);
			}


			static void Remove(
				util::RegistryKeyType key,
				 boost::optional<SQLiteTransaction&> transaction = boost::optional<SQLiteTransaction&>()
			){
				SQLite* sqlite = DBModule::GetSQLite();
				std::stringstream query;
				query
					<< "DELETE FROM " << K::TABLE.NAME
					<< " WHERE " << TABLE_COL_ID << "=" << key;
				sqlite->execUpdate(query.str(), transaction);
			}
		};



// IMPLEMENTATIONS ==============================================================================


		template<class K>
		std::string SQLiteTableSyncTemplate<K>::_fieldsGetter;


		template <class K>
		std::string SQLiteTableSyncTemplate<K>::_GetIndexName(
			const SQLiteTableSync::Index& index
		){
			std::stringstream s;
			s << TABLE.NAME;
			BOOST_FOREACH(const std::string& field, index.fields)
			{
				s << "_" << field;
			}
			return s.str();
		}




		template <class K>
		std::string synthese::db::SQLiteTableSyncTemplate<K>::_GetSQLIndexSchema(
			const SQLiteTableSync::Index& index
		){
			// Creation of the statement
			std::stringstream s;
			s	<< "CREATE INDEX " << _GetIndexName(index)
				<< " ON " << TABLE.NAME << "(";
			BOOST_FOREACH(const std::string& field, index.fields)
			{
				if (field != index.fields[0])
					s << ",";
				s << field;
			}
			s << ")";

			return s.str();
		}



		template <class K>
		bool SQLiteTableSyncTemplate<K>::_HasField(
			const std::string& name
		){
			for(size_t i(0); !_FIELDS[i].empty(); ++i)
			{
				if(_FIELDS[i].name == name) return true;
			}
			return false;
		}



		template <class K>
		std::string SQLiteTableSyncTemplate<K>::_GetSQLTriggerNoUpdate(
		) {
			std::vector<std::string> nonUpdatableColumns;
			for(size_t i(0); !_FIELDS[i].empty(); ++i)
			{
				if(_FIELDS[i].updatable) continue;

				nonUpdatableColumns.push_back(_FIELDS[i].name);
			}

			// If no non updatable field, return empty trigger
			if (nonUpdatableColumns.empty()) return std::string();

			std::stringstream columnList;
			BOOST_FOREACH(const std::string& field, nonUpdatableColumns)
			{
				if (!columnList.str().empty())
					columnList << ", ";
				columnList << field;
			}

			std::stringstream sql;
			sql << "CREATE TRIGGER "
				<< TABLE.NAME << "_no_update"
				<< " BEFORE UPDATE OF "
				<< columnList.str() << " ON " << TABLE.NAME
				<< " BEGIN SELECT RAISE (ABORT, 'Update of "
				<< columnList.str() << " in " << TABLE.NAME << " is forbidden.') WHERE "
				<< TABLE.TRIGGER_OVERRIDE_CLAUSE << "; END";

			return sql.str();
		}



		template <class K>
		void synthese::db::SQLiteTableSyncTemplate<K>::_UpdateSchema(
			SQLite* sqlite
		){
			std::string tableSchema = _GetSQLFieldsSchema();
			std::string triggerNoUpdate = _GetSQLTriggerNoUpdate();

			// * Fields getter
			std::stringstream fieldsGetter;
			for(size_t i(0); !_FIELDS[i].empty(); ++i)
			{
				if(i>0)
				{
					fieldsGetter << ",";
				}
				if(_FIELDS[i].isGeometry())
				{
					fieldsGetter << "AsBinary(";
					if(DBModule::GetStorageCoordinatesSystem().getSRID() != CoordinatesSystem::GetInstanceCoordinatesSystem().getSRID())
					{
						fieldsGetter << "Transform(";
					}
					fieldsGetter << _FIELDS[i].name;
					if(DBModule::GetStorageCoordinatesSystem().getSRID() != CoordinatesSystem::GetInstanceCoordinatesSystem().getSRID())
					{
						fieldsGetter << "," << CoordinatesSystem::GetInstanceCoordinatesSystem().getSRID() << ")";
					}
					fieldsGetter << ") AS " << _FIELDS[i].name;
				}
				else
				{
					fieldsGetter << _FIELDS[i].name;
				}
			}
			_fieldsGetter = fieldsGetter.str();


			// Check if the table already exists
			std::string sql = "SELECT * FROM SQLITE_MASTER WHERE name='" + TABLE.NAME + "' AND type='table'";
			SQLiteResultSPtr res = sqlite->execQuery (sql);
			if (res->next () == false)
			{
				// Create the table if it does not already exist.
				sqlite->execUpdate (tableSchema);

				// Insert some triggers to prevent non allowed operations
				if (!triggerNoUpdate.empty()) sqlite->execUpdate (triggerNoUpdate);
			}
			else if(
				(SQLiteTableSync::GetSQLSchemaDb(sqlite, TABLE.NAME) != tableSchema) ||
				SQLiteTableSync::GetTriggerNoUpdateDb(sqlite, TABLE.NAME) != triggerNoUpdate
			){
				std::vector<std::string> dbCols = SQLiteTableSync::GetTableColumnsDb(sqlite, TABLE.NAME);

				// Filter columns that are not in new table format
				std::stringstream colsStr;
				std::stringstream filteredColsStr;

				BOOST_FOREACH(const std::string& dbCol, dbCols)
				{
					if (!colsStr.str().empty()) colsStr << ",";
					colsStr << dbCol;

					if (_HasField(dbCol))
					{
						if (!filteredColsStr.str().empty()) filteredColsStr << ",";
						filteredColsStr << dbCol;
					}
				}

				std::string buTableName = TABLE.NAME + "_backup";
				std::stringstream str;
				str << "BEGIN TRANSACTION; ";

				// Drop triggers
				if (!SQLiteTableSync::GetTriggerNoUpdateDb(sqlite, TABLE.NAME).empty())
				{
					str << "DROP TRIGGER " << TABLE.NAME << "_no_update ;";
				}

				// Backup of old data in a temporary table
				str << "CREATE TEMPORARY TABLE " << buTableName << " (" 
					<< colsStr.str () << "); "
					<< "INSERT INTO " << buTableName 
					<< " SELECT " << colsStr.str () << " FROM " << TABLE.NAME << "; "

					// Deletion of the table with the old schema
					<< "DROP TABLE " << TABLE.NAME << "; "

					// Creation of the table with the new schema
					<< _GetTableCreationSQL()

					// Restoration of the data in the table
					<< "INSERT INTO " << TABLE.NAME << " (" << filteredColsStr.str () << ")"
					<< " SELECT " << filteredColsStr.str () << " FROM " << buTableName << "; "

					// Deletion of the temporary table
					<< "DROP TABLE " << buTableName << "; "
				;

				// Redefine triggers if necessary
				if(	!triggerNoUpdate.empty())
				{
					str << triggerNoUpdate << " ;";
				}

				str << "COMMIT;";

				sqlite->execUpdate (str.str ());
			}


			// Indexes
			for(size_t i(0); !_INDEXES[i].empty(); ++i)
			{
				// Search if the index exists
				std::stringstream sql;
				sql << "SELECT sql FROM SQLITE_MASTER WHERE name='" << _GetIndexName(_INDEXES[i])
					<< "' AND type='index'";

				SQLiteResultSPtr res = sqlite->execQuery (sql.str());

				// The index already exists
				if (res->next ())
				{
					// The index already exists and is identical : do nothing
					if (res->getText("sql") == _GetSQLIndexSchema(_INDEXES[i])) continue;

					// Drop the old index
					std::stringstream drop;
					drop << "DROP INDEX " << _GetIndexName(_INDEXES[i]);
					sqlite->execUpdate(drop.str());
				}

				// Creation of the index
				sqlite->execUpdate(_GetSQLIndexSchema(_INDEXES[i]));
			}
		}



		template <class K>
		std::string synthese::db::SQLiteTableSyncTemplate<K>::_GetSQLFieldsSchema(
		){
			std::stringstream sql;
			sql << "CREATE TABLE " << TABLE.NAME << " ("
				<< _FIELDS[0].name << " " << _FIELDS[0].getSQLType()
				<< " UNIQUE PRIMARY KEY ON CONFLICT ROLLBACK";

			for(size_t i(1); !_FIELDS[i].empty(); ++i)
			{
				sql << ", ";
				if(_FIELDS[i].isGeometry())
				{
					sql << "\"";
				}
				sql << _FIELDS[i].name;
				if(_FIELDS[i].isGeometry())
				{
					sql << "\"";
				}
				sql << " " << _FIELDS[i].getSQLType();
			}
			sql << ")";
			return sql.str();
		}



		template <class K>
		std::string synthese::db::SQLiteTableSyncTemplate<K>::_GetTableCreationSQL()
		{
			std::stringstream sql;

			// Init and primary key
			sql << "CREATE TABLE " << TABLE.NAME << " ("
				<< _FIELDS[0].name << " " << _FIELDS[0].getSQLType()
				<< " UNIQUE PRIMARY KEY ON CONFLICT ROLLBACK";

			// Non geometry columns
			size_t i(1);
			for(; !_FIELDS[i].empty() && !_FIELDS[i].isGeometry(); ++i)
			{
				sql << ", ";
				sql << _FIELDS[i].name;
				sql << " " << _FIELDS[i].getSQLType();
			}
			sql << ");";

			// Geometry columns
			for(; !_FIELDS[i].empty() && _FIELDS[i].isGeometry(); ++i)
			{
				sql << "SELECT AddGeometryColumn('" << TABLE.NAME << "','" <<
					_FIELDS[i].name << "'," << DBModule::GetStorageCoordinatesSystem().getSRID() <<
					",'" << _FIELDS[i].getSQLType() << "',2);" <<
					"SELECT CreateSpatialIndex('" << TABLE.NAME << "','" <<
					_FIELDS[i].name << "');"
				;
			}

			// Control
			if(!_FIELDS[i].empty())
			{
				util::Log::GetInstance().error(
					"Fields are present after geometry column and will be ignored (possible data loss)."
				);
			}

			return sql.str();
		}


		template <class K>
		boost::shared_ptr<boost::mutex> SQLiteTableSyncTemplate<K>::_idMutex(new boost::mutex); 

		template <class K>
		util::RegistryObjectType SQLiteTableSyncTemplate<K>::_autoIncrementValue(1); 
	}
}

#endif // SYNTHESE_SQLiteTableSyncTemplate_H__

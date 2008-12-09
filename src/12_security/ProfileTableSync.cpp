/** ProfileTableSync class implementation.
	@file ProfileTableSync.cpp

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

#include "ProfileTableSync.h"
#include "UserTableSyncException.h"
#include "Right.h"
#include "Profile.h"

#include "DBModule.h"
#include "SQLiteResult.h"
#include "SQLite.h"
#include "SQLiteException.h"

#include "Conversion.h"
#include "01_util/Log.h"
#include "Factory.h"

#include <sstream>

#include <boost/tokenizer.hpp>


using namespace std;
using boost::shared_ptr;

namespace synthese
{
	using namespace util;
	using namespace db;
	using namespace security;

	namespace util
	{
		template<> const string FactorableTemplate<SQLiteTableSync,ProfileTableSync>::FACTORY_KEY("12.01 Profile");
	}

	namespace db
	{
		template<> const std::string SQLiteTableSyncTemplate<ProfileTableSync>::TABLE_NAME = "t027_profiles";
		template<> const int SQLiteTableSyncTemplate<ProfileTableSync>::TABLE_ID = 27;
		template<> const bool SQLiteTableSyncTemplate<ProfileTableSync>::HAS_AUTO_INCREMENT = true;

		template<> void SQLiteDirectTableSyncTemplate<ProfileTableSync,Profile>::Load(
			Profile* profile,
			const db::SQLiteResultSPtr& rows,
			Env* env,
			LinkLevel linkLevel
		){
			profile->setName(rows->getText ( ProfileTableSync::TABLE_COL_NAME));
			profile->setParent(rows->getLongLong ( ProfileTableSync::TABLE_COL_PARENT_ID));
			ProfileTableSync::setRightsFromString(profile, rows->getText ( ProfileTableSync::TABLE_COL_RIGHTS_STRING));
		}



		template<> void SQLiteDirectTableSyncTemplate<ProfileTableSync,Profile>::Save(Profile* profile )
		{
			try
			{
				SQLite* sqlite = DBModule::GetSQLite();
				if (profile->getKey() <= 0)
					profile->setKey(getId());
				
				stringstream query;
				query
					<< "REPLACE INTO " << TABLE_NAME
					<< " VALUES(" 
					<< Conversion::ToString(profile->getKey())
					<< "," << Conversion::ToSQLiteString(profile->getName())
					<< "," << Conversion::ToString(profile->getParentId())
					<< "," << Conversion::ToSQLiteString(ProfileTableSync::getRightsString(profile))
					<< ")";
				sqlite->execUpdate(query.str());
			}
			catch (SQLiteException& e)
			{
				throw UserTableSyncException("Insert/Update error " + e.getMessage());
			}
			catch (...)
			{
				throw UserTableSyncException("Unknown Insert/Update error");
			}
		}


		template<> void SQLiteDirectTableSyncTemplate<ProfileTableSync,Profile>::Unlink(Profile* profile, Env* env)
		{
		}
	}

	namespace security
	{
		const std::string ProfileTableSync::RIGHT_SEPARATOR = "|";
		const std::string ProfileTableSync::RIGHT_VALUE_SEPARATOR = ",";

		const std::string ProfileTableSync::TABLE_COL_NAME = "name";
		const std::string ProfileTableSync::TABLE_COL_PARENT_ID = "parent";
		const std::string ProfileTableSync::TABLE_COL_RIGHTS_STRING = "rights";

		ProfileTableSync::ProfileTableSync()
			: db::SQLiteRegistryTableSyncTemplate<ProfileTableSync,Profile>()
		{
			addTableColumn(TABLE_COL_ID, "INTEGER", false);
			addTableColumn(TABLE_COL_NAME, "TEXT", true);
			addTableColumn(TABLE_COL_PARENT_ID, "INTEGER", true);
			addTableColumn(TABLE_COL_RIGHTS_STRING, "TEXT", true);
		}


		void ProfileTableSync::Search(
			Env& env,
			std::string name
			, string right
			, int first /*= 0*/
			, int number /*= -1*/ 
			, bool orderByName
			, bool raisingOrder,
			LinkLevel linkLevel
		){
			stringstream query;
			query
				<< " SELECT *"
				<< " FROM " << TABLE_NAME					
				<< " WHERE 1 ";
			if (!name.empty())
				query << " AND " << TABLE_COL_NAME << " LIKE " << Conversion::ToSQLiteString(name);
			if (!right.empty())
				query << " AND " << TABLE_COL_RIGHTS_STRING << " LIKE " << Conversion::ToSQLiteString(right);
			if (orderByName)
				query << " ORDER BY " << TABLE_COL_NAME << (raisingOrder ? " ASC" : " DESC");
			if (number > 0)
				query << " LIMIT " << Conversion::ToString(number + 1);
			if (first > 0)
				query << " OFFSET " << Conversion::ToString(first);

			LoadFromQuery(query.str(), env, linkLevel);
		}



		void ProfileTableSync::Search(
			Env& env,
			shared_ptr<const Profile> parent
			, int first /*= 0*/,
			int number, /*= -1*/
			LinkLevel linkLevel
		){
			stringstream query;
			query
				<< " SELECT *"
				<< " FROM " << TABLE_NAME					
				<< " WHERE " 
				<< TABLE_COL_PARENT_ID << "=" << (parent.get() ? Conversion::ToString(parent->getKey()) : "0");
			if (number > 0)
				query << " LIMIT " << Conversion::ToString(number + 1);
			if (first > 0)
				query << " OFFSET " << Conversion::ToString(first);

			LoadFromQuery(query.str(), env, linkLevel);
		}



		std::string ProfileTableSync::getRightsString(const Profile* p)
		{
			stringstream s;
			
			for (RightsVector::const_iterator it = p->getRights().begin(); it != p->getRights().end(); ++it)
			{
				shared_ptr<const Right> right = it->second;
				if (it != p->getRights().begin())
					s	<< RIGHT_SEPARATOR;
				s	<< right->getFactoryKey() 
					<< RIGHT_VALUE_SEPARATOR << right->getParameter()
					<< RIGHT_VALUE_SEPARATOR << (static_cast<int>(right->getPrivateRightLevel()))
					<< RIGHT_VALUE_SEPARATOR << (static_cast<int>(right->getPublicRightLevel()))
					;
			}
			return s.str();
		}

		void ProfileTableSync::setRightsFromString(Profile* profile, const std::string& text )
		{
			typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
			boost::char_separator<char> sep(RIGHT_SEPARATOR.c_str ());

			// CLeaning the profile
			profile->cleanRights();

			// Parsing
			tokenizer parametersTokens (text, sep);
			for (tokenizer::iterator parameterToken = parametersTokens.begin();
				parameterToken != parametersTokens.end (); ++ parameterToken)
			{
				tokenizer valuesToken(*parameterToken, boost::char_separator<char>(RIGHT_VALUE_SEPARATOR.c_str()));
				tokenizer::iterator it = valuesToken.begin();

				try
				{
					shared_ptr<Right> right(Factory<Right>::create(*it));

					++it;
					right->setParameter(*it);

					++it;
					right->setPrivateLevel(static_cast<RightLevel>(Conversion::ToInt(*it)));

					++it;
					right->setPublicLevel(static_cast<RightLevel>(Conversion::ToInt(*it)));

					profile->addRight(right);
				}
				catch (FactoryException<Right> e)
				{
					continue;
				}
			}
		}
	}
}

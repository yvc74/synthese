
/** UserTableSync class header.
	@file UserTableSync.h

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

#ifndef SYNTHESE_UserTableSync_H__
#define SYNTHESE_UserTableSync_H__

#include <boost/logic/tribool.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>
#include <string>
#include <iostream>

#include "SQLiteNoSyncTableSyncTemplate.h"

namespace synthese
{
	namespace security
	{
		class User;

		/** User SQLite table synchronizer.
			@ingroup m12LS refLS
			@todo Update the opened session on user update
		*/

		class UserTableSync : public db::SQLiteNoSyncTableSyncTemplate<UserTableSync,User>
		{
		public:
			static const std::string TABLE_COL_LOGIN;
			static const std::string TABLE_COL_NAME;
			static const std::string TABLE_COL_SURNAME;
			static const std::string TABLE_COL_PASSWORD;
			static const std::string TABLE_COL_PROFILE_ID;
			static const std::string TABLE_COL_ADDRESS;
			static const std::string TABLE_COL_POST_CODE;
			static const std::string TABLE_COL_CITY_TEXT;
			static const std::string TABLE_COL_CITY_ID;
			static const std::string TABLE_COL_COUNTRY;
			static const std::string TABLE_COL_EMAIL;
			static const std::string TABLE_COL_PHONE;
			static const std::string COL_LOGIN_AUTHORIZED;
			static const std::string COL_BIRTH_DATE;


			UserTableSync();

			/** Gets a user in the database, founded by his login.
				@param login login to search
				@return boost::shared_ptr<User> Shared pointer to a new user linked-object.
				@author Hugues Romain
				@date 2007
			*/
			static boost::shared_ptr<User> getUserFromLogin(const std::string& login);

			static bool loginExists(const std::string& login);

			/** User search.
				@param login login to search : use LIKE syntax
				@param name name to search : use LIKE syntax
				@param surname name to search : use LIKE syntax
				@param phone phone to search (LIKE syntax)
				@param profileId Profile ID which user must belong (UNKNOWN_VALUE = filter deactivated)
				@param emptyLogin User without login acceptation (true = user must have a login, false = user must not have a login, indeterminate = filter deactivated)
				@param first First user to answer
				@param number Number of users to answer (-1 = all) The size of the vector is less or equal to number, then all users were returned despite of the number limit. If the size is greater than number (actually equal to number + 1) then there is others users to show. Test it to know if the situation needs a "click for more" button.
				@param orderByLogin Order the results by login
				@param orderByName Order the results by name and surname
				@param orderByProfile Order the results by profile name
				@param raisingOrder True = Ascendant order, false = descendant order
				@return vector<share_ptr<User>> Founded vector of shared pointers to User linked-objects. 
				@warning only one of the orderBy parameters must be true, or no one. More of one true value will throw an exception.
				@throw UserTableSyncException If the query has failed (does not occurs in normal case)
				@author Hugues Romain
				@date 2006				
			*/
			static void Search(
				util::Env& env,
				const std::string login = std::string("%")
				, const std::string name = std::string("%")
				, const std::string surname = std::string("%")
				, const std::string phone = std::string("%")
				, uid profileId = UNKNOWN_VALUE
				, boost::logic::tribool emptyLogin = boost::logic::indeterminate
				, int first = 0
				, int number = -1
				, bool orderByLogin = true
				, bool orderByName = false
				, bool orderByProfileName = false
				, bool raisingOrder = true
				, util::LinkLevel linkLevel = util::UP_LINKS_LOAD_LEVEL
			);
		};

	}
}
#endif // SYNTHESE_UserTableSync_H__


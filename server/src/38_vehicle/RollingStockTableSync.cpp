
/** RollingStockTableSync class implementation.
	@file RollingStockTableSync.cpp
	@author Hugues Romain
	@date 2007

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

#include "RollingStockTableSync.hpp"

#include "DataSourceLinksField.hpp"
#include "ImportableTableSync.hpp"
#include "Profile.h"
#include "ReplaceQuery.h"
#include "SelectQuery.hpp"
#include "Session.h"
#include "User.h"

#include <boost/foreach.hpp>

using namespace std;
using namespace boost;

namespace synthese
{
	using namespace db;
	using namespace util;
	using namespace vehicle;
	using namespace security;
	using namespace impex;

	namespace util
	{
		template<> const string FactorableTemplate<DBTableSync,RollingStockTableSync>::FACTORY_KEY("35.10.07 Rolling Stock");
	}

	namespace db
	{
		template<> const DBTableSync::Format DBTableSyncTemplate<RollingStockTableSync>::TABLE(
			"t049_rolling_stock"
		);

		template<> const Field DBTableSyncTemplate<RollingStockTableSync>::_FIELDS[]=
		{
			Field()
		};

		template<>
		DBTableSync::Indexes DBTableSyncTemplate<RollingStockTableSync>::GetIndexes()
		{
			DBTableSync::Indexes r;
			r.push_back(DBTableSync::Index(TridentKey::FIELD.name.c_str(), IsTridentReference::FIELD.name.c_str(), ""));
			return r;
		}



		template<> bool DBTableSyncTemplate<RollingStockTableSync>::CanDelete(
			const server::Session* session,
			util::RegistryKeyType object_id
		){
			return true; // TODO create a vehicle right
//			return session && session->hasProfile() && session->getUser()->getProfile()->isAuthorized<TransportNetworkRight>(DELETE_RIGHT);
		}



		template<> void DBTableSyncTemplate<RollingStockTableSync>::BeforeDelete(
			util::RegistryKeyType id,
			db::DBTransaction& transaction
		){
		}



		template<> void DBTableSyncTemplate<RollingStockTableSync>::AfterDelete(
			util::RegistryKeyType id,
			db::DBTransaction& transaction
		){
		}



		template<> void DBTableSyncTemplate<RollingStockTableSync>::LogRemoval(
			const server::Session* session,
			util::RegistryKeyType id
		){
			//TODO Log the removal
		}
	}



	namespace vehicle
	{
		RollingStockTableSync::SearchResult RollingStockTableSync::Search(
			Env& env,
			optional<string> tridentKey,
			bool tridentReference,
			bool orderByName,
			bool raisingOrder,
			int first /*= 0*/,
			int number /*= 0*/,
			LinkLevel linkLevel
		){
			SelectQuery<RollingStockTableSync> query;
			if(tridentKey)
			{
				query.addWhereField(TridentKey::FIELD.name, *tridentKey);
			}
			if(tridentReference)
			{
				query.addWhereField(IsTridentReference::FIELD.name, 1);
			}
			if(orderByName)
			{
				query.addOrderField(SimpleObjectFieldDefinition<Name>::FIELD.name, raisingOrder);
			}
			query.setNumber(number);
			query.setFirst(first);

			return LoadFromQuery(query, env, linkLevel);
		}



		RollingStockTableSync::SearchResult RollingStockTableSync::SearchUsedModes(
			Env& env,
			LinkLevel linkLevel
		){
			stringstream query;

			query 	<< "SELECT *"
					<< "FROM " << TABLE.NAME 
					<< " WHERE id IN (SELECT DISTINCT rolling_stock_id FROM t009_lines)";

			return LoadFromQuery(query.str(), env, linkLevel);
		}



		RollingStockTableSync::Labels RollingStockTableSync::GetLabels(
			std::string unknownLabel
		){
			Labels result;
			if(!unknownLabel.empty())
			{
				result.push_back(make_pair(optional<RegistryKeyType>(),unknownLabel));
			}
			SearchResult modes(Search(Env::GetOfficialEnv()));
			BOOST_FOREACH(SearchResult::value_type& mode, modes)
			{
				result.push_back(make_pair(mode->getKey(), mode->getName()));
			}
			return result;
		}



		db::RowsList RollingStockTableSync::SearchForAutoComplete(
				const boost::optional<std::string> prefix,
				const boost::optional<std::size_t> limit,
				const boost::optional<std::string> optionalParameter
			) const {
				RowsList result;

				SelectQuery<RollingStockTableSync> query;
				Env env;
				if(prefix) query.addWhereField(SimpleObjectFieldDefinition<Name>::FIELD.name, "%"+ *prefix +"%", ComposedExpression::OP_LIKE);
				if(limit) query.setNumber(*limit);
				query.addOrderField(SimpleObjectFieldDefinition<Name>::FIELD.name,true);
				RollingStockTableSync::SearchResult elements(RollingStockTableSync::LoadFromQuery(query, env, UP_LINKS_LOAD_LEVEL));
				BOOST_FOREACH(const boost::shared_ptr<RollingStock>& elem, elements)
				{
					result.push_back(std::make_pair(elem->getKey(), elem->getName()));
				}
				return result;
		} ;
	}
}


/** InterSYNTHESEDB class implementation.
	@file InterSYNTHESEDB.cpp

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

#include "DBInterSYNTHESE.hpp"

#include "DBException.hpp"
#include "DBDirectTableSync.hpp"
#include "DBModule.h"
#include "DBTransaction.hpp"
#include "Env.h"
#include "Field.hpp"
#include "InterSYNTHESEContent.hpp"
#include "InterSYNTHESEModule.hpp"
#include "InterSYNTHESEQueue.hpp"
#include "InterSYNTHESESlave.hpp"

using namespace boost;
using namespace std;

namespace synthese
{
	using namespace db;
	using namespace inter_synthese;
	using namespace util;

	template<>
	const string FactorableTemplate<InterSYNTHESESyncTypeFactory, db::DBInterSYNTHESE>::FACTORY_KEY = "db";

	namespace db
	{
		const string DBInterSYNTHESE::TYPE_SQL = "sql";
		const string DBInterSYNTHESE::TYPE_REPLACE_STATEMENT = "rstmt";
		const string DBInterSYNTHESE::TYPE_DELETE_STATEMENT = "dstmt";
		const string DBInterSYNTHESE::FIELD_SEPARATOR = ":";

		DBInterSYNTHESE::DBInterSYNTHESE():
			FactorableTemplate<InterSYNTHESESyncTypeFactory, DBInterSYNTHESE>()
		{
		}



		bool DBInterSYNTHESE::sync( const std::string& parameter ) const
		{
			size_t i(0);
			for(; i<parameter.size() && parameter[i]!=FIELD_SEPARATOR[0]; ++i) ;
			if(i == parameter.size())
			{
				return false;
			}
			string reqType(parameter.substr(0, i));
			if(reqType == TYPE_SQL)
			{
				try
				{
					string sql(parameter.substr(i+1));
					trim(sql);
					if(sql.substr(0, 11) == "DELETE FROM")
					{
						DB& db(*DBModule::GetDB());
						db.execUpdate(sql);
					}
					else if(sql.substr(0, 12) == "REPLACE INTO")
					{
						size_t j(13);
						for(; j<sql.size() && sql[j]!=' '; ++j) ;
						string tableName = sql.substr(13, j-13);

						vector<string> p;
						split(p, sql, is_any_of("(,"));
						if(p.size() > 2)
						{
							RegistryKeyType id(lexical_cast<RegistryKeyType>(p[1]));
						
							DB& db(*DBModule::GetDB());
							db.execUpdate(sql);

							db.addDBModifEvent(
								DB::DBModifEvent(
									tableName,
				// TODO remove MODIF_UPDATE and _objectAdded attribute or find an other way to choose between the two types
				//					_objectAdded ? DB::MODIF_INSERT : DB::MODIF_UPDATE,
									DB::MODIF_INSERT,
									id
								),
								optional<DBTransaction&>()
							);

							// Inter-SYNTHESE sync
							inter_synthese::InterSYNTHESEContent content(
								DBInterSYNTHESE::FACTORY_KEY,
								lexical_cast<string>(decodeTableId(id)),
								DBInterSYNTHESE::GetSQLContent(sql)
							);
							inter_synthese::InterSYNTHESEModule::Enqueue(
								content,
								optional<DBTransaction&>()
							);
						}
					}
				}
				catch(...)
				{
					return false;
				}
			}
			else if(reqType == TYPE_DELETE_STATEMENT)
			{
				string idStr(parameter.substr(i+1));
				try
				{
					RegistryKeyType id(lexical_cast<RegistryKeyType>(idStr));
					RegistryTableType tableId(decodeTableId(id));
					string tableName(DBModule::GetTableSync(tableId)->getFormat().NAME);

					// STMT execution
					_transaction->addDeleteStmt(id);
/*					DB& db(*DBModule::GetDB());
					db.deleteRow(id);

					db.addDBModifEvent(
						DB::DBModifEvent(
							tableName,
							DB::MODIF_DELETE,
							id
						),
						optional<DBTransaction&>()
					);

					// Inter-SYNTHESE sync
					inter_synthese::InterSYNTHESEContent iSContent(
						DBInterSYNTHESE::FACTORY_KEY,
						lexical_cast<string>(tableId),
						DBInterSYNTHESE::GetDeleteStmtContent(id)
					);
					inter_synthese::InterSYNTHESEModule::Enqueue(
						iSContent,
						optional<DBTransaction&>()
					);
*/				}
				catch(...)
				{
					return false;
				}
			}
			else if(reqType == TYPE_REPLACE_STATEMENT)
			{
				++i;
				size_t l=i;
				for(; i<parameter.size() && parameter[i]!=FIELD_SEPARATOR[0]; ++i) ;
				if(i == parameter.size())
				{
					return false;
				}
				
				try
				{
					// Table number
					string tableNumberStr(parameter.substr(l, i-l));
					RegistryTableType tableId(lexical_cast<RegistryTableType>(tableNumberStr));
					RegistryKeyType id(0);
					DBRecord r(*DBModule::GetTableSync(tableId));
					string tableName(r.getTable()->getFormat().NAME);
					++i;

					// Fields loop
					typedef std::map<
						std::string,
						boost::optional<std::string>
					> StmtFields;

					StmtFields stmtFields;

					while(i<parameter.size())
					{
						// Field name
						l = i;
						for(; i<parameter.size() && parameter[i]!=FIELD_SEPARATOR[0]; ++i) ;
						if(i == parameter.size())
						{
							return false;
						}
						string fieldName(parameter.substr(l, i-l));
						++i;

						// Is null
						if(i == parameter.size())
						{
							return false;
						}
						bool isNull(parameter[i] == '1');
						++i;
						if(i == parameter.size())
						{
							return false;
						}
						++i;

						if(isNull)
						{
							stmtFields.insert(make_pair(fieldName, optional<string>()));
							for(; i<parameter.size() && parameter[i]!=FIELD_SEPARATOR[0]; ++i) ;
						}
						else
						{
							// Size
							if(i == parameter.size())
							{
								return false;
							}
							l = i;
							for(; i<parameter.size() && parameter[i]!=FIELD_SEPARATOR[0]; ++i) ;
							size_t dataSize(lexical_cast<size_t>(parameter.substr(l, i-l)));
							++i;

							// Content
							if(i + dataSize > parameter.size())
							{
								return false;
							}
							string content(parameter.substr(i, dataSize));
							stmtFields.insert(make_pair(fieldName, content));
							i+=dataSize;

							// Saving of id
							if(fieldName == TABLE_COL_ID)
							{
								id = lexical_cast<RegistryKeyType>(content);
							}
						}
						++i;
					}

					// Content extraction
					DBContent content;
					BOOST_FOREACH(const FieldsList::value_type& field, r.getTable()->getFieldsList())
					{
						StmtFields::const_iterator it(stmtFields.find(field.name));
						if(it == stmtFields.end())
						{
							return false;
						}

						if(!it->second)
						{
							content.push_back(Cell(optional<string>()));
						}
						else
						{
							switch(field.type)
							{
							case SQL_INTEGER:
								content.push_back(Cell(lexical_cast<RegistryKeyType>(*it->second)));
								break;

							case SQL_DOUBLE:
								content.push_back(Cell(lexical_cast<double>(*it->second)));
								break;

							case SQL_BOOLEAN:
								content.push_back(Cell(lexical_cast<bool>(*it->second)));
								break;

							default:
								content.push_back(Cell(it->second));
								break;

							}
						}
					}

					r.setContent(content);

					_transaction->addReplaceStmt(r);
					// STMT execution
/*					DB& db(*DBModule::GetDB());
					db.saveRecord(r);

					db.addDBModifEvent(
						DB::DBModifEvent(
							tableName,
							DB::MODIF_INSERT,
							id
						),
						optional<DBTransaction&>()
					);

					// Inter-SYNTHESE sync
					inter_synthese::InterSYNTHESEContent iSContent(
						DBInterSYNTHESE::FACTORY_KEY,
						lexical_cast<string>(tableId),
						DBInterSYNTHESE::GetReplaceStmtContent(r)
					);
					inter_synthese::InterSYNTHESEModule::Enqueue(
						iSContent,
						optional<DBTransaction&>()
					);
*/				}
				catch (DBException&)
				{
					return false;
				}
			}
			else
			{
				return false;
			}

			return true;
		}



		void DBInterSYNTHESE::initQueue(
			const InterSYNTHESESlave& slave,
			const std::string& perimeter
		) const	{

			try
			{
				// Detection of the table by id
				RegistryTableType tableId(
					lexical_cast<RegistryTableType>(perimeter)
				);
				shared_ptr<DBTableSync> tableSync(
					DBModule::GetTableSync(
						tableId
				)	);

				shared_ptr<DBDirectTableSync> directTableSync(
					dynamic_pointer_cast<DBDirectTableSync, DBTableSync>(
						tableSync
				)	);
				if(!directTableSync.get())
				{
					return;
				}

				// Getting all requests
				Env env;
				DBDirectTableSync::RegistrableSearchResult result(
					directTableSync->search(
						string(),
						env
				)	);
				DBTransaction transaction;

				// Add the clean request
				transaction.addQuery("DELETE FROM "+ tableSync->getFormat().NAME);

				// Build the dump
				BOOST_FOREACH(const DBDirectTableSync::RegistrableSearchResult::value_type& it, result)
				{
					directTableSync->saveRegistrable(*it, transaction);
				}

				// Export
				DBTransaction saveTransaction;
				BOOST_FOREACH(const DBTransaction::Query& query, transaction.getQueries())
				{
					stringstream content;
					RequestEnqueue visitor(content);
					apply_visitor(visitor, query);
					slave.enqueue(
						DBInterSYNTHESE::FACTORY_KEY,
						content.str(),
						saveTransaction,
						true
					);
				}
				saveTransaction.run();
			}
			catch (bad_lexical_cast&)
			{
			}
			catch(DBException&)
			{
			}
		}



		string DBInterSYNTHESE::GetSQLContent( const std::string& sql )
		{
			stringstream content;
			RequestEnqueue visitor(content);
			visitor(sql);
			return content.str();
		}



		string DBInterSYNTHESE::GetReplaceStmtContent(
			const DBRecord& r
		){
			stringstream content;
			RequestEnqueue visitor(content);
			visitor(r);
			return content.str();
		}



		std::string DBInterSYNTHESE::GetDeleteStmtContent( util::RegistryKeyType id )
		{
			stringstream content;
			RequestEnqueue visitor(content);
			visitor(id);
			return content.str();
		}



		DBInterSYNTHESE::RequestEnqueue::RequestEnqueue(
			std::stringstream& result
		):	_result(result)
		{}



		void DBInterSYNTHESE::RequestEnqueue::operator()( const std::string& sql )
		{
			_result <<
				DBInterSYNTHESE::TYPE_SQL << DBInterSYNTHESE::FIELD_SEPARATOR <<
				sql;
		}



		void DBInterSYNTHESE::RequestEnqueue::operator()( const DBRecord& r )
		{
			_result <<
				DBInterSYNTHESE::TYPE_REPLACE_STATEMENT << DBInterSYNTHESE::FIELD_SEPARATOR <<
				r.getTable()->getFormat().ID << DBInterSYNTHESE::FIELD_SEPARATOR;

			ContentGetter visitor(_result);
			DBContent::const_iterator it(r.getContent().begin());
			BOOST_FOREACH(const FieldsList::value_type& field, r.getTable()->getFieldsList())
			{
				// Name
				_result << field.name << DBInterSYNTHESE::FIELD_SEPARATOR;

				// Size and content
				apply_visitor(visitor, *it);
				_result << DBInterSYNTHESE::FIELD_SEPARATOR;

				// Next content
				++it;
			}
		}



		void DBInterSYNTHESE::RequestEnqueue::operator()(
			util::RegistryKeyType id
		){
			_result <<
				DBInterSYNTHESE::TYPE_DELETE_STATEMENT << DBInterSYNTHESE::FIELD_SEPARATOR <<
				id
			;
		}



		DBInterSYNTHESE::ContentGetter::ContentGetter(
			std::stringstream& result
		):	_result(result)
		{}



		void DBInterSYNTHESE::ContentGetter::operator()( const int& i ) const
		{
			string s(lexical_cast<string>(i));
			_result <<
				0 << DBInterSYNTHESE::FIELD_SEPARATOR <<
				s.size() << DBInterSYNTHESE::FIELD_SEPARATOR <<
				s
			;
		}

		
#ifndef _WINDOWS
		void DBInterSYNTHESE::ContentGetter::operator()( const size_t& i ) const
		{
			string s(lexical_cast<string>(i));
			_result <<
				0 << DBInterSYNTHESE::FIELD_SEPARATOR <<
				s.size() << DBInterSYNTHESE::FIELD_SEPARATOR <<
				s
			;
		}
#endif


		void DBInterSYNTHESE::ContentGetter::operator()( const double& d ) const
		{
			string s(lexical_cast<string>(d));
			_result <<
				0 << DBInterSYNTHESE::FIELD_SEPARATOR <<
				s.size() << DBInterSYNTHESE::FIELD_SEPARATOR <<
				s
			;
		}



		void DBInterSYNTHESE::ContentGetter::operator()( const util::RegistryKeyType& id ) const
		{
			string s(lexical_cast<string>(id));
			_result <<
				0 << DBInterSYNTHESE::FIELD_SEPARATOR <<
				s.size() << DBInterSYNTHESE::FIELD_SEPARATOR <<
				s
			;
		}



		void DBInterSYNTHESE::ContentGetter::operator()( const boost::optional<std::string>& str ) const
		{
			if(str)
			{
				_result <<
					0 << DBInterSYNTHESE::FIELD_SEPARATOR <<
					str->size() << DBInterSYNTHESE::FIELD_SEPARATOR <<
					*str
				;
			}
			else
			{
				_result <<
					1 << DBInterSYNTHESE::FIELD_SEPARATOR <<
					0 << DBInterSYNTHESE::FIELD_SEPARATOR
				;
			}
		}



		void DBInterSYNTHESE::ContentGetter::operator()( const boost::optional<Blob>& blob ) const
		{
			if(blob)
			{
				_result <<
					0 << DBInterSYNTHESE::FIELD_SEPARATOR <<
					blob->second << DBInterSYNTHESE::FIELD_SEPARATOR;
				for(size_t i(0); i<blob->second; ++i)
				{
					_result.put(blob->first[i]);
				}
			}
			else
			{
				_result <<
					1 << DBInterSYNTHESE::FIELD_SEPARATOR <<
					0 << DBInterSYNTHESE::FIELD_SEPARATOR
				;
			}
		}



		
		bool DBInterSYNTHESE::mustBeEnqueued(
			const std::string& configPerimeter,
			const std::string& messagePerimeter
		) const {
			return configPerimeter == messagePerimeter &&
				messagePerimeter != InterSYNTHESEQueue::TABLE_NAME;
		}



		void DBInterSYNTHESE::initSync() const
		{
			assert(!_transaction.get());

			_transaction.reset(new DBTransaction);
		}



		void DBInterSYNTHESE::closeSync() const
		{
			assert(_transaction.get());

			_transaction->run();
			_transaction.reset();
		}
}	}


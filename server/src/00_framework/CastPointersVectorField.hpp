
/** CastPointerVectorField class header.
	@file CastPointersVectorField.hpp

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

#ifndef SYNTHESE__CastPointersVectorField_hpp__
#define SYNTHESE__CastPointersVectorField_hpp__

#include "SimpleObjectFieldDefinition.hpp"

#include "DBModule.h" // Temporary modules dependencies rule violation : will be useless when Env::getEditable will be able to chose a registry dynamically.

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <vector>

namespace synthese
{
	//////////////////////////////////////////////////////////////////////////
	/// Cast Pointers vector field.
	/// @ingroup m00
	template<class C, class T>
	class CastPointersVectorField:
		public SimpleObjectFieldDefinition<C>
	{
	public:
		typedef std::vector<T*> Type;

	public:
		//////////////////////////////////////////////////////////////////////////
		/// Conversion of a date into a string to be stored (SQL format).
		/// @param d the date to convert
		/// @return the converted string
		static std::string ToString(const typename CastPointersVectorField<C, T>::Type& p)
		{
			std::stringstream s;
			bool first(true);
			BOOST_FOREACH(T* ptr, p)
			{
				if(!ptr)
				{
					continue;
				}
				if(first)
				{
					first = false;
				}
				else
				{
					s << ",";
				}
				s << ptr->getKey();
			}
			return s.str();
		}


	public:
		static bool LoadFromRecord(
			typename CastPointersVectorField<C, T>::Type& fieldObject,
			ObjectBase& object,
			const Record& record,
			const util::Env& env
		){
			if(!record.isDefined(SimpleObjectFieldDefinition<C>::FIELD.name))
			{
				return false;
			}

			typename CastPointersVectorField<C, T>::Type value;
			std::string text(record.get<std::string>(SimpleObjectFieldDefinition<C>::FIELD.name));
			if(!text.empty())
			{
				std::vector<std::string> s;
				boost::algorithm::split(s, text, boost::is_any_of(","));
				BOOST_FOREACH(const std::string& item, s)
				{
					try
					{
						T* lObject(
							dynamic_cast<T*>(
								db::DBModule::GetEditableObject( // Temporary modules dependencies rule violation : will be useless when Env::getEditable will be able to chose a registry dynamically.
									boost::lexical_cast<util::RegistryKeyType>(item),
									const_cast<util::Env&>(env)
								).get()
						) );
						if(!lObject)
						{
							util::Log::GetInstance().warn(
								"Data corrupted in the "+ SimpleObjectFieldDefinition<C>::FIELD.name +" field at the load of the "+
								object.getClassName() +" object " + boost::lexical_cast<std::string>(object.getKey()) +" : item " +
								item + " was not convertible into the specified type."
							);
							continue;
						}
						value.push_back(
							lObject
						);
					}
					catch(boost::bad_lexical_cast&)
					{
						util::Log::GetInstance().warn(
							"Data corrupted in the "+ SimpleObjectFieldDefinition<C>::FIELD.name +" field at the load of the "+
							object.getClassName() +" object " + boost::lexical_cast<std::string>(object.getKey()) +" : " +
							item + " is not a valid id."
						);
					}
					catch(util::ObjectNotFoundException<util::Registrable>&)
					{
						util::Log::GetInstance().warn(
							"Data corrupted in the "+ SimpleObjectFieldDefinition<C>::FIELD.name +" field at the load of the "+
							object.getClassName() +" object " + boost::lexical_cast<std::string>(object.getKey()) +" : item " +
							item + " was not found."
						);
					}
				}
			}

			if(value == fieldObject)
			{
				return false;
			}
			else
			{
				fieldObject = value;
				return true;
			}
		}



		static void SaveToFilesMap(
			const typename CastPointersVectorField<C, T>::Type& fieldObject,
			const ObjectBase& object,
			FilesMap& map
		){
			SimpleObjectFieldDefinition<C>::_SaveToFilesMap(
				fieldObject,
				map,
				ToString
			);
		}



		static void SaveToParametersMap(
			const typename CastPointersVectorField<C, T>::Type& fieldObject,
			const ObjectBase& object,
			util::ParametersMap& map,
			const std::string& prefix,
			boost::logic::tribool withFiles
		){
			SimpleObjectFieldDefinition<C>::_SaveToParametersMap(
				fieldObject,
				map,
				prefix,
				withFiles,
				ToString
			);
		}



		static void SaveToParametersMap(
			const typename CastPointersVectorField<C, T>::Type& fieldObject,
			util::ParametersMap& map,
			const std::string& prefix,
			boost::logic::tribool withFiles
		){
			SimpleObjectFieldDefinition<C>::_SaveToParametersMap(
				fieldObject,
				map,
				prefix,
				withFiles,
				ToString
			);
		}




		static void SaveToDBContent(
			const typename CastPointersVectorField<C, T>::Type& fieldObject,
			const ObjectBase& object,
			DBContent& content
		){
			std::string s(ToString(fieldObject));
			content.push_back(Cell(s));
		}



		static void GetLinkedObjectsIds(
			LinkedObjectsIds& list, 
			const Record& record
		){
			std::string text(record.getDefault<std::string>(SimpleObjectFieldDefinition<C>::FIELD.name));
			if(text.empty())
			{
				return;
			}
			std::vector<std::string> s;
			boost::algorithm::split(s, text, boost::is_any_of(","));
			BOOST_FOREACH(const std::string& item, s)
			{
				try
				{
					util::RegistryKeyType id(
						boost::lexical_cast<util::RegistryKeyType>(item)
					);
					if(id > 0)
					{
						list.push_back(id);
					}
				}
				catch(boost::bad_lexical_cast&)
				{
				}
		}	}
	};

	#define FIELD_CAST_POINTERS_VECTOR(N, T) struct N : public CastPointersVectorField<N, T> {};
}

#endif

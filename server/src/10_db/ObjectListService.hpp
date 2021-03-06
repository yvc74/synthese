
//////////////////////////////////////////////////////////////////////////
/// ObjectListService class header.
///	@file ObjectListService.hpp
///	@author Marc Jambert
///	@date 2014
///
///	This file belongs to the SYNTHESE project (public transportation specialized software)
///	Copyright (C) 2002 Hugues Romain - RCSmobility <contact@rcsmobility.com>
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
///	along with this program; if not, write to the Free Software
///	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef SYNTHESE_ObjectListService_H__
#define SYNTHESE_ObjectListService_H__

#include "Function.h"
#include "FactorableTemplate.h"
#include "ParametersMap.h"

#include <boost/logic/tribool.hpp>

namespace synthese
{
	class ObjectBase;

	namespace db
	{
		class DBDirectTableSync;

		//////////////////////////////////////////////////////////////////////////
		/// 10.15 Function : ObjectListService.
		/// @ingroup m10Functions refFunctions
		///	@author Marc Jambert
		///	@date 2015
		/// @since 3.10.0
		//////////////////////////////////////////////////////////////////////////
		class ObjectListService:
			public util::FactorableTemplate<server::Function, ObjectListService>
		{
		public:
			static const std::string PARAMETER_TABLE_ID;

			static const std::string ATTR_ID;

		protected:
			boost::shared_ptr<DBDirectTableSync> _tableSync;
			boost::shared_ptr<ObjectBase> _value;
			util::ParametersMap _values;

		protected:
			//////////////////////////////////////////////////////////////////////////
			/// Generates a generic parameters map from the service parameters.
			/// @return The generated parameters map
			util::ParametersMap _getParametersMap() const;



			//////////////////////////////////////////////////////////////////////////
			/// Reads the parameters of the service on a generic parameters map.
			/// @param map Parameters map to interpret
			/// @exception Exception Occurs when some parameters are missing or incorrect.
			virtual void _setFromParametersMap(const util::ParametersMap& map);

		public:


			//////////////////////////////////////////////////////////////////////////
			/// The action execution code.
			/// @param request the request which has launched the action
			virtual util::ParametersMap run(std::ostream& stream, const server::Request& request) const;



			//////////////////////////////////////////////////////////////////////////
			/// Tests if the service can be launched in the current session.
			/// @param session the current session
			/// @return true if the service can be launched in the current session
			virtual bool isAuthorized(const server::Session* session) const;



			//////////////////////////////////////////////////////////////////////////
			/// Gets the Mime type of the content generated by the function.
			/// @return the Mime type of the content generated by the function
			/// @author User
			/// @date 2014
			virtual std::string getOutputMimeType() const;



			//! @name Setters
			//@{
				void setTableId(
					util::RegistryTableType tableId
				);

				template<class ObjectClass>
				void setTable()
				{
					setTableId(ObjectClass::CLASS_NUMBER);
				}

			//@}
		};
	}
}

#endif // SYNTHESE_ObjectListService_H__


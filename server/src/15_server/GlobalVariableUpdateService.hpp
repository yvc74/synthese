
//////////////////////////////////////////////////////////////////////////
/// GlobalVariableUpdateService class header.
///	@file GlobalVariableUpdateService.hpp
///	@author Hugues Romain
///	@date 2012
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

#ifndef SYNTHESE_GlobalVariableUpdateService_H__
#define SYNTHESE_GlobalVariableUpdateService_H__

#include "Function.h"
#include "FactorableTemplate.h"

namespace synthese
{
	namespace server
	{
		//////////////////////////////////////////////////////////////////////////
		/// 15.15 Action : GlobalVariableUpdateService.
		/// @ingroup m15Actions refActions
		///	@author Hugues Romain
		///	@date 2012
		/// @since 3.5.0
		//////////////////////////////////////////////////////////////////////////
		/// Key : GlobalVariableUpdateService
		///
		/// Parameters :
		///	<dl>
		///	<dt>actionParamid</dt><dd>id of the object to update</dd>
		///	</dl>
		class GlobalVariableUpdateService:
			public util::FactorableTemplate<server::Function, GlobalVariableUpdateService>
		{
		public:
			static const std::string PARAMETER_VARIABLE;
			static const std::string PARAMETER_VALUE;
			static const std::string PARAMETER_PERSISTENT;
			static const std::string TAG_VARIABLES;
			static const std::string TAG_KEY;
			static const std::string TAG_VALUE;

		private:
			std::string _variable;
			std::string _value;
			bool _persistent;

		protected:
			//////////////////////////////////////////////////////////////////////////
			/// Generates a generic parameters map from the action parameters.
			/// @return The generated parameters map
			util::ParametersMap _getParametersMap() const;



			//////////////////////////////////////////////////////////////////////////
			/// Reads the parameters of the action on a generic parameters map.
			/// @param map Parameters map to interpret
			/// @exception ActionException Occurs when some parameters are missing or incorrect.
			void _setFromParametersMap(const util::ParametersMap& map);

		public:
			GlobalVariableUpdateService();

			//////////////////////////////////////////////////////////////////////////
			/// The action execution code.
			/// @param request the request which has launched the action
			virtual util::ParametersMap run(std::ostream& stream, const server::Request& request) const;
			


			//////////////////////////////////////////////////////////////////////////
			/// Tests if the action can be launched in the current session.
			/// @param session the current session
			/// @return true if the action can be launched in the current session
			virtual bool isAuthorized(const server::Session* session) const;

			//////////////////////////////////////////////////////////////////////////
			/// Gets the Mime type of the content generated by the function.
			/// @return the Mime type of the content generated by the function
			/// @author Hugues Romain
			/// @date 2012
			virtual std::string getOutputMimeType() const;

			server::FunctionAPI getAPI() const;

			//! @name Setters
			//@{
				// void setObject(boost::shared_ptr<Object> value) { _object = value; }
			//@}
		};
	}
}

#endif // SYNTHESE_GlobalVariableUpdateService_H__


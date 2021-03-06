
//////////////////////////////////////////////////////////////////////////////////////////
///	MessagesSectionsService class header.
///	@file MessagesSectionsService.hpp
///	@author hromain
///	@date 2013
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

#ifndef SYNTHESE_MessagesSectionsService_H__
#define SYNTHESE_MessagesSectionsService_H__

#include "FactorableTemplate.h"
#include "Function.h"

namespace synthese
{
	namespace messages
	{
		class MessagesSection;
		
		//////////////////////////////////////////////////////////////////////////
		///	17.15 Function : MessagesSectionsService.
		/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Messages_sections
		//////////////////////////////////////////////////////////////////////////
		///	@ingroup m17Functions refFunctions
		///	@author hromain
		///	@date 2013
		/// @since 3.9.0
		class MessagesSectionsService:
			public util::FactorableTemplate<server::Function,MessagesSectionsService>
		{
		public:
			static const std::string TAG_SECTION;
			
		protected:
			//! \name Page parameters
			//@{
			//@}
			
			struct ElementLess : public std::binary_function<const MessagesSection*, const MessagesSection*, bool>
			{
				bool operator()(const MessagesSection* left, const MessagesSection* right) const;
			};
			
			//////////////////////////////////////////////////////////////////////////
			/// Conversion from attributes to generic parameter maps.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Messages_sections#Request
			//////////////////////////////////////////////////////////////////////////
			///	@return Generated parameters map
			/// @author hromain
			/// @date 2013
			/// @since 3.9.0
			util::ParametersMap _getParametersMap() const;
			
			
			
			//////////////////////////////////////////////////////////////////////////
			/// Conversion from generic parameters map to attributes.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/Messages_sections#Request
			//////////////////////////////////////////////////////////////////////////
			///	@param map Parameters map to interpret
			/// @author hromain
			/// @date 2013
			/// @since 3.9.0
			virtual void _setFromParametersMap(
				const util::ParametersMap& map
			);
			
		private:
			static void _outputSection(util::ParametersMap& pm, const MessagesSection& section);
			

		public:
			//////////////////////////////////////////////////////////////////////////
			/// Display of the content generated by the function.
			/// @param stream Stream to display the content on.
			/// @param request the current request
			/// @author hromain
			/// @date 2013
			virtual util::ParametersMap run(std::ostream& stream, const server::Request& request) const;
			
			
			
			//////////////////////////////////////////////////////////////////////////
			/// Gets if the function can be run according to the user of the session.
			/// @param session the current session
			/// @return true if the function can be run
			/// @author hromain
			/// @date 2013
			virtual bool isAuthorized(const server::Session* session) const;



			//////////////////////////////////////////////////////////////////////////
			/// Gets the Mime type of the content generated by the function.
			/// @return the Mime type of the content generated by the function
			/// @author hromain
			/// @date 2013
			virtual std::string getOutputMimeType() const;
		};
}	}

#endif // SYNTHESE_MessagesSectionsService_H__


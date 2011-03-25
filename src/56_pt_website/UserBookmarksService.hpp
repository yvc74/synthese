
//////////////////////////////////////////////////////////////////////////////////////////
/// UserBookmarksService class header.
///	@file UserBookmarksService.hpp
///	@author RCSobility
///	@date 2011
///
///	This file belongs to the SYNTHESE project (public transportation specialized software)
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
///	along with this program; if not, write to the Free Software
///	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef SYNTHESE_UserBookmarksService_H__
#define SYNTHESE_UserBookmarksService_H__

#include "FactorableTemplate.h"
#include "Function.h"

namespace synthese
{
	namespace security
	{
		class User;
	}

	namespace cms
	{
		class Webpage;
	}

	namespace pt_website
	{
		//////////////////////////////////////////////////////////////////////////
		///	56.15 Function : UserBookmarksService.
		/// See https://extranet-rcsmobility.com/projects/synthese/wiki/User_bookmarks
		//////////////////////////////////////////////////////////////////////////
		///	@ingroup m56Functions refFunctions
		///	@author Hugues Romain
		///	@date 2011
		/// @since 3.2.1
		class UserBookmarksService:
			public util::FactorableTemplate<server::Function,UserBookmarksService>
		{
		public:
			static const std::string PARAMETER_USER_ID;
			static const std::string PARAMETER_ITEM_DISPLAY_TEMPLATE_ID;
			
		protected:
			static const std::string DATA_RANK;
			static const std::string DATA_ORIGIN_CITY_NAME;
			static const std::string DATA_ORIGIN_PLACE_NAME;
			static const std::string DATA_DESTINATION_CITY_NAME;
			static const std::string DATA_DESTINATION_PLACE_NAME;

			//! \name Page parameters
			//@{
				boost::shared_ptr<const security::User> _user;
				boost::shared_ptr<const cms::Webpage> _itemDisplayTemplate;
			//@}
			
			
			//////////////////////////////////////////////////////////////////////////
			/// Conversion from attributes to generic parameter maps.
			/// See https://extranet-rcsmobility.com/projects/synthese/wiki/User_bookmarks#Request
			//////////////////////////////////////////////////////////////////////////
			///	@return Generated parameters map
			/// @author RCSobility
			/// @date 2011
			/// @since 3.2.1
			server::ParametersMap _getParametersMap() const;
			
			
			
			//////////////////////////////////////////////////////////////////////////
			/// Conversion from generic parameters map to attributes.
			/// See https://extranet-rcsmobility.com/projects/synthese/wiki/User_bookmarks#Request
			//////////////////////////////////////////////////////////////////////////
			///	@param map Parameters map to interpret
			/// @author RCSobility
			/// @date 2011
			/// @since 3.2.1
			virtual void _setFromParametersMap(
				const server::ParametersMap& map
			);
			
			
		public:
			//! @name Setters
			//@{
			//	void setObject(boost::shared_ptr<const Object> value) { _object = value; }
			//@}



			//////////////////////////////////////////////////////////////////////////
			/// Display of the content generated by the function.
			/// See https://extranet-rcsmobility.com/projects/synthese/wiki/User_bookmarks#Response
			//////////////////////////////////////////////////////////////////////////
			/// @param stream Stream to display the content on.
			/// @param request the current request
			/// @author RCSobility
			/// @date 2011
			virtual void run(std::ostream& stream, const server::Request& request) const;
			
			
			
			//////////////////////////////////////////////////////////////////////////
			/// Gets if the function can be run according to the user of the session.
			/// @param session the current session
			/// @return true if the function can be run
			/// @author RCSobility
			/// @date 2011
			virtual bool isAuthorized(const server::Session* session) const;



			//////////////////////////////////////////////////////////////////////////
			/// Gets the Mime type of the content generated by the function.
			/// @return the Mime type of the content generated by the function
			/// @author RCSobility
			/// @date 2011
			virtual std::string getOutputMimeType() const;
		};
	}
}

#endif // SYNTHESE_UserBookmarksService_H__

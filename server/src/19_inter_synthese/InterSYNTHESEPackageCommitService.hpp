
//////////////////////////////////////////////////////////////////////////////////////////
///	InterSYNTHESEPackageCommitService class header.
///	@file InterSYNTHESEPackageCommitService.hpp
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

#ifndef SYNTHESE_InterSYNTHESEPackageCommitService_H__
#define SYNTHESE_InterSYNTHESEPackageCommitService_H__

#include "FactorableTemplate.h"
#include "Function.h"

namespace synthese
{
	namespace security
	{
		class User;
	}

	namespace inter_synthese
	{
		class InterSYNTHESEPackageContent;
		class InterSYNTHESEPackage;

		//////////////////////////////////////////////////////////////////////////
		///	19.15 Function : InterSYNTHESEPackageCommitService.
		/// See https://extranet.rcsmobility.com/projects/synthese/wiki/
		//////////////////////////////////////////////////////////////////////////
		///	@ingroup m19Functions refFunctions
		///	@author hromain
		///	@date 2013
		/// @since 3.9.0
		class InterSYNTHESEPackageCommitService:
			public util::FactorableTemplate<server::Function,InterSYNTHESEPackageCommitService>
		{
		public:
			static const std::string PARAMETER_PACKAGE_ID;
			static const std::string PARAMETER_USER;
			static const std::string PARAMETER_PASSWORD;
			static const std::string PARAMETER_RELEASE_LOCK;
			static const std::string PARAMETER_CONTENT;
			static const std::string PARAMETER_CREATE_PACKAGE;
			
		protected:
			//! \name Page parameters
			//@{
				boost::shared_ptr<security::User> _user; // Here because the user is read at the save query
				InterSYNTHESEPackage* _package;
				std::string _userLogin;
				std::string _password;
				bool _releaseLock;
				std::string _contentStr;
				std::auto_ptr<InterSYNTHESEPackageContent> _content;
				bool _createPackage;
			//@}
			
			
			//////////////////////////////////////////////////////////////////////////
			/// Conversion from attributes to generic parameter maps.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/#Request
			//////////////////////////////////////////////////////////////////////////
			///	@return Generated parameters map
			/// @author hromain
			/// @date 2013
			/// @since 3.9.0
			util::ParametersMap _getParametersMap() const;
			
			
			
			//////////////////////////////////////////////////////////////////////////
			/// Conversion from generic parameters map to attributes.
			/// See https://extranet.rcsmobility.com/projects/synthese/wiki/#Request
			//////////////////////////////////////////////////////////////////////////
			///	@param map Parameters map to interpret
			/// @author hromain
			/// @date 2013
			/// @since 3.9.0
			virtual void _setFromParametersMap(
				const util::ParametersMap& map
			);
			
			
		public:
			InterSYNTHESEPackageCommitService();


			//! @name Setters
			//@{
				void setPackage(InterSYNTHESEPackage* value){ _package = value; }
				void setUser(const std::string& value){ _userLogin = value; }
				void setPassword(const std::string& value){ _password = value; }
				void setContentStr(const std::string& value){ _contentStr = value; }
				void setReleaseLock(bool value){ _releaseLock = value; }
				void setCreatePackage(bool value){ _createPackage = value; }
			//@}



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

#endif // SYNTHESE_InterSYNTHESEPackageCommitService_H__



//////////////////////////////////////////////////////////////////////////////////////////
///	InterSYNTHESEObjectCreateService class header.
///	@file InterSYNTHESEObjectCreateService.hpp
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

#ifndef SYNTHESE_InterSYNTHESEObjectCreateService_H__
#define SYNTHESE_InterSYNTHESEObjectCreateService_H__

#include <boost/thread/thread.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "FactorableTemplate.h"
#include "Function.h"
#include "ObjectCreateService.hpp"

namespace synthese
{
	namespace inter_synthese
	{

		//////////////////////////////////////////////////////////////////////////
		///	19.15 Function : InterSYNTHESEObjectCreateService.
		//////////////////////////////////////////////////////////////////////////
		///	@ingroup m19Functions refFunctions
		///	@author Thomas Puigt
		///	@date 2014
		///	@since 3.8.0
		class InterSYNTHESEObjectCreateService:
			public util::FactorableTemplate<db::ObjectCreateService,InterSYNTHESEObjectCreateService>
		{
		public:
            static const std::string QUEUE_IDS_SEPARATOR;
            static const std::string NO_ITEM_IN_QUEUE;

            static const std::string ATTR_QUEUEIDS;

            static const std::string FORMAT_JSON;
            static const std::string PARAMETER_GETQUEUEID;
            static const std::string TAG_OBJECT_AND_INTERSYNTHESE_KEYS;

            typedef std::set<util::RegistryKeyType> QueueIds;

		private:
			//! \name Page parameters
			//@{
				bool _queueIdsRequested;
			//@}
			
			
			//////////////////////////////////////////////////////////////////////////
			/// Conversion from attributes to generic parameter maps.
			//////////////////////////////////////////////////////////////////////////
			///	@return Generated parameters map
			///	@author Thomas Puigt
			///	@date 2014
			///	@since 3.8.0
			util::ParametersMap _getParametersMap() const;
			
			
			
			//////////////////////////////////////////////////////////////////////////
			/// Conversion from generic parameters map to attributes.
			//////////////////////////////////////////////////////////////////////////
			///	@param map Parameters map to interpret
			///	@author Thomas Puigt
			///	@date 2014
			///	@since 3.8.0
			virtual void _setFromParametersMap(
				const util::ParametersMap& map
			);
			


		public:
			InterSYNTHESEObjectCreateService();



			//! @name Setters
			//@{
                void setQueueIdsRequested(bool isRequested){ _queueIdsRequested = isRequested; }
			//@}



			//////////////////////////////////////////////////////////////////////////
			/// Display of the content generated by the function.
			/// @param stream Stream to display the content on.
			/// @param request the current request
			/// @author Thomas Puigt
			/// @date 2014
            virtual util::ParametersMap run(std::ostream& stream, const server::Request& request) const;
			
			
			
			//////////////////////////////////////////////////////////////////////////
			/// Gets if the function can be run according to the user of the session.
			/// @param session the current session
			/// @return true if the function can be run
			/// @author Thomas Puigt
			/// @date 2014
			virtual bool isAuthorized(const server::Session* session) const;



            //////////////////////////////////////////////////////////////////////////
            /// Gets the Mime type of the content generated by the function.
            /// @return the Mime type of the content generated by the function
            /// @author Thomas Puigt
            /// @date 2014
            virtual std::string getOutputMimeType() const;



		};
}	}

#endif // SYNTHESE_InterSYNTHESEObjectCreateService_H__



/** BookableCommercialLineAdmin class header.
	@file BookableCommercialLineAdmin.h
	@author Hugues Romain
	@date 2008

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

#ifndef SYNTHESE_BookableCommercialLineAdmin_H__
#define SYNTHESE_BookableCommercialLineAdmin_H__

#include "AdminInterfaceElementTemplate.h"

#include "DateTime.h"

namespace synthese
{
	namespace env
	{
		class CommercialLine;
		class ScheduledService;
	}

	namespace resa
	{
		/** BookableCommercialLineAdmin Class.
			@ingroup m31Admin refAdmin
			@author Hugues Romain
			@date 2008
		*/
		class BookableCommercialLineAdmin : public admin::AdminInterfaceElementTemplate<BookableCommercialLineAdmin>
		{
			bool											_displayCancelled;
			boost::shared_ptr<const env::CommercialLine>	_line;
			time::Date										_date;
			bool											_hideOldServices;
			boost::shared_ptr<const env::ScheduledService>	_service;

		public:
			static const std::string PARAMETER_DISPLAY_CANCELLED;
			static const std::string PARAMETER_DATE;
			static const std::string PARAMETER_SERVICE;

			BookableCommercialLineAdmin();
			
			void setService(boost::shared_ptr<const env::ScheduledService> value);
			void setCommercialLine(boost::shared_ptr<env::CommercialLine> value);
			boost::shared_ptr<const env::CommercialLine> getCommercialLine() const;

			
			/** Initialization of the parameters from a parameters map.
				@param map The parameters map to use for the initialization.
				@throw AdminParametersException if a parameter has incorrect value.
				@author Hugues Romain
				@date 2008
			*/
			void setFromParametersMap(
				const server::ParametersMap& map,
				bool doDisplayPreparationActions = true
			);
			
			
			
			/** Parameters map generator, used when building an url to the admin page.
					@return server::ParametersMap The generated parameters map
					@author Hugues Romain
					@date 2007					
				*/
			virtual server::ParametersMap getParametersMap() const;



			/** Display of the content of the admin element.
				@param stream Stream to write on.
				@author Hugues Romain
				@date 2008
			*/
			void display(std::ostream& stream, interfaces::VariablesMap& variables
			) const;
			
			/** Authorization control.
				@return bool True if the displayed page can be displayed
				@author Hugues Romain
				@date 2008
			*/
			bool isAuthorized() const;
			
			
			
			/** Title generator.
				@return The title of the page
				@author Hugues Romain
				@date 2008
			*/
			virtual std::string getTitle() const;
		};
	}
}

#endif // SYNTHESE_BookableCommercialLineAdmin_H__

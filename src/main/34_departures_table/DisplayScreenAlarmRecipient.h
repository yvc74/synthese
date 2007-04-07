
/** DisplayScreenAlarmRecipient class header.
	@file DisplayScreenAlarmRecipient.h

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

#ifndef SYNTHESE_DisplayScreenAlarmRecipient_h__
#define SYNTHESE_DisplayScreenAlarmRecipient_h__

#include "17_messages/AlarmRecipientTemplate.h"
#include "DisplayScreen.h"

namespace synthese
{

	namespace departurestable
	{

		/** DisplayScreenAlarmRecipient class.
			@ingroup m34 refAlarmrecipients
		*/
		class DisplayScreenAlarmRecipient : public messages::AlarmRecipientTemplate<DisplayScreen>
		{
		public:
			static const std::string PARAMETER_SEARCH_UID;
			static const std::string PARAMETER_SEARCH_PLACE;
			static const std::string PARAMETER_SEARCH_LINE;
			static const std::string PARAMETER_SEARCH_TYPE;
			static const std::string PARAMETER_SEARCH_STATUS;
			static const std::string PARAMETER_SEARCH_MESSAGE;

			DisplayScreenAlarmRecipient();

			void displayBroadcastListEditor(std::ostream& stream, const messages::Alarm* alarm, const server::ParametersMap& request, server::FunctionRequest<admin::AdminRequest>& searchRequest, server::FunctionRequest<admin::AdminRequest>& addRequest, server::FunctionRequest<admin::AdminRequest>& removeRequest);

			virtual messages::AlarmRecipientSearchFieldsMap getSearchFields(html::HTMLForm& form, const server::ParametersMap& parameters) const;

			void addObject(const messages::Alarm* alarm, uid objectId);
			void removeObject(const messages::Alarm* alarm, uid objectId);
		};
	}
}

#endif // SYNTHESE_DisplayScreenAlarmRecipient_h__

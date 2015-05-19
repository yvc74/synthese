/** FileNotificationChannel implementation
	@file FileNotificationChannel.cpp
	@author Yves Martin
	@date 2015

	This file belongs to the SYNTHESE project (public transportation specialized software)
	Copyright (C) 2015 Hugues Romain - RCSmobility <contact@rcsmobility.com>

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

#include <FileNotificationChannel.hpp>
#include <ParametersMap.h>

#include <string>

using namespace std;

namespace synthese
{
	using namespace util;
	using namespace messages;

	namespace util
	{
		template<> const string FactorableTemplate<NotificationChannel, FileNotificationChannel>::FACTORY_KEY("File");
	}

	namespace messages
	{
		const string FileNotificationChannel::PARAMETER_FILE_PATH = "file_path";
		const string FileNotificationChannel::PARAMETER_FILE_CONTENT = "file_content";



		FileNotificationChannel::FileNotificationChannel()
		{
			// Prepare internal state for notifications
		};



		// Provide its own script fields list
		std::vector<std::string> FileNotificationChannel::_getScriptParameterNames() const
		{
			vector<string> result;
			result.push_back(PARAMETER_FILE_PATH);
			result.push_back(PARAMETER_FILE_CONTENT);
			return result;
		}



		bool FileNotificationChannel::notify(const boost::shared_ptr<NotificationEvent> event)
		{
			// TODO notify
			return true;
		}
	}
}

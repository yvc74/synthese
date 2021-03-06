﻿
/** MessageTag class implementation.
	@file MessageTag.cpp

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

#include "MessageTag.hpp"

#include "MessagesRight.h"
#include "Profile.h"
#include "Session.h"
#include "User.h"

namespace synthese
{
	using namespace messages;
	using namespace util;

	CLASS_DEFINITION(MessageTag, "t123_message_tags", 123)

	namespace messages
	{
		MessageTag::MessageTag(
			RegistryKeyType id /*= 0*/
		):	Registrable(id),
			Object<MessageTag, MessageTagRecord>(
		Schema(
			FIELD_VALUE_CONSTRUCTOR(Key, id),
			FIELD_DEFAULT_CONSTRUCTOR(Name)
		)	)
		{
		}


		bool MessageTag::allowUpdate(const server::Session* session) const
		{
			return session && session->hasProfile() && session->getUser()->getProfile()->isAuthorized<MessagesRight>(security::WRITE);
		}

		bool MessageTag::allowCreate(const server::Session* session) const
		{
			return session && session->hasProfile() && session->getUser()->getProfile()->isAuthorized<MessagesRight>(security::WRITE);
		}

		bool MessageTag::allowDelete(const server::Session* session) const
		{
			return session && session->hasProfile() && session->getUser()->getProfile()->isAuthorized<MessagesRight>(security::DELETE_RIGHT);
		}
}	}

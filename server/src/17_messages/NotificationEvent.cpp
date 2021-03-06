/** NotificationEvent class implementation.
	@file NotificationEvent.cpp
	@author Ivan Ivanov
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

#include "Alarm.h"
#include "AlarmTableSync.h"
#include "Env.h"
#include "Field.hpp"
#include "Log.h"
#include "NotificationEvent.hpp"
#include "NotificationEventTableSync.hpp"
#include "NotificationProvider.hpp"
#include "NotificationProviderTableSync.hpp"
#include "Registry.h"
#include "UtilTypes.h"

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/special_defs.hpp>
#include <boost/fusion/container/map.hpp>
#include <boost/optional/optional.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

using namespace boost;
using namespace std;

namespace synthese
{
	using namespace db;
	using namespace messages;
	using namespace util;

	CLASS_DEFINITION(NotificationEvent, "t108_notification_events", 108)

	FIELD_DEFINITION_OF_OBJECT(NotificationEvent, "notification_event_id", "notification_event_ids")

	FIELD_DEFINITION_OF_TYPE(EventType, "type", SQL_INTEGER)
	FIELD_DEFINITION_OF_TYPE(Status, "status", SQL_INTEGER)
	FIELD_DEFINITION_OF_TYPE(Expiration, "expiration", SQL_DATETIME)
	FIELD_DEFINITION_OF_TYPE(Attempts, "attempts", SQL_INTEGER)
	FIELD_DEFINITION_OF_TYPE(LastAttempt, "last_attempt", SQL_DATETIME)

	namespace messages {

		const char* NotificationEvent::TYPE_NAMES[] =
			{
				"NONE",
				"BEGIN",
				"END",
				"REMINDER",
				"BEFORE_UPDATE",
				"UPDATE"
			};

		/// Default constructor as a Registrable
		/// @param id registry key type
		NotificationEvent::NotificationEvent(
			RegistryKeyType id /*= 0*/
		) : Registrable(id),
			Object<NotificationEvent, NotificationEventRecord>(
				Schema(
					FIELD_VALUE_CONSTRUCTOR(Key, id),
					FIELD_DEFAULT_CONSTRUCTOR(Alarm),
					FIELD_DEFAULT_CONSTRUCTOR(NotificationProvider),
					FIELD_VALUE_CONSTRUCTOR(EventType, NONE),
					FIELD_VALUE_CONSTRUCTOR(Status, READY),
					FIELD_VALUE_CONSTRUCTOR(Time, posix_time::second_clock::local_time()),
					FIELD_VALUE_CONSTRUCTOR(Expiration, posix_time::not_a_date_time),
					FIELD_VALUE_CONSTRUCTOR(Attempts, 0),
					FIELD_VALUE_CONSTRUCTOR(LastAttempt, posix_time::not_a_date_time)
				)	)
		{ };

		NotificationEvent::NotificationEvent(
			RegistryKeyType id,
			Alarm& alarm,
			NotificationProvider& notificationProvider,
			const NotificationType eventType,
			const boost::posix_time::ptime& expiration /* = boost::posix_time::not_a_date_time */
		) : Registrable(id),
			Object<NotificationEvent, NotificationEventRecord>(
				Schema(
					FIELD_VALUE_CONSTRUCTOR(Key, id),
					FIELD_VALUE_CONSTRUCTOR(Alarm, boost::optional<Alarm&>(alarm)),
					FIELD_VALUE_CONSTRUCTOR(NotificationProvider, boost::optional<NotificationProvider&>(notificationProvider)),
					FIELD_VALUE_CONSTRUCTOR(EventType, eventType),
					FIELD_VALUE_CONSTRUCTOR(Status, READY),
					FIELD_VALUE_CONSTRUCTOR(Time, posix_time::second_clock::local_time()),
					FIELD_VALUE_CONSTRUCTOR(Expiration, expiration),
					FIELD_VALUE_CONSTRUCTOR(Attempts, 0),
					FIELD_VALUE_CONSTRUCTOR(LastAttempt, posix_time::not_a_date_time)
				)	)
		{ };



		bool NotificationEvent::isExpired() const
		{
			return !get<Expiration>().is_not_a_date_time()
				&& get<Expiration>() < posix_time::second_clock::local_time();
		}



		void NotificationEvent::markSuccessful()
		{
			this->set<Status>(SUCCESS);

		}



		boost::shared_ptr<NotificationEvent> NotificationEvent::createEvent(
			Env& env,
			const Alarm& alarm,
			const NotificationProvider* provider,
			const NotificationType type,
			const bool saveNow /* = true */
		) {
			boost::shared_ptr<NotificationProvider> itself(
				NotificationProviderTableSync::GetEditable(provider->getKey(), env));
			boost::shared_ptr<Alarm> alarmRef(
				AlarmTableSync::GetEditable(alarm.getKey(), env));

			boost::shared_ptr<NotificationEvent> event(new NotificationEvent(
				NotificationEventTableSync::getId(),
				*alarmRef,
				*itself,
				type
			)	);

			env.getEditableRegistry<NotificationEvent>().add(event);
			if (saveNow)
			{
				NotificationEventTableSync::Save(event.get());
			}

			return event;
		}



		boost::shared_ptr<NotificationEvent> NotificationEvent::findLastEvent(
			const Alarm& alarm,
			const NotificationProvider* provider,
			const NotificationType type
		) {
			boost::shared_ptr<NotificationEvent> result;
			Env env = Env::GetOfficialEnv();
			NotificationEventTableSync::SearchResult resultEvents(
				NotificationEventTableSync::Search(
					env, alarm.getKey(), provider->getKey(), type, 0, 1
			)	);

			if (resultEvents.size() > 0)
			{
				result = resultEvents[0];
			}
			return result;
		}



		boost::shared_ptr<NotificationEvent> NotificationEvent::findOrCreateEvent(
			const Alarm& alarm,
			const NotificationProvider* provider,
			const NotificationType type,
			const bool hold_event /* = false */
		) {
			bool creation = false;

			boost::shared_ptr<NotificationEvent> result = findLastEvent(alarm, provider, type);
			if (result)
			{
				// If last event corresponds to a previous application
				// period of the same alarm, create a new event
				if (!result->get<LastAttempt>().is_not_a_date_time()
					 && result->get<Status>() >= FAILED)
				{
					if (type == BEGIN
						&& result->get<Time>() < alarm.getLastActivationStart())
					{
						creation = true;
					}
					if (type == END
						&& result->get<Time>() < alarm.getLastActivationEnd())
					{
						creation = true;
					}
					if (type == UPDATE || type == REMINDER)
					{
						creation = true;
					}
				}
			}
			else
			{
				creation = true; // no previous event
			}

			if(false == creation)
			{
				util::Log::GetInstance().debug("Notification event already exists for alarm " + alarm.getShortMessage());
			}
			else
			{
				result = createEvent(Env::GetOfficialEnv(), alarm, provider, type, false);
			}

			result->expireEvents();

			if (hold_event)
			{
				result->set<Status>(HOLD);
			}

			NotificationEventTableSync::Save(result.get());
			return result;
		}



		void NotificationEvent::expireEvents()
		{
			const posix_time::ptime now = posix_time::second_clock::local_time();
			const Alarm alarm = get<Alarm>().get();
			const NotificationProvider* provider = &(get<NotificationProvider>().get());
			const NotificationType type = get<EventType>();

			// Set or refresh event expiration for begin event only
			if (type != END)
			{
				if (!alarm.isApplicable(now))
				{
					// Alarm is no longer active right now. Should be discarded
					set<Expiration>(now);
				}
				else
				{
					// May be not_a_date_time is not end date specified
					set<Expiration>(alarm.getApplicationEnd(now));
				}
			}

			// Get previous event according to this event type
			boost::shared_ptr<NotificationEvent> previousEvent;
			if (type == BEGIN)
			{
				previousEvent = findLastEvent(alarm, provider, END);
			}
			else if (type == UPDATE || type == END)
			{
				previousEvent = findLastEvent(alarm, provider, BEGIN);
			}
			// Expire previous event
			if (previousEvent)
			{
				previousEvent->set<Expiration>(now);
				NotificationEventTableSync::Save(previousEvent.get());
			}
			// In case of END, also expire UPDATE event if any
			if (type == END)
			{
				previousEvent = findLastEvent(alarm, provider, UPDATE);
				if (previousEvent)
				{
					previousEvent->set<Expiration>(now);
					NotificationEventTableSync::Save(previousEvent.get());
				}
			}
		}

	}
}


/** ResaLogAdmin class implementation.
	@file ResaLogAdmin.cpp
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

#include "ResaLogAdmin.h"

#include "ResaModule.h"
#include "ResaDBLog.h"
#include "ResaRight.h"
#include "CancelReservationAction.h"
#include "Request.h"
#include "AdminRequest.h"
#include "AdminParametersException.h"
#include "ModuleAdmin.h"
#include "AdminInterfaceElement.h"
#include "SearchFormHTMLTable.h"
#include "DBLogEntry.h"
#include "DBLogEntryTableSync.h"

using namespace std;
using namespace boost;

namespace synthese
{
	using namespace admin;
	using namespace interfaces;
	using namespace server;
	using namespace util;
	using namespace resa;
	using namespace html;
	using namespace time;
	using namespace dblog;
	using namespace security;

	namespace util
	{
		template<> const string FactorableTemplate<AdminInterfaceElement, ResaLogAdmin>::FACTORY_KEY("ResaLogAdmin");
	}

	namespace admin
	{
		template<> const string AdminInterfaceElementTemplate<ResaLogAdmin>::ICON("book_open.png");
		template<> const string AdminInterfaceElementTemplate<ResaLogAdmin>::DEFAULT_TITLE("Journal");
	}

	namespace resa
	{
		ResaLogAdmin::ResaLogAdmin()
			: AdminInterfaceElementTemplate<ResaLogAdmin>(),
			_log("log")
		{ }
		
		void ResaLogAdmin::setFromParametersMap(
			const ParametersMap& map,
			bool doDisplayPreparationActions
		){
			_log.set(
				map,
				ResaDBLog::FACTORY_KEY
			);
		}
		
		
		
		server::ParametersMap ResaLogAdmin::getParametersMap() const
		{
			ParametersMap m;
			return m;
		}

		
		void ResaLogAdmin::display(ostream& stream, VariablesMap& variables) const
		{
			// Results
			_log.display(
				stream,
				FunctionRequest<AdminRequest>(_request),
				true,
				true
			);
		}

		bool ResaLogAdmin::isAuthorized(
		) const	{
			return _request->isAuthorized<ResaRight>(READ, UNKNOWN_RIGHT_LEVEL);
		}
		
		AdminInterfaceElement::PageLinks ResaLogAdmin::getSubPagesOfModule(
				const std::string& moduleKey,
				boost::shared_ptr<const AdminInterfaceElement> currentPage
		) const	{
			AdminInterfaceElement::PageLinks links;
			if(moduleKey == ResaModule::FACTORY_KEY)
			{
				if(dynamic_cast<const ResaLogAdmin*>(currentPage.get()))
				{
					AddToLinks(links, currentPage);
				}
				else
				{
					AddToLinks(links, getNewPage());
				}
			}
			return links;
		}
	}
}

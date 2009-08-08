
/** TimetableAdmin class implementation.
	@file TimetableAdmin.cpp
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

#include "TimetableAdmin.h"
#include "TimetableModule.h"

#include "Date.h"

#include "PropertiesHTMLTable.h"
#include "ActionResultHTMLTable.h"
#include "HTMLModule.h"

#include "PublicTransportStopZoneConnectionPlace.h"
#include "City.h"
#include "Calendar.h"
#include "Line.h"
#include "CommercialLine.h"

#include "Timetable.h"
#include "TimetableRight.h"
#include "TimetableTableSync.h"
#include "TimetableRow.h"
#include "TimetableRowTableSync.h"
#include "TimetableUpdateAction.h"
#include "TimetableRowAddAction.h"

#include "AdminActionFunctionRequest.hpp"
#include "AdminFunctionRequest.hpp"

#include "ModuleAdmin.h"
#include "AdminInterfaceElement.h"
#include "AdminParametersException.h"

#include <boost/foreach.hpp>

using namespace std;
using namespace boost;

namespace synthese
{
	using namespace admin;
	using namespace interfaces;
	using namespace server;
	using namespace util;
	using namespace timetables;
	using namespace html;
	using namespace db;
	using namespace time;
	using namespace security;
	using namespace env;

	namespace util
	{
		template<> const string FactorableTemplate<AdminInterfaceElement, TimetableAdmin>::FACTORY_KEY("TimetableAdmin");
	}

	namespace admin
	{
		template<> const string AdminInterfaceElementTemplate<TimetableAdmin>::ICON("table.png");
		template<> const string AdminInterfaceElementTemplate<TimetableAdmin>::DEFAULT_TITLE("Fiche horaire inconnue");
	}

	namespace timetables
	{
		const string TimetableAdmin::PARAMETER_RANK("rk");

		TimetableAdmin::TimetableAdmin()
			: AdminInterfaceElementTemplate<TimetableAdmin>()
		{ }
		
		void TimetableAdmin::setFromParametersMap(
			const ParametersMap& map,
			bool objectWillBeCreatedLater
		){
			if(objectWillBeCreatedLater) return;
			
			try
			{
				_timetable = TimetableTableSync::Get(
					map.get<RegistryKeyType>(Request::PARAMETER_OBJECT_ID),
					_getEnv()
				);
			}
			catch(...)
			{
				throw AdminParametersException("No such timetable");
			}
			if (_timetable->getIsBook())
				throw AdminParametersException("Timetable is document");
			_requestParameters.setFromParametersMap(map.getMap(), PARAMETER_RANK);
		}
		
		
		
		server::ParametersMap TimetableAdmin::getParametersMap() const
		{
			ParametersMap m(_requestParameters.getParametersMap());
			return m;
		}
		
		
		
		void TimetableAdmin::display(
			ostream& stream,
			VariablesMap& variables,
					const server::FunctionRequest<admin::AdminRequest>& _request
		) const	{
			// Requests
			AdminActionFunctionRequest<TimetableUpdateAction,TimetableAdmin> updateRequest(_request);
			
			AdminActionFunctionRequest<TimetableRowAddAction,TimetableAdmin> addRowRequest(_request);
			addRowRequest.getAction()->setTimetable(_timetable);

			AdminFunctionRequest<TimetableAdmin> searchRequest(_request);

			// Display
			stream << "<h1>Propri�t�s</h1>";

			PropertiesHTMLTable pt(updateRequest.getHTMLForm("update"));
			stream << pt.open();

			stream << pt.close();

			stream << "<h1>Contenu</h1>";

			// Search
			TimetableRowTableSync::SearchResult rows(
				TimetableRowTableSync::Search(
					_getEnv(),
					_timetable->getKey()
					, _requestParameters.orderField == PARAMETER_RANK
					, _requestParameters.raisingOrder
					, _requestParameters.first
					, _requestParameters.maxSize
			)	);
			
			ActionResultHTMLTable::HeaderVector h;
			h.push_back(make_pair(string(), HTMLModule::getHTMLImage("arrow_up.png", "^")));
			h.push_back(make_pair(string(), HTMLModule::getHTMLImage("arrow_down.png", "V")));
			h.push_back(make_pair(PARAMETER_RANK, "Rang"));
			h.push_back(make_pair(string(), "Commune"));
			h.push_back(make_pair(string(), "Arr�t"));
			h.push_back(make_pair(string(), "Arr"));
			h.push_back(make_pair(string(), "Dep"));
			h.push_back(make_pair(string(), "Obl"));
			h.push_back(make_pair(string(), "Sel"));
			h.push_back(make_pair(string(), "Aff"));
			h.push_back(make_pair(string(), "Action"));
			ActionResultHTMLTable t(
				h,
				searchRequest.getHTMLForm(),
				_requestParameters,
				rows,
				addRowRequest.getHTMLForm("addrow"),
				TimetableRowAddAction::PARAMETER_RANK
			);

			stream << t.open();

			int maxRank(TimetableRowTableSync::GetMaxRank(_timetable->getKey()));
			int lastRank(UNKNOWN_VALUE);
			BOOST_FOREACH(shared_ptr<TimetableRow> row, rows)
			{
				lastRank = row->getRank();

				stream << t.row(Conversion::ToString(lastRank));
				stream << t.col();
				if (lastRank > 0)
					stream << HTMLModule::getHTMLLink(string(), HTMLModule::getHTMLImage("arrow_up.png", "^"));
				stream << t.col();
				if (lastRank < maxRank)
					stream << HTMLModule::getHTMLLink(string(), HTMLModule::getHTMLImage("arrow_down.png", "V"));;
				stream << t.col() << lastRank;
				stream << t.col() << row->getPlace()->getCity()->getName();
				stream << t.col() << row->getPlace()->getName();
				stream <<
					t.col() <<
					(	row->getIsArrival() ?
						HTMLModule::getHTMLImage("bullet_green.png","Arriv�e possible") :
						HTMLModule::getHTMLImage("bullet_white.png", "Arriv�e impossible")
					)
				;
				stream <<
					t.col() <<
					(	row->getIsDeparture() ?
						HTMLModule::getHTMLImage("bullet_green.png", "D�part possible") :
						HTMLModule::getHTMLImage("bullet_white.png", "D�part impossible")
					)
				;
				stream <<
					t.col() <<
					(	(row->getCompulsory() == PassageObligatoire) ?
						HTMLModule::getHTMLImage("bullet_green.png", "Obligatoire") :
						HTMLModule::getHTMLImage("bullet_white.png", "Non obligatoire")
					)
				;
				stream <<
					t.col() <<
					(	(row->getCompulsory() == PassageSuffisant) ?
						HTMLModule::getHTMLImage("bullet_green.png", "Suffisant") :
						HTMLModule::getHTMLImage("bullet_white.png", "Non suffisant")
					)
				;
				stream <<
					t.col() <<
					(	(false) ?
						HTMLModule::getHTMLImage("bullet_green.png", "Affich�") :
						HTMLModule::getHTMLImage("bullet_white.png", "Non affich�")
					)
				;
				stream << t.col() << HTMLModule::getLinkButton(string(), "Supprimer", "Etes-vous s�r de vouloir supprimer l\\'arr�t ?");
			}
			stream << t.row(Conversion::ToString(UNKNOWN_VALUE));
			stream << t.col();
			stream << t.col();
			stream << t.col() << ++lastRank;
			stream << t.col() << t.getActionForm().getTextInput(TimetableRowAddAction::PARAMETER_CITY_NAME, string(), "(commune)");
			stream << t.col() << t.getActionForm().getTextInput(TimetableRowAddAction::PARAMETER_PLACE_NAME, string(), "(arr�t)");
			stream << t.col() << t.getActionForm().getCheckBox(TimetableRowAddAction::PARAMETER_IS_ARRIVAL, Conversion::ToString(true), true);
			stream << t.col() << t.getActionForm().getCheckBox(TimetableRowAddAction::PARAMETER_IS_DEPARTURE, Conversion::ToString(true), true);
			stream << t.col() << t.getActionForm().getCheckBox(TimetableRowAddAction::PARAMETER_IS_COMPULSORY, Conversion::ToString(true), false);
			stream << t.col() << t.getActionForm().getCheckBox(TimetableRowAddAction::PARAMETER_IS_SUFFICIENT, Conversion::ToString(true), true);
			stream << t.col() << t.getActionForm().getCheckBox(TimetableRowAddAction::PARAMETER_IS_DISPLAYED, Conversion::ToString(true), true);
			stream << t.col() << t.getActionForm().getSubmitButton("Ajouter");

			stream << t.close();

			stream << "<h1>R�sultat simul�</h1>";

			auto_ptr<TimetableGenerator> g(_timetable->getGenerator(_getEnv()));
			Calendar c;
			Date d(TIME_CURRENT);
			for (int i(0); i<60; ++i)
			{
				c.setActive(d);
				d++;
			}
			g->setBaseCalendar(c);
			g->build();

			stream << "<h2>Fiche horaire</h2>";

			HTMLTable tf(0,"adminresults");
			stream << tf.open();
			stream << tf.row();
			stream << tf.col() << "Ligne";
			
			BOOST_FOREACH(const Line* line, g->getLines())
			{
				stream <<
					tf.col(1, line->getCommercialLine()->getStyle()) <<
					line->getCommercialLine()->getShortName()
				;
			}

			const TimetableGenerator::Rows& grows(g->getRows());
			for (TimetableGenerator::Rows::const_iterator it(grows.begin()); it !=grows.end(); ++it)
			{
				stream << tf.row();
				stream << tf.col() << it->getPlace()->getFullName();
				vector<Schedule> cols(g->getSchedulesByRow(it));
				for (vector<Schedule>::const_iterator its(cols.begin()); its != cols.end(); ++its)
				{
					stream << tf.col();
					if (!its->getHour().isUnknown())
						stream << its->toString();
				}
			}
			stream << tf.close();

			stream << "<h2>Renvois</h2>";

		}

		bool TimetableAdmin::isAuthorized(
				const server::FunctionRequest<admin::AdminRequest>& _request
			) const
		{
			return _request.isAuthorized<TimetableRight>(READ);
		}
		


		std::string TimetableAdmin::getTitle() const
		{
			return _timetable.get() ? _timetable->getTitle() : DEFAULT_TITLE;
		}
		
		void TimetableAdmin::setTimetable(boost::shared_ptr<Timetable> timetable)
		{
			_timetable = const_pointer_cast<const Timetable, Timetable>(timetable);
		}
	
		boost::shared_ptr<const Timetable> TimetableAdmin::getTimetable() const
		{
			return _timetable;
		}


		bool TimetableAdmin::_hasSameContent(const AdminInterfaceElement& other) const
		{
			return _timetable == static_cast<const TimetableAdmin&>(other)._timetable;
		}
			
	}
}

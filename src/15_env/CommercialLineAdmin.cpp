////////////////////////////////////////////////////////////////////////////////
/// CommercialLineAdmin class implementation.
///	@file CommercialLineAdmin.cpp
///	@author Hugues Romain
///
///	This file belongs to the SYNTHESE project (public transportation specialized
///	software)
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
///	along with this program; if not, write to the Free Software Foundation,
///	Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
////////////////////////////////////////////////////////////////////////////////

#include "CommercialLineAdmin.h"
#include "TransportNetworkAdmin.h"
#include "EnvModule.h"

#include "CommercialLine.h"
#include "CommercialLineTableSync.h"
#include "Line.h"
#include "LineAdmin.h"
#include "LineTableSync.h"
#include "TransportNetworkRight.h"

#include "AdminRequest.h"
#include "QueryString.h"

#include "AdminParametersException.h"
#include "SearchFormHTMLTable.h"

using namespace std;
using namespace boost;

namespace synthese
{
	using namespace admin;
	using namespace interfaces;
	using namespace server;
	using namespace util;
	using namespace env;
	using namespace security;
	using namespace html;
	using namespace time;
	
	

	namespace util
	{
		template<> const string FactorableTemplate<AdminInterfaceElement, CommercialLineAdmin>::FACTORY_KEY("CommercialLineAdmin");
	}

	namespace admin
	{
		template<> const string AdminInterfaceElementTemplate<CommercialLineAdmin>::ICON("chart_line.png");
		template<> const string AdminInterfaceElementTemplate<CommercialLineAdmin>::DEFAULT_TITLE("Ligne inconnue");
	}

	namespace env
	{
		const string CommercialLineAdmin::TAB_DATES("da");
		const string CommercialLineAdmin::TAB_ROUTES("ro");
		const string CommercialLineAdmin::PARAMETER_SEARCH_NAME("na");
		const string CommercialLineAdmin::PARAMETER_DATES_START("ds");
		const string CommercialLineAdmin::PARAMETER_DATES_END("de");

		CommercialLineAdmin::CommercialLineAdmin()
			: AdminInterfaceElementTemplate<CommercialLineAdmin>()
		{ }
		
		void CommercialLineAdmin::setFromParametersMap(
			const ParametersMap& map,
			bool doDisplayPreparationActions
		){
			_searchName = map.getString(PARAMETER_SEARCH_NAME, false, FACTORY_KEY);
			_startDate = map.getOptionalDate(PARAMETER_DATES_START);
			_endDate = map.getOptionalDate(PARAMETER_DATES_END);

			_requestParameters.setFromParametersMap(map.getMap(), PARAMETER_SEARCH_NAME, 100);

			try
			{
				_cline = CommercialLineTableSync::Get(map.getUid(QueryString::PARAMETER_OBJECT_ID, true, FACTORY_KEY), _env, UP_LINKS_LOAD_LEVEL);
			}
			catch (...)
			{
				throw AdminParametersException("No such line");
			}

			if(!doDisplayPreparationActions) return;

			LineTableSync::Search(
				_env,
				_cline->getKey(),
				UNKNOWN_VALUE,
				_requestParameters.first,
				_requestParameters.maxSize,
				_requestParameters.orderField == PARAMETER_SEARCH_NAME,
				_requestParameters.raisingOrder
			);
			_resultParameters.setFromResult(_requestParameters, _env.getEditableRegistry<Line>());

			_runHours = getCommercialLineRunHours(_cline->getKey(), _startDate, _endDate);
		}
		
		
		
		server::ParametersMap CommercialLineAdmin::getParametersMap() const
		{
			ParametersMap m;
			return m;
		}



		void CommercialLineAdmin::display(ostream& stream, VariablesMap& variables) const
		{
			////////////////////////////////////////////////////////////////////
			// TAB STOPS
			if (openTabContent(stream, TAB_ROUTES))
			{
				// Requests
				FunctionRequest<AdminRequest> searchRequest(_request);
				searchRequest.getFunction()->setPage<CommercialLineAdmin>();
				searchRequest.setObjectId(_cline->getKey());

				FunctionRequest<AdminRequest> lineOpenRequest(_request);
				lineOpenRequest.getFunction()->setPage<LineAdmin>();

				// Search form
				stream << "<h1>Recherche</h1>";
				SearchFormHTMLTable s(searchRequest.getHTMLForm("search"));
				stream << s.open();
				stream << s.cell("Nom", s.getForm().getTextInput(PARAMETER_SEARCH_NAME, _searchName));
				HTMLForm sortedForm(s.getForm());
				stream << s.close();


				// Results display
				stream << "<h1>Parcours de la ligne</h1>";

				ResultHTMLTable::HeaderVector h;
				h.push_back(make_pair(PARAMETER_SEARCH_NAME, "Nom"));
				h.push_back(make_pair(string(), "Actions"));
				ResultHTMLTable t(h,sortedForm,_requestParameters, _resultParameters);

				stream << t.open();
				BOOST_FOREACH(shared_ptr<Line> line, _env.getRegistry<Line>())
				{
					lineOpenRequest.setObjectId(line->getKey());
					stream << t.row();
					stream << t.col();
					stream << line->getName();
					stream << t.col();
					stream << HTMLModule::getLinkButton(lineOpenRequest.getURL(), "Ouvrir", string(), "chart_line_edit.png");
				}
				stream << t.close();
			}
			////////////////////////////////////////////////////////////////////
			// TAB STOPS
			if (openTabContent(stream, TAB_DATES))
			{
				stream << "<style>td.red {background-color:red;width:8px; height:8px; color:white; text-align:center; } td.green {background-color:#008000;width:10px; height:10px; color:white; text-align:center; }</style>"; 
				HTMLTable::ColsVector cols;
				cols.push_back("Date");
				for(int i(0); i<=23; ++i)
				{
					cols.push_back(Conversion::ToString(i));
				}
				optional<Date> date;
				int lastHour;
				HTMLTable t(cols, ResultHTMLTable::CSS_CLASS);
				stream << t.open();
				BOOST_FOREACH(const RunHours::value_type& it, _runHours)
				{
					if(!date || date != it.first.first)
					{
						if (date)
						{
							for(int i(lastHour+1); i<=23; ++i)
							{
								stream << t.col(1, "red") << "0";
							}
							for((*date)++; *date < it.first.first; (*date)++)
							{
								stream << t.row();
								stream << t.col(1, string(), true) << date->toString();
								for(int i(0); i<=23; ++i)
								{
									stream << t.col(1, "red") << "0";
								}
							}
						}
						date = it.first.first;
						stream << t.row();
						stream << t.col(1, string(), true) << date->toString();
						lastHour = -1;
					}
					for(int i(lastHour+1); i<it.first.second; ++i)
					{
						stream << t.col(1, "red") << "0";
					}
					stream << t.col(1, "green") << it.second;
					lastHour = it.first.second;
				}
				if(date)
				{
					for(int i(lastHour+1); i<=23; ++i)
					{
						stream << t.col(1, "red") << "0";
					}
				}
				stream << t.close();
			}

			////////////////////////////////////////////////////////////////////
			// END TABS
			closeTabContent(stream);
		}

		bool CommercialLineAdmin::isAuthorized() const
		{
			if (_cline.get() == NULL) return false;
			return _request->isAuthorized<TransportNetworkRight>(READ);
		}
		
		AdminInterfaceElement::PageLinks CommercialLineAdmin::getSubPagesOfParent(
			const PageLink& parentLink
			, const AdminInterfaceElement& currentPage
		) const	{
			AdminInterfaceElement::PageLinks links;
			return links;
		}
		
		std::string CommercialLineAdmin::getTitle() const
		{
			return _cline.get() ? "<span class=\"" + _cline->getStyle() +"\">" + _cline->getShortName() + "</span>" : DEFAULT_TITLE;
		}

		std::string CommercialLineAdmin::getParameterName() const
		{
			return _cline.get() ? QueryString::PARAMETER_OBJECT_ID : string();
		}

		std::string CommercialLineAdmin::getParameterValue() const
		{
			return _cline.get() ? Conversion::ToString(_cline->getKey()) : string();
		}

		AdminInterfaceElement::PageLinks CommercialLineAdmin::getSubPages( const AdminInterfaceElement& currentPage
		) const	{
			AdminInterfaceElement::PageLinks links;
			if (currentPage.getFactoryKey() == CommercialLineAdmin::FACTORY_KEY && _cline->getKey() == static_cast<const CommercialLineAdmin&>(currentPage).getCommercialLine()->getKey()
				|| currentPage.getFactoryKey() == LineAdmin::FACTORY_KEY && _cline->getKey() == static_cast<const LineAdmin&>(currentPage).getLine()->getCommercialLine()->getKey()
				)
			{
				Env env;
				LineTableSync::Search(env, _cline->getKey());
				const Registry<Line>& lines(env.getRegistry<Line>());
				for(Registry<Line>::const_iterator it(lines.begin()); it != lines.end(); ++it)
				{
					PageLink link(getPageLink());
					link.factoryKey = LineAdmin::FACTORY_KEY;
					link.icon = LineAdmin::ICON;
					link.name = (*it)->getName();
					link.parameterName = QueryString::PARAMETER_OBJECT_ID;
					link.parameterValue = Conversion::ToString((*it)->getKey());
					links.push_back(link);
				}
			}
			return links;
		}

		void CommercialLineAdmin::_buildTabs(
		) const {
			_tabs.clear();

			_tabs.push_back(Tab("Parcours", TAB_ROUTES, true));
			_tabs.push_back(Tab("Dates de fonctionnement", TAB_DATES, true));

			_tabBuilded = true;
		}

		boost::shared_ptr<const CommercialLine> CommercialLineAdmin::getCommercialLine() const
		{
			return _cline;
		}
	}
}

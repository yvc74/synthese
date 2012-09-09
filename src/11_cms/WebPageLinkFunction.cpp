
/** WebPageLinkFunction class implementation.
	@file WebPageLinkFunction.cpp
	@author Hugues Romain
	@date 2010

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

#include "RequestException.h"
#include "Request.h"
#include "WebPageLinkFunction.hpp"
#include "HTMLModule.h"
#include "Webpage.h"
#include "StaticFunctionRequest.h"
#include "WebPageDisplayFunction.h"
#include "URI.hpp"

using namespace std;
using namespace boost;

namespace synthese
{
	using namespace util;
	using namespace server;
	using namespace security;
	using namespace html;

	template<> const string util::FactorableTemplate<Function,cms::WebPageLinkFunction>::FACTORY_KEY("link");

	namespace cms
	{
		const string WebPageLinkFunction::PARAMETER_TARGET("target");
		const string WebPageLinkFunction::PARAMETER_TEXT("text");
		const string WebPageLinkFunction::PARAMETER_USE_SMART_URL("use_smart_url");
		const string WebPageLinkFunction::PARAMETER_CONFIRM("confirm");
		const string WebPageLinkFunction::PARAMETER_CLASS = "class";



		ParametersMap WebPageLinkFunction::_getParametersMap() const
		{
			ParametersMap map;
			if(_target.get())
			{
				map.insert(PARAMETER_TARGET, _target->getKey());
			}
			map.insert(PARAMETER_TEXT, _text);
			map.insert(PARAMETER_USE_SMART_URL, _useSmartURL);
			map.insert(PARAMETER_CONFIRM, _confirm);
			return map;
		}



		void WebPageLinkFunction::_setFromParametersMap(const ParametersMap& map)
		{
			try
			{
				_target = Env::GetOfficialEnv().get<Webpage>(map.get<RegistryKeyType>(PARAMETER_TARGET));
			}
			catch(ObjectNotFoundException<Webpage>&)
			{
				throw RequestException("No such web page");
			}
			optional<string> ot(map.getOptional<string>(PARAMETER_TEXT, false));
			_text = ot ? *ot : _target->getName();
			_useSmartURL = map.getDefault<bool>(PARAMETER_USE_SMART_URL, true);

			_confirm = map.getDefault<string>(PARAMETER_CONFIRM, string(), false);

			// Class
			_class = map.getDefault<string>(PARAMETER_CLASS);

			// Additional parameters
			BOOST_FOREACH(const ParametersMap::Map::value_type& item, map.getMap())
			{
				if(item.first == PARAMETER_TEXT ||
					item.first == PARAMETER_TARGET ||
					item.first == PARAMETER_USE_SMART_URL ||
					item.first == PARAMETER_CONFIRM ||
					item.first == PARAMETER_CLASS
				){
					continue;
				}

				// Using the getValue virtual method instead of item.second in order to handle DelayedEvaluationParametersMap
				_parameters.insert(item.first, map.getValue(item.first));
			}
		}



		util::ParametersMap WebPageLinkFunction::run(
			std::ostream& stream,
			const Request& request
		) const {
			string url;
			shared_ptr<const WebPageDisplayFunction> webpageDisplayFunction(
				dynamic_pointer_cast<const WebPageDisplayFunction,const Function>(
					request.getFunction()
			)	);

			if(	_useSmartURL &&
				!_target->get<SmartURLPath>().empty()
			){	// URL is smart URL
				url = _target->get<SmartURLPath>();

				// Parameters
				if(!_parameters.getMap().empty())
				{
					stringstream uri;
					_parameters.outputURI(uri);
					url += Request::PARAMETER_STARTER + uri.str();
				}
			}
			else
			{	// Classic URL
				StaticFunctionRequest<WebPageDisplayFunction> openRequest(request, false);
				openRequest.getFunction()->setPage(_target.get());
				openRequest.getFunction()->setDontRedirectIfSmartURL(_templateParameters.getDefault<bool>(WebPageDisplayFunction::PARAMETER_DONT_REDIRECT_IF_SMART_URL, false));
				if(!_target->getRoot()->get<ClientURL>().empty())
				{
					openRequest.setClientURL(_target->getRoot()->get<ClientURL>());
				}
				url = openRequest.getURL();

				// Parameters
				if(!_parameters.getMap().empty())
				{
					stringstream uri;
					_parameters.outputURI(uri);
					url += URI::PARAMETER_SEPARATOR + uri.str();
				}
			}
			stream << HTMLModule::getHTMLLink(url, _text, _confirm, false, string(), _class);

			return util::ParametersMap();
		}



		bool WebPageLinkFunction::isAuthorized(
			const Session* session
		) const {
			return true;
		}



		std::string WebPageLinkFunction::getOutputMimeType() const
		{
			return "text/html";
		}



		WebPageLinkFunction::WebPageLinkFunction():
			_useSmartURL(true)
		{

		}
}	}
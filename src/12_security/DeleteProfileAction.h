
/** DeleteProfileAction class header.
	@file DeleteProfileAction.h

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

#ifndef SYNTHESE_DeleteProfileAction_H__
#define SYNTHESE_DeleteProfileAction_H__

#include <boost/shared_ptr.hpp>

#include "Action.h"
#include "FactorableTemplate.h"

namespace synthese
{
	namespace security
	{
		class Profile;

		/** DeleteProfileAction action class.
			@ingroup m12Actions refActions
		*/
		class DeleteProfileAction:
			public util::FactorableTemplate<server::Action, DeleteProfileAction>
		{
		public:
			static const std::string PARAMETER_PROFILE_ID;

		private:
			boost::shared_ptr<const Profile>	_profile;

		protected:
			/** Conversion from attributes to generic parameter maps.
			*/
			server::ParametersMap getParametersMap() const;

			/** Conversion from generic parameters map to attributes.
			Removes the used parameters from the map.
			*/
			void _setFromParametersMap(const server::ParametersMap& map);

		public:
			/** Action to run, defined by each subclass.
			*/
			void run();

			virtual bool _isAuthorized() const;
			
			void setProfile(boost::shared_ptr<Profile> value);
		};
	}
}

#endif // SYNTHESE_DeleteProfileAction_H__

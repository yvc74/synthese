
/** VinciReturnGuaranteeAction class header.
	@file VinciReturnGuaranteeAction.h

	This file belongs to the VINCI BIKE RENTAL SYNTHESE module
	Copyright (C) 2006 Vinci Park 
	Contact : Rapha�l Murat - Vinci Park <rmurat@vincipark.com>

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

#ifndef SYNTHESE_ReturnGuaranteeAction_H__
#define SYNTHESE_ReturnGuaranteeAction_H__


#include "30_server/Action.h"

namespace synthese
{
	namespace accounts
	{
		class Transaction;
	}

	namespace vinci
	{
		/** Guarantee return Action Class.
			@ingroup m71Actions refActions
		*/
		class VinciReturnGuaranteeAction : public server::Action
		{
		public:
			static const std::string PARAMETER_GUARANTEE_ID;

		private:
			accounts::Transaction* _guarantee;
			
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

			VinciReturnGuaranteeAction();
			~VinciReturnGuaranteeAction();
		};
	}
}

#endif // SYNTHESE_AddGuaranteeAction_H__



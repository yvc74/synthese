
/** JourneyLineListInterfaceElement class header.
	@file JourneyLineListInterfaceElement.h

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

#include "11_interfaces/LibraryInterfaceElement.h"

namespace synthese
{
	namespace interfaces
	{
		class ValueInterfaceElement;
	}

	namespace routeplanner
	{
		class JourneyLineListInterfaceElement : public interfaces::LibraryInterfaceElement
		{
		private:
			interfaces::ValueInterfaceElement* _displayPedestrianLines;
			interfaces::ValueInterfaceElement* _rowStartHtml;
			interfaces::ValueInterfaceElement* _rowEndHtml;
			interfaces::ValueInterfaceElement* _pixelWidth;
			interfaces::ValueInterfaceElement* _pixelHeight;
			~JourneyLineListInterfaceElement();

		public:
			/** Display.
				@param stream Stream to write on
				@param parameters Runtime parameters
				@param object (Journey*) Journey to display
				@param site Site to display
			*/
			void display(std::ostream& stream, const interfaces::ParametersVector& parameters, const void* object = NULL, const server::Request* request = NULL) const;

			/** Parser.
			@param text Text to parse : standard list of parameters :
				-# Affichage des lignes � pied
				-# HTML en d�but de ligne (conseill� tr)
				-# HTML en fin de ligne (conseill� /tr)
				-# Largeur en pixels de la case de lignes
				-# Hauteur en pixels de la case de lignes
			*/
			void storeParameters(interfaces::ValueElementList& vel);
		};

	}
}

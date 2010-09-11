
/** NavstreetFileFormat class header.
	@file NavstreetFileFormat.hpp

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

#ifndef SYNTHESE_road_NavstreetFileFormat_hpp__
#define SYNTHESE_road_NavstreetFileFormat_hpp__

#include "FileFormatTemplate.h"

#include <map>
#include <ostream>
#include <string>

namespace synthese
{
	namespace geography
	{
		class City;
	}

	namespace road
	{
		//////////////////////////////////////////////////////////////////////////
		/// 34.16 : Navstreets file format class.
		///	@ingroup m34
		/// @author Hugues Romain
		/// @since 3.2.0
		/// @date 2010
		/// @todo Replace cdbfile.h use by shapefil.h dbf support and remove cdbfile.h
		/// @bug the imported files must allow writing (it is not used by synthese)
		/// @bug the circulation side on the streets is forced to right
		//////////////////////////////////////////////////////////////////////////
		/// A large documentation about Navstreets format can be found at
		/// http://www.nn4d.com/ (valid user required, registration is free)
		///
		///	Parameters :
		///	<ul>
		///		<li>Key of the datasource : Navstreets</li>
		///		<li>1mtdarea : path to the mtdarea dbase file</li>
		///		<li>2streets : path to the streets dbase file</li>
		///	</ul>
		///
		/// Necessary files and used fields :
		///	<ul>
		///		<li><b>mtdarea.dbf</b> : administrative areas</li>
		///		<ul>
		///			<li>AREA_ID : internal id used in streets file to link streets and settlements</li>
		///			<li>ADMIN_LVL : type of area : 3=departement, 4=city, 5=settlement.</li>
		///			<li>GOVT_CODE : official code of the area</li>
		///			<li>...</li>
		///			<li>linking : type 4 area are linked with SYNTHESE cities (code = govt_code of departement + govt_code of city)
		///			and type 5 area is linked with the streets file but is not used in SYNTHESE.</li>
		///		</ul>
		///		<li><b>streets.dbf</b> and <b>streets.shp</b> : nodes and streets. There must be a shapefile
		///		named like the dbase file in the same directory. Its extension must be shp.</li>
		///	</ul>
		///
		/// Imported data :
		///	<ul>
		///		<li>Crossing (read in streets file)</li>
		///		<li>RoadChunk with geometry (read in streets file)</li>
		///		<li>Road (generated)</li>
		///		<li>RoadPlace (generated)</li>
		///	</ul>
		///
		/// Limitations :
		///	<ul>
		///		<li>A city is identified by its french INSEE code, corresponding to
		///		local codes present in mtdarea file. Other way of city linking is not
		///		handled right now. (this limitation can be cancelled by introducing
		///		a new parameter to choose the way of city linking)</li>
		///		<li>The coordinates present in the shapefile can be only in French Lambert IIe projection.
		///		(this limitation can be cancelled by reading the associated .prj file and choose an
		///		other CoordinatesSystem)</li>
		///	</ul>
		///
		/// The import is done in 4 steps :
		///	<ul>
		///		<li>Reading of department codes in mtdarea file</li>
		///		<li>Reading of city codes in mtdarea file</li>
		///		<li>Reading of nodes in streets file</li>
		///		<li>Reading of edges in streets file</li>
		///	</ul>
		///
		class NavstreetsFileFormat:
			public impex::FileFormatTemplate<NavstreetsFileFormat>
		{
		public:
			//! @name Tables
			//@{
				static const std::string FILE_MTDAREA;
				static const std::string FILE_STREETS;
			//@}

		private:
			//! @name Fields of streets table
			//@{
				static const std::string _FIELD_LINK_ID;
				static const std::string _FIELD_ST_NAME;
				static const std::string _FIELD_REF_IN_ID;
				static const std::string _FIELD_NREF_IN_ID;
				static const std::string _FIELD_L_AREA_ID;
				static const std::string _FIELD_R_AREA_ID;
				static const std::string _FIELD_L_REFADDR;
				static const std::string _FIELD_L_NREFADDR;
				static const std::string _FIELD_R_REFADDR;
				static const std::string _FIELD_R_NREFADDR;
				static const std::string _FIELD_L_ADDRSCH;
				static const std::string _FIELD_R_ADDRSCH;
			//@}

			//! @name Fields of mtdarea table
			//@{
				static const std::string _FIELD_AREA_ID;
				static const std::string _FIELD_AREACODE_3;
				static const std::string _FIELD_AREACODE_4;
				static const std::string _FIELD_GOVT_CODE;
				static const std::string _FIELD_ADMIN_LVL;
			//@}

			typedef std::map<int, geography::City*> _CitiesMap;

			_CitiesMap _citiesMap;	//!< Correspondence table between Navstreets and SYNTHESE id for streets

		protected:

			//////////////////////////////////////////////////////////////////////////
			/// Controls that all necessary input files are available.
			/// @param paths available files
			/// @result true if all necessary files are present
			virtual bool _controlPathsMap(
				const FilePathsMap& paths
			);



			//////////////////////////////////////////////////////////////////////////
			/// Reads an input files and load its content to the memory.
			/// @param filePath file to read
			/// @param os stream to write log messages on
			/// @param key type of the file
			/// @author Hugues Romain
			/// @since 3.2.0
			/// @date 2010
			virtual void _parse(
				const boost::filesystem::path& filePath,
				std::ostream& os,
				std::string key
			);



		public:
			NavstreetsFileFormat(
				util::Env* env = NULL
			);
			~NavstreetsFileFormat();

			//////////////////////////////////////////////////////////////////////////
			/// Export of SYNTHESE data into Navstreets format : not implemented.
			/// @param os stream to write log messages on
			/// @author Hugues Romain
			/// @since 3.2.0
			/// @date 2010
			//////////////////////////////////////////////////////////////////////////
			/// This method does nothing.
			virtual void build(std::ostream& os);



			//////////////////////////////////////////////////////////////////////////
			/// Saves all imported data.
			/// @param os stream to write log messages on
			/// @author Hugues Romain
			/// @since 3.2.0
			/// @date 2010
			virtual void save(std::ostream& os) const;
		};
	}
}

#endif // SYNTHESE_road_NavstreetFileFormat_hpp__

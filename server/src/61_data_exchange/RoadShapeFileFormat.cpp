
/** RoadShapeFileFormat class implementation.
	@file RoadShapeFileFormat.cpp

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

#include "RoadShapeFileFormat.hpp"

#include "AdminFunctionRequest.hpp"
#include "CityTableSync.h"
#include "CoordinatesSystem.hpp"
#include "Crossing.h"
#include "CrossingTableSync.hpp"
#include "DataSource.h"
#include "DBTransaction.hpp"
#include "EdgeProjector.hpp"
#include "PublicPlaceTableSync.h"
#include "PublicPlaceEntranceTableSync.hpp"
#include "RoadChunkTableSync.h"
#include "RoadTableSync.h"
#include "VirtualShapeVirtualTable.hpp"

#include <fstream>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
using namespace geos::geom;

namespace synthese
{
	using namespace algorithm;
	using namespace data_exchange;
	using namespace road;
	using namespace util;
	using namespace impex;
	using namespace geography;
	using namespace graph;
	using namespace db;
	using namespace server;

	namespace util
	{
		template<> const string FactorableTemplate<FileFormat,RoadShapeFileFormat>::FACTORY_KEY("RoadShapeFile");
	}

	namespace data_exchange
	{
		const string RoadShapeFileFormat::Importer_::FILE_ROAD_PLACES = "road_places";
		const string RoadShapeFileFormat::Importer_::FILE_ROAD_CHUNKS = "road_chunks";
		const string RoadShapeFileFormat::Importer_::FILE_PUBLIC_PLACES = "public_places";

		const string RoadShapeFileFormat::Importer_::FIELD_GEOMETRY = "Geometry";

		const string RoadShapeFileFormat::Importer_::PARAMETER_FIELD_ROAD_PLACES_CITY_CODE = "field_road_places_city_code";
		const string RoadShapeFileFormat::Importer_::PARAMETER_FIELD_ROAD_PLACES_CODE = "field_road_places_code";
		const string RoadShapeFileFormat::Importer_::PARAMETER_FIELD_ROAD_PLACES_NAME = "field_road_places_name";
		const string RoadShapeFileFormat::Importer_::PARAMETER_FIELD_ROAD_CHUNKS_FROM_LEFT = "field_road_chunks_from_left";
		const string RoadShapeFileFormat::Importer_::PARAMETER_FIELD_ROAD_CHUNKS_TO_LEFT = "field_road_chunks_to_left";
		const string RoadShapeFileFormat::Importer_::PARAMETER_FIELD_ROAD_CHUNKS_FROM_RIGHT = "field_road_chunks_from_right";
		const string RoadShapeFileFormat::Importer_::PARAMETER_FIELD_ROAD_CHUNKS_TO_RIGHT = "field_road_chunks_to_right";
		const string RoadShapeFileFormat::Importer_::PARAMETER_FIELD_ROAD_CHUNKS_ROAD_PLACE = "field_road_chunks_road_place";
		const string RoadShapeFileFormat::Importer_::PARAMETER_FIELD_ROAD_CHUNKS_CODE = "field_road_chunks_code";
		const string RoadShapeFileFormat::Importer_::PARAMETER_FIELD_ROAD_CHUNKS_START_NODE = "field_road_chunks_start_node";
		const string RoadShapeFileFormat::Importer_::PARAMETER_FIELD_ROAD_CHUNKS_END_NODE = "field_road_chunks_end_node";
		const string RoadShapeFileFormat::Importer_::PARAMETER_FIELD_PUBLIC_PLACES_CODE = "field_public_places_code";
		const string RoadShapeFileFormat::Importer_::PARAMETER_FIELD_PUBLIC_PLACES_NAME = "field_public_places_name";
		const string RoadShapeFileFormat::Importer_::PARAMETER_FIELD_PUBLIC_PLACES_ROAD_CHUNK = "field_public_places_road_chunk";
		const string RoadShapeFileFormat::Importer_::PARAMETER_FIELD_PUBLIC_PLACES_NUMBER = "field_public_places_number";
		const string RoadShapeFileFormat::Importer_::PARAMETER_FIELD_PUBLIC_PLACES_CITY_CODE = "field_public_places_city_code";
	}

	namespace impex
	{
		template<> const MultipleFileTypesImporter<RoadShapeFileFormat>::Files MultipleFileTypesImporter<RoadShapeFileFormat>::FILES(
			RoadShapeFileFormat::Importer_::FILE_ROAD_PLACES.c_str(),
			RoadShapeFileFormat::Importer_::FILE_ROAD_CHUNKS.c_str(),
			RoadShapeFileFormat::Importer_::FILE_PUBLIC_PLACES.c_str(),
		"");
	}

	namespace data_exchange
	{
		RoadShapeFileFormat::Importer_::Importer_(
			util::Env& env,
			const impex::Import& import,
			impex::ImportLogLevel minLogLevel,
			const std::string& logPath,
			boost::optional<std::ostream&> outputStream,
			util::ParametersMap& pm
		):	impex::Importer(env, import, minLogLevel, logPath, outputStream, pm),
			impex::MultipleFileTypesImporter<RoadShapeFileFormat>(env, import, minLogLevel, logPath, outputStream, pm),
			RoadFileFormat(env, import, minLogLevel, logPath, outputStream, pm),
			_roadPlaces(*import.get<impex::DataSource>(), env)
		{}

		bool RoadShapeFileFormat::Importer_::_checkPathsMap() const
		{
			return true;
		}



		bool RoadShapeFileFormat::Importer_::_parse(
			const boost::filesystem::path& filePath,
			const std::string& key
		) const {

			// DataSource
			DataSource& dataSource(*_import.get<DataSource>());

			// SRID
			if(!dataSource.get<CoordinatesSystem>())
			{
				_logError("SRID must be set in datasource.");
				return false;
			}

			// Road places
			if(key == FILE_ROAD_PLACES)
			{
				// Ckeck of field names
				if(	_roadPlacesCityCodeField.empty() ||
					_roadPlacesCodeField.empty() ||
					_roadPlacesNameField.empty()
				){
					_logError("City code, name, and code field names must be defined for road places file. Load interrupted.");
					return false;
				}

				// Loading the file into SQLite as virtual table
				VirtualShapeVirtualTable table(
					filePath,
					dataSource.get<Charset>(),
					dataSource.get<CoordinatesSystem>()->getSRID()
				);
				stringstream query;
				query << "SELECT *, AsText(" << RoadShapeFileFormat::Importer_::FIELD_GEOMETRY << ") AS " << RoadShapeFileFormat::Importer_::FIELD_GEOMETRY << "_ASTEXT" << " FROM " << table.getName();
				DBResultSPtr rows(DBModule::GetDB()->execQuery(query.str()));

				// Load of each record
				while(rows->next())
				{
					// City field
					boost::shared_ptr<City> city(
						CityTableSync::GetEditableFromCode(rows->getText(_roadPlacesCityCodeField), _env)
					);
					if(!city.get())
					{
						_logWarning("Unknown city "+ rows->getText(_roadPlacesCityCodeField) +" in road place "+ rows->getText(_roadPlacesCodeField) +". Road place is ignored.");
						continue;
					}

					// Name field
					string name(rows->getText(_roadPlacesNameField));

					// Code field
					string code(
						rows->getText(_roadPlacesCodeField)
					);
					if(code.empty())
					{
						_logWarning("Each road place code must be non empty");
						continue;
					}

					// Import
					_createOrUpdateRoadPlace(
						_roadPlaces,
						code,
						name,
						*city
					);
				}

				return true;
			}
			else if(key == FILE_ROAD_CHUNKS)
			{
				// Ckeck of field names
				if(	_roadChunksStartNodeField.empty() ||
					_roadChunksEndNodeField.empty() ||
					_roadChunksCodeField.empty() ||
					_roadChunksRoadPlaceField.empty() // Todo turn it into optional field
				){
					_logError("Start node, end node, road place code, and code field names must be defined for road chunks file. Load interrupted.");
					return false;
				}
				bool withHouseNumbers(
					!_roadChunksFromRightField.empty() &&
					!_roadChunksToRightField.empty() &&
					!_roadChunksFromLeftField.empty() &&
					!_roadChunksToLeftField.empty()
				);

				// Geometry factory
				const GeometryFactory& geometryFactory(dataSource.get<CoordinatesSystem>()->getGeometryFactory());

				// Crossings
				ImportableTableSync::ObjectBySource<CrossingTableSync> crossings(dataSource, _env);

				// Loading the file into SQLite as virtual table
				VirtualShapeVirtualTable table(
					filePath,
					dataSource.get<Charset>(),
					dataSource.get<CoordinatesSystem>()->getSRID()
				);
				stringstream query;
				query << "SELECT *, AsText(" << RoadShapeFileFormat::Importer_::FIELD_GEOMETRY << ") AS " << RoadShapeFileFormat::Importer_::FIELD_GEOMETRY << "_ASTEXT" << " FROM " << table.getName();
				DBResultSPtr rows(DBModule::GetDB()->execQuery(query.str()));

				// Load of each record
				while(rows->next())
				{
					// Code
					string code(rows->getText(_roadChunksCodeField));
					if(code.empty())
					{
						_logWarning("Each road chunk code must be non empty");
						continue;
					}

					// RoadPlace
					RoadPlace* roadPlace(
						_getRoadPlace(
							_roadPlaces,
							rows->getText(_roadChunksRoadPlaceField)
					)	);
					if(!roadPlace)
					{
						continue;
					}

					// Geometry
					boost::shared_ptr<LineString> geometry(
						dynamic_pointer_cast<LineString, Geometry>(
							rows->getGeometryFromWKT(RoadShapeFileFormat::Importer_::FIELD_GEOMETRY+"_ASTEXT", geometryFactory)
					)	);
					if(!geometry.get())
					{
						_logWarning("Empty geometry in the "+ code +" road chunk.");
						continue;
					}

					// Start node
					string startNodeCode(rows->getText(_roadChunksStartNodeField));
					if(startNodeCode.empty())
					{
						_logWarning("The start node code is empty in the road chunk "+ code);
						continue;
					}
					boost::shared_ptr<Point> startPoint(
						dataSource.get<CoordinatesSystem>()->createPoint(
							geometry->getCoordinatesRO()->getX(0),
							geometry->getCoordinatesRO()->getY(0)
					)	);
					Crossing* startCrossing(
						_createOrUpdateCrossing(crossings, startNodeCode, startPoint)
					);

					// End node
					string endNodeCode(rows->getText(_roadChunksEndNodeField));
					if(endNodeCode.empty())
					{
						_logWarning("The end node code is empty in the road chunk "+ code);
						continue;
					}
					boost::shared_ptr<Point> endPoint(
						dataSource.get<CoordinatesSystem>()->createPoint(
							geometry->getCoordinatesRO()->getX(geometry->getCoordinatesRO()->size() - 1),
							geometry->getCoordinatesRO()->getY(geometry->getCoordinatesRO()->size() - 1)
					)	);
					Crossing* endCrossing(
						_createOrUpdateCrossing(crossings, endNodeCode, endPoint)
					);

					// House numbers
					HouseNumberBounds leftHouseNumbers;
					HouseNumberBounds rightHouseNumbers;
					HouseNumberingPolicy leftHouseNumberingPolicy(ALL_NUMBERS);
					HouseNumberingPolicy rightHouseNumberingPolicy(ALL_NUMBERS);
					if(withHouseNumbers)
					{
						// Left
						if(!rows->getText(_roadChunksFromLeftField).empty() && !rows->getText(_roadChunksToLeftField).empty())
						{
							leftHouseNumbers = make_pair(
								lexical_cast<HouseNumber>(
									trim_copy(rows->getText(_roadChunksFromLeftField))
								),
								lexical_cast<HouseNumber>(
									trim_copy(rows->getText(_roadChunksToLeftField))
							)	);
							if(leftHouseNumbers->first % 2 && leftHouseNumbers->second % 2)
							{
								leftHouseNumberingPolicy = ODD_NUMBERS;
							}
							else if(!(leftHouseNumbers->first % 2) && !(leftHouseNumbers->second % 2))
							{
								leftHouseNumberingPolicy = EVEN_NUMBERS;
							}
						}

						// Right
						if(!rows->getText(_roadChunksFromRightField).empty() && !rows->getText(_roadChunksToRightField).empty())
						{
							rightHouseNumbers = make_pair(
								lexical_cast<HouseNumber>(
									trim_copy(rows->getText(_roadChunksFromRightField))
								),
								lexical_cast<HouseNumber>(
									trim_copy(rows->getText(_roadChunksToRightField))
							)	);
							if(rightHouseNumbers->first % 2 && rightHouseNumbers->second % 2)
							{
								rightHouseNumberingPolicy = ODD_NUMBERS;
							}
							else if(!(rightHouseNumbers->first % 2) && !(rightHouseNumbers->second % 2))
							{
								rightHouseNumberingPolicy = EVEN_NUMBERS;
							}
						}
					}

					// Import
					_roadChunks.insert(
						make_pair(
							code,
							_addRoadChunk(
								*roadPlace,
								*startCrossing,
								*endCrossing,
								geometry,
								rightHouseNumberingPolicy,
								leftHouseNumberingPolicy,
								rightHouseNumbers,
								leftHouseNumbers
					)	)	);
				}
			}
			// Public places
			else if(key == FILE_PUBLIC_PLACES)
			{
				// Ckeck of field names
				if(	_publicPlacesCodeField.empty() ||
					_publicPlacesNameField.empty() ||
					_publicPlacesCityCodeField.empty() ||
					_publicPlacesRoadChunkField.empty() || // Todo turn it into optional field
					_publicPlacesNumberField.empty() // Todo turn it into optional field
				){
					_logError("Code, name, city code, road chunk, and number field names must be defined for public places file. Load interrupted.");
					return false;
				}

				// Geometry factory
				const GeometryFactory& geometryFactory(dataSource.get<CoordinatesSystem>()->getGeometryFactory());

				// Public places
				ImportableTableSync::ObjectBySource<PublicPlaceTableSync> publicPlaces(dataSource, _env);
				ImportableTableSync::ObjectBySource<PublicPlaceEntranceTableSync> publicPlaceEntrances(dataSource, _env);

				// Loading the file into SQLite as virtual table
				VirtualShapeVirtualTable table(
					filePath,
					dataSource.get<Charset>(),
					dataSource.get<CoordinatesSystem>()->getSRID()
				);
				stringstream query;
				query << "SELECT *, AsText(" << RoadShapeFileFormat::Importer_::FIELD_GEOMETRY << ") AS " << RoadShapeFileFormat::Importer_::FIELD_GEOMETRY << "_ASTEXT" << " FROM " << table.getName();
				DBResultSPtr rows(DBModule::GetDB()->execQuery(query.str()));

				// Load of each record
				while(rows->next())
				{
					//////////////////////////////////////////////////////////////////////////
					// Public place

					// Code
					string code(rows->getText(_publicPlacesCodeField));
					if(code.empty())
					{
						_logWarning("Each road chunk code must be non empty");
						continue;
					}

					// City field
					string cityCode(rows->getText(_publicPlacesCityCodeField));
					boost::shared_ptr<City> city(
						CityTableSync::GetEditableFromCode(cityCode, _env)
					);
					if(!city.get())
					{
						_logWarning("Unknown city "+ cityCode +" in public place "+ code +". Public place is ignored.");
						continue;
					}

					// Name field
					string name(rows->getText(_publicPlacesNameField));

					// Geometry
					boost::shared_ptr<Point> geometry(
						dynamic_pointer_cast<Point, Geometry>(
							rows->getGeometryFromWKT(RoadShapeFileFormat::Importer_::FIELD_GEOMETRY+"_ASTEXT", geometryFactory)
					)	);
					if(!geometry.get())
					{
						_logWarning("Empty geometry in the "+ code +" road place.");
						continue;
					}

					// Import
					PublicPlace* publicPlace(
						_createOrUpdatePublicPlace(
							publicPlaces,
							code,
							name,
							geometry,
							*city
					)	);


					//////////////////////////////////////////////////////////////////////////
					// Public place entrance

					// House number
					optional<HouseNumber> houseNumber;
					if(rows->getInt(_publicPlacesNumberField) > 0)
					{
						houseNumber = rows->getInt(_publicPlacesNumberField);
					}

					// Road chunk
					string roadChunkId(rows->getText(_publicPlacesRoadChunkField));
					map<string, RoadChunk*>::iterator itRoadChunk(_roadChunks.find(roadChunkId));
					if(itRoadChunk == _roadChunks.end())
					{
						_logWarning("Unknown road chunk "+ roadChunkId +" in public place "+ code +". Public place entrance is ignored.");
						continue;
					}
					RoadChunk* roadChunk = itRoadChunk->second;

					EdgeProjector<RoadChunk*>::From paths;
					paths.push_back(roadChunk);
					EdgeProjector<RoadChunk*> projector(paths, 10000);
					try
					{
						EdgeProjector<RoadChunk*>::PathNearby projection(
							projector.projectEdge(*geometry->getCoordinate())
						);

						MetricOffset metricOffset(
							projection.get<2>()
						);

						// Import
						_createOrUpdatePublicPlaceEntrance(
							publicPlaceEntrances,
							code,
							optional<const string&>(),
							metricOffset,
							houseNumber,
							*roadChunk,
							*publicPlace
						);
					}
					catch(EdgeProjector<boost::shared_ptr<RoadChunk> >::NotFoundException)
					{
					}
				}
			}

			_logDebug("<b>SUCCESS : Data loaded</b>");

			return true;
		}



		DBTransaction RoadShapeFileFormat::Importer_::_save() const
		{
			DBTransaction transaction;
			BOOST_FOREACH(const Registry<Crossing>::value_type& crossing, _env.getEditableRegistry<Crossing>())
			{
				CrossingTableSync::Save(crossing.second.get(), transaction);
			}
			BOOST_FOREACH(const Registry<RoadPlace>::value_type& roadplace, _env.getEditableRegistry<RoadPlace>())
			{
				RoadPlaceTableSync::Save(roadplace.second.get(), transaction);
			}
			BOOST_FOREACH(const Registry<Road>::value_type& road, _env.getEditableRegistry<Road>())
			{
				RoadTableSync::Save(road.second.get(), transaction);
			}
			BOOST_FOREACH(const Registry<RoadChunk>::value_type& roadChunk, _env.getEditableRegistry<RoadChunk>())
			{
				RoadChunkTableSync::Save(roadChunk.second.get(), transaction);
			}
			BOOST_FOREACH(const Registry<PublicPlace>::value_type& publicPlace, _env.getEditableRegistry<PublicPlace>())
			{
				PublicPlaceTableSync::Save(publicPlace.second.get(), transaction);
			}
			BOOST_FOREACH(const Registry<PublicPlaceEntrance>::value_type& publicPlaceEntrance, _env.getEditableRegistry<PublicPlaceEntrance>())
			{
				PublicPlaceEntranceTableSync::Save(publicPlaceEntrance.second.get(), transaction);
			}
			return transaction;
		}



		util::ParametersMap RoadShapeFileFormat::Importer_::_getParametersMap() const
		{
			ParametersMap map;
			map.insert(PARAMETER_FIELD_ROAD_PLACES_CITY_CODE, _roadPlacesCityCodeField);
			map.insert(PARAMETER_FIELD_ROAD_PLACES_CODE, _roadPlacesCodeField);
			map.insert(PARAMETER_FIELD_ROAD_PLACES_NAME, _roadPlacesNameField);
			map.insert(PARAMETER_FIELD_ROAD_CHUNKS_FROM_LEFT, _roadChunksFromLeftField);
			map.insert(PARAMETER_FIELD_ROAD_CHUNKS_TO_LEFT, _roadChunksToLeftField);
			map.insert(PARAMETER_FIELD_ROAD_CHUNKS_FROM_RIGHT, _roadChunksFromRightField);
			map.insert(PARAMETER_FIELD_ROAD_CHUNKS_TO_RIGHT, _roadChunksToRightField);
			map.insert(PARAMETER_FIELD_ROAD_CHUNKS_ROAD_PLACE, _roadChunksRoadPlaceField);
			map.insert(PARAMETER_FIELD_ROAD_CHUNKS_CODE, _roadChunksCodeField);
			map.insert(PARAMETER_FIELD_ROAD_CHUNKS_START_NODE, _roadChunksStartNodeField);
			map.insert(PARAMETER_FIELD_ROAD_CHUNKS_END_NODE, _roadChunksEndNodeField);
			map.insert(PARAMETER_FIELD_PUBLIC_PLACES_CODE, _publicPlacesCodeField);
			map.insert(PARAMETER_FIELD_PUBLIC_PLACES_NAME, _publicPlacesNameField);
			map.insert(PARAMETER_FIELD_PUBLIC_PLACES_ROAD_CHUNK, _publicPlacesRoadChunkField);
			map.insert(PARAMETER_FIELD_PUBLIC_PLACES_NUMBER, _publicPlacesNumberField);
			map.insert(PARAMETER_FIELD_PUBLIC_PLACES_CITY_CODE, _publicPlacesCityCodeField);
			return map;
		}



		void RoadShapeFileFormat::Importer_::_setFromParametersMap( const util::ParametersMap& map )
		{
			_roadPlacesCityCodeField = map.getDefault<string>(PARAMETER_FIELD_ROAD_PLACES_CITY_CODE);
			_roadPlacesCodeField = map.getDefault<string>(PARAMETER_FIELD_ROAD_PLACES_CODE);
			_roadPlacesNameField = map.getDefault<string>(PARAMETER_FIELD_ROAD_PLACES_NAME);
			_roadChunksFromLeftField = map.getDefault<string>(PARAMETER_FIELD_ROAD_CHUNKS_FROM_LEFT);
			_roadChunksToLeftField = map.getDefault<string>(PARAMETER_FIELD_ROAD_CHUNKS_TO_LEFT);
			_roadChunksFromRightField = map.getDefault<string>(PARAMETER_FIELD_ROAD_CHUNKS_FROM_RIGHT);
			_roadChunksToRightField = map.getDefault<string>(PARAMETER_FIELD_ROAD_CHUNKS_TO_RIGHT);
			_roadChunksRoadPlaceField = map.getDefault<string>(PARAMETER_FIELD_ROAD_CHUNKS_ROAD_PLACE);
			_roadChunksCodeField = map.getDefault<string>(PARAMETER_FIELD_ROAD_CHUNKS_CODE);
			_roadChunksStartNodeField = map.getDefault<string>(PARAMETER_FIELD_ROAD_CHUNKS_START_NODE);
			_roadChunksEndNodeField = map.getDefault<string>(PARAMETER_FIELD_ROAD_CHUNKS_END_NODE);
			_publicPlacesCodeField = map.getDefault<string>(PARAMETER_FIELD_PUBLIC_PLACES_CODE);
			_publicPlacesNameField = map.getDefault<string>(PARAMETER_FIELD_PUBLIC_PLACES_NAME);
			_publicPlacesRoadChunkField = map.getDefault<string>(PARAMETER_FIELD_PUBLIC_PLACES_ROAD_CHUNK);
			_publicPlacesNumberField = map.getDefault<string>(PARAMETER_FIELD_PUBLIC_PLACES_NUMBER);
			_publicPlacesCityCodeField = map.getDefault<string>(PARAMETER_FIELD_PUBLIC_PLACES_CITY_CODE);
		}
}	}

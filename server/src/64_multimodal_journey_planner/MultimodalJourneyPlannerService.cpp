﻿
/** MultimodalJourneyPlannerService class implementation.
	@file MultimodalJourneyPlannerService.cpp

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

#include "MultimodalJourneyPlannerService.hpp"

#include "AccessParameters.h"
#include "AlgorithmLogger.hpp"
#include "AStarShortestPathCalculator.hpp"
#include "CommercialLine.h"
#include "ContinuousService.h"
#include "Crossing.h"
#include "Edge.h"
#include "JourneyPattern.hpp"
#include "Junction.hpp"
#include "MultimodalJourneyPlannerResult.h"
#include "NamedPlace.h"
#include "ParametersMap.h"
#include "PlacesListService.hpp"
#include "PTModule.h"
#include "PTRoutePlannerResult.h"
#include "PTTimeSlotRoutePlanner.h"
#include "PublicBikeJourneyPlanner.hpp"
#include "Request.h"
#include "RequestException.h"
#include "RoadChunk.h"
#include "RoadChunkEdge.hpp"
#include "RoadJourneyPlanner.h"
#include "RoadJourneyPlannerResult.h"
#include "RoadPath.hpp"
#include "ScheduledService.h"
#include "Service.h"
#include "Session.h"
#include "TransportNetwork.h"
#include "VertexAccessMap.h"
#include "Log.h"

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <geos/algorithm/CGAlgorithms.h>
#include <geos/geom/MultiLineString.h>
#include <sstream>

using namespace std;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;

namespace synthese
{
	using namespace geography;
	using namespace util;

	template<> const string util::FactorableTemplate<multimodal_journey_planner::MultimodalJourneyPlannerService::_FunctionWithSite,multimodal_journey_planner::MultimodalJourneyPlannerService>::FACTORY_KEY("multimodal_journey_planner");

	namespace multimodal_journey_planner
	{
		const string MultimodalJourneyPlannerService::PARAMETER_DEPARTURE_CITY_TEXT = "departure_city";
		const string MultimodalJourneyPlannerService::PARAMETER_ARRIVAL_CITY_TEXT = "arrival_city";
		const string MultimodalJourneyPlannerService::PARAMETER_DEPARTURE_PLACE_TEXT = "departure_place";
		const string MultimodalJourneyPlannerService::PARAMETER_ARRIVAL_PLACE_TEXT = "arrival_place";
		const string MultimodalJourneyPlannerService::PARAMETER_DEPARTURE_DAY = "departure_date";
		const string MultimodalJourneyPlannerService::PARAMETER_DEPARTURE_TIME = "departure_time";
		const string MultimodalJourneyPlannerService::PARAMETER_MAX_TRANSPORT_CONNECTION_COUNT = "max_transport_connection_count";
		const string MultimodalJourneyPlannerService::PARAMETER_USE_WALK = "use_walk";
		const string MultimodalJourneyPlannerService::PARAMETER_USE_PT = "use_pt";
		const string MultimodalJourneyPlannerService::PARAMETER_USE_PUBLICBIKE = "use_public_bike";
		const string MultimodalJourneyPlannerService::PARAMETER_LOGGER_PATH = "logger_path";

		const string MultimodalJourneyPlannerService::PARAMETER_ASTAR_FOR_WALK = "astar_for_walk"; //TODO : remove when algorithm is chosen

		const string MultimodalJourneyPlannerService::DATA_MULTIMODAL_JOURNEY_PLANNER_RESULT = "MultimodalJourneyPlannerResult";
		const string MultimodalJourneyPlannerService::DATA_ERROR_MESSAGE("error_message");


		MultimodalJourneyPlannerService::MultimodalJourneyPlannerService(
		):	_departureDay(boost::gregorian::day_clock::local_day()),
			_departureTime(not_a_date_time),
			_loggerPath()
		{}



		ParametersMap MultimodalJourneyPlannerService::_getParametersMap() const
		{
			ParametersMap map(FunctionWithSiteBase::_getParametersMap());

			// Departure place
			if(_departure_place.placeResult.value.get())
			{
				if(dynamic_cast<NamedPlace*>(_departure_place.placeResult.value.get()))
				{
					map.insert(
						PARAMETER_DEPARTURE_PLACE_TEXT,
						dynamic_cast<NamedPlace*>(_departure_place.placeResult.value.get())->getFullName()
					);
				}
				else if(dynamic_cast<City*>(_departure_place.placeResult.value.get()))
				{
					map.insert(
						PARAMETER_DEPARTURE_CITY_TEXT,
						dynamic_cast<City*>(_departure_place.placeResult.value.get())->getName()
					);
				}
			}

			// Arrival place
			if(_arrival_place.placeResult.value.get())
			{
				if(dynamic_cast<NamedPlace*>(_arrival_place.placeResult.value.get()))
				{
					map.insert(
						PARAMETER_ARRIVAL_PLACE_TEXT,
						dynamic_cast<NamedPlace*>(_arrival_place.placeResult.value.get())->getFullName()
					);
				}
				else if(dynamic_cast<City*>(_arrival_place.placeResult.value.get()))
				{
					map.insert(
						PARAMETER_ARRIVAL_CITY_TEXT,
						dynamic_cast<City*>(_arrival_place.placeResult.value.get())->getName()
					);
				}
			}

			// Departure day
			if(!_departureDay.is_not_a_date())
			{
				map.insert(PARAMETER_DEPARTURE_DAY, _departureDay);
			}

			// Departure time
			if(!_departureTime.is_not_a_date_time())
			{
				map.insert(PARAMETER_DEPARTURE_TIME, _departureTime);
			}

			// Max transport connection count
			if(_maxTransportConnectionCount)
			{
				map.insert(
					PARAMETER_MAX_TRANSPORT_CONNECTION_COUNT,
					_maxTransportConnectionCount.get()
				);
			}

			// Use walk
			map.insert(PARAMETER_USE_WALK, _useWalk);

			// Use pt
			map.insert(PARAMETER_USE_PT, _useWalk);

			// Use public_bike
			map.insert(PARAMETER_USE_PUBLICBIKE, _usePublicBike);

			// Logger path
			if(!_loggerPath.empty())
			{
				map.insert(PARAMETER_LOGGER_PATH, _loggerPath.string());
			}

			return map;
		}



		void MultimodalJourneyPlannerService::_setFromParametersMap(const ParametersMap& map)
		{
			Log::GetInstance().debug("MultimodalJourneyPlannerService::_setFromParametersMap : start");

			_FunctionWithSite::_setFromParametersMap(map);

			// Departure place
			if( // Two fields input
				map.isDefined(PARAMETER_DEPARTURE_CITY_TEXT) &&
				map.isDefined(PARAMETER_DEPARTURE_PLACE_TEXT)
			){
				_originCityText = map.getDefault<string>(PARAMETER_DEPARTURE_CITY_TEXT);
				_originPlaceText = map.getDefault<string>(PARAMETER_DEPARTURE_PLACE_TEXT);
				if(!_originCityText.empty() && !_originPlaceText.empty())
				{
					_departure_place = road::RoadModule::ExtendedFetchPlace(_originCityText, _originPlaceText);
				}
				else if (!_originPlaceText.empty())
				{
					// One field input
					pt_website::PlacesListService placesListService;
					placesListService.setNumber(1);
					placesListService.setCoordinatesSystem(&CoordinatesSystem::GetInstanceCoordinatesSystem());

					placesListService.setText(_originPlaceText);
					_departure_place.placeResult = placesListService.getPlaceFromBestResult(
						placesListService.runWithoutOutput()
					);
				}
				else
				{
					throw server::RequestException("Empty departure place.");
				}
			}
			else if(map.isDefined(PARAMETER_DEPARTURE_PLACE_TEXT))
			{ // One field input
				pt_website::PlacesListService placesListService;
				placesListService.setNumber(1);
				placesListService.setCoordinatesSystem(&CoordinatesSystem::GetInstanceCoordinatesSystem());

				placesListService.setText(map.get<string>(PARAMETER_DEPARTURE_PLACE_TEXT));
				_departure_place.placeResult = placesListService.getPlaceFromBestResult(
					placesListService.runWithoutOutput()
				);
			}
			else
			{
				throw server::RequestException("Empty departure place.");
			}

			Log::GetInstance().debug("MultimodalJourneyPlannerService::_setFromParametersMap : after matching of departure place");

			// Destination
			if( // Two fields input
				map.isDefined(PARAMETER_ARRIVAL_CITY_TEXT) &&
				map.isDefined(PARAMETER_ARRIVAL_PLACE_TEXT)
			){
				_destinationCityText = map.getDefault<string>(PARAMETER_ARRIVAL_CITY_TEXT);
				_destinationPlaceText = map.getDefault<string>(PARAMETER_ARRIVAL_PLACE_TEXT);
				if(!_destinationCityText.empty() && !_destinationPlaceText.empty())
				{
					_arrival_place = road::RoadModule::ExtendedFetchPlace(_destinationCityText, _destinationPlaceText);
				}
				else if (!_destinationPlaceText.empty())
				{
					// One field input
					pt_website::PlacesListService placesListService;
					placesListService.setNumber(1);
					placesListService.setCoordinatesSystem(&CoordinatesSystem::GetInstanceCoordinatesSystem());

					placesListService.setText(_destinationPlaceText);
					_arrival_place.placeResult = placesListService.getPlaceFromBestResult(
						placesListService.runWithoutOutput()
					);
				}
				else
				{
					throw server::RequestException("Empty arrival place.");
				}
			}
			else if(map.isDefined(PARAMETER_ARRIVAL_PLACE_TEXT))
			{ // One field input
				pt_website::PlacesListService placesListService;
				placesListService.setNumber(1);
				placesListService.setCoordinatesSystem(&CoordinatesSystem::GetInstanceCoordinatesSystem());

				placesListService.setText(map.get<string>(PARAMETER_ARRIVAL_PLACE_TEXT));
				_arrival_place.placeResult = placesListService.getPlaceFromBestResult(
					placesListService.runWithoutOutput()
				);
			}
			else
			{
				throw server::RequestException("Empty arrival place.");
			}

			Log::GetInstance().debug("MultimodalJourneyPlannerService::_setFromParametersMap : after matching of arrival place");

			// Date parameters
			try
			{
				// by day and time
				if(!map.getDefault<string>(PARAMETER_DEPARTURE_DAY).empty())
				{
					// Day
					_departureDay = from_string(map.get<string>(PARAMETER_DEPARTURE_DAY));

					// Time
					if(!map.getDefault<string>(PARAMETER_DEPARTURE_TIME).empty())
					{
						_departureTime = duration_from_string(map.get<string>(PARAMETER_DEPARTURE_TIME));
					}
				}
			}
			catch(ParametersMap::MissingParameterException& e)
			{
				throw server::RequestException(e.what());
			}

			// Maximum allowed transport connections
			_maxTransportConnectionCount= map.getOptional<size_t>(PARAMETER_MAX_TRANSPORT_CONNECTION_COUNT);

			// TEMP
			_aStarForWalk = map.getDefault<bool>(PARAMETER_ASTAR_FOR_WALK, false);
			
			_useWalk = map.getDefault<bool>(PARAMETER_USE_WALK, false);
			_usePt = map.getDefault<bool>(PARAMETER_USE_PT, false);
			_usePublicBike = map.getDefault<bool>(PARAMETER_USE_PUBLICBIKE, false);

			std::string pathString(map.getDefault<string>(PARAMETER_LOGGER_PATH, ""));
			if (!pathString.empty())
			{
				_loggerPath = boost::filesystem::path(pathString);
			}
			Function::setOutputFormatFromMap(map,string());
		}



		util::ParametersMap MultimodalJourneyPlannerService::run(
			ostream& stream,
			const server::Request& request
		) const	{

			Log::GetInstance().debug("MultimodalJourneyPlannerService::run");

			//////////////////////////////////////////////////////////////////////////
			// Display
			ParametersMap pm;

			if(!_departure_place.placeResult.value || !_arrival_place.placeResult.value)
			{
				pm.insert(DATA_ERROR_MESSAGE, string("Departure or arrival place not found"));

				if(_outputFormat == MimeTypes::XML)
				{
					pm.outputXML(
						stream,
						DATA_MULTIMODAL_JOURNEY_PLANNER_RESULT,
						true,
						"http://schemas.open-transport.org/smile/MultimodalJourneyPlanner.xsd"
					);
				}
				else if(_outputFormat == MimeTypes::JSON)
				{
					pm.outputJSON(stream, DATA_MULTIMODAL_JOURNEY_PLANNER_RESULT);
				}

				return pm;
			}

			Place* departure = _departure_place.placeResult.value.get();
			Place* arrival = _arrival_place.placeResult.value.get();

			graph::AccessParameters pedestrianAccessParameters(graph::USER_PEDESTRIAN, false, false, 72000, boost::posix_time::hours(24), 1.111);

			ptime startDate;
			if (_departureTime.is_not_a_date_time())
			{
				startDate = ptime(boost::posix_time::second_clock::local_time());
			}
			else
			{
				startDate = ptime(_departureDay, _departureTime);
			}
			ptime endDate = startDate + hours(24);
			
			if (_useWalk && _aStarForWalk)
			{
				Log::GetInstance().debug("MultimodalJourneyPlannerService::run : before A* walk");
				// Astar algorithm
				algorithm::AStarShortestPathCalculator r(
					departure,
					arrival,
					startDate,
					pedestrianAccessParameters
				);

				algorithm::AStarShortestPathCalculator::ResultPath path(r.run());

				Log::GetInstance().debug("MultimodalJourneyPlannerService::run : after A* walk");
				
				if(!path.empty())
				{
					boost::shared_ptr<ParametersMap> submapJourney(new ParametersMap);
					submapJourney->insert("departure_date_time", startDate);

					// Departure place
					boost::shared_ptr<ParametersMap> submapDeparturePlace(new ParametersMap);
					if(dynamic_cast<const NamedPlace*>(departure))
					{
						submapDeparturePlace->insert("name", dynamic_cast<const NamedPlace*>(departure)->getFullName());
						submapDeparturePlace->insert("type", dynamic_cast<const NamedPlace*>(departure)->getFactoryKey());
						submapDeparturePlace->insert("id", dynamic_cast<const NamedPlace*>(departure)->getKey());
					}
					else
					{
						submapDeparturePlace->insert("name", dynamic_cast<const City*>(departure)->getName());
						string strCityType("City");
						submapDeparturePlace->insert("type", strCityType);
						submapDeparturePlace->insert("id", dynamic_cast<const City*>(departure)->getKey());
					}

					if(departure->getPoint().get() &&
						!departure->getPoint()->isEmpty())
					{
						boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
							*(departure->getPoint())
						)	);
						submapDeparturePlace->insert("longitude", wgs84Point->getX());
						submapDeparturePlace->insert("latitude", wgs84Point->getY());
					}

					// Arrival place
					boost::shared_ptr<ParametersMap> submapArrivalPlace(new ParametersMap);
					if(dynamic_cast<const NamedPlace*>(arrival))
					{
						submapArrivalPlace->insert("name", dynamic_cast<const NamedPlace*>(arrival)->getFullName());
						submapArrivalPlace->insert("type", dynamic_cast<const NamedPlace*>(arrival)->getFactoryKey());
						submapArrivalPlace->insert("id", dynamic_cast<const NamedPlace*>(arrival)->getKey());
					}
					else
					{
						submapArrivalPlace->insert("name", dynamic_cast<const City*>(arrival)->getName());
						string strCityType("City");
						submapArrivalPlace->insert("type", strCityType);
						submapArrivalPlace->insert("id", dynamic_cast<const City*>(arrival)->getKey());
					}

					if(arrival->getPoint().get() &&
						!arrival->getPoint()->isEmpty())
					{
						boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
							*(arrival->getPoint())
						)	);
						submapArrivalPlace->insert("longitude", wgs84Point->getX());
						submapArrivalPlace->insert("latitude", wgs84Point->getY());
					}

					submapJourney->insert("departure", submapDeparturePlace);
					submapJourney->insert("arrival", submapArrivalPlace);

					int curDistance(0);
					boost::posix_time::ptime arrivalTime;
					boost::posix_time::ptime departureTime(startDate);
					bool first(true);
					vector<geos::geom::Geometry*> geometries;
					vector<geos::geom::Geometry*> curGeom;
					double speed(pedestrianAccessParameters.getApproachSpeed());
					boost::shared_ptr<ParametersMap> submapLegDeparturePlace(new ParametersMap);
					boost::shared_ptr<ParametersMap> submapLegArrivalPlace(new ParametersMap);
					road::RoadPlace* curRoadPlace(NULL);
					const road::RoadChunkEdge* lastChunk;

					BOOST_FOREACH(const road::RoadChunkEdge* chunk, path)
					{
						const road::Road* road(chunk->getRoadChunk()->getRoad());
						lastChunk = chunk;
						boost::shared_ptr<geos::geom::LineString> geometry = chunk->getRealGeometry();
						boost::shared_ptr<geos::geom::Geometry> geometryProjected(CoordinatesSystem::GetCoordinatesSystem(4326).convertGeometry(*geometry));
						double distance(0);
						if(geometry)
						{
							geos::geom::CoordinateSequence* coordinates = geometry->getCoordinates();
							distance = geos::algorithm::CGAlgorithms::length(coordinates);
							delete coordinates;
						}

						if(first)
						{
							curDistance = static_cast<int>(distance);
							arrivalTime = startDate + boost::posix_time::seconds(static_cast<int>(distance / speed));
							curRoadPlace = &*road->getAnyRoadPlace();
							curGeom.push_back(geometryProjected.get()->clone());
							first = false;

							// Departure place
							if(dynamic_cast<const road::Crossing*>(chunk->getFromVertex()->getHub()))
							{
								submapLegDeparturePlace->insert("name", (string)("Croisement"));
								submapLegDeparturePlace->insert("type", (string)("crossing"));
								submapLegDeparturePlace->insert("id", dynamic_cast<const road::Crossing*>(chunk->getFromVertex()->getHub())->getKey());
							}

							if(chunk->getFromVertex()->getGeometry().get() &&
								!chunk->getFromVertex()->getGeometry()->isEmpty())
							{
								boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
									*(chunk->getFromVertex()->getGeometry())
								)	);
								submapLegDeparturePlace->insert("longitude", wgs84Point->getX());
								submapLegDeparturePlace->insert("latitude", wgs84Point->getY());
							}
						}
						else if(curRoadPlace->getName() == road->getAnyRoadPlace()->getName())
						{
							curDistance += static_cast<int>(distance);
							arrivalTime = arrivalTime + boost::posix_time::seconds(static_cast<int>(distance / speed));
							curGeom.push_back(geometryProjected.get()->clone());
						}
						else
						{
							geos::geom::MultiLineString* multiLineString = CoordinatesSystem::GetCoordinatesSystem(4326).getGeometryFactory().createMultiLineString(curGeom);
							geometries.push_back(multiLineString->clone());

							boost::shared_ptr<ParametersMap> submapLeg(new ParametersMap);
							submapLeg->insert("departure_date_time", departureTime);
							submapLeg->insert("departure", submapLegDeparturePlace);

							// Arrival place
							if(dynamic_cast<const road::Crossing*>(chunk->getFromVertex()->getHub()))
							{
								submapLegArrivalPlace->insert("name", (string)("Croisement"));
								submapLegArrivalPlace->insert("type", (string)("crossing"));
								submapLegArrivalPlace->insert("id", dynamic_cast<const road::Crossing*>(chunk->getFromVertex()->getHub())->getKey());
							}

							if(chunk->getFromVertex()->getGeometry().get() &&
								!chunk->getFromVertex()->getGeometry()->isEmpty())
							{
								boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
									*(chunk->getFromVertex()->getGeometry())
								)	);
								submapLegArrivalPlace->insert("longitude", wgs84Point->getX());
								submapLegArrivalPlace->insert("latitude", wgs84Point->getY());
							}
							submapLeg->insert("arrival", submapLegArrivalPlace);

							submapLegDeparturePlace.reset(new ParametersMap);
							submapLegArrivalPlace.reset(new ParametersMap);

							submapLeg->insert("arrival_date_time", arrivalTime);
							submapLeg->insert("geometry", multiLineString->toString());
							boost::shared_ptr<ParametersMap> submapWalkAttributes(new ParametersMap);
							submapWalkAttributes->insert("length", curDistance);

							boost::shared_ptr<ParametersMap> submapLegRoad(new ParametersMap);
							submapLegRoad->insert("name", curRoadPlace->getName());
							submapLegRoad->insert("id", curRoadPlace->getKey());

							submapWalkAttributes->insert("road", submapLegRoad);

							submapLeg->insert("walk_attributes", submapWalkAttributes);

							submapJourney->insert("leg", submapLeg);

							delete multiLineString;
							curDistance = static_cast<int>(distance);
							departureTime = arrivalTime;
							arrivalTime = departureTime + boost::posix_time::seconds(static_cast<int>(distance / speed));
							curRoadPlace = &*road->getAnyRoadPlace();

							BOOST_FOREACH(geos::geom::Geometry* geomToDelete, curGeom)
							{
								delete geomToDelete;
							}

							curGeom.clear();
							curGeom.push_back(geometryProjected.get()->clone());

							// New departure place
							if(dynamic_cast<const road::Crossing*>(chunk->getFromVertex()->getHub()))
							{
								submapLegDeparturePlace->insert("name", (string)("Croisement"));
								submapLegDeparturePlace->insert("type", (string)("crossing"));
								submapLegDeparturePlace->insert("id", dynamic_cast<const road::Crossing*>(chunk->getFromVertex()->getHub())->getKey());
							}

							if(chunk->getFromVertex()->getGeometry().get() &&
								!chunk->getFromVertex()->getGeometry()->isEmpty())
							{
								boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
									*(chunk->getFromVertex()->getGeometry())
								)	);
								submapLegDeparturePlace->insert("longitude", wgs84Point->getX());
								submapLegDeparturePlace->insert("latitude", wgs84Point->getY());
							}
						}
					}

					geos::geom::MultiLineString* multiLineString = CoordinatesSystem::GetCoordinatesSystem(4326).getGeometryFactory().createMultiLineString(curGeom);
					geometries.push_back(multiLineString->clone());

					BOOST_FOREACH(geos::geom::Geometry* geomToDelete, curGeom)
					{
						delete geomToDelete;
					}
					curGeom.clear();
					boost::shared_ptr<ParametersMap> submapLeg(new ParametersMap);
					submapLeg->insert("departure_date_time", departureTime);
					submapLeg->insert("departure", submapLegDeparturePlace);

					// Arrival place
					if(dynamic_cast<const road::Crossing*>(lastChunk->getFromVertex()->getHub()))
					{
						submapLegArrivalPlace->insert("name", (string)("Croisement"));
						submapLegArrivalPlace->insert("type", (string)("crossing"));
						submapLegArrivalPlace->insert("id", dynamic_cast<const road::Crossing*>(lastChunk->getFromVertex()->getHub())->getKey());
					}

					if(lastChunk->getFromVertex()->getGeometry().get() &&
						!lastChunk->getFromVertex()->getGeometry()->isEmpty())
					{
						boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
							*(lastChunk->getFromVertex()->getGeometry())
						)	);
						submapLegArrivalPlace->insert("longitude", wgs84Point->getX());
						submapLegArrivalPlace->insert("latitude", wgs84Point->getY());
					}
					submapLeg->insert("arrival", submapLegArrivalPlace);

					submapLeg->insert("arrival_date_time", arrivalTime);
					submapLeg->insert("geometry", multiLineString->toString());
					boost::shared_ptr<ParametersMap> submapWalkAttributes(new ParametersMap);
					submapWalkAttributes->insert("length", curDistance);

					boost::shared_ptr<ParametersMap> submapLegRoad(new ParametersMap);
					submapLegRoad->insert("name", curRoadPlace->getName());
					submapLegRoad->insert("id", curRoadPlace->getKey());

					submapWalkAttributes->insert("road", submapLegRoad);

					submapLeg->insert("walk_attributes", submapWalkAttributes);

					submapJourney->insert("leg", submapLeg);

					delete multiLineString;

					submapJourney->insert("arrival_date_time", arrivalTime);
					geos::geom::GeometryCollection* geometryCollection(CoordinatesSystem::GetCoordinatesSystem(4326).getGeometryFactory().createGeometryCollection(geometries));
					BOOST_FOREACH(geos::geom::Geometry* geomToDelete, geometries)
					{
						delete geomToDelete;
					}
					geometries.clear();
					submapJourney->insert("geometry", geometryCollection->toString());
					delete geometryCollection;
					pm.insert("journey", submapJourney);
					
				}

				Log::GetInstance().debug("MultimodalJourneyPlannerService::run : after A* walk data processing");
			}
			else if (_useWalk)
			{
				Log::GetInstance().debug("MultimodalJourneyPlannerService::run : before SYNTHESE walk");

				// Classical synthese algorithm
				// TODO : Note we cannot really use AlgorithmLogger right now since not targetted for pure walk journey search 
				algorithm::AlgorithmLogger logger;
				road_journey_planner::RoadJourneyPlanner rjp(
					departure,
					arrival,
					startDate,
					startDate,
					endDate,
					endDate,
					1,
					pedestrianAccessParameters,
					algorithm::DEPARTURE_FIRST,
					logger,
					true
				);
				road_journey_planner::RoadJourneyPlannerResult results = rjp.run();

				Log::GetInstance().debug("MultimodalJourneyPlannerService::run : after SYNTHESE walk");
				
				if(!results.getJourneys().empty())
				{
					for (road_journey_planner::RoadJourneyPlannerResult::Journeys::const_iterator it(results.getJourneys().begin()); it != results.getJourneys().end(); ++it)
					{
						boost::shared_ptr<ParametersMap> submapJourney(new ParametersMap);
						submapJourney->insert("departure_date_time", it->getFirstDepartureTime());
						submapJourney->insert("arrival_date_time", it->getFirstArrivalTime());

						// Departure place
						boost::shared_ptr<ParametersMap> submapDeparturePlace(new ParametersMap);
						if(dynamic_cast<const road::Crossing*>(it->getOrigin()->getHub()))
						{
							if(dynamic_cast<const NamedPlace*>(departure))
							{
								submapDeparturePlace->insert("name", dynamic_cast<const NamedPlace*>(departure)->getFullName());
								submapDeparturePlace->insert("type", dynamic_cast<const NamedPlace*>(departure)->getFactoryKey());
								submapDeparturePlace->insert("id", dynamic_cast<const NamedPlace*>(departure)->getKey());
							}
							else
							{
								submapDeparturePlace->insert("name", dynamic_cast<const City*>(departure)->getName());
								string strCityType("City");
								submapDeparturePlace->insert("type", strCityType);
								submapDeparturePlace->insert("id", dynamic_cast<const City*>(departure)->getKey());
							}
						}
						else
						{
							submapDeparturePlace->insert("name", dynamic_cast<const NamedPlace*>(it->getOrigin()->getHub())->getFullName());
							submapDeparturePlace->insert("type", dynamic_cast<const NamedPlace*>(it->getOrigin()->getHub())->getFactoryKey());
							submapDeparturePlace->insert("id", dynamic_cast<const NamedPlace*>(it->getOrigin()->getHub())->getKey());
						}

						if(it->getOrigin()->getFromVertex()->getGeometry().get() &&
							!it->getOrigin()->getFromVertex()->getGeometry()->isEmpty())
						{
							boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
								*(it->getOrigin()->getFromVertex()->getGeometry())
							)	);
							submapDeparturePlace->insert("longitude", wgs84Point->getX());
							submapDeparturePlace->insert("latitude", wgs84Point->getY());
						}

						// Arrival place
						boost::shared_ptr<ParametersMap> submapArrivalPlace(new ParametersMap);
						if(dynamic_cast<const road::Crossing*>(it->getDestination()->getHub()))
						{
							if(dynamic_cast<const NamedPlace*>(arrival))
							{
								submapArrivalPlace->insert("name", dynamic_cast<const NamedPlace*>(arrival)->getFullName());
								submapArrivalPlace->insert("type", dynamic_cast<const NamedPlace*>(arrival)->getFactoryKey());
								submapArrivalPlace->insert("id", dynamic_cast<const NamedPlace*>(arrival)->getKey());
							}
							else
							{
								submapArrivalPlace->insert("name", dynamic_cast<const City*>(arrival)->getName());
								string strCityType("City");
								submapArrivalPlace->insert("type", strCityType);
								submapArrivalPlace->insert("id", dynamic_cast<const City*>(arrival)->getKey());
							}
						}
						else
						{
							submapArrivalPlace->insert("name", dynamic_cast<const NamedPlace*>(it->getDestination()->getHub())->getFullName());
							submapArrivalPlace->insert("type", dynamic_cast<const NamedPlace*>(it->getDestination()->getHub())->getFactoryKey());
							submapArrivalPlace->insert("id", dynamic_cast<const NamedPlace*>(it->getDestination()->getHub())->getKey());
						}

						if(it->getDestination()->getFromVertex()->getGeometry().get() &&
							!it->getDestination()->getFromVertex()->getGeometry()->isEmpty())
						{
							boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
								*(it->getDestination()->getFromVertex()->getGeometry())
							)	);
							submapArrivalPlace->insert("longitude", wgs84Point->getX());
							submapArrivalPlace->insert("latitude", wgs84Point->getY());
						}

						submapJourney->insert("departure", submapDeparturePlace);
						submapJourney->insert("arrival", submapArrivalPlace);

						graph::Journey::ServiceUses::const_iterator its(it->getServiceUses().begin());
						vector<boost::shared_ptr<geos::geom::Geometry> > geometriesSPtr; // To keep shared_ptr's in scope !
						vector<geos::geom::Geometry*> allGeometries;
						while(true)
						{
							boost::shared_ptr<ParametersMap> submapLeg(new ParametersMap);
							submapLeg->insert("departure_date_time", its->getDepartureDateTime());
							// Departure place
							boost::shared_ptr<ParametersMap> submapDeparturePlace(new ParametersMap);
							if(dynamic_cast<const road::Crossing*>(its->getRealTimeDepartureVertex()->getHub()))
							{
								submapDeparturePlace->insert("name", (string)("Croisement"));
								submapDeparturePlace->insert("type", (string)("crossing"));
								submapDeparturePlace->insert("id", dynamic_cast<const road::Crossing*>(its->getRealTimeDepartureVertex()->getHub())->getKey());
							}

							if(its->getRealTimeDepartureVertex()->getGeometry().get() &&
								!its->getRealTimeDepartureVertex()->getGeometry()->isEmpty())
							{
								boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
									*(its->getRealTimeDepartureVertex()->getGeometry())
								)	);
								submapDeparturePlace->insert("longitude", wgs84Point->getX());
								submapDeparturePlace->insert("latitude", wgs84Point->getY());
							}

							submapLeg->insert("departure", submapDeparturePlace);

							const road::Road* road(dynamic_cast<const road::RoadPath*>(its->getService()->getPath())->getRoad());

							std::string roadName = road->getAnyRoadPlace()->getName();
							if(roadName.empty()) {
								if(	road->get<RoadTypeField>() == road::ROAD_TYPE_PEDESTRIANPATH ||
									road->get<RoadTypeField>() == road::ROAD_TYPE_PEDESTRIANSTREET
								){
									roadName="Chemin Piéton";
								}
								else if(road->get<RoadTypeField>() == road::ROAD_TYPE_STEPS) {
									roadName="Escaliers";
								}
								else if(road->get<RoadTypeField>() == road::ROAD_TYPE_BRIDGE) {
									roadName="Pont / Passerelle";
								}
								else if(road->get<RoadTypeField>() == road::ROAD_TYPE_TUNNEL) {
									roadName="Tunnel";
								}
								else {
									roadName="Route sans nom";
								}
							}
							double dst = its->getDistance();
							vector<geos::geom::Geometry*> geometries;
							graph::Journey::ServiceUses::const_iterator next = its+1;
							boost::shared_ptr<geos::geom::LineString> geometry(its->getGeometry());
							if(geometry.get())
							{
								boost::shared_ptr<geos::geom::Geometry> wgs84LineString(CoordinatesSystem::GetCoordinatesSystem(4326).convertGeometry(
									*geometry
								)	);
								geometries.push_back(wgs84LineString.get());
								allGeometries.push_back(wgs84LineString.get());
								geometriesSPtr.push_back(wgs84LineString);
							}
							while (next != it->getServiceUses().end())
							{
								string nextRoadName(
									dynamic_cast<const road::RoadPath*>(next->getService()->getPath())->getRoad()->getAnyRoadPlace()->getName()
								);
								if(!roadName.compare(nextRoadName))
								{
									++its;
									dst += its->getDistance();
									boost::shared_ptr<geos::geom::LineString> geometry(its->getGeometry());
									if(geometry.get())
									{
										boost::shared_ptr<geos::geom::Geometry> wgs84LineString(CoordinatesSystem::GetCoordinatesSystem(4326).convertGeometry(
											*geometry
										)	);
										geometries.push_back(wgs84LineString.get());
										allGeometries.push_back(wgs84LineString.get());
										geometriesSPtr.push_back(wgs84LineString);
									}
									next = its+1;
								}
								else
								{
									break;
								}
							}
							boost::shared_ptr<geos::geom::MultiLineString> multiLineString(
								CoordinatesSystem::GetCoordinatesSystem(4326).getGeometryFactory().createMultiLineString(
									geometries
							)	);
							submapLeg->insert("arrival_date_time", its->getArrivalDateTime());
							submapLeg->insert("geometry", multiLineString->toString());
							boost::shared_ptr<ParametersMap> submapWalkAttributes(new ParametersMap);
							submapWalkAttributes->insert("length", dst);

							// Arrival place
							boost::shared_ptr<ParametersMap> submapArrivalPlace(new ParametersMap);
							if(dynamic_cast<const road::Crossing*>(its->getRealTimeArrivalVertex()->getHub()))
							{
								submapArrivalPlace->insert("name", (string)("Croisement"));
								submapArrivalPlace->insert("type", (string)("crossing"));
								submapArrivalPlace->insert("id", dynamic_cast<const road::Crossing*>(its->getRealTimeArrivalVertex()->getHub())->getKey());
							}

							if(its->getRealTimeArrivalVertex()->getGeometry().get() &&
								!its->getRealTimeArrivalVertex()->getGeometry()->isEmpty())
							{
								boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
									*(its->getRealTimeArrivalVertex()->getGeometry())
								)	);
								submapArrivalPlace->insert("longitude", wgs84Point->getX());
								submapArrivalPlace->insert("latitude", wgs84Point->getY());
							}

							submapLeg->insert("arrival", submapArrivalPlace);

							boost::shared_ptr<ParametersMap> submapLegRoad(new ParametersMap);
							submapLegRoad->insert("name", roadName);
							submapLegRoad->insert("id", road->getAnyRoadPlace()->getKey());

							submapWalkAttributes->insert("road", submapLegRoad);

							submapLeg->insert("walk_attributes", submapWalkAttributes);

							submapJourney->insert("leg", submapLeg);

						   // Next service use
						   if(its == (it->getServiceUses().end()-1)) break;
						   ++its;
						}

						boost::shared_ptr<geos::geom::MultiLineString> multiLineString(
							CoordinatesSystem::GetCoordinatesSystem(4326).getGeometryFactory().createMultiLineString(
								allGeometries
						)	);
						submapJourney->insert("geometry", multiLineString->toString());
						pm.insert("journey", submapJourney);
					}
				}

				Log::GetInstance().debug("MultimodalJourneyPlannerService::run : after SYNTHESE walk data processing");
			}

			// TC
			if (_usePt)
			{
				Log::GetInstance().debug("MultimodalJourneyPlannerService::run : before pt");

				// Initialization
				graph::AccessParameters approachAccessParameters(
					graph::USER_PEDESTRIAN,			// user class code
					false,							// DRT only
					false,							// without DRT
					_useWalk ? 1000 : 0,			// max approach distance
					boost::posix_time::minutes(23),	// max approach time
					1.111,							// approach speed
					_maxTransportConnectionCount	// max transport connection count (ie : max number of used transport services - 1)
				);

				boost::shared_ptr<algorithm::AlgorithmLogger> logger(new algorithm::AlgorithmLogger(_loggerPath));

				pt_journey_planner::PTTimeSlotRoutePlanner r(
					_departure_place.placeResult.value.get(),
					_arrival_place.placeResult.value.get(),
					startDate,
					endDate,
					startDate,
					endDate,
					3,
					approachAccessParameters,
					algorithm::DEPARTURE_FIRST,
					false,
					*logger
				);
				// Computing
				pt_journey_planner::PTRoutePlannerResult tcResult = r.run();

				Log::GetInstance().debug("MultimodalJourneyPlannerService::run : after pt");

				if(!tcResult.getJourneys().empty())
				{
					for (pt_journey_planner::PTRoutePlannerResult::Journeys::const_iterator it(tcResult.getJourneys().begin()); it != tcResult.getJourneys().end(); ++it)
					{
						boost::shared_ptr<ParametersMap> submapJourney(new ParametersMap);
						submapJourney->insert("departure_date_time", it->getFirstDepartureTime());
						submapJourney->insert("arrival_date_time", it->getFirstArrivalTime());

						// Departure place
						boost::shared_ptr<ParametersMap> submapDeparturePlace(new ParametersMap);
						if(dynamic_cast<const road::Crossing*>(it->getOrigin()->getHub()))
						{
							if(dynamic_cast<const NamedPlace*>(departure))
							{
								submapDeparturePlace->insert("name", dynamic_cast<const NamedPlace*>(departure)->getFullName());
								submapDeparturePlace->insert("type", dynamic_cast<const NamedPlace*>(departure)->getFactoryKey());
								submapDeparturePlace->insert("id", dynamic_cast<const NamedPlace*>(departure)->getKey());
							}
							else
							{
								submapDeparturePlace->insert("name", dynamic_cast<const City*>(departure)->getName());
								string strCityType("City");
								submapDeparturePlace->insert("type", strCityType);
								submapDeparturePlace->insert("id", dynamic_cast<const City*>(departure)->getKey());
							}
						}
						else
						{
							submapDeparturePlace->insert("name", dynamic_cast<const NamedPlace*>(it->getOrigin()->getHub())->getFullName());
							submapDeparturePlace->insert("type", dynamic_cast<const NamedPlace*>(it->getOrigin()->getHub())->getFactoryKey());
							submapDeparturePlace->insert("id", dynamic_cast<const NamedPlace*>(it->getOrigin()->getHub())->getKey());
						}

						if(it->getOrigin()->getFromVertex()->getGeometry().get() &&
							!it->getOrigin()->getFromVertex()->getGeometry()->isEmpty())
						{
							boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
								*(it->getOrigin()->getFromVertex()->getGeometry())
							)	);
							submapDeparturePlace->insert("longitude", wgs84Point->getX());
							submapDeparturePlace->insert("latitude", wgs84Point->getY());
						}

						// Arrival place
						boost::shared_ptr<ParametersMap> submapArrivalPlace(new ParametersMap);
						if(dynamic_cast<const road::Crossing*>(it->getDestination()->getHub()))
						{
							if(dynamic_cast<const NamedPlace*>(arrival))
							{
								submapArrivalPlace->insert("name", dynamic_cast<const NamedPlace*>(arrival)->getFullName());
								submapArrivalPlace->insert("type", dynamic_cast<const NamedPlace*>(arrival)->getFactoryKey());
								submapArrivalPlace->insert("id", dynamic_cast<const NamedPlace*>(arrival)->getKey());
							}
							else
							{
								submapArrivalPlace->insert("name", dynamic_cast<const City*>(arrival)->getName());
								string strCityType("City");
								submapArrivalPlace->insert("type", strCityType);
								submapArrivalPlace->insert("id", dynamic_cast<const City*>(arrival)->getKey());
							}
						}
						else
						{
							submapArrivalPlace->insert("name", dynamic_cast<const NamedPlace*>(it->getDestination()->getHub())->getFullName());
							submapArrivalPlace->insert("type", dynamic_cast<const NamedPlace*>(it->getDestination()->getHub())->getFactoryKey());
							submapArrivalPlace->insert("id", dynamic_cast<const NamedPlace*>(it->getDestination()->getHub())->getKey());
						}

						if(it->getDestination()->getFromVertex()->getGeometry().get() &&
							!it->getDestination()->getFromVertex()->getGeometry()->isEmpty())
						{
							boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
								*(it->getDestination()->getFromVertex()->getGeometry())
							)	);
							submapArrivalPlace->insert("longitude", wgs84Point->getX());
							submapArrivalPlace->insert("latitude", wgs84Point->getY());
						}

						submapJourney->insert("departure", submapDeparturePlace);
						submapJourney->insert("arrival", submapArrivalPlace);

						graph::Journey::ServiceUses::const_iterator its(it->getServiceUses().begin());
						vector<boost::shared_ptr<geos::geom::Geometry> > geometriesSPtr; // To keep shared_ptr's in scope !
						vector<geos::geom::Geometry*> allGeometries;
						while(true)
						{
							const road::RoadPath* road(dynamic_cast<const road::RoadPath*> (its->getService()->getPath()));
							const pt::Junction* junction(dynamic_cast<const pt::Junction*> (its->getService()->getPath()));

							if (road)
							{
								// Approach leg
								boost::shared_ptr<ParametersMap> submapWalkLeg(new ParametersMap);
								submapWalkLeg->insert("departure_date_time", its->getDepartureDateTime());
								// Departure place
								boost::shared_ptr<ParametersMap> submapDeparturePlace(new ParametersMap);
								if(dynamic_cast<const road::Crossing*>(its->getRealTimeDepartureVertex()->getHub()))
								{
									submapDeparturePlace->insert("name", (string)("Croisement"));
									submapDeparturePlace->insert("type", (string)("crossing"));
									submapDeparturePlace->insert("id", dynamic_cast<const road::Crossing*>(its->getRealTimeDepartureVertex()->getHub())->getKey());
								}

								if(its->getRealTimeDepartureVertex()->getGeometry().get() &&
									!its->getRealTimeDepartureVertex()->getGeometry()->isEmpty())
								{
									boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
										*(its->getRealTimeDepartureVertex()->getGeometry())
									)	);
									submapDeparturePlace->insert("longitude", wgs84Point->getX());
									submapDeparturePlace->insert("latitude", wgs84Point->getY());
								}

								submapWalkLeg->insert("departure", submapDeparturePlace);

								const road::Road* road(dynamic_cast<const road::RoadPath*>(its->getService()->getPath())->getRoad());

								std::string roadName = road->getAnyRoadPlace()->getName();
								if(roadName.empty()) {
									if(	road->get<RoadTypeField>() == road::ROAD_TYPE_PEDESTRIANPATH ||
										road->get<RoadTypeField>() == road::ROAD_TYPE_PEDESTRIANSTREET
									){
										roadName="Chemin Pi&eacute;ton";
									}
									else if(road->get<RoadTypeField>() == road::ROAD_TYPE_STEPS) {
										roadName="Escaliers";
									}
									else if(road->get<RoadTypeField>() == road::ROAD_TYPE_BRIDGE) {
										roadName="Pont / Passerelle";
									}
									else if(road->get<RoadTypeField>() == road::ROAD_TYPE_TUNNEL) {
										roadName="Tunnel";
									}
									else {
										roadName="Route sans nom";
									}
								}
								double dst = its->getDistance();
								vector<geos::geom::Geometry*> geometries;
								graph::Journey::ServiceUses::const_iterator next = its+1;
								boost::shared_ptr<geos::geom::LineString> geometry(its->getGeometry());
								if(geometry.get())
								{
									boost::shared_ptr<geos::geom::Geometry> wgs84LineString(CoordinatesSystem::GetCoordinatesSystem(4326).convertGeometry(
										*geometry
									)	);
									geometries.push_back(wgs84LineString.get());
									allGeometries.push_back(wgs84LineString.get());
									geometriesSPtr.push_back(wgs84LineString);
								}

								while (next != it->getServiceUses().end() &&
									dynamic_cast<const road::RoadPath*> (next->getService()->getPath()))
								{
									string nextRoadName(
										dynamic_cast<const road::RoadPath*>(next->getService()->getPath())->getRoad()->getAnyRoadPlace()->getName()
									);
									if(!roadName.compare(nextRoadName))
									{
										++its;
										dst += its->getDistance();
										boost::shared_ptr<geos::geom::LineString> geometry(its->getGeometry());
										if(geometry.get())
										{
											boost::shared_ptr<geos::geom::Geometry> wgs84LineString(CoordinatesSystem::GetCoordinatesSystem(4326).convertGeometry(
												*geometry
											)	);
											geometries.push_back(wgs84LineString.get());
											allGeometries.push_back(wgs84LineString.get());
											geometriesSPtr.push_back(wgs84LineString);
										}
										next = its+1;
									}
									else
									{
										break;
									}
								}

								boost::shared_ptr<geos::geom::MultiLineString> multiLineString(
									CoordinatesSystem::GetCoordinatesSystem(4326).getGeometryFactory().createMultiLineString(
										geometries
								)	);
								submapWalkLeg->insert("arrival_date_time", its->getArrivalDateTime());
								submapWalkLeg->insert("geometry", multiLineString->toString());
								boost::shared_ptr<ParametersMap> submapWalkAttributes(new ParametersMap);
								submapWalkAttributes->insert("length", dst);

								// Arrival place
								boost::shared_ptr<ParametersMap> submapArrivalPlace(new ParametersMap);
								if(dynamic_cast<const road::Crossing*>(its->getRealTimeArrivalVertex()->getHub()))
								{
									submapArrivalPlace->insert("name", (string)("Croisement"));
									submapArrivalPlace->insert("type", (string)("crossing"));
									submapArrivalPlace->insert("id", dynamic_cast<const road::Crossing*>(its->getRealTimeArrivalVertex()->getHub())->getKey());
								}

								if(its->getRealTimeArrivalVertex()->getGeometry().get() &&
									!its->getRealTimeArrivalVertex()->getGeometry()->isEmpty())
								{
									boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
										*(its->getRealTimeArrivalVertex()->getGeometry())
									)	);
									submapArrivalPlace->insert("longitude", wgs84Point->getX());
									submapArrivalPlace->insert("latitude", wgs84Point->getY());
								}

								submapWalkLeg->insert("arrival", submapArrivalPlace);

								boost::shared_ptr<ParametersMap> submapLegRoad(new ParametersMap);
								submapLegRoad->insert("name", roadName);
								submapLegRoad->insert("id", road->getAnyRoadPlace()->getKey());

								submapWalkAttributes->insert("road", submapLegRoad);

								submapWalkLeg->insert("walk_attributes", submapWalkAttributes);
								submapJourney->insert("leg", submapWalkLeg);
							}
							else if (junction)
							{
								// TODO (junction is a walk_leg between 2 pt_leg)
							}
							else
							{
								//pt_leg
								boost::shared_ptr<ParametersMap> submapPtLeg(new ParametersMap);
								submapPtLeg->insert("departure_date_time", its->getDepartureDateTime());
								submapPtLeg->insert("arrival_date_time", its->getArrivalDateTime());

								// Departure place
								boost::shared_ptr<ParametersMap> submapDeparturePlace(new ParametersMap);
								submapDeparturePlace->insert("name", dynamic_cast<const NamedPlace*>(its->getRealTimeDepartureVertex()->getHub())->getFullName());
								submapDeparturePlace->insert("type", dynamic_cast<const NamedPlace*>(its->getRealTimeDepartureVertex()->getHub())->getFactoryKey());
								submapDeparturePlace->insert("id", dynamic_cast<const NamedPlace*>(its->getRealTimeDepartureVertex()->getHub())->getKey());

								if(its->getRealTimeDepartureVertex()->getGeometry().get() &&
									!its->getRealTimeDepartureVertex()->getGeometry()->isEmpty())
								{
									boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
										*(its->getRealTimeDepartureVertex()->getGeometry())
									)	);
									submapDeparturePlace->insert("longitude", wgs84Point->getX());
									submapDeparturePlace->insert("latitude", wgs84Point->getY());
								}

								submapPtLeg->insert("departure", submapDeparturePlace);

								// Arrival place
								boost::shared_ptr<ParametersMap> submapArrivalPlace(new ParametersMap);
								submapArrivalPlace->insert("name", dynamic_cast<const NamedPlace*>(its->getRealTimeArrivalVertex()->getHub())->getFullName());
								submapArrivalPlace->insert("type", dynamic_cast<const NamedPlace*>(its->getRealTimeArrivalVertex()->getHub())->getFactoryKey());
								submapArrivalPlace->insert("id", dynamic_cast<const NamedPlace*>(its->getRealTimeArrivalVertex()->getHub())->getKey());

								if(its->getRealTimeArrivalVertex()->getGeometry().get() &&
									!its->getRealTimeArrivalVertex()->getGeometry()->isEmpty())
								{
									boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
										*(its->getRealTimeArrivalVertex()->getGeometry())
									)	);
									submapArrivalPlace->insert("longitude", wgs84Point->getX());
									submapArrivalPlace->insert("latitude", wgs84Point->getY());
								}

								submapPtLeg->insert("arrival", submapArrivalPlace);

								boost::shared_ptr<ParametersMap> submapPtAttributes(new ParametersMap);

								const pt::JourneyPattern* line(static_cast<const pt::JourneyPattern*>(its->getService()->getPath()));
								if (line &&
									line->getCommercialLine() &&
									line->getCommercialLine()->getNetwork())
								{
									boost::shared_ptr<ParametersMap> submapNetwork(new ParametersMap);
									submapNetwork->insert("id", line->getCommercialLine()->getNetwork()->getKey());
									submapNetwork->insert("name", line->getCommercialLine()->getNetwork()->getName());
									submapNetwork->insert("image", line->getCommercialLine()->getNetwork()->get<pt::Image>());
									submapPtAttributes->insert("network", submapNetwork);
								}
								if (line &&
									line->getCommercialLine())
								{
									boost::shared_ptr<ParametersMap> submapCommercialLine(new ParametersMap);
									submapCommercialLine->insert("id", line->getCommercialLine()->getKey());
									submapCommercialLine->insert("name", line->getCommercialLine()->getName());
									submapCommercialLine->insert("image", line->getCommercialLine()->getImage());
									submapCommercialLine->insert("style", line->getCommercialLine()->getStyle());
									submapCommercialLine->insert("color", line->getCommercialLine()->getColor());
									submapPtAttributes->insert("line", submapCommercialLine);
								}
								const pt::ContinuousService* continuousService(dynamic_cast<const pt::ContinuousService*>(its->getService()));
								const pt::ScheduledService* scheduledService(dynamic_cast<const pt::ScheduledService*>(its->getService()));
								if (continuousService)
								{
									boost::shared_ptr<ParametersMap> submapContinuousService(new ParametersMap);
									submapContinuousService->insert("id", continuousService->getKey());
									submapContinuousService->insert("number", continuousService->getServiceNumber());
									submapPtAttributes->insert("service", submapContinuousService);
								}
								if (scheduledService)
								{
									boost::shared_ptr<ParametersMap> submapSchedulesBasedService(new ParametersMap);
									submapSchedulesBasedService->insert("id", scheduledService->getKey());
									submapSchedulesBasedService->insert("number", scheduledService->getServiceNumber());
									submapPtAttributes->insert("service", submapSchedulesBasedService);
								}

								submapPtLeg->insert("pt_attributes", submapPtAttributes);

								// Geometry
								boost::shared_ptr<geos::geom::LineString> geometry(its->getGeometry());
								if(geometry.get())
								{
									boost::shared_ptr<geos::geom::Geometry> wgs84LineString(CoordinatesSystem::GetCoordinatesSystem(4326).convertGeometry(
										*geometry
									)	);
									allGeometries.push_back(wgs84LineString.get());
									geometriesSPtr.push_back(wgs84LineString);

									submapPtLeg->insert("geometry", wgs84LineString->toString());
								}

								submapJourney->insert("leg", submapPtLeg);
							}

							// Next service use
							if(its == (it->getServiceUses().end()-1)) break;
							++its;
						}

						boost::shared_ptr<geos::geom::MultiLineString> multiLineString(
							CoordinatesSystem::GetCoordinatesSystem(4326).getGeometryFactory().createMultiLineString(
								allGeometries
						)	);
						submapJourney->insert("geometry", multiLineString->toString());
						pm.insert("journey", submapJourney);
					}
				}

				Log::GetInstance().debug("MultimodalJourneyPlannerService::run : after pt data processing");
			}

			if (_usePublicBike && _aStarForWalk)
			{
				// Initialization
				graph::AccessParameters approachAccessParameters(
					graph::USER_PEDESTRIAN,			// user class code
					false,							// DRT only
					false,							// without DRT
					_useWalk ? 1000 : 0,			// max approach distance
					boost::posix_time::minutes(23),	// max approach time
					_useWalk ? 1.111 : 0.0,			// approach speed
					_maxTransportConnectionCount	// max transport connection count (ie : max number of used transport services - 1)
				);

				// A* algorithm
				algorithm::AlgorithmLogger logger;
				graph::AccessParameters bikeAccessParameters(graph::USER_BIKE, false, false, 72000, boost::posix_time::hours(24), 4.167);
				public_biking::PublicBikeJourneyPlanner pbjp(
					departure,
					arrival,
					startDate,
					endDate,
					approachAccessParameters,
					bikeAccessParameters,
					logger
				);
				std::vector<public_biking::PublicBikeJourneyPlanner::PublicBikeJourneyPlannerAStarResult> result(pbjp.runAStar());

				BOOST_FOREACH(public_biking::PublicBikeJourneyPlanner::PublicBikeJourneyPlannerAStarResult subResult, result)
				{
					boost::shared_ptr<ParametersMap> submapJourney(new ParametersMap);
					if(!subResult.get<0>().empty())
					{
						submapJourney->insert("departure_date_time", subResult.get<0>().getFirstDepartureTime());
					}
					else
					{
						submapJourney->insert("departure_date_time", startDate);
					}

					// Departure place
					boost::shared_ptr<ParametersMap> submapDeparturePlace(new ParametersMap);
					if(subResult.get<0>().empty() ||
						dynamic_cast<const road::Crossing*>(subResult.get<0>().getOrigin()->getHub()))
					{
						if(dynamic_cast<const NamedPlace*>(departure))
						{
							submapDeparturePlace->insert("name", dynamic_cast<const NamedPlace*>(departure)->getFullName());
							submapDeparturePlace->insert("type", dynamic_cast<const NamedPlace*>(departure)->getFactoryKey());
							submapDeparturePlace->insert("id", dynamic_cast<const NamedPlace*>(departure)->getKey());
						}
						else
						{
							submapDeparturePlace->insert("name", dynamic_cast<const City*>(departure)->getName());
							string strCityType("City");
							submapDeparturePlace->insert("type", strCityType);
							submapDeparturePlace->insert("id", dynamic_cast<const City*>(departure)->getKey());
						}
					}
					else
					{
						submapDeparturePlace->insert("name", dynamic_cast<const NamedPlace*>(subResult.get<0>().getOrigin()->getHub())->getFullName());
						submapDeparturePlace->insert("type", dynamic_cast<const NamedPlace*>(subResult.get<0>().getOrigin()->getHub())->getFactoryKey());
						submapDeparturePlace->insert("id", dynamic_cast<const NamedPlace*>(subResult.get<0>().getOrigin()->getHub())->getKey());
					}

					if(!subResult.get<0>().empty() &&
						subResult.get<0>().getOrigin()->getFromVertex()->getGeometry().get() &&
						!subResult.get<0>().getOrigin()->getFromVertex()->getGeometry()->isEmpty())
					{
						boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
							*(subResult.get<0>().getOrigin()->getFromVertex()->getGeometry())
						)	);
						submapDeparturePlace->insert("longitude", wgs84Point->getX());
						submapDeparturePlace->insert("latitude", wgs84Point->getY());
					}

					// Arrival place
					boost::shared_ptr<ParametersMap> submapArrivalPlace(new ParametersMap);
					if(subResult.get<2>().empty() ||
						dynamic_cast<const road::Crossing*>(subResult.get<2>().getDestination()->getHub()))
					{
						if(dynamic_cast<const NamedPlace*>(arrival))
						{
							submapArrivalPlace->insert("name", dynamic_cast<const NamedPlace*>(arrival)->getFullName());
							submapArrivalPlace->insert("type", dynamic_cast<const NamedPlace*>(arrival)->getFactoryKey());
							submapArrivalPlace->insert("id", dynamic_cast<const NamedPlace*>(arrival)->getKey());
						}
						else
						{
							submapArrivalPlace->insert("name", dynamic_cast<const City*>(arrival)->getName());
							string strCityType("City");
							submapArrivalPlace->insert("type", strCityType);
							submapArrivalPlace->insert("id", dynamic_cast<const City*>(arrival)->getKey());
						}
					}
					else
					{
						submapArrivalPlace->insert("name", dynamic_cast<const NamedPlace*>(subResult.get<2>().getDestination()->getHub())->getFullName());
						submapArrivalPlace->insert("type", dynamic_cast<const NamedPlace*>(subResult.get<2>().getDestination()->getHub())->getFactoryKey());
						submapArrivalPlace->insert("id", dynamic_cast<const NamedPlace*>(subResult.get<2>().getDestination()->getHub())->getKey());
					}

					if(!subResult.get<2>().empty() &&
						subResult.get<2>().getDestination()->getFromVertex()->getGeometry().get() &&
						!subResult.get<2>().getDestination()->getFromVertex()->getGeometry()->isEmpty())
					{
						boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
							*(subResult.get<2>().getDestination()->getFromVertex()->getGeometry())
						)	);
						submapArrivalPlace->insert("longitude", wgs84Point->getX());
						submapArrivalPlace->insert("latitude", wgs84Point->getY());
					}

					// Display of approach at the departure
					graph::Journey::ServiceUses::const_iterator its(subResult.get<0>().getServiceUses().begin());
					vector<boost::shared_ptr<geos::geom::Geometry> > geometriesSPtr; // To keep shared_ptr's in scope !
					vector<geos::geom::Geometry*> allGeometries;
					while(true)
					{
						if(its == (subResult.get<0>().getServiceUses().end())) break;
						boost::shared_ptr<ParametersMap> submapLeg(new ParametersMap);
						submapLeg->insert("departure_date_time", its->getDepartureDateTime());
						// Departure place
						boost::shared_ptr<ParametersMap> submapDeparturePlace(new ParametersMap);
						if(dynamic_cast<const road::Crossing*>(its->getRealTimeDepartureVertex()->getHub()))
						{
							submapDeparturePlace->insert("name", (string)("Croisement"));
							submapDeparturePlace->insert("type", (string)("crossing"));
							submapDeparturePlace->insert("id", dynamic_cast<const road::Crossing*>(its->getRealTimeDepartureVertex()->getHub())->getKey());
						}

						if(its->getRealTimeDepartureVertex()->getGeometry().get() &&
							!its->getRealTimeDepartureVertex()->getGeometry()->isEmpty())
						{
							boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
								*(its->getRealTimeDepartureVertex()->getGeometry())
							)	);
							submapDeparturePlace->insert("longitude", wgs84Point->getX());
							submapDeparturePlace->insert("latitude", wgs84Point->getY());
						}

						submapLeg->insert("departure", submapDeparturePlace);

						const road::Road* road(dynamic_cast<const road::RoadPath*>(its->getService()->getPath())->getRoad());

						std::string roadName = road->getAnyRoadPlace()->getName();
						if(roadName.empty()) {
							if(	road->get<RoadTypeField>() == road::ROAD_TYPE_PEDESTRIANPATH ||
								road->get<RoadTypeField>() == road::ROAD_TYPE_PEDESTRIANSTREET
							){
								roadName="Chemin Piéton";
							}
							else if(road->get<RoadTypeField>() == road::ROAD_TYPE_STEPS) {
								roadName="Escaliers";
							}
							else if(road->get<RoadTypeField>() == road::ROAD_TYPE_BRIDGE) {
								roadName="Pont / Passerelle";
							}
							else if(road->get<RoadTypeField>() == road::ROAD_TYPE_TUNNEL) {
								roadName="Tunnel";
							}
							else {
								roadName="Route sans nom";
							}
						}
						double dst = its->getDistance();
						vector<geos::geom::Geometry*> geometries;
						graph::Journey::ServiceUses::const_iterator next = its+1;
						boost::shared_ptr<geos::geom::LineString> geometry(its->getGeometry());
						if(geometry.get())
						{
							boost::shared_ptr<geos::geom::Geometry> wgs84LineString(CoordinatesSystem::GetCoordinatesSystem(4326).convertGeometry(
								*geometry
							)	);
							geometries.push_back(wgs84LineString.get());
							allGeometries.push_back(wgs84LineString.get());
							geometriesSPtr.push_back(wgs84LineString);
						}
						while (next != subResult.get<0>().getServiceUses().end())
						{
							string nextRoadName(
								dynamic_cast<const road::RoadPath*>(next->getService()->getPath())->getRoad()->getAnyRoadPlace()->getName()
							);
							if(!roadName.compare(nextRoadName))
							{
								++its;
								dst += its->getDistance();
								boost::shared_ptr<geos::geom::LineString> geometry(its->getGeometry());
								if(geometry.get())
								{
									boost::shared_ptr<geos::geom::Geometry> wgs84LineString(CoordinatesSystem::GetCoordinatesSystem(4326).convertGeometry(
										*geometry
									)	);
									geometries.push_back(wgs84LineString.get());
									allGeometries.push_back(wgs84LineString.get());
									geometriesSPtr.push_back(wgs84LineString);
								}
								next = its+1;
							}
							else
							{
								break;
							}
						}
						boost::shared_ptr<geos::geom::MultiLineString> multiLineString(
							CoordinatesSystem::GetCoordinatesSystem(4326).getGeometryFactory().createMultiLineString(
								geometries
						)	);
						submapLeg->insert("arrival_date_time", its->getArrivalDateTime());
						submapLeg->insert("geometry", multiLineString->toString());
						boost::shared_ptr<ParametersMap> submapWalkAttributes(new ParametersMap);
						submapWalkAttributes->insert("length", dst);

						boost::shared_ptr<ParametersMap> submapLegRoad(new ParametersMap);
						submapLegRoad->insert("name", roadName);
						submapLegRoad->insert("id", road->getAnyRoadPlace()->getKey());

						submapWalkAttributes->insert("road", submapLegRoad);

						submapLeg->insert("walk_attributes", submapWalkAttributes);

						// Arrival place
						boost::shared_ptr<ParametersMap> submapArrivalPlace(new ParametersMap);
						if(dynamic_cast<const road::Crossing*>(its->getRealTimeArrivalVertex()->getHub()))
						{
							submapArrivalPlace->insert("name", (string)("Croisement"));
							submapArrivalPlace->insert("type", (string)("crossing"));
							submapArrivalPlace->insert("id", dynamic_cast<const road::Crossing*>(its->getRealTimeArrivalVertex()->getHub())->getKey());
						}

						if(its->getRealTimeArrivalVertex()->getGeometry().get() &&
							!its->getRealTimeArrivalVertex()->getGeometry()->isEmpty())
						{
							boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
								*(its->getRealTimeArrivalVertex()->getGeometry())
							)	);
							submapArrivalPlace->insert("longitude", wgs84Point->getX());
							submapArrivalPlace->insert("latitude", wgs84Point->getY());
						}

						submapLeg->insert("arrival", submapArrivalPlace);

						submapJourney->insert("leg", submapLeg);

						// Next service use
						if(its == (subResult.get<0>().getServiceUses().end()-1)) break;
						++its;
					}
					boost::posix_time::ptime arrivalTime;
					if(!subResult.get<1>().empty())
					{
						int curDistance(0);
						boost::posix_time::ptime departureTime(subResult.get<0>().empty() ?
							startDate : subResult.get<0>().getFirstArrivalTime());
						bool first(true);
						vector<geos::geom::Geometry*> geometries;
						vector<geos::geom::Geometry*> curGeom;
						double speed(bikeAccessParameters.getApproachSpeed());
						boost::shared_ptr<ParametersMap> submapLegDeparturePlace(new ParametersMap);
						boost::shared_ptr<ParametersMap> submapLegArrivalPlace(new ParametersMap);
						road::RoadPlace* curRoadPlace(NULL);
						const road::RoadChunkEdge* lastChunk;

						BOOST_FOREACH(const road::RoadChunkEdge* chunk, subResult.get<1>())
						{
							const road::Road* road(chunk->getRoadChunk()->getRoad());
							lastChunk = chunk;
							boost::shared_ptr<geos::geom::LineString> geometry = chunk->getRealGeometry();
							boost::shared_ptr<geos::geom::Geometry> geometryProjected(CoordinatesSystem::GetCoordinatesSystem(4326).convertGeometry(*geometry));
							allGeometries.push_back(geometryProjected.get());
							geometriesSPtr.push_back(geometryProjected);
							double distance(0);
							if(geometry)
							{
								geos::geom::CoordinateSequence* coordinates = geometry->getCoordinates();
								distance = geos::algorithm::CGAlgorithms::length(coordinates);
								delete coordinates;
							}

							if(first)
							{
								curDistance = static_cast<int>(distance);
								arrivalTime = startDate + boost::posix_time::seconds(static_cast<int>(distance / speed));
								curRoadPlace = &*road->getAnyRoadPlace();
								curGeom.push_back(geometryProjected.get()->clone());
								first = false;

								// Departure place
								if(dynamic_cast<const road::Crossing*>(chunk->getFromVertex()->getHub()))
								{
									submapLegDeparturePlace->insert("name", (string)("Croisement"));
									submapLegDeparturePlace->insert("type", (string)("crossing"));
									submapLegDeparturePlace->insert("id", dynamic_cast<const road::Crossing*>(chunk->getFromVertex()->getHub())->getKey());
								}

								if(chunk->getFromVertex()->getGeometry().get() &&
									!chunk->getFromVertex()->getGeometry()->isEmpty())
								{
									boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
										*(chunk->getFromVertex()->getGeometry())
									)	);
									submapLegDeparturePlace->insert("longitude", wgs84Point->getX());
									submapLegDeparturePlace->insert("latitude", wgs84Point->getY());
								}
							}
							else if(curRoadPlace->getName() == road->getAnyRoadPlace()->getName())
							{
								curDistance += static_cast<int>(distance);
								arrivalTime = arrivalTime + boost::posix_time::seconds(static_cast<int>(distance / speed));
								curGeom.push_back(geometryProjected.get()->clone());
							}
							else
							{
								geos::geom::MultiLineString* multiLineString = CoordinatesSystem::GetCoordinatesSystem(4326).getGeometryFactory().createMultiLineString(curGeom);
								geometries.push_back(multiLineString->clone());

								boost::shared_ptr<ParametersMap> submapLeg(new ParametersMap);
								submapLeg->insert("departure_date_time", departureTime);
								submapLeg->insert("departure", submapLegDeparturePlace);

								// Arrival place
								if(dynamic_cast<const road::Crossing*>(chunk->getFromVertex()->getHub()))
								{
									submapLegArrivalPlace->insert("name", (string)("Croisement"));
									submapLegArrivalPlace->insert("type", (string)("crossing"));
									submapLegArrivalPlace->insert("id", dynamic_cast<const road::Crossing*>(chunk->getFromVertex()->getHub())->getKey());
								}

								if(chunk->getFromVertex()->getGeometry().get() &&
									!chunk->getFromVertex()->getGeometry()->isEmpty())
								{
									boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
										*(chunk->getFromVertex()->getGeometry())
									)	);
									submapLegArrivalPlace->insert("longitude", wgs84Point->getX());
									submapLegArrivalPlace->insert("latitude", wgs84Point->getY());
								}
								submapLeg->insert("arrival", submapLegArrivalPlace);

								submapLegDeparturePlace.reset(new ParametersMap);
								submapLegArrivalPlace.reset(new ParametersMap);

								submapLeg->insert("arrival_date_time", arrivalTime);
								submapLeg->insert("geometry", multiLineString->toString());
								boost::shared_ptr<ParametersMap> submapWalkAttributes(new ParametersMap);
								submapWalkAttributes->insert("length", curDistance);

								boost::shared_ptr<ParametersMap> submapLegRoad(new ParametersMap);
								submapLegRoad->insert("name", curRoadPlace->getName());
								submapLegRoad->insert("id", curRoadPlace->getKey());

								submapWalkAttributes->insert("road", submapLegRoad);

								submapLeg->insert("bike_attributes", submapWalkAttributes);

								submapJourney->insert("leg", submapLeg);

								delete multiLineString;
								curDistance = static_cast<int>(distance);
								departureTime = arrivalTime;
								arrivalTime = departureTime + boost::posix_time::seconds(static_cast<int>(distance / speed));
								curRoadPlace = &*road->getAnyRoadPlace();

								BOOST_FOREACH(geos::geom::Geometry* geomToDelete, curGeom)
								{
									delete geomToDelete;
								}

								curGeom.clear();
								curGeom.push_back(geometryProjected.get()->clone());

								// New departure place
								if(dynamic_cast<const road::Crossing*>(chunk->getFromVertex()->getHub()))
								{
									submapLegDeparturePlace->insert("name", (string)("Croisement"));
									submapLegDeparturePlace->insert("type", (string)("crossing"));
									submapLegDeparturePlace->insert("id", dynamic_cast<const road::Crossing*>(chunk->getFromVertex()->getHub())->getKey());
								}

								if(chunk->getFromVertex()->getGeometry().get() &&
									!chunk->getFromVertex()->getGeometry()->isEmpty())
								{
									boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
										*(chunk->getFromVertex()->getGeometry())
									)	);
									submapLegDeparturePlace->insert("longitude", wgs84Point->getX());
									submapLegDeparturePlace->insert("latitude", wgs84Point->getY());
								}
							}
						}

						geos::geom::MultiLineString* multiLineString = CoordinatesSystem::GetCoordinatesSystem(4326).getGeometryFactory().createMultiLineString(curGeom);
						geometries.push_back(multiLineString->clone());

						BOOST_FOREACH(geos::geom::Geometry* geomToDelete, curGeom)
						{
							delete geomToDelete;
						}
						curGeom.clear();
						boost::shared_ptr<ParametersMap> submapLeg(new ParametersMap);
						submapLeg->insert("departure_date_time", departureTime);
						submapLeg->insert("departure", submapLegDeparturePlace);

						// Arrival place
						if(dynamic_cast<const road::Crossing*>(lastChunk->getFromVertex()->getHub()))
						{
							submapLegArrivalPlace->insert("name", (string)("Croisement"));
							submapLegArrivalPlace->insert("type", (string)("crossing"));
							submapLegArrivalPlace->insert("id", dynamic_cast<const road::Crossing*>(lastChunk->getFromVertex()->getHub())->getKey());
						}

						if(lastChunk->getFromVertex()->getGeometry().get() &&
							!lastChunk->getFromVertex()->getGeometry()->isEmpty())
						{
							boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
								*(lastChunk->getFromVertex()->getGeometry())
							)	);
							submapLegArrivalPlace->insert("longitude", wgs84Point->getX());
							submapLegArrivalPlace->insert("latitude", wgs84Point->getY());
						}
						submapLeg->insert("arrival", submapLegArrivalPlace);

						submapLeg->insert("arrival_date_time", arrivalTime);
						submapLeg->insert("geometry", multiLineString->toString());
						boost::shared_ptr<ParametersMap> submapWalkAttributes(new ParametersMap);
						submapWalkAttributes->insert("length", curDistance);

						boost::shared_ptr<ParametersMap> submapLegRoad(new ParametersMap);
						submapLegRoad->insert("name", curRoadPlace->getName());
						submapLegRoad->insert("id", curRoadPlace->getKey());

						submapWalkAttributes->insert("road", submapLegRoad);

						submapLeg->insert("bike_attributes", submapWalkAttributes);

						submapJourney->insert("leg", submapLeg);

						delete multiLineString;

						BOOST_FOREACH(geos::geom::Geometry* geomToDelete, geometries)
						{
							delete geomToDelete;
						}
						geometries.clear();
					}

					// Display of approach at the arrival
					its = subResult.get<2>().getServiceUses().begin();
					boost::posix_time::time_duration timeShift;
					while(true)
					{
						if(its == (subResult.get<2>().getServiceUses().end())) break;
						boost::shared_ptr<ParametersMap> submapLeg(new ParametersMap);
						timeShift = arrivalTime - its->getDepartureDateTime();
						submapLeg->insert("departure_date_time", its->getDepartureDateTime() + timeShift);
						// Departure place
						boost::shared_ptr<ParametersMap> submapDeparturePlace(new ParametersMap);
						if(dynamic_cast<const road::Crossing*>(its->getRealTimeDepartureVertex()->getHub()))
						{
							submapDeparturePlace->insert("name", (string)("Croisement"));
							submapDeparturePlace->insert("type", (string)("crossing"));
							submapDeparturePlace->insert("id", dynamic_cast<const road::Crossing*>(its->getRealTimeDepartureVertex()->getHub())->getKey());
						}

						if(its->getRealTimeDepartureVertex()->getGeometry().get() &&
							!its->getRealTimeDepartureVertex()->getGeometry()->isEmpty())
						{
							boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
								*(its->getRealTimeDepartureVertex()->getGeometry())
							)	);
							submapDeparturePlace->insert("longitude", wgs84Point->getX());
							submapDeparturePlace->insert("latitude", wgs84Point->getY());
						}

						submapLeg->insert("departure", submapDeparturePlace);

						const road::Road* road(dynamic_cast<const road::RoadPath*>(its->getService()->getPath())->getRoad());

						std::string roadName = road->getAnyRoadPlace()->getName();
						if(roadName.empty()) {
							if(	road->get<RoadTypeField>() == road::ROAD_TYPE_PEDESTRIANPATH ||
								road->get<RoadTypeField>() == road::ROAD_TYPE_PEDESTRIANSTREET
							){
								roadName="Chemin Piéton";
							}
							else if(road->get<RoadTypeField>() == road::ROAD_TYPE_STEPS) {
								roadName="Escaliers";
							}
							else if(road->get<RoadTypeField>() == road::ROAD_TYPE_BRIDGE) {
								roadName="Pont / Passerelle";
							}
							else if(road->get<RoadTypeField>() == road::ROAD_TYPE_TUNNEL) {
								roadName="Tunnel";
							}
							else {
								roadName="Route sans nom";
							}
						}
						double dst = its->getDistance();
						vector<geos::geom::Geometry*> geometries;
						graph::Journey::ServiceUses::const_iterator next = its+1;
						boost::shared_ptr<geos::geom::LineString> geometry(its->getGeometry());
						if(geometry.get())
						{
							boost::shared_ptr<geos::geom::Geometry> wgs84LineString(CoordinatesSystem::GetCoordinatesSystem(4326).convertGeometry(
								*geometry
							)	);
							geometries.push_back(wgs84LineString.get());
							allGeometries.push_back(wgs84LineString.get());
							geometriesSPtr.push_back(wgs84LineString);
						}
						while (next != subResult.get<2>().getServiceUses().end())
						{
							string nextRoadName(
								dynamic_cast<const road::RoadPath*>(next->getService()->getPath())->getRoad()->getAnyRoadPlace()->getName()
							);
							if(!roadName.compare(nextRoadName))
							{
								++its;
								dst += its->getDistance();
								boost::shared_ptr<geos::geom::LineString> geometry(its->getGeometry());
								if(geometry.get())
								{
									boost::shared_ptr<geos::geom::Geometry> wgs84LineString(CoordinatesSystem::GetCoordinatesSystem(4326).convertGeometry(
										*geometry
									)	);
									geometries.push_back(wgs84LineString.get());
									allGeometries.push_back(wgs84LineString.get());
									geometriesSPtr.push_back(wgs84LineString);
								}
								next = its+1;
							}
							else
							{
								break;
							}
						}
						boost::shared_ptr<geos::geom::MultiLineString> multiLineString(
							CoordinatesSystem::GetCoordinatesSystem(4326).getGeometryFactory().createMultiLineString(
								geometries
						)	);
						submapLeg->insert("arrival_date_time", its->getArrivalDateTime() + timeShift);
						submapLeg->insert("geometry", multiLineString->toString());
						boost::shared_ptr<ParametersMap> submapWalkAttributes(new ParametersMap);
						submapWalkAttributes->insert("length", dst);

						boost::shared_ptr<ParametersMap> submapLegRoad(new ParametersMap);
						submapLegRoad->insert("name", roadName);
						submapLegRoad->insert("id", road->getAnyRoadPlace()->getKey());

						submapWalkAttributes->insert("road", submapLegRoad);

						submapLeg->insert("walk_attributes", submapWalkAttributes);

						// Arrival place
						boost::shared_ptr<ParametersMap> submapArrivalPlace(new ParametersMap);
						if(dynamic_cast<const road::Crossing*>(its->getRealTimeArrivalVertex()->getHub()))
						{
							submapArrivalPlace->insert("name", (string)("Croisement"));
							submapArrivalPlace->insert("type", (string)("crossing"));
							submapArrivalPlace->insert("id", dynamic_cast<const road::Crossing*>(its->getRealTimeArrivalVertex()->getHub())->getKey());
						}

						if(its->getRealTimeArrivalVertex()->getGeometry().get() &&
							!its->getRealTimeArrivalVertex()->getGeometry()->isEmpty())
						{
							boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
								*(its->getRealTimeArrivalVertex()->getGeometry())
							)	);
							submapArrivalPlace->insert("longitude", wgs84Point->getX());
							submapArrivalPlace->insert("latitude", wgs84Point->getY());
						}

						submapLeg->insert("arrival", submapArrivalPlace);

						submapJourney->insert("leg", submapLeg);

						// Next service use
						if(its == (subResult.get<2>().getServiceUses().end()-1)) break;
						++its;
					}

					if(!subResult.get<2>().empty())
					{
						submapJourney->insert("arrival_date_time", subResult.get<2>().getFirstArrivalTime() + timeShift);
					}
					else
					{
						submapJourney->insert("arrival_date_time", arrivalTime);
					}

					submapJourney->insert("departure", submapDeparturePlace);
					submapJourney->insert("arrival", submapArrivalPlace);
					boost::shared_ptr<geos::geom::MultiLineString> multiLineString(
						CoordinatesSystem::GetCoordinatesSystem(4326).getGeometryFactory().createMultiLineString(
							allGeometries
					)	);
					submapJourney->insert("geometry", multiLineString->toString());

					pm.insert("journey", submapJourney);
				}
				Log::GetInstance().debug("MultimodalJourneyPlannerService::run : after A* bike journey planner data processing");
			}
			else if (_usePublicBike)
			{
				Log::GetInstance().debug("MultimodalJourneyPlannerService::run : before SYNTHESE bike");

				// Initialization
				graph::AccessParameters approachAccessParameters(
					graph::USER_PEDESTRIAN,			// user class code
					false,							// DRT only
					false,							// without DRT
					_useWalk ? 1000 : 0,			// max approach distance
					boost::posix_time::minutes(23),	// max approach time
					_useWalk ? 1.111 : 0.0,			// approach speed
					_maxTransportConnectionCount	// max transport connection count (ie : max number of used transport services - 1)
				);

				// Classical synthese algorithm
				algorithm::AlgorithmLogger logger;
				graph::AccessParameters bikeAccessParameters(graph::USER_BIKE, false, false, 72000, boost::posix_time::hours(24), 4.167);
				public_biking::PublicBikeJourneyPlanner pbjp(
					departure,
					arrival,
					startDate,
					endDate,
					approachAccessParameters,
					bikeAccessParameters,
					logger
				);
				public_biking::PublicBikeJourneyPlannerResult results = pbjp.run();

				if(!results.getJourneys().empty())
				{
					for (public_biking::PublicBikeJourneyPlannerResult::Journeys::const_iterator it(results.getJourneys().begin()); it != results.getJourneys().end(); ++it)
					{
						boost::shared_ptr<ParametersMap> submapJourney(new ParametersMap);
						submapJourney->insert("departure_date_time", it->getFirstDepartureTime());
						submapJourney->insert("arrival_date_time", it->getFirstArrivalTime());

						// Departure place
						boost::shared_ptr<ParametersMap> submapDeparturePlace(new ParametersMap);
						if(dynamic_cast<const road::Crossing*>(it->getOrigin()->getHub()))
						{
							if(dynamic_cast<const NamedPlace*>(departure))
							{
								submapDeparturePlace->insert("name", dynamic_cast<const NamedPlace*>(departure)->getFullName());
								submapDeparturePlace->insert("type", dynamic_cast<const NamedPlace*>(departure)->getFactoryKey());
								submapDeparturePlace->insert("id", dynamic_cast<const NamedPlace*>(departure)->getKey());
							}
							else
							{
								submapDeparturePlace->insert("name", dynamic_cast<const City*>(departure)->getName());
								string strCityType("City");
								submapDeparturePlace->insert("type", strCityType);
								submapDeparturePlace->insert("id", dynamic_cast<const City*>(departure)->getKey());
							}
						}
						else
						{
							submapDeparturePlace->insert("name", dynamic_cast<const NamedPlace*>(it->getOrigin()->getHub())->getFullName());
							submapDeparturePlace->insert("type", dynamic_cast<const NamedPlace*>(it->getOrigin()->getHub())->getFactoryKey());
							submapDeparturePlace->insert("id", dynamic_cast<const NamedPlace*>(it->getOrigin()->getHub())->getKey());
						}

						if(it->getOrigin()->getFromVertex()->getGeometry().get() &&
							!it->getOrigin()->getFromVertex()->getGeometry()->isEmpty())
						{
							boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
								*(it->getOrigin()->getFromVertex()->getGeometry())
							)	);
							submapDeparturePlace->insert("longitude", wgs84Point->getX());
							submapDeparturePlace->insert("latitude", wgs84Point->getY());
						}

						// Arrival place
						boost::shared_ptr<ParametersMap> submapArrivalPlace(new ParametersMap);
						if(dynamic_cast<const road::Crossing*>(it->getDestination()->getHub()))
						{
							if(dynamic_cast<const NamedPlace*>(arrival))
							{
								submapArrivalPlace->insert("name", dynamic_cast<const NamedPlace*>(arrival)->getFullName());
								submapArrivalPlace->insert("type", dynamic_cast<const NamedPlace*>(arrival)->getFactoryKey());
								submapArrivalPlace->insert("id", dynamic_cast<const NamedPlace*>(arrival)->getKey());
							}
							else
							{
								submapArrivalPlace->insert("name", dynamic_cast<const City*>(arrival)->getName());
								string strCityType("City");
								submapArrivalPlace->insert("type", strCityType);
								submapArrivalPlace->insert("id", dynamic_cast<const City*>(arrival)->getKey());
							}
						}
						else
						{
							submapArrivalPlace->insert("name", dynamic_cast<const NamedPlace*>(it->getDestination()->getHub())->getFullName());
							submapArrivalPlace->insert("type", dynamic_cast<const NamedPlace*>(it->getDestination()->getHub())->getFactoryKey());
							submapArrivalPlace->insert("id", dynamic_cast<const NamedPlace*>(it->getDestination()->getHub())->getKey());
						}

						if(it->getDestination()->getFromVertex()->getGeometry().get() &&
							!it->getDestination()->getFromVertex()->getGeometry()->isEmpty())
						{
							boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
								*(it->getDestination()->getFromVertex()->getGeometry())
							)	);
							submapArrivalPlace->insert("longitude", wgs84Point->getX());
							submapArrivalPlace->insert("latitude", wgs84Point->getY());
						}

						submapJourney->insert("departure", submapDeparturePlace);
						submapJourney->insert("arrival", submapArrivalPlace);

						graph::Journey::ServiceUses::const_iterator its(it->getServiceUses().begin());
						vector<boost::shared_ptr<geos::geom::Geometry> > geometriesSPtr; // To keep shared_ptr's in scope !
						vector<geos::geom::Geometry*> allGeometries;
						while(true)
						{
							boost::shared_ptr<ParametersMap> submapLeg(new ParametersMap);
							submapLeg->insert("departure_date_time", its->getDepartureDateTime());
							// Departure place
							boost::shared_ptr<ParametersMap> submapDeparturePlace(new ParametersMap);
							if(dynamic_cast<const road::Crossing*>(its->getRealTimeDepartureVertex()->getHub()))
							{
								submapDeparturePlace->insert("name", (string)("Croisement"));
								submapDeparturePlace->insert("type", (string)("crossing"));
								submapDeparturePlace->insert("id", dynamic_cast<const road::Crossing*>(its->getRealTimeDepartureVertex()->getHub())->getKey());
							}

							if(its->getRealTimeDepartureVertex()->getGeometry().get() &&
								!its->getRealTimeDepartureVertex()->getGeometry()->isEmpty())
							{
								boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
									*(its->getRealTimeDepartureVertex()->getGeometry())
								)	);
								submapDeparturePlace->insert("longitude", wgs84Point->getX());
								submapDeparturePlace->insert("latitude", wgs84Point->getY());
							}

							submapLeg->insert("departure", submapDeparturePlace);

							const road::Road* road(dynamic_cast<const road::RoadPath*>(its->getService()->getPath())->getRoad());

							std::string roadName = road->getAnyRoadPlace()->getName();
							std::size_t userClassRank = its->getUserClassRank();
							if(roadName.empty()) {
								if(	road->get<RoadTypeField>() == road::ROAD_TYPE_PEDESTRIANPATH ||
									road->get<RoadTypeField>() == road::ROAD_TYPE_PEDESTRIANSTREET
								){
									roadName="Chemin Piéton";
								}
								else if(road->get<RoadTypeField>() == road::ROAD_TYPE_STEPS) {
									roadName="Escaliers";
								}
								else if(road->get<RoadTypeField>() == road::ROAD_TYPE_BRIDGE) {
									roadName="Pont / Passerelle";
								}
								else if(road->get<RoadTypeField>() == road::ROAD_TYPE_TUNNEL) {
									roadName="Tunnel";
								}
								else {
									roadName="Route sans nom";
								}
							}
							double dst = its->getDistance();
							vector<geos::geom::Geometry*> geometries;
							graph::Journey::ServiceUses::const_iterator next = its+1;
							boost::shared_ptr<geos::geom::LineString> geometry(its->getGeometry());
							if(geometry.get())
							{
								boost::shared_ptr<geos::geom::Geometry> wgs84LineString(CoordinatesSystem::GetCoordinatesSystem(4326).convertGeometry(
									*geometry
								)	);
								geometries.push_back(wgs84LineString.get());
								allGeometries.push_back(wgs84LineString.get());
								geometriesSPtr.push_back(wgs84LineString);
							}
							while (next != it->getServiceUses().end())
							{
								string nextRoadName(
									dynamic_cast<const road::RoadPath*>(next->getService()->getPath())->getRoad()->getAnyRoadPlace()->getName()
								);
								std::size_t nextUserClassRank = next->getUserClassRank();
								if(!roadName.compare(nextRoadName) &&
									userClassRank == nextUserClassRank)
								{
									++its;
									dst += its->getDistance();
									boost::shared_ptr<geos::geom::LineString> geometry(its->getGeometry());
									if(geometry.get())
									{
										boost::shared_ptr<geos::geom::Geometry> wgs84LineString(CoordinatesSystem::GetCoordinatesSystem(4326).convertGeometry(
											*geometry
										)	);
										geometries.push_back(wgs84LineString.get());
										allGeometries.push_back(wgs84LineString.get());
										geometriesSPtr.push_back(wgs84LineString);
									}
									next = its+1;
								}
								else
								{
									break;
								}
							}
							boost::shared_ptr<geos::geom::MultiLineString> multiLineString(
								CoordinatesSystem::GetCoordinatesSystem(4326).getGeometryFactory().createMultiLineString(
									geometries
							)	);
							submapLeg->insert("arrival_date_time", its->getArrivalDateTime());
							submapLeg->insert("geometry", multiLineString->toString());
							if (graph::USER_CLASS_CODE_OFFSET + its->getUserClassRank() == graph::USER_BIKE)
							{
								boost::shared_ptr<ParametersMap> submapBikeAttributes(new ParametersMap);
								submapBikeAttributes->insert("length", dst);

								boost::shared_ptr<ParametersMap> submapLegRoad(new ParametersMap);
								submapLegRoad->insert("name", roadName);
								submapLegRoad->insert("id", road->getAnyRoadPlace()->getKey());

								submapBikeAttributes->insert("road", submapLegRoad);

								submapLeg->insert("bike_attributes", submapBikeAttributes);
							}
							else if (graph::USER_CLASS_CODE_OFFSET + its->getUserClassRank() == graph::USER_PEDESTRIAN)
							{
								boost::shared_ptr<ParametersMap> submapWalkAttributes(new ParametersMap);
								submapWalkAttributes->insert("length", dst);

								boost::shared_ptr<ParametersMap> submapLegRoad(new ParametersMap);
								submapLegRoad->insert("name", roadName);
								submapLegRoad->insert("id", road->getAnyRoadPlace()->getKey());

								submapWalkAttributes->insert("road", submapLegRoad);

								submapLeg->insert("walk_attributes", submapWalkAttributes);
							}

							// Arrival place
							boost::shared_ptr<ParametersMap> submapArrivalPlace(new ParametersMap);
							if(dynamic_cast<const road::Crossing*>(its->getRealTimeArrivalVertex()->getHub()))
							{
								submapArrivalPlace->insert("name", (string)("Croisement"));
								submapArrivalPlace->insert("type", (string)("crossing"));
								submapArrivalPlace->insert("id", dynamic_cast<const road::Crossing*>(its->getRealTimeArrivalVertex()->getHub())->getKey());
							}

							if(its->getRealTimeArrivalVertex()->getGeometry().get() &&
								!its->getRealTimeArrivalVertex()->getGeometry()->isEmpty())
							{
								boost::shared_ptr<geos::geom::Point> wgs84Point(CoordinatesSystem::GetCoordinatesSystem(4326).convertPoint(
									*(its->getRealTimeArrivalVertex()->getGeometry())
								)	);
								submapArrivalPlace->insert("longitude", wgs84Point->getX());
								submapArrivalPlace->insert("latitude", wgs84Point->getY());
							}

							submapLeg->insert("arrival", submapArrivalPlace);

							submapJourney->insert("leg", submapLeg);

							// Next service use
							if(its == (it->getServiceUses().end()-1)) break;
							++its;
						}

						boost::shared_ptr<geos::geom::MultiLineString> multiLineString(
							CoordinatesSystem::GetCoordinatesSystem(4326).getGeometryFactory().createMultiLineString(
								allGeometries
						)	);
						submapJourney->insert("geometry", multiLineString->toString());
						pm.insert("journey", submapJourney);
					}
				}

				Log::GetInstance().debug("MultimodalJourneyPlannerService::run : after SYNTHESE bike");
			}

			Log::GetInstance().debug("MultimodalJourneyPlannerService::run : before output conversion to " + boost::lexical_cast<std::string>(_outputFormat));

			if(_outputFormat == MimeTypes::XML)
			{
				pm.outputXML(
					stream,
					DATA_MULTIMODAL_JOURNEY_PLANNER_RESULT,
					true,
					"http://schemas.open-transport.org/smile/MultimodalJourneyPlanner.xsd"
				);
			}
			else if(_outputFormat == MimeTypes::JSON)
			{
				pm.outputJSON(stream, DATA_MULTIMODAL_JOURNEY_PLANNER_RESULT);
			}

			Log::GetInstance().debug("MultimodalJourneyPlannerService::run : end");

			return pm;
		}



		bool MultimodalJourneyPlannerService::isAuthorized(
			const server::Session* session
		) const {
			return true;
		}



		std::string MultimodalJourneyPlannerService::getOutputMimeType() const
		{
			return _outputFormat;
		}
}	}

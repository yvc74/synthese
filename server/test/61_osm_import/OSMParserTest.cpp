
/** OsmParserTest class implementation.
	@file OsmParserTest.cpp
	@author Marc Jambert

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
#include <boost/test/auto_unit_test.hpp>

#include "OSMParser.hpp"
#include "OSMLocale.hpp"
#include "OSMEntityHandler.hpp"

#include <boost/tuple/tuple.hpp>
#include <fstream>
#include <iostream>

#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>

namespace synthese
{
	namespace data_exchange
	{

		typedef boost::shared_ptr<geos::geom::Geometry> GeometryPtr;
		typedef boost::shared_ptr<geos::geom::Point> PointPtr;
		typedef boost::shared_ptr<geos::geom::LineString> LineStringPtr;

		typedef boost::tuple<std::string, std::string, GeometryPtr> CityTuple;
		typedef boost::tuple<OSMId, std::string, road::RoadType, GeometryPtr> RoadTuple;
		typedef boost::tuple<OSMId, PointPtr> CrossingTuple;
		typedef boost::tuple<size_t, graph::MetricOffset, TrafficDirection, double, bool, bool, bool, LineStringPtr> RoadChunkTuple;
		typedef boost::tuple<HouseNumber, OSMId, std::string, PointPtr> HouseTuple;


		class FakeOSMEntityHandler : public OSMEntityHandler
		{
		public:
			std::vector<CityTuple> handledCities;
			std::vector<RoadTuple> handledRoads;
			std::vector<CrossingTuple> handledCrossings;
			std::vector<RoadChunkTuple> handledRoadChunks;
			std::vector<HouseTuple> handledHouses;

			void handleCity(const std::string& cityName, const std::string& cityCode, GeometryPtr boundary)
			{
				handledCities.push_back(boost::make_tuple(cityName, cityCode, boundary));
			}

			void handleRoad(const OSMId& roadSourceId, 
					const std::string& name,
					const road::RoadType& roadType, 
					GeometryPtr path)
			{
				handledRoads.push_back(boost::make_tuple(roadSourceId, name, roadType, path));
			}

			void handleCrossing(const OSMId& crossingSourceId, PointPtr point)
			{
				handledCrossings.push_back(boost::make_tuple(crossingSourceId, point));
			}

			void handleRoadChunk(size_t rank, 
										 graph::MetricOffset metricOffset,
										 TrafficDirection trafficDirection,
										 double maxSpeed,
					                     bool isDrivable,
					                     bool isBikable,
					                     bool isWalkable,
										 LineStringPtr path)
			{
				handledRoadChunks.push_back(boost::make_tuple(rank, metricOffset, trafficDirection, maxSpeed, isDrivable, isBikable, isWalkable, path));
			}

			void handleHouse(const HouseNumber& houseNumber,
									 const std::string& streetName,
									 PointPtr point)
			{
				handledHouses.push_back(boost::make_tuple(houseNumber, 0, streetName, point));
			}

			void handleHouse(const HouseNumber& houseNumber,
							 const OSMId& roadSourceId,
							 PointPtr point)
			{
				handledHouses.push_back(boost::make_tuple(houseNumber, roadSourceId, "", point));
			}


		};


		void initialize_coordinates_systems()
		{
			CoordinatesSystem::AddCoordinatesSystem(
				4326,
				"WGS 84",
				"+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"
			);
			CoordinatesSystem::AddCoordinatesSystem(
				27572,
				"NTF (Paris) / Lambert zone II",
				"+proj=lcc +lat_1=46.8 +lat_0=46.8 +lon_0=0 +k_0=0.99987742 +x_0=600000 +y_0=2200000 +a=6378249.2 +b=6356515 +towgs84=-168,-60,320,0,0,0,0 +pm=paris +units=m +no_defs"
			);
			CoordinatesSystem::SetDefaultCoordinatesSystems(27572);
		}

		void check_city_handled_with_boundary(const CityTuple&   handledCity,
											  const std::string& expectedCityName,
											  const std::string& expectedCityCode,
											  const std::string& expectedWktBoundary)
		{
			BOOST_CHECK_EQUAL(expectedCityName, handledCity.get<0>());
			BOOST_CHECK_EQUAL(expectedCityCode, handledCity.get<1>());
			BOOST_CHECK(handledCity.get<2>().get() != 0);
			BOOST_CHECK_EQUAL(expectedWktBoundary, handledCity.get<2>()->toString());
		}


		void check_city_handled_without_boundary(const CityTuple& handledCity,
												 const std::string& expectedCityName,
												 const std::string& expectedCityCode)
		{
			BOOST_CHECK_EQUAL(expectedCityName, handledCity.get<0>());
			BOOST_CHECK_EQUAL(expectedCityCode, handledCity.get<1>());
			BOOST_CHECK_MESSAGE(handledCity.get<2>().get() == 0, "No boundary was expected!");
		}


		BOOST_AUTO_TEST_CASE (should_find_five_swiss_cities_without_boundaries_from_osm_file)
		{
			initialize_coordinates_systems();
			std::ifstream osmStream("five_swiss_cities_with_incomplete_boundaries.osm");
			FakeOSMEntityHandler fakeOSMEntityHandler;
			OSMParser parser(
				std::cout,
				CoordinatesSystem::GetCoordinatesSystem(4326),
				fakeOSMEntityHandler,
				OSMLocale::OSMLocale_CH
			);
			parser.parse(osmStream);
			osmStream.close();

			BOOST_CHECK_EQUAL(5, fakeOSMEntityHandler.handledCities.size());
			std::vector<boost::tuple<std::string, std::string, GeometryPtr> >::iterator it =
					fakeOSMEntityHandler.handledCities.begin();
			check_city_handled_without_boundary(*it++, "Hauterive (NE)", "6454");
			check_city_handled_without_boundary(*it++, "Neuchâtel", "6458");
			check_city_handled_without_boundary(*it++, "Saint-Blaise", "6459");
			check_city_handled_without_boundary(*it++, "Valangin", "6485");
			check_city_handled_without_boundary(*it++, "Val-de-Ruz", "6487");
		}


		BOOST_AUTO_TEST_CASE (should_find_ten_french_cities_only_one_with_complete_boundary)
		{
			initialize_coordinates_systems();
			std::ifstream osmStream("ten_french_cities_only_one_with_complete_boundary.osm");
			FakeOSMEntityHandler fakeOSMEntityHandler;
			OSMParser parser(
				std::cout,
				CoordinatesSystem::GetCoordinatesSystem(4326),
				fakeOSMEntityHandler,
				OSMLocale::OSMLocale_FR);
			parser.parse(osmStream);
			osmStream.close();

			const std::string capenduBoundary("MULTIPOLYGON ((("
											  "2.5228000000000002 43.2010000000000005, 2.5228799999999998 43.2008999999999972, 2.5239500000000001 43.2004999999999981, "
											  "2.5252100000000000 43.2002000000000024, 2.5259700000000000 43.2002000000000024, 2.5269800000000000 43.2002000000000024, "
											  "2.5304000000000002 43.2000999999999991, 2.5322100000000001 43.1998999999999995, 2.5325099999999998 43.1998999999999995, "
											  "2.5338300000000000 43.1998000000000033, 2.5352399999999999 43.1998000000000033, 2.5372900000000000 43.1998999999999995, "
											  "2.5392399999999999 43.1998000000000033, 2.5411899999999998 43.1995999999999967, 2.5432500000000000 43.1995999999999967, "
											  "2.5459299999999998 43.1995000000000005, 2.5490300000000001 43.1995999999999967, 2.5501600000000000 43.1993999999999971, "
											  "2.5511300000000001 43.1989999999999981, 2.5521600000000002 43.1991000000000014, 2.5546199999999999 43.1991000000000014, "
											  "2.5556500000000000 43.1987999999999985, 2.5568300000000002 43.1980000000000004, 2.5578500000000002 43.1972000000000023, "
											  "2.5590700000000002 43.1968999999999994, 2.5598600000000000 43.1968000000000032, 2.5608599999999999 43.1970000000000027, "
											  "2.5614900000000000 43.1968999999999994, 2.5620799999999999 43.1965000000000003, 2.5622799999999999 43.1957000000000022, "
											  "2.5618699999999999 43.1953000000000031, 2.5626799999999998 43.1948000000000008, 2.5639699999999999 43.1946000000000012, "
											  "2.5651799999999998 43.1946000000000012, 2.5663399999999998 43.1948999999999970, 2.5674700000000001 43.1950999999999965, "
											  "2.5682100000000001 43.1959000000000017, 2.5690300000000001 43.1968000000000032, 2.5695399999999999 43.1974000000000018, "
											  "2.5700900000000000 43.1985999999999990, 2.5709399999999998 43.1993999999999971, 2.5724300000000002 43.2002999999999986, "
											  "2.5736699999999999 43.2008000000000010, 2.5750899999999999 43.2006000000000014, 2.5761099999999999 43.2000999999999991, "
											  "2.5765699999999998 43.1998999999999995, 2.5769099999999998 43.2000000000000028, 2.5771400000000000 43.2002000000000024, "
											  "2.5772699999999999 43.2004999999999981, 2.5782699999999998 43.2012000000000000, 2.5792099999999998 43.2012000000000000, "
											  "2.5801400000000001 43.2015000000000029, 2.5811700000000002 43.2017999999999986, 2.5818200000000000 43.2019000000000020, "
											  "2.5825200000000001 43.2019000000000020, 2.5831300000000001 43.2017000000000024, 2.5847400000000000 43.2006999999999977, "
											  "2.5871700000000000 43.1995000000000005, 2.5880100000000001 43.1991000000000014, 2.5876399999999999 43.1985999999999990, "
											  "2.5873300000000001 43.1953999999999994, 2.5871100000000000 43.1936000000000035, 2.5870600000000001 43.1923999999999992, "
											  "2.5865700000000000 43.1908999999999992, 2.5858800000000000 43.1890000000000001, 2.5856800000000000 43.1884000000000015, "
											  "2.5855399999999999 43.1876000000000033, 2.5852700000000000 43.1848999999999990, 2.5852900000000001 43.1839999999999975, "
											  "2.5857100000000002 43.1841000000000008, 2.5862799999999999 43.1841000000000008, 2.5861600000000000 43.1833999999999989, "
											  "2.5860900000000000 43.1831999999999994, 2.5851899999999999 43.1828999999999965, 2.5833900000000001 43.1824999999999974, "
											  "2.5824699999999998 43.1822000000000017, 2.5814400000000002 43.1816999999999993, 2.5809099999999998 43.1814999999999998, "
											  "2.5802700000000001 43.1811999999999969, 2.5798800000000002 43.1811999999999969, 2.5794199999999998 43.1805999999999983, "
											  "2.5790500000000001 43.1805000000000021, 2.5786300000000000 43.1803000000000026, 2.5781299999999998 43.1799000000000035, "
											  "2.5777999999999999 43.1799999999999997, 2.5766900000000001 43.1794000000000011, 2.5764200000000002 43.1790999999999983, "
											  "2.5752000000000002 43.1784999999999997, 2.5729400000000000 43.1783000000000001, 2.5737600000000000 43.1777000000000015, "
											  "2.5722399999999999 43.1773000000000025, 2.5721300000000000 43.1771000000000029, 2.5716000000000001 43.1769000000000034, "
											  "2.5712999999999999 43.1768000000000001, 2.5709900000000001 43.1764000000000010, 2.5708400000000000 43.1760000000000019, "
											  "2.5704300000000000 43.1756999999999991, 2.5698599999999998 43.1754000000000033, 2.5698500000000002 43.1751999999999967, "
											  "2.5697399999999999 43.1751000000000005, 2.5695999999999999 43.1749000000000009, 2.5693500000000000 43.1747000000000014, "
											  "2.5691999999999999 43.1747000000000014, 2.5671900000000001 43.1734999999999971, 2.5677300000000001 43.1732999999999976, "
											  "2.5680499999999999 43.1730999999999980, 2.5681699999999998 43.1730999999999980, 2.5681799999999999 43.1724000000000032, "
											  "2.5678999999999998 43.1724000000000032, 2.5682600000000000 43.1713999999999984, 2.5683900000000000 43.1709999999999994, "
											  "2.5684800000000001 43.1700000000000017, 2.5656300000000001 43.1694000000000031, 2.5620300000000000 43.1687000000000012, "
											  "2.5618599999999998 43.1683999999999983, 2.5618200000000000 43.1677999999999997, 2.5618400000000001 43.1676000000000002, "
											  "2.5617399999999999 43.1674000000000007, 2.5613899999999998 43.1666999999999987, 2.5609500000000001 43.1662999999999997, "
											  "2.5606100000000001 43.1659999999999968, 2.5600999999999998 43.1657000000000011, 2.5578799999999999 43.1644000000000005, "
											  "2.5554399999999999 43.1631000000000000, 2.5539100000000001 43.1632000000000033, 2.5494599999999998 43.1629000000000005, "
											  "2.5467399999999998 43.1627999999999972, 2.5427300000000002 43.1625999999999976, 2.5377700000000001 43.1623999999999981, "
											  "2.5307499999999998 43.1634000000000029, 2.5291199999999998 43.1655999999999977, 2.5291399999999999 43.1711000000000027, "
											  "2.5291700000000001 43.1715000000000018, 2.5292900000000000 43.1726000000000028, 2.5273300000000001 43.1777999999999977, "
											  "2.5272199999999998 43.1796000000000006, 2.5274000000000001 43.1805000000000021, 2.5275799999999999 43.1813999999999965, "
											  "2.5278600000000000 43.1829999999999998, 2.5278700000000001 43.1833999999999989, 2.5278600000000000 43.1861000000000033, "
											  "2.5289400000000000 43.1897999999999982, 2.5282300000000002 43.1972000000000023, 2.5278900000000002 43.1972999999999985, "
											  "2.5250499999999998 43.1987000000000023, 2.5228600000000001 43.2000999999999991, 2.5226999999999999 43.2006999999999977, "
											  "2.5228000000000002 43.2010000000000005)))");

			BOOST_CHECK_EQUAL(10, fakeOSMEntityHandler.handledCities.size());
			std::vector<boost::tuple<std::string, std::string, GeometryPtr> >::iterator it =
					fakeOSMEntityHandler.handledCities.begin();
			check_city_handled_without_boundary(*it++, "Trèbes", "11397");
			check_city_handled_without_boundary(*it++, "Badens", "11023");
			check_city_handled_without_boundary(*it++, "Blomac", "11042");
			check_city_handled_without_boundary(*it++, "Marseillette", "11220");
			check_city_handled_without_boundary(*it++, "Barbaira", "11027");
			check_city_handled_with_boundary(*it++, "Capendu", "11068", capenduBoundary);
			check_city_handled_without_boundary(*it++, "Douzens", "11122");
			check_city_handled_without_boundary(*it++, "Comigne", "11095");
			check_city_handled_without_boundary(*it++, "Pradelles-en-Val", "11298");

		}


		BOOST_AUTO_TEST_CASE (should_find_all_roads_from_ten_french_cities)
		{
			initialize_coordinates_systems();
			std::ifstream osmStream("ten_french_cities_only_one_with_complete_boundary.osm");
			FakeOSMEntityHandler fakeOSMEntityHandler;
			OSMParser parser(
				std::cout,
				CoordinatesSystem::GetCoordinatesSystem(4326),
				fakeOSMEntityHandler,
				OSMLocale::OSMLocale_FR
			);
			parser.parse(osmStream);
			osmStream.close();

			BOOST_CHECK_EQUAL(452,  fakeOSMEntityHandler.handledRoads.size());
			BOOST_CHECK_EQUAL(1310, fakeOSMEntityHandler.handledRoadChunks.size());
			BOOST_CHECK_EQUAL(1310, fakeOSMEntityHandler.handledCrossings.size());
			BOOST_CHECK_EQUAL(132,  fakeOSMEntityHandler.handledHouses.size());
		}

/*
		BOOST_AUTO_TEST_CASE (should_parse_full_swiss)
		{
			FakeOSMEntityHandler fakeOSMEntityHandler;
			//std::ifstream osmStream("/home/mjambert/workspace/rcsmobility/gitlab/switzerland-tests/robot/resources/data/swiss.osm");
			std::ifstream osmStream("larger_swiss_tile.osm");
			OSMParser parser(std::cout, fakeOSMEntityHandler, OSMLocale::OSMLocale_FR);
			parser.parse(osmStream);
			osmStream.close();


			BOOST_CHECK_EQUAL(649, fakeOSMEntityHandler.handledRoads.size());
		}
		*/
	}
}
	

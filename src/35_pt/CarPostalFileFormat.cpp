
/** CarPostalFileFormat class implementation.
	@file CarPostalFileFormat.cpp

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

#include "CarPostalFileFormat.hpp"
#include "DataSource.h"
#include "StopPoint.hpp"
#include "StopArea.hpp"
#include "StopAreaTableSync.hpp"
#include "City.h"
#include "CityTableSync.h"
#include "DBTransaction.hpp"
#include "JourneyPatternTableSync.hpp"
#include "ScheduledServiceTableSync.h"
#include "CommercialLineTableSync.h"
#include "LineStopTableSync.h"
#include "Calendar.h"
#include "ImportFunction.h"
#include "AdminFunctionRequest.hpp"
#include "PropertiesHTMLTable.h"
#include "DataSourceAdmin.h"
#include "PTFileFormat.hpp"
#include "DesignatedLinePhysicalStop.hpp"
#include "IConv.hpp"
#include "AdminActionFunctionRequest.hpp"
#include "HTMLModule.h"
#include "HTMLForm.h"
#include "StopPointAddAction.hpp"
#include "StopPointUpdateAction.hpp"
#include "PTPlaceAdmin.h"
#include "StopPointAdmin.hpp"
#include "StopAreaAddAction.h"
#include "TransportNetworkTableSync.h"
#include "RollingStockTableSync.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <map>
#include <fstream>
#include <geos/geom/Point.h>
#include <geos/opDistance.h>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace geos::geom;

namespace synthese
{
	using namespace util;
	using namespace impex;
	using namespace pt;
	using namespace road;
	using namespace admin;
	using namespace geography;
	using namespace db;
	using namespace graph;
	using namespace calendar;
	using namespace server;
	using namespace html;



	namespace util
	{
		template<> const string FactorableTemplate<FileFormat,CarPostalFileFormat>::FACTORY_KEY("CarPostal");
	}

	namespace pt
	{
		const string CarPostalFileFormat::Importer_::FILE_KOORD = "koord";
		const string CarPostalFileFormat::Importer_::FILE_BITFELD = "bitfeld";
		const string CarPostalFileFormat::Importer_::FILE_ECKDATEN = "eckdaten";
		const string CarPostalFileFormat::Importer_::FILE_ZUGDAT = "zugdat";

		const string CarPostalFileFormat::Importer_::PARAMETER_SHOW_STOPS_ONLY = "show_stops_only";
		const string CarPostalFileFormat::Importer_::PARAMETER_NETWORK_ID = "network_id";
		const string CarPostalFileFormat::Importer_::PARAMETER_TRANSPORT_MODE_ID = "transport_mode_id";
	}

	namespace impex
	{
		template<> const MultipleFileTypesImporter<CarPostalFileFormat>::Files MultipleFileTypesImporter<CarPostalFileFormat>::FILES(
			CarPostalFileFormat::Importer_::FILE_KOORD.c_str(),
			CarPostalFileFormat::Importer_::FILE_ECKDATEN.c_str(),
			CarPostalFileFormat::Importer_::FILE_BITFELD.c_str(),
			CarPostalFileFormat::Importer_::FILE_ZUGDAT.c_str(),
		"");
	}

	namespace pt
	{
		bool CarPostalFileFormat::Importer_::_checkPathsMap() const
		{
			FilePathsMap::const_iterator it(_pathsMap.find(FILE_ECKDATEN));
			if(it == _pathsMap.end() || it->second.empty()) return false;
			it = _pathsMap.find(FILE_BITFELD);
			if(it == _pathsMap.end() || it->second.empty()) return false;
			it = _pathsMap.find(FILE_KOORD);
			if(it == _pathsMap.end() || it->second.empty()) return false;
			it = _pathsMap.find(FILE_ZUGDAT);
			if(it == _pathsMap.end() || it->second.empty()) return false;
			return true;
		}



		DBTransaction CarPostalFileFormat::Importer_::_save() const
		{
			DBTransaction transaction;
			BOOST_FOREACH(Registry<JourneyPattern>::value_type line, _env.getRegistry<JourneyPattern>())
			{
				JourneyPatternTableSync::Save(line.second.get(), transaction);
			}
			BOOST_FOREACH(Registry<DesignatedLinePhysicalStop>::value_type lineStop, _env.getRegistry<DesignatedLinePhysicalStop>())
			{
				LineStopTableSync::Save(lineStop.second.get(), transaction);
			}
			BOOST_FOREACH(const Registry<ScheduledService>::value_type& service, _env.getRegistry<ScheduledService>())
			{
				if(service.second->empty())
				{
					ScheduledServiceTableSync::RemoveRow(service.second->getKey(), transaction);
				}
				else
				{
					ScheduledServiceTableSync::Save(service.second.get(), transaction);
				}
			}
			return transaction;
		}


		bool CarPostalFileFormat::Importer_::_parse(
			const path& filePath,
			std::ostream& os,
			const std::string& key,
			boost::optional<const admin::AdminRequest&> adminRequest
		) const {
			ifstream inFile;
			inFile.open(filePath.file_string().c_str());
			if(!inFile)
			{
				throw Exception("Could no open the file " + filePath.file_string());
			}
			bool error(false);

			// 0 : Coordinates
			if(key == FILE_KOORD)
			{
				string line;

				while(getline(inFile, line))
				{
					// Operator code
					string operatorCode(line.substr(0, 7));

					// Point
					double x(1000 * lexical_cast<double>(line.substr(10,7)));
					double y(1000 * lexical_cast<double>(line.substr(18,7)));
					shared_ptr<Point> coord;
					if(x && y)
					{
						coord = _dataSource.getCoordinatesSystem()->createPoint(x, y);
					}

					// Names
					if(!_dataSource.getCharset().empty())
					{
						line = IConv::IConv(_dataSource.getCharset(), "UTF-8").convert(line);
					}
					vector<string> cols;
					std::string times(line.substr(26));
					boost::algorithm::split( cols, times, boost::algorithm::is_any_of(","));
					string cityName(cols[0]);
					string name((cols.size() == 1) ? "Arrêt" : cols[1]);

					// Search for an existing stop
					set<StopPoint*> stops(
						_stopPoints.get(operatorCode)
					);

					if(stops.empty() || _showStopsOnly)
					{
						Bahnhof bahnhof;
						bahnhof.cityName = cityName;
						bahnhof.name = name;
						bahnhof.coords = coord;
						bahnhof.operatorCode = operatorCode;

						if(stops.empty())
						{
							bahnhof.stop = NULL;
							_nonLinkedBahnhofs[bahnhof.operatorCode] = bahnhof;
						}
						else
						{
							bahnhof.stop = *stops.begin();
							_linkedBahnhofs[bahnhof.operatorCode] = bahnhof;
						}
					}
				}

				// If at least a stop import has failed, no import but an admin screen if possible
				if(!_nonLinkedBahnhofs.empty() && adminRequest)
				{
					os << "<h1>Arrêts non liés à SYNTHESE</h1>";

					HTMLTable::ColsVector c;
					c.push_back("Code");
					c.push_back("Localité");
					c.push_back("Nom");
					c.push_back("Coords fichier");
					c.push_back("Coords fichier");
					c.push_back("Coords fichier (origine)");
					c.push_back("Coords fichier (origine)");
					c.push_back("Actions");

					HTMLTable t(c, ResultHTMLTable::CSS_CLASS);
					os << t.open();
					os.precision(0);
					BOOST_FOREACH(const Bahnhofs::value_type& bahnhof, _nonLinkedBahnhofs)
					{
						// Projected point
						shared_ptr<Point> projected;
						if(bahnhof.second.coords.get())
						{
							projected = CoordinatesSystem::GetInstanceCoordinatesSystem().convertPoint(
								*bahnhof.second.coords
							);
						}

						os << t.row();
						os << t.col();
						os << bahnhof.first;

						os << t.col();
						os << bahnhof.second.cityName;

						os << t.col();
						os << bahnhof.second.name;

						if(projected.get())
						{
							os << t.col();
							os << fixed << projected->getX();

							os << t.col();
							os << fixed << projected->getY();
						}
						else
						{
							os << t.col();
							os << t.col();
						}

						if(bahnhof.second.coords.get())
						{
							os << t.col();
							os << fixed << bahnhof.second.coords->getX();

							os << t.col();
							os << fixed << bahnhof.second.coords->getY();

							os << t.col();
							AdminActionFunctionRequest<StopPointAddAction, DataSourceAdmin> addRequest(*adminRequest);
							addRequest.getAction()->setName(bahnhof.second.name);
							addRequest.getAction()->setCityName(bahnhof.second.cityName);
							addRequest.getAction()->setCreateCityIfNecessary(true);
							Importable::DataSourceLinks links;
							links.insert(make_pair(&_dataSource, bahnhof.first));
							addRequest.getAction()->setDataSourceLinks(links);
							addRequest.getAction()->setPoint(DBModule::GetStorageCoordinatesSystem().convertPoint(*bahnhof.second.coords));
							os << HTMLModule::getLinkButton(addRequest.getURL(), "Ajouter");
						}
						else
						{
							os << t.col();
							os << t.col();
							os << t.col();
						}

					}
					os << t.close();
				}

				// Display of linked stops if specified
				if(_showStopsOnly && adminRequest && !_linkedBahnhofs.empty())
				{
					os << "<h1>Arrêts liés à SYNTHESE</h1>";

					HTMLTable::ColsVector c;
					c.push_back("Code");
					c.push_back("Zone d'arrêt SYNTHESE");
					c.push_back("Arrêt physique");
					c.push_back("Localité");
					c.push_back("Nom");
					c.push_back("Coords SYNTHESE");
					c.push_back("Coords SYNTHESE");
					c.push_back("Coords fichier");
					c.push_back("Coords fichier");
					c.push_back("Coords fichier (origine)");
					c.push_back("Coords fichier (origine)");
					c.push_back("Distance");
					c.push_back("Actions");

					AdminFunctionRequest<PTPlaceAdmin> openRequest(*adminRequest);
					AdminFunctionRequest<StopPointAdmin> openPhysicalRequest(*adminRequest);

					HTMLTable t(c, ResultHTMLTable::CSS_CLASS);
					os << t.open();
					os.precision(0);
					BOOST_FOREACH(const Bahnhofs::value_type& bahnhof, _linkedBahnhofs)
					{
						// Projected point
						shared_ptr<Point> projected;
						if(bahnhof.second.coords.get())
						{
							projected = CoordinatesSystem::GetInstanceCoordinatesSystem().convertPoint(
								*bahnhof.second.coords
							);
						}

						os << t.row();
						os << t.col();
						os << bahnhof.first;

						os << t.col();
						openRequest.getPage()->setConnectionPlace(_env.getSPtr(bahnhof.second.stop->getConnectionPlace()));
						os << HTMLModule::getHTMLLink(openRequest.getURL(), bahnhof.second.stop->getConnectionPlace()->getFullName());

						os << t.col();
						openPhysicalRequest.getPage()->setStop(_env.getSPtr(bahnhof.second.stop));
						os << HTMLModule::getHTMLLink(openPhysicalRequest.getURL(), bahnhof.second.stop->getName());

						os << t.col();
						os << bahnhof.second.cityName;

						os << t.col();
						os << bahnhof.second.name;

						if(bahnhof.second.stop->getGeometry().get())
						{
							os << t.col() << std::fixed << bahnhof.second.stop->getGeometry()->getX();
							os << t.col() << std::fixed << bahnhof.second.stop->getGeometry()->getY();
						}
						else
						{
							os << t.col() << "(non localisé)";
							os << t.col() << "(non localisé)";
						}

						if(bahnhof.second.coords.get())
						{
							double distance(-1);
							if (bahnhof.second.stop->getGeometry().get() && projected.get())
							{
								distance = geos::operation::distance::DistanceOp::distance(
									*projected,
									*bahnhof.second.stop->getGeometry()
								);
							}

							if(projected.get())
							{
								os << t.col();
								os << fixed << projected->getX();

								os << t.col();
								os << fixed << projected->getY();
							}
							else
							{
								os << t.col();
								os << t.col();
							}

							if(bahnhof.second.coords.get())
							{
								os << t.col();
								os << fixed << bahnhof.second.coords->getX();

								os << t.col();
								os << fixed << bahnhof.second.coords->getY();
							}
							else
							{
								os << t.col();
								os << t.col();
							}

							os << t.col();
							if(distance == 0)
							{
								os << "identiques";
							}
							else if(distance > 0)
							{
								os << distance << " m";
							}

							os << t.col();
							if(distance != 0)
							{
								AdminActionFunctionRequest<StopPointUpdateAction, DataSourceAdmin> moveRequest(*adminRequest);
								moveRequest.getAction()->setStop(_env.getEditableSPtr(bahnhof.second.stop));
								moveRequest.getAction()->setPoint(DBModule::GetStorageCoordinatesSystem().convertPoint(*bahnhof.second.coords));
								os << HTMLModule::getLinkButton(moveRequest.getHTMLForm().getURL(), "Mettre à jour coordonnées");
							}
						}
						else
						{
							os << t.col();
							os << t.col();
							os << t.col();
							os << t.col();
							os << t.col();
							os << t.col();
						}
					}
					os << t.close();
				}

				error = !_nonLinkedBahnhofs.empty() || _showStopsOnly;

			} // 1 : Time period
			else if(key == FILE_ECKDATEN)
			{
				// This file should already loaded at parameters reading
				error = !_startDate || !_endDate;

			} // 2 : Nodes
			else if(key == FILE_BITFELD)
			{
				string line;
				while(getline(inFile, line))
				{
					int id(lexical_cast<int>(line.substr(0,6)));
					string calendarString(line.substr(7));

					date curDate(*_startDate);
					bool first(true);

					Calendar calendar;

					BOOST_FOREACH(char c, calendarString)
					{
						int bits(0);
						if((c >= '0') && (c <= '9')) bits = (c - '0');
						else if((c >= 'A') && (c <= 'F')) bits = (c - 'A' + 10);
						else if((c >= 'a') && (c <= 'f')) bits = (c - 'a' + 10);

						for(int i(0); i<4; ++i)
						{
							if(!first || i>=2)
							{
								if(curDate > _endDate)
								{
									break;
								}
								if(bits & 8)
								{
									calendar.setActive(curDate);
								}
								curDate += days(1);
							}
							bits = bits << 1;
						}
						if(curDate > _endDate)
						{
							break;
						}
						first = false;
					}

					_calendarMap[id] = calendar;
				}
			} // 3 : Services
			else if (key == FILE_ZUGDAT)
			{
				string line;
				shared_ptr<ScheduledService> service;
				string serviceNumber;
				string lineNumber;
				int calendarNumber;
				JourneyPattern::StopsWithDepartureArrivalAuthorization stops;
				vector<time_duration> departures;
				vector<time_duration> arrivals;
				Calendar mask;
				for(date curDate(*_startDate); curDate <= *_endDate; curDate += days(1))
				{
					mask.setActive(curDate);
				}
				bool serviceMustBeAvoided(false);
				ImportableTableSync::ObjectBySource<CommercialLineTableSync> lines(_dataSource, _env);

				while(getline(inFile, line))
				{
					if(line.substr(0,2) == "*Z")
					{
						serviceNumber = line.substr(3,5);
						lineNumber = line.substr(9,6);
						stops.clear();
						departures.clear();
						arrivals.clear();
						serviceMustBeAvoided = false;
					}
					else if(line.substr(0,5) == "*A VE")
					{
						calendarNumber = lexical_cast<int>(line.substr(22,6));
					}
					else if(line.substr(0,1) != "*")
					{
						set<StopPoint*> stopsSet(
							_stopPoints.get(line.substr(0,7))
						);
						if(stopsSet.empty())
						{
							os << "WARN : stop " << line << " not found<br />";
							serviceMustBeAvoided = true;
						}
						else
						{
							string departureTime(line.substr(34,4));
							string arrivalTime(line.substr(29,4));
							bool isDeparture(departureTime != "9999" && departureTime != "    ");
							bool isArrival(arrivalTime != "9999" && arrivalTime != "    ");

							stops.push_back(
								JourneyPattern::StopWithDepartureArrivalAuthorization(
									stopsSet,
									optional<MetricOffset>(),
									isDeparture,
									isArrival
							)	);

							arrivals.push_back(
								isArrival ?
								hours(lexical_cast<int>(arrivalTime.substr(0,2))) + minutes(lexical_cast<int>(arrivalTime.substr(2,2))) :
								minutes(0)
							);
							departures.push_back(
								isDeparture ?
								hours(lexical_cast<int>(departureTime.substr(0,2))) + minutes(lexical_cast<int>(departureTime.substr(2,2))) :
								minutes(0)
							);
						}
					}

					// End of service
					if(line.size() < 54 && !serviceMustBeAvoided)
					{
						// Line
						CommercialLine* line(
							PTFileFormat::CreateOrUpdateLine(
								lines,
								lineNumber,
								optional<const string&>(),
								optional<const string&>(),
								optional<RGBColor>(),
								*_network,
								_dataSource,
								_env,
								os
						)	);

						// Update of the name with the code if nothing else is defined
						if(line->getName().empty())
						{
							line->setName(lineNumber);
						}

						// Wayback
						int numericServiceNumber(0);
						try
						{
							numericServiceNumber = lexical_cast<int>(serviceNumber);
						}
						catch(bad_lexical_cast&)
						{
						}
						bool wayBack = (numericServiceNumber % 2 == 1);
					
						// Journey pattern
						JourneyPattern* route(
							PTFileFormat::CreateOrUpdateRoute(
								*line,
								optional<const string&>(),
								optional<const string&>(),
								optional<const string&>(),
								optional<Destination*>(),
								optional<const RuleUser::Rules&>(),
								wayBack,
								_transportMode.get(),
								stops,
								_dataSource,
								_env,
								os,
								true
						)	);

						// Service
						ScheduledService* service(
							PTFileFormat::CreateOrUpdateService(
								*route,
								departures,
								arrivals,
								serviceNumber,
								_dataSource,
								_env,
								os
						)	);

						// Calendar
						if(service)
						{
							*service |= _calendarMap[calendarNumber];
						}
					}
				}
			}
			inFile.close();

			return true;
		}



		void CarPostalFileFormat::Importer_::displayAdmin(
			std::ostream& stream,
			const AdminRequest& request
		) const {

			stream << "<h1>Horaires</h1>";
			AdminFunctionRequest<DataSourceAdmin> importRequest(request);
			PropertiesHTMLTable t(importRequest.getHTMLForm());
			stream << t.open();
			stream << t.title("Propriétés");
			stream << t.cell("Effectuer import", t.getForm().getOuiNonRadioInput(DataSourceAdmin::PARAMETER_DO_IMPORT, false));
			stream << t.title("Données");
			stream << t.cell("Koord", t.getForm().getTextInput(_getFileParameterName(FILE_KOORD), _pathsMap[FILE_KOORD].file_string()));
			stream << t.cell("Eckdaten", t.getForm().getTextInput(_getFileParameterName(FILE_ECKDATEN), _pathsMap[FILE_ECKDATEN].file_string()));
			stream << t.cell("Bitfeld", t.getForm().getTextInput(_getFileParameterName(FILE_BITFELD), _pathsMap[FILE_BITFELD].file_string()));
			stream << t.cell("Zugdat", t.getForm().getTextInput(_getFileParameterName(FILE_ZUGDAT), _pathsMap[FILE_ZUGDAT].file_string()));
			stream << t.title("Paramètres");
			stream << t.cell("Uniquement afficher liste d'arrêts", t.getForm().getOuiNonRadioInput(PARAMETER_SHOW_STOPS_ONLY, _showStopsOnly));
			stream << t.cell("Effacer données existantes", t.getForm().getOuiNonRadioInput(PTDataCleanerFileFormat::PARAMETER_CLEAN_OLD_DATA, _cleanOldData));
			stream << t.cell("Ne pas importer données anciennes", t.getForm().getOuiNonRadioInput(PTDataCleanerFileFormat::PARAMETER_FROM_TODAY, _fromToday));
			stream << t.cell("Réseau", t.getForm().getTextInput(PARAMETER_NETWORK_ID, _network.get() ? lexical_cast<string>(_network->getKey()) : string()));
			stream << t.cell("Mode de transport", t.getForm().getTextInput(PARAMETER_TRANSPORT_MODE_ID, _transportMode.get() ? lexical_cast<string>(_transportMode->getKey()) : string()));
			stream << t.close();
		}



		util::ParametersMap CarPostalFileFormat::Importer_::_getParametersMap() const
		{
			ParametersMap pm(PTDataCleanerFileFormat::_getParametersMap());

			// Show stops only
			pm.insert(PARAMETER_SHOW_STOPS_ONLY, _showStopsOnly);

			// Network
			if(_network.get())
			{
				pm.insert(PARAMETER_NETWORK_ID, _network->getKey());
			}

			// Transport mode
			if(_transportMode.get())
			{
				pm.insert(PARAMETER_TRANSPORT_MODE_ID, _transportMode->getKey());
			}

			return pm;
		}



		void CarPostalFileFormat::Importer_::_setFromParametersMap( const util::ParametersMap& pm )
		{
			PTDataCleanerFileFormat::_setFromParametersMap(pm);

			// Show stops only
			_showStopsOnly = pm.getDefault<bool>(PARAMETER_SHOW_STOPS_ONLY, false);

			// Transport network
			if(pm.isDefined(PARAMETER_NETWORK_ID)) try
			{
				_network = TransportNetworkTableSync::GetEditable(pm.get<RegistryKeyType>(PARAMETER_NETWORK_ID), _env);
			}
			catch (ObjectNotFoundException<TransportNetwork>&)
			{
			}

			// Transport mode
			if(pm.isDefined(PARAMETER_TRANSPORT_MODE_ID)) try
			{
				_transportMode = RollingStockTableSync::GetEditable(pm.get<RegistryKeyType>(PARAMETER_TRANSPORT_MODE_ID), _env);
			}
			catch (ObjectNotFoundException<RollingStock>&)
			{
			}

			// Import dates
			FilePathsMap::const_iterator it(_pathsMap.find(FILE_ECKDATEN));
			if(it != _pathsMap.end())
			{
				ifstream inFile;
				inFile.open(it->second.file_string().c_str());
				if(!inFile)
				{
					throw Exception("Could no open the calendar file.");
				}

				date now(day_clock::local_day());
				string line;
				if(!getline(inFile, line))
				{
					throw Exception("Inconsistent Eckdaten file");
				}
				_startDate = date(
					lexical_cast<int>(line.substr(6,4)),
					lexical_cast<int>(line.substr(3,2)),
					lexical_cast<int>(line.substr(0,2))
				);
				if(*_startDate < now && _fromToday)
				{
					_startDate = now;
				}

				if(!getline(inFile, line))
				{
					throw Exception("Inconsistent Eckdaten file");
				}
				_endDate = date(
					lexical_cast<int>(line.substr(6,4)),
					lexical_cast<int>(line.substr(3,2)),
					lexical_cast<int>(line.substr(0,2))
				);
			}
		}
}	}

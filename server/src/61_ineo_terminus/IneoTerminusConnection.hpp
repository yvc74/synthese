﻿
/** IneoTerminusConnection class header.
	@file IneoTerminusConnection.hpp

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

#ifndef SYNTHESE_pt_IneoTerminusConnection_hpp__
#define SYNTHESE_pt_IneoTerminusConnection_hpp__

#include "IConv.hpp"
#include "ParametersMap.h"
#include "UtilTypes.h"
#include "XmlParser.h"

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/logic/tribool.hpp>
#include <set>
#include <deque>
#include <string>

namespace synthese
{

	namespace pt
	{
		class StopPoint;
	}

	namespace ineo_terminus
	{
		/** IneoTerminusConnection class.
			@ingroup m35
		*/
		class IneoTerminusConnection
		{
		public:
			static const std::string MODULE_PARAM_INEO_TERMINUS_PORT;
			static const std::string MODULE_PARAM_INEO_TERMINUS_NETWORK;
			static const std::string MODULE_PARAM_INEO_TERMINUS_DATASOURCE;
			static const std::string MODULE_PARAM_INEO_TERMINUS_TICK_INTERVAL;
			static const std::string MODULE_PARAM_INEO_TERMINUS_XSD_LOCATION;
			static const std::string MODULE_PARAM_INEO_TERMINUS_PASSENGER_FAKE_BROADCAST;
			static const std::string MODULE_PARAM_INEO_TERMINUS_DRIVER_FAKE_BROADCAST;
			static const std::string MODULE_PARAM_INEO_TERMINUS_PPDS_FAKE_BROADCAST;
			static const std::string MODULE_PARAM_INEO_TERMINUS_GIROUETTE_FAKE_BROADCAST;
			static const std::string MODULE_PARAM_INEO_TERMINUS_SONOPASSENGER_FAKE_BROADCAST;
			static const std::string MODULE_PARAM_INEO_TERMINUS_SONODRIVER_FAKE_BROADCAST;
			static const std::string MODULE_PARAM_INEO_TERMINUS_SONOSTOPPOINT_FAKE_BROADCAST;
			static const std::string MODULE_PARAM_INEO_TERMINUS_BIVGENERAL_FAKE_BROADCAST;
			static const std::string MODULE_PARAM_INEO_TERMINUS_BIVLINEMAN_FAKE_BROADCAST;
			static const std::string MODULE_PARAM_INEO_TERMINUS_BIVLINEAUTO_FAKE_BROADCAST;
			static const std::string INEO_TERMINUS_XML_HEADER;

			enum Status
			{
				offline,	// The client is not connected
				online,		// The client is connected
				connect		// The client must be connected
			};

			typedef enum
			{
				AucuneErreur = 0,
				BorneInconnue = 10,
				LigneInconnue = 11,
				ConducteurInconnu = 12,
				ServiceVoitureInconnu = 13,
				VoitureInconnue = 14,
				VehiculeInconnu = 15,
				ArretInconnu = 16,
				GroupeInconnu = 17,
				ChainageInconnu = 18,
				EcranInconnu = 19,
				ProgrammationInconnue = 20,
				ProgrammationPerimee = 21,
				ProgrammationImpossiblePriorite = 22,
				ProgrammationImpossibleCollision = 23,
				MessageInconnu = 24,
				MessageVide = 25,
				EtatBorneInconnu = 26,
				VehiculePasEnService = 27,
				ValeurBaliseHorsLimites = 28,
				ValeurBaliseCaractereInterdit = 29,
				TropDeMessagesPourBorne = 30,
				AutreErreur = 99
			} IneoApplicationError;


		private:
			static boost::shared_ptr<IneoTerminusConnection> _theConnection;
			static int _idRequest;
			static std::map<std::string, util::RegistryKeyType> _fakeBroadcastPoints;
			static std::set<std::string> _creationRequestTags;
			static std::set<std::string> _deletionRequestTags;
			static std::set<std::string> _creationOrDeletionResponseTags;
			static std::set<std::string> _getStatesResponseTags;

			std::string _ineoPort;
			util::RegistryKeyType _ineoNetworkID;
			util::RegistryKeyType _ineoDatasource;
			int _ineoTickInterval;
			std::string _ineoXsdLocation;

			mutable Status _status;

			typedef std::pair<XMLResults, XMLNode> XMLParserResult;

			struct Recipient
			{
				std::string type;
				std::string name;
				std::string parameter;
			};
			
			static XMLNode ParseInput(
				const std::string& xml
			);

			class tcp_connection : public boost::enable_shared_from_this<tcp_connection>
			{
			public:
				tcp_connection(
					boost::asio::io_service& io_service,
					util::RegistryKeyType network_id,
					util::RegistryKeyType datasource_id
				) :	_socket(io_service),
					_network_id(network_id),
					_datasource_id(datasource_id),
					_iconv("ISO-8859-1","UTF-8") {}

				~tcp_connection();

				boost::asio::ip::tcp::socket& socket();

				void start();

				void sendMessage(
					std::string message
				);

			private:
				void handle_write(
					const boost::system::error_code& error
				);

				void handle_read(
					const boost::system::error_code& error,
					size_t bytes_transferred
				);

				boost::asio::ip::tcp::socket _socket;
				util::RegistryKeyType _network_id;
				util::RegistryKeyType _datasource_id;
				util::IConv _iconv;
				boost::shared_ptr<boost::asio::streambuf> _buf;

				//generic enums
				typedef enum
				{
					Immediat = 0,
					Differe = 1,
					Repete = 2
				} Dispatching;

				// generic structs
				struct Messaging
				{
					std::string name;
					Dispatching dispatching;
					boost::posix_time::ptime startDate;
					boost::posix_time::ptime stopDate;
					boost::posix_time::time_duration startHour;
					boost::posix_time::time_duration stopHour;
					int repeatPeriod;
					boost::logic::tribool inhibition;
					std::string color;
					int codeGirouette;
					boost::logic::tribool activateHeadJingle;
					boost::logic::tribool activateBackJingle;
					boost::logic::tribool confirm;
					std::string startStopPoint;
					std::string endStopPoint;
					boost::logic::tribool diodFlashing;
					boost::logic::tribool alternance;
					boost::logic::tribool multipleStop;
					boost::logic::tribool terminusOrStop;
					std::string way;
					std::string stopPoint;
					int numberShow;
					boost::logic::tribool ttsBroadcasting;
					boost::logic::tribool jingle;
					std::string chaining;
					boost::logic::tribool priority;
					boost::logic::tribool varying;
					int duration;
					std::string content;
					std::string contentTts;
					std::string contentScrolling;
					std::vector<IneoTerminusConnection::Recipient> recipients;

					Messaging();
				};

				// Response generators

				bool _checkStatusRequest(
					XMLNode& node,
					std::string& response
				);

				bool _createMessageRequest(
					XMLNode& node,
					std::string& response
				);

				bool _deleteMessageRequest(
					XMLNode& node,
					std::string& response
				);

				bool _getStatesResponse(
					XMLNode& node,
					std::string& response
				);

				std::string _generateResponse(XMLNode& requestNode, const IneoApplicationError& errorCode);

				void _copyXMLNode(
					XMLNode& node,
					const int tabDepth,
					std::stringstream& outputStream
				);

				// generic parsers
				std::vector<IneoTerminusConnection::Recipient> _readRecipients(XMLNode node);
				Messaging _readMessagingNode(XMLNode node, std::string messagerieName);

				bool _buildMessagingParametersMap(
					std::vector<Messaging> messages,
					util::RegistryKeyType fakeBroadCastPoint,
					boost::shared_ptr<util::ParametersMap> parametersMap,
					IneoApplicationError& errorCode
				);

				bool _createMessages(std::vector<Messaging> messages, util::RegistryKeyType fakeBroadCastPoint, IneoApplicationError& errorCode);

				//generic writers
				bool _addRecipientsPM(util::ParametersMap& pm, std::vector<IneoTerminusConnection::Recipient>, IneoApplicationError& errorCode);

				std::set<synthese::pt::StopPoint*> _findIneoStopPoint(const std::string& ineoStopPointCode);
			};

			class tcp_server
			{
			public:
				tcp_server(
					boost::asio::io_service& io_service,
					std::string port,
					util::RegistryKeyType network_id,
					util::RegistryKeyType datasource_id
				);

			private:
				void start_accept();

				void handle_accept(
					tcp_connection* new_connection,
					const boost::system::error_code& error
				);

				boost::asio::io_service& _io_service;
				util::RegistryKeyType _network_id;
				util::RegistryKeyType _datasource_id;
				boost::asio::ip::tcp::acceptor _acceptor;
			};

			std::deque<IneoTerminusConnection::tcp_connection*> _livingConnections;
			std::deque<std::string> _messagesToSend;

			boost::mutex _connectionsMutex;
			boost::recursive_mutex _messagesMutex;
			boost::mutex _requestIdsMutex;

			IneoTerminusConnection();
			void _sendMessage();
			const std::string _buildGetStatesRequest(const std::string& ineoMessageType);

		public:

			/// @name Setters
			//@{
				void setIneoPort(const std::string& value){ _ineoPort = value; }
				void setIneoNetworkID(const util::RegistryKeyType& value){ _ineoNetworkID = value; }
				void setIneoDatasource(const util::RegistryKeyType& value){ _ineoDatasource = value; }
				void setIneoTickInterval(const int value) { _ineoTickInterval = value; }
				void setIneoXSDLocation(const std::string& value){ _ineoXsdLocation = value; }
				void setStatus(const Status& value){ _status = value; }
			//@}

			/// @name Getters
			//@{
				const std::string& getIneoPort() const { return _ineoPort; }
				const util::RegistryKeyType& getIneoNetworkID() const { return _ineoNetworkID; }
				const util::RegistryKeyType& getIneoDatasource() const { return _ineoDatasource; }
				int getIneoTickInterval() const { return _ineoTickInterval; }
				const std::string& getIneoXSDLocation() const { return _ineoXsdLocation; }
				const Status& getStatus() const { return _status; }
			//@}
			
			void addConnection(tcp_connection* new_connection);
			void removeConnection(tcp_connection* connection_to_remove);
			void setActiveConnection(tcp_connection* active_connection);

			void addMessage(std::string new_message);

			void synchronizeMessages();

			static void RunThread();

			static void ParameterCallback(
				const std::string& name,
				const std::string& value
			);

			static void MessageSender();
			static void Initialize();
			static boost::shared_ptr<IneoTerminusConnection> GetTheConnection(){ return _theConnection; }
			int getNextRequestID();
		};
}	}

#endif // SYNTHESE_pt_IneoTerminusConnection_hpp__


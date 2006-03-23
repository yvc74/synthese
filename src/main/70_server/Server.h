#ifndef SYNTHESE_SERVER_SERVER_H
#define SYNTHESE_SERVER_SERVER_H

#include "module.h"


#include <deque>
#include <iostream>
#include <string>

#include "Request.h"
#include "RequestDispatcher.h"

#include "00_tcp/TcpServerSocket.h"
#include "00_tcp/TcpService.h"


namespace synthese
{
namespace server
{



/** Main server class.

This class holds :
- the TCP server 
- request queue (thread-safe access)
- the threading mechanism 
- the request dispatcher instance

@ingroup m70
*/
class Server
{
 private:

    int _port;
    int _nbThreads;

 protected:


 public:

    Server (int port = 8899,
	    int nbThreads = 10);

    ~Server ();


    //! @name Getters/Setters
    //@{

    //@}


    //! @name Query methods
    //@{
    //@}


    //! @name Update methods
    //@{
    void run ();


    //@}

 protected:

    void registerHandlers ();


};


}
}



#endif

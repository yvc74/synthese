#ifndef SYNTHESE_LSXML_LINELS_H
#define SYNTHESE_LSXML_LINELS_H


#include "module.h"
#include <string>

class XMLNode;


namespace synthese
{
namespace env
{
    class Environment;
    class Line;
}

namespace lsxml
{


/** Line XML loading/saving service class.

Sample XML format :

<line id="L5"
      axisId="A2"
      firstYear="2006"
      lastYear="2007"/>

 @ingroup m16
*/
class LineLS
{
 public:

    static const std::string LINE_TAG;
    static const std::string LINE_ID_ATTR;
    static const std::string LINE_AXISID_ATTR;
    static const std::string LINE_FIRSTYEAR_ATTR;
    static const std::string LINE_LASTYEAR_ATTR;

 private:

    LineLS ();
    ~LineLS();


 public:
    
    
    //! @name Query methods.
    //@{

    /** Loads a line from an XML DOM node.
	Conversion from ids to pointers is done thanks to the
	environment parameter.
     */
    static synthese::env::Line* Load (
	XMLNode& node,
	const synthese::env::Environment& environment);

    /** @todo Not implemented.
     **/
    static XMLNode* Save (
	const synthese::env::Line* line);
    //@}


};



}
}


#endif


#include "Schedule.h"
#include "assert.h"


namespace synthese
{
namespace time
{




Schedule::Schedule ( const Hour& hour, DaysDuration daysSinceDeparture )
        : _hour ( hour )
        , _daysSinceDeparture ( daysSinceDeparture )
{
    assert ( ( _daysSinceDeparture >= 0 ) &&
             ( _daysSinceDeparture <= 28 ) );
}



Schedule::Schedule ( const Schedule& ref )
        : _hour ( ref._hour )
        , _daysSinceDeparture ( ref._daysSinceDeparture )
{
    assert ( ( _daysSinceDeparture >= 0 ) &&
             ( _daysSinceDeparture <= 28 ) );

}


Schedule::~Schedule ()
{
}


const Hour&
Schedule::getHour() const
{
    return _hour;
}



HourValue
Schedule::getHours() const
{
    return _hour.getHours ();
}



MinuteValue
Schedule::getMinutes() const
{
    return _hour.getMinutes ();
}



DaysDuration
Schedule::getDaysSinceDeparture () const
{
    return _daysSinceDeparture;
}


void
Schedule::setDaysSinceDeparture ( DaysDuration daysSinceDeparture )
{
    _daysSinceDeparture = daysSinceDeparture;
}




Schedule&
Schedule::setMinimum()
{
    _daysSinceDeparture = 0;
    _hour.setTimePattern( TIME_MIN, TIME_MIN );
    return ( *this );
}


Schedule&
Schedule::setMaximum()
{
    _daysSinceDeparture = 255;
    _hour.setTimePattern( TIME_MAX, TIME_MAX );
    return ( *this );

}


Schedule&
Schedule::operator += ( MinutesDuration op )
{
    _daysSinceDeparture = ( _daysSinceDeparture + ( _hour += op ) );
    return ( *this );
}



bool
operator < ( const Schedule& op1, const Schedule& op2 )
{
    return ( op1.getDaysSinceDeparture () < op2.getDaysSinceDeparture () ||
             op1.getDaysSinceDeparture () == op2.getDaysSinceDeparture () &&
             op1.getHour() < op2.getHour () );
}



bool
operator < ( const Schedule& op1, const Hour& op2 )
{
    return ( op1.getHour () < op2 );
}


bool operator <= ( const Schedule& op1, const Schedule& op2 )
{
    return ( op1.getDaysSinceDeparture () < op2.getDaysSinceDeparture () ||
             op1.getDaysSinceDeparture () == op2.getDaysSinceDeparture () &&
             op1.getHour () <= op2.getHour () );

}



bool
operator <= ( const Schedule& op1, const Hour& op2 )
{
    return ( op1.getHour () <= op2 );

}



bool
operator >= ( const Schedule& op1, const Schedule& op2 )
{
    return ( op1.getDaysSinceDeparture () > op2.getDaysSinceDeparture () ||
             op1.getDaysSinceDeparture () == op2.getDaysSinceDeparture () &&
             op1.getHour () >= op2.getHour () );
}



bool
operator >= ( const Schedule& op1, const Hour& op2 )
{
    return ( op1.getHour () >= op2 );
}




bool
operator > ( const Schedule& op1, const Hour& op2 )
{
    return ( op1.getHour () > op2 );
}



MinutesDuration
operator - ( const Schedule& op1, const Schedule& op2 )
{
    MinutesDuration result;
    int retain = 0;

    // 1: Hour
    result = op1.getHour () - op2.getHour ();

    if ( result < 0 )
    {
        retain = 1;
        result += MINUTES_PER_DAY;
    }

    // 2: Days since departure
    result += ( op1.getDaysSinceDeparture () - op2.getDaysSinceDeparture () - retain )
              * MINUTES_PER_DAY;

    return result;
}




std::ostream&
operator<< ( std::ostream& os, const Schedule& op )
{
    os << op.getDaysSinceDeparture ();
    os << op.getHour ();
    return os ;
}




Schedule&
Schedule::operator = ( const std::string& op )
{
    if ( op.size () == 0 )
    {
        _daysSinceDeparture = 0;
        _hour = op;
    }
    else if ( op[ 0 ] == TIME_MIN )
        setMinimum();
    else if ( op[ 0 ] == TIME_MAX )
        setMaximum();
    else
    {
        _daysSinceDeparture = atoi ( op.substr ( 0, 1 ).c_str () );
        _hour = op.substr( 1 );
    }
    return *this;
}




}
}

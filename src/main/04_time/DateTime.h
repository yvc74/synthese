
/** DateTime class header.
	@file DateTime.h

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

#ifndef SYNTHESE_TIME_DATETIME_H
#define SYNTHESE_TIME_DATETIME_H


#include "module.h"

#include "Date.h"
#include "Hour.h"


#include <iostream>
#include <string>



namespace synthese
{
	namespace time
	{


		class Schedule;

		/** DateTime
			@ingroup m04
		*/
		class DateTime
		{
			private:

				Date _date; //!< Date
				Hour _hour; //!< Hour

			public:

				DateTime ( int day = TIME_CURRENT, int month = TIME_SAME, int year = TIME_SAME,
						int hours = TIME_SAME, int minutes = TIME_SAME );

				DateTime ( const DateTime& ref);
				DateTime ( const Date& date );

				~DateTime();

				//! @name Getters/Setters
				//@{
				const Date& getDate() const;
				const Hour& getHour() const;

				int getYear () const;
				int getDay() const;
				int getMonth() const;
				int getHours() const;
				int getMinutes() const;

				void setHour ( const Hour& );
				//@}

				//! @name Update methods
				//@{
				void updateDateTime ( int day = TIME_CURRENT, int month = TIME_SAME,
									int year = TIME_SAME, int hours = TIME_SAME,
									int minutes = TIME_SAME );

				void updateDate ( int day = TIME_CURRENT, int month = TIME_SAME, int year = TIME_SAME );
				void updateHour ( int hours = TIME_SAME, int minutes = TIME_SAME );

				void updateHour ( const std::string& );
				void subDaysDuration ( int daysDuration );
				void addDaysDuration ( int daysDuration = 1);
				//@}


				//! @name Query methods
				//@{
				bool isValid () const;
				bool isUnknown() const;
				std::string toInternalString () const;
				std::string toSQLiteString(bool withApostrophes = true) const;
				std::string toString() const;
				//@}



				/** Adds one day to this DateTime
				*/
				DateTime& operator ++ ( int );


				/** Subs one day to this DateTime
				*/
				DateTime& operator -- ( int );

				DateTime& operator += ( int minutesDuration );
				DateTime& operator -= ( int minutesDuration );

				DateTime& operator = ( const DateTime& ref);

				DateTime& operator = ( const std::string& );
				DateTime& operator = ( const Date& );
				DateTime& operator = ( const Hour& );


				/** Sets schedule hour to this DateTime.
				*/
				DateTime& operator = ( const Schedule& );

			/** Constructs a DateTime from an SQL timestamp string (AAAAMMJJhhmmss);
				seconds are ignored.
			*/
			static DateTime FromSQLTimestamp (const std::string& sqlTimestamp);

			/** Constructs a DateTime from a string AAAA/MM/JJ hh:mm:ss;
				seconds are ignored.
			*/
			static DateTime FromString (const std::string& str);


		};


		std::ostream& operator<< ( std::ostream& os, const DateTime& op );



		bool operator == ( const DateTime& op1, const DateTime& op2 );
		bool operator != ( const DateTime& op1, const DateTime& op2 );
		bool operator <= ( const DateTime& op1, const DateTime& op2 );
		bool operator <= ( const DateTime& op1, const Schedule& op2 );
		bool operator < ( const DateTime& op1, const DateTime& op2 );
		bool operator < ( const DateTime& op1, const Schedule& op2 );
		bool operator >= ( const DateTime& op1, const DateTime& op2 );
		bool operator >= ( const DateTime& op1, const Schedule& op2 );
		bool operator > ( const DateTime& op1, const DateTime& op2 );
		bool operator > ( const DateTime& op1, const Schedule& op2 );


		/**
		* @return The number of minutes between two DateTime objects.
		*/
		int operator - ( const DateTime& op1, const DateTime& op2 );

		DateTime operator + ( const DateTime& op, int minutesDuration );
		DateTime operator - ( const DateTime& op, int minutesDuration );


	}
}

#endif


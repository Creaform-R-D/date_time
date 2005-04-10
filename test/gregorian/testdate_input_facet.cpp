/* Copyright (c) 2005 CrystalClear Software, Inc.
 * Use, modification and distribution is subject to the
 * Boost Software License, Version 1.0. (See accompanying
 * file LICENSE-1.0 or http://www.boost.org/LICENSE-1.0)
 * Author: Jeff Garland, Bart Garst
 * $Date$
 */

#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/testfrmwk.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// for tests that are expected to fail
template<class temporal_type, class exception_type>
bool failure_test(temporal_type component,
                  const std::string& input,
                  exception_type /*except*/,
                  boost::gregorian::date_input_facet* facet)
{
  using namespace boost::gregorian;
  bool result = false;
  std::istringstream iss(input);
  iss.imbue(std::locale(std::locale::classic(), facet));
  try {
    iss >> component;
  }
  catch(exception_type e) {
    result = true;
  }
  catch(...) {
    result = false;
  }

  return result;
}

int main(){
  using namespace boost::gregorian;
 
  // set up initial objects
  date d(not_a_date_time);
  days dd(not_a_date_time);
  greg_month m(1);
  greg_weekday gw(0);
  greg_day gd(1);
  greg_year gy(2000);
  // exceptions for failure_tests
  std::ios_base::failure e_failure("default");
  bad_month e_bad_month;
  bad_year e_bad_year;
  bad_day_of_month e_bad_day_of_month;
  bad_weekday e_bad_weekday;
  bad_day_of_year e_bad_day_of_year;

  // default format tests: date, days, month, weekday, day, year
  std::istringstream iss("2005-Jan-15 21 Feb Tue 4 2002");
  iss >> d;
  check("Default format date", d == date(2005,Jan,15));
  iss >> dd;
  check("Default (only) format positive days", dd == days(21));
  iss >> m;
  check("Default format month", m == greg_month(2));
  iss >> gw;
  check("Default format weekday", gw == greg_weekday(2));
  iss >> gd;
  check("Default (only) format day of month", gd == greg_day(4));
  iss >> gy;
  check("Default format year", gy == greg_year(2002));
  // failure tests
  check("Input Misspelled in year (date)", 
      failure_test(d, "205-Jan-15", e_bad_year, new date_input_facet()));
  check("Input Misspelled in month (date)", 
      failure_test(d, "2005-Jsn-15", e_bad_month, new date_input_facet()));
  check("Input Misspelled in day (date)", 
      failure_test(d, "2005-Jan-51", e_bad_day_of_month, new date_input_facet()));
  check("Input Misspelled greg_weekday", 
      failure_test(gw, "San", e_bad_weekday, new date_input_facet()));
  check("Input Misspelled month", 
      failure_test(m, "Jsn", e_bad_month, new date_input_facet()));
  check("Bad Input greg_day", 
      failure_test(gd, "Sun", e_bad_day_of_month, new date_input_facet()));
  check("Input Misspelled greg_year", 
      failure_test(gy, "205", e_bad_year, new date_input_facet()));

  // change to full length names, iso date format, and 2 digit year
  date_input_facet* facet = new date_input_facet();
  facet->set_iso_format();
  facet->month_format("%B");
  facet->weekday_format("%A");
  facet->year_format("%y");
  iss.str("20050115 -55 February Tuesday 02");
  iss.imbue(std::locale(std::locale::classic(), facet));

  iss >> d;
  check("ISO format date", d == date(2005,Jan,15));
  iss >> dd;
  check("Default (only) format negative days", dd == days(-55));
  iss >> m;
  check("Full format month", m == greg_month(2));
  iss >> gw;
  check("Full format weekday", gw == greg_weekday(2));
  iss >> gy;
  check("2 digit format year", gy == greg_year(2002));

  // All days, month, weekday, day, and year formats have been tested
  // begin testing other date formats
  facet->set_iso_extended_format();
  iss.str("2005-01-15");
  iss >> d;
  check("ISO Extended format date", d == date(2005,Jan,15));

  facet->format("%B %d, %Y");
  iss.str("March 15, 2006");
  iss >> d;
  check("Custom date format: \"%B %d, %Y\" => 'March 15, 2006'", 
      d == date(2006,Mar,15));

  facet->format("%Y-%j"); // Ordinal format ISO8601(2000 sect 5.2.2.1 extended)
  iss.str("2006-074");
  iss >> d;
  check("Custom date format: \"%Y-%j\" => '2006-074'", 
      d == date(2006,Mar,15));
  check("Bad input Custom date format: \"%Y-%j\" => '2006-74'", 
      failure_test(d, "2006-74", e_bad_day_of_year, facet));

  // date_period tests

  // A date_period is constructed with an open range. So the periods
  // [2000-07--04/2000-07-25) <-- open range
  // And
  // [2000-07--04/2000-07-24] <-- closed range
  // Are equal
  date begin(2002, Jul, 4);
  days len(21);
  date_period dp(date(2000,Jan,1), days(1));
  iss.str("[2002-07-04/2002-07-24]");
  facet->set_iso_extended_format();
  iss >> dp;
  check("Default period (closed range)", dp == date_period(begin,len));

  // open range
  period_parser pp(period_parser::AS_OPEN_RANGE);
  iss.str("[2002-07-04/2002-07-25)");
  facet->period_parser(pp);
  iss >> dp;
  check("Open range period", dp == date_period(begin,len));
  // custom period delimiters
  pp.delimiter_strings(" to ", "from ", " exclusive", " inclusive");
  iss.str("from-2002-07-04-to-2002-07-25-exclusive");
  facet->period_parser(pp);
  iss >> dp;
  check("Open range period - custom delimiters", dp == date_period(begin,len));
  pp.range_option(period_parser::AS_CLOSED_RANGE);
  iss.str("from 2002-07-04 to 2002-07-24 inclusive");
  facet->period_parser(pp);
  iss >> dp;
  check("Closed range period - custom delimiters", dp == date_period(begin,len));

  
  // date_generator tests

  // date_generators use formats contained in the 
  // date_input_facet for weekdays and months
  // reset month & weekday formats to defaults
  facet->month_format("%b");
  facet->weekday_format("%a");

  partial_date pd(1,Jan);
  nth_kday_of_month nkd(nth_kday_of_month::first, Sunday, Jan);
  first_kday_of_month fkd(Sunday, Jan);
  last_kday_of_month lkd(Sunday, Jan);
  first_kday_before fkb(Sunday);
  first_kday_after fka(Sunday);
  // using default date_generator_parser "nth_strings"
  iss.str("29 Feb");
  iss >> pd; 
  // Feb-29 is a valid date_generator, get_date() will fail in a non-leap year
  check("Default strings, partial_date", 
      pd.get_date(2004) == date(2004,Feb,29));
  iss.str("second Mon of Mar");
  iss >> nkd; 
  check("Default strings, nth_day_of_the_week_in_month", 
      nkd.get_date(2004) == date(2004,Mar,8));
  iss.str("first Tue of Apr");
  iss >> fkd; 
  check("Default strings, first_day_of_the_week_in_month", 
      fkd.get_date(2004) == date(2004,Apr,6));
  iss.str("last Wed of May");
  iss >> lkd; 
  check("Default strings, last_day_of_the_week_in_month", 
      lkd.get_date(2004) == date(2004,May,26));
  iss.str("Thu before");
  iss >> fkb; 
  check("Default strings, first_day_of_the_week_before", 
      fkb.get_date(date(2004,Feb,8)) == date(2004,Feb,5));
  iss.str("Fri after");
  iss >> fka; 
  check("Default strings, first_day_of_the_week_after", 
      fka.get_date(date(2004,Feb,1)) == date(2004,Feb,6));
  // failure tests
  check("Incorrect elements (date_generator)", // after/before type mixup
      failure_test(fkb, "Fri after", e_failure, new date_input_facet()));
  check("Incorrect elements (date_generator)", // first/last type mixup
      failure_test(lkd, "first Tue of Apr", e_failure, new date_input_facet()));
  check("Incorrect elements (date_generator)", // 'in' is wrong 
      failure_test(nkd, "second Mon in Mar", e_failure, new date_input_facet()));

  // date_generators - custom element strings
  facet->date_gen_element_strings("1st","2nd","3rd","4th","5th","final","prior to","past","in");
  iss.str("3rd Sat in Jul");
  iss >> nkd;
  check("Custom strings, nth_day_of_the_week_in_month", 
      nkd.get_date(2004) == date(2004,Jul,17));
  iss.str("1st Wed in May");
  iss >> fkd; 
  check("Custom strings, first_day_of_the_week_in_month", 
      fkd.get_date(2004) == date(2004,May,5));
  iss.str("final Tue in Apr");
  iss >> lkd; 
  check("Custom strings, last_day_of_the_week_in_month", 
      lkd.get_date(2004) == date(2004,Apr,27));
  iss.str("Fri prior to");
  iss >> fkb; 
  check("Custom strings, first_day_of_the_week_before", 
      fkb.get_date(date(2004,Feb,8)) == date(2004,Feb,6));
  iss.str("Thu past");
  iss >> fka; 
  check("Custom strings, first_day_of_the_week_after", 
      fka.get_date(date(2004,Feb,1)) == date(2004,Feb,5));

  // date_generators - special case with empty element string
  /* Doesn't work. Empty string returns -1 from string_parse_tree 
   * because it attempts to match the next set of characters in the 
   * stream to the wrong element. Ex. It attempts to match "Mar" to 
   * the 'of' element in the test below.
   * 
  facet->date_gen_element_strings("1st","2nd","3rd","4th","5th","final","prior to","past",""); // the 'of' string is an empty string
  iss.str("final Mon Mar");
  iss >> lkd; 
  check("Special case, empty element string", 
      lkd.get_date(2005) == date(2005,Mar,28));
      */
  

  // special values tests (date and days only)
  iss.str("minimum-date-time +infinity");
  iss >> d;
  iss >> dd;
  check("Special values, default strings, min_date_time date",
      d == date(min_date_time));
  check("Special values, default strings, pos_infin days",
      dd == days(pos_infin));
  iss.str("-infinity maximum-date-time");
  iss >> d;
  iss >> dd;
  check("Special values, default strings, neg_infin date",
      d == date(neg_infin));
  check("Special values, default strings, max_date_time days",
      dd == days(max_date_time));
  iss.str("not-a-date-time");
  iss >> d;
  check("Special values, default strings, not_a_date_time date",
      d == date(not_a_date_time));

  // special values custom, strings
  special_values_parser svp("NADT", "MINF", "INF", "MINDT", "MAXDT");
  facet->special_values_parser(svp);
  iss.str("MINDT INF");
  iss >> d;
  iss >> dd;
  check("Special values, custom strings, min_date_time date",
      d == date(min_date_time));
  check("Special values, custom strings, pos_infin days",
      dd == days(pos_infin));
  iss.str("MINF MAXDT");
  iss >> d;
  iss >> dd;
  check("Special values, custom strings, neg_infin date",
      d == date(neg_infin));
  check("Special values, custom strings, max_date_time days",
      dd == days(max_date_time));
  iss.str("NADT");
  iss >> dd;
  check("Special values, custom strings, not_a_date_time days",
      dd == days(not_a_date_time));
  // failure test
  check("Misspelled input, special_value date", 
      failure_test(d, "NSDT", e_bad_year, new date_input_facet()));
  check("Misspelled input, special_value days", 
      failure_test(dd, "NSDT", e_failure, new date_input_facet()));

  {
    // German names. Please excuse any errors, I don't speak German and 
    // had to rely on an on-line translation service.
    // These tests check one of each (at least) from all sets of custom strings

    // create a custom format_date_parser
    std::string m_a[] = {"Jan","Feb","Mar","Apr","Mai",
                         "Jun","Jul","Aug","Sep","Okt","Nov","Dez"};
    std::string m_f[] = {"Januar","Februar","Marz","April",
                         "Mai","Juni","Juli","August",
                         "September","Oktober","November","Dezember"};
    std::string w_a[] = {"Son", "Mon", "Die","Mit", "Don", "Fre", "Sam"};
    std::string w_f[] = {"Sonntag", "Montag", "Dienstag","Mittwoch",
                         "Donnerstag", "Freitag", "Samstag"};
    typedef boost::date_time::format_date_parser<date, char> date_parser;
    date_parser::input_collection_type months_abbrev;
    date_parser::input_collection_type months_full;
    date_parser::input_collection_type wkdays_abbrev;
    date_parser::input_collection_type wkdays_full;
    months_abbrev.assign(m_a, m_a+12);
    months_full.assign(m_f, m_f+12);
    wkdays_abbrev.assign(w_a, w_a+7);
    wkdays_full.assign(w_f, w_f+7);
    date_parser d_parser("%B %d %Y",
                         months_abbrev, months_full, 
                         wkdays_abbrev, wkdays_full);

    // create a special_values parser
    special_values_parser sv_parser("NichtDatumzeit",
                                    "Negativ Unendlichkeit",
                                    "Positiv Unendlichkeit",
                                    "Wenigstes Datum",
                                    "Maximales Datum");

    // create a period_parser
    period_parser p_parser; // default will do
    // create date_generator_parser
    typedef boost::date_time::date_generator_parser<date,char> date_gen_parser;
    date_gen_parser dg_parser("Zuerst","Zweitens","Dritt","Viert",
                              "F�nft","Letzt","Vor","Nach","Von");

    // create the date_input_facet
    date_input_facet* de_facet = 
      new date_input_facet("%B %d %Y",
                           d_parser,
                           sv_parser,
                           p_parser,
                           dg_parser);
    std::istringstream iss;
    iss.imbue(std::locale(std::locale::classic(), de_facet));
    // June 06 2005, Dec, minimum date, Tues
    iss.str("Juni 06 2005 Dez Wenigstes Datum Die");
    iss >> d;
    iss >> m;
    check("German names: date", d == date(2005, Jun, 6));
    check("German names: month", m == greg_month(Dec));
    iss >> d;
    iss >> gw;
    check("German names: special value date", d == date(min_date_time));
    check("German names: short weekday", gw == greg_weekday(Tuesday));
    de_facet->weekday_format("%A"); // long weekday
    // Tuesday, Second Tuesday of Mar
    iss.str("Dienstag Zweitens Dienstag von Mar");
    iss >> gw;
    iss >> nkd;
    check("German names: long weekday", gw == greg_weekday(Tuesday));
    check("German names, nth_day_of_the_week_in_month", 
        nkd.get_date(2005) == date(2005,Mar,8));
    // Tuesday after
    iss.str("Dienstag Nach");
    iss >> fka; 
    check("German names, first_day_of_the_week_after", 
        fka.get_date(date(2005,Apr,5)) == date(2005,Apr,12));
  }

  return printTestStats();
}
                      
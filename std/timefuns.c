/*  Some Macros one may easily use in his Objects/Rooms/Monsters.... :      */
#include <libtime.h>
#include <language.h>


int time, year, season, week, day, hour, minute, second, weekday;

string *seasonarray = ({ "Spring", "Summer", "Fall", "Winter" });

string *quarterarray = ({ "first quarter", "second quarter", "third quarter", "fourth quarter" });

string *weekdayarray = ({ "NeverSeen", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" });

public nomask string time_get_string(string str);
public nomask string time_get_description(string str);
public int time_get_int(string str);
public int query_is_day();
public int query_is_night();


public int time_get_int(string str)
{
    int accum;

    time = time();
    time = time - START_TIME * MUD_SECONDS_PER_SECOND;
    time = time * MUD_SECONDS_PER_SECOND;
    /* time == time since start of time */

    accum = time;

    year   = accum / S_PER_YEAR;
    accum -= year * S_PER_YEAR;
    season = accum / S_PER_SEASON;
    accum -= season * S_PER_SEASON;
    week   = accum / S_PER_WEEK;
    accum -= week * S_PER_WEEK;
    day    = accum / S_PER_DAY;
    accum -= day * S_PER_DAY;
    hour   = accum / S_PER_HOUR;
    accum -= hour * S_PER_HOUR;
    minute = accum / SECONDS_PER_MINUTE;
    accum -= minute * SECONDS_PER_MINUTE;
    second = accum;

    if(!str)
        return 9999;

    switch(str)
    {
        case "Year":
        case "year":
            return year + FIRST_YEAR;
        case "Season":
        case "season":
            return season;
        case "Week":
        case "week":
            return week + 1;
        case "Day":
        case "day":
            return day + 1;
        case "Hour":
        case "hour":
            return hour;
        case "Minute":
        case "minute":
            return minute;
        case "Second":
        case "second":
            return second;
        default:
            return 9999;
    }
}


public nomask string time_get_description(string str)
{
    if(!str)
        return "String required!\n";

    if(str == "date")
    {
        return "the " + LANG_WORD(WEEK) + " " + weekdayarray[DAY] + " of " +
        seasonarray[SEASON] + ", " + YEAR;
    }

    if(str == "time")
    {
        return "the " + quarterarray[(MINUTE/15)] + " of the " + LANG_WORD(HOUR) + " hour";
    }

    if(str == "exacttime")
    {
        return time_get_string("time");
    }

    if(str == "weekday")
    {
        return weekdayarray[WEEKDAY];
    }

    return "Bad String\n";
}



public nomask string time_get_string(string str)
{
    string daystr, seasonstr, hourstr, minutestr, secondstr, weekstr, retstr, temp;
    int day, season, week, hour, minute, second;

    day = DAY;
    season = SEASON;
    week = WEEK;
    hour = HOUR;
    minute = MINUTE;
    second = SECOND;

    daystr = ""+day;
    weekstr = "0"+week;
    seasonstr = ""+season;
    hourstr = ""+hour;
    minutestr = ""+minute;
    secondstr = ""+second;

    if(day < 10)
        daystr="0"+day;

    if(season < 10)
        seasonstr="0"+season;

    if(hour < 10)
        hourstr="0"+hour;

    if(minute < 10)
        minutestr="0"+minute;

    if(second < 10)
        secondstr="0"+second;

    if(!str)
        return "String required!\n";

    if(str=="date")
    {
        retstr = daystr + "-" + weekstr + "-" + seasonstr + "-" + YEAR;
        return retstr;
    }
    else if(str=="time")
    {
        if(hour == 12)
        {
            temp = "PM";
        }

        if(hour == 0)
        {
            hour = 12;
            hourstr = ""+hour;
            temp = "AM";
        }
        else if(hour > 12)
        {
            hour -= 12;
            hourstr = ""+hour;
            temp = "PM";
        }
        else
            temp = "AM";

        retstr = hourstr + ":" + minutestr + ":" + secondstr + " "+temp;
        return retstr;
    }
    else if(str=="season")
    {
        retstr = seasonarray[season];
        return retstr;
    }

    return "Unknown String!\n";
}

public int query_is_day()
{
    if(HOUR >= NIGHT_BEGINS || HOUR < NIGHT_ENDS)
    return 0;
    else
    return 1;
}

public int query_is_night()
{
    if(HOUR >= NIGHT_BEGINS || HOUR < NIGHT_ENDS)
        return 1;
    else
        return 0;
}

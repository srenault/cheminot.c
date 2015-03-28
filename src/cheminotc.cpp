#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <list>
#include <queue>
#include <numeric>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <memory>
#include "cheminotc.h"
#include "fastmktime/fastmktime.h"

namespace cheminotc
{

    template <typename T>
    std::string to_string(T value)
    {
        std::ostringstream os ;
        os << value ;
        return os.str();
    }

    void oops(std::string message)
    {
        throw std::runtime_error(message);
    }

    double distance(const LatLng &a, const LatLng &b)
    {
        double radius = 6371;
        double nDLat = (b.lat - a.lat) * (M_PI/180);
        double nDLon = (b.lng - a.lng) * (M_PI/180);

        double nLatA =  a.lat * (M_PI/180);
        double nLatB =  b.lat * (M_PI/180);

        double nA = pow ( sin(nDLat/2), 2 ) +
            cos(nLatA) * cos(nLatB) *
            pow ( sin(nDLon/2), 2 );

        double nC = 2 * atan2( sqrt(nA), sqrt( 1 - nA ));
        return radius * nC;
    }

    bool isSubwayTrip(const std::string &stopId)
    {
        std::string subwayTripPrefix = "SUBWAY:";
        return stopId.find(subwayTripPrefix) == 0;
    }

    bool isParis(const std::string &id)
    {
        auto it = std::find_if(parisStopIds.begin(), parisStopIds.end(), [&id](std:: string vid)
        {
            return id == vid;
        });
        return it != parisStopIds.end();
    };

    tm getNow()
    {
        time_t rawtime;
        time(&rawtime);
        tm *info = gmtime(&rawtime);
        return *info;
    }

    std::string formatTime(tm time)
    {
        return to_string(time.tm_hour) + ":" + to_string(time.tm_min);
    }

    std::string formatDate(tm time)
    {
        return to_string(time.tm_mday) + "/" + to_string(time.tm_mon + 1) + "/" + to_string(time.tm_year);
    }

    std::string formatDateTime(tm datetime)
    {
        return formatDate(datetime) + " " + formatTime(datetime);
    }

    tm parseTime(tm dateref, std::string datetime)
    {
        tm time;
        strptime(datetime.c_str(), "%H:%M", &time);
        dateref.tm_hour = time.tm_hour;
        dateref.tm_min = time.tm_min;
        dateref.tm_sec = 0;
        return dateref;
    }

    tm parseDate(std::string datetime)
    {
        tm date;
        strptime(datetime.c_str(), "%d/%m/%Y", &date);
        date.tm_hour = 0;
        date.tm_min = 0;
        date.tm_sec = 0;
        return date;
    }

    tm asDateTime(time_t t)
    {
        tm dateTime;
        return *(localtime (&t));
    }

    time_t asTimestamp(tm a)
    {
        time_t timestamp = fastmktime::mk(&a);
        return timestamp;
    }

    tm addMinutes(tm datetime, int n)
    {
        datetime.tm_min += n;
        fastmktime::mk(&datetime);
        return datetime;
    }

    tm addHours(tm datetime, int n)
    {
        datetime.tm_hour += n;
        fastmktime::mk(&datetime);
        return datetime;
    }

    tm minusHours(tm datetime, int n)
    {
        datetime.tm_hour -= n;
        fastmktime::mk(&datetime);
        return datetime;
    }

    tm addDays(tm datetime, int n)
    {
        datetime.tm_mday += n;
        fastmktime::mk(&datetime);
        return datetime;
    }

    bool hasSameTime(const tm &a, const tm &b)
    {
        return (a.tm_hour == b.tm_hour) && (a.tm_min == b.tm_min);
    }

    bool hasSameDate(const tm &a, const tm &b)
    {
        return (a.tm_year == b.tm_year) && (a.tm_mon == b.tm_mon) && (a.tm_mday == b.tm_mday);
    }

    bool hasSameDateTime(const tm &a, const tm &b)
    {
        return hasSameTime(a, b) && hasSameDate(a, b);
    }

    bool timeIsBeforeEq(const tm &a, const tm &b)
    {
        if(a.tm_hour > b.tm_hour)
        {
            return false;
        }
        else if(a.tm_hour < b.tm_hour)
        {
            return true;
        }
        else
        {
            if(a.tm_min > b.tm_min)
            {
                return false;
            }
            else
            {
                return true;
            }
        }
    }

    bool timeIsBeforeNotEq(const tm &a, const tm &b)
    {
        return timeIsBeforeEq(a, b) && !hasSameTime(a, b);
    }

    bool dateIsBeforeEq(const tm &a, const tm &b)
    {
        if(a.tm_year > b.tm_year)
        {
            return false;
        }
        else if(a.tm_year < b.tm_year)
        {
            return true;
        }
        else
        {
            if(a.tm_mon > b.tm_mon)
            {
                return false;
            }
            else if(a.tm_mon < b.tm_mon)
            {
                return true;
            }
            else
            {
                if(a.tm_mday > b.tm_mday)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }
    }

    bool dateIsBeforeNotEq(const tm &a, const tm &b)
    {
        return dateIsBeforeEq(a, b) && !hasSameDate(a, b);
    }

    bool datetimeIsBeforeEq(const tm &a, const tm &b)
    {
        if(dateIsBeforeEq(a, b))
        {
            if(hasSameDate(a, b))
            {
                return timeIsBeforeEq(a, b);
            }
            else
            {
                return true;
            }
        }
        else
        {
            return false;
        }
    }

    bool datetimeIsBeforeNotEq(const tm &a, const tm &b)
    {
        return datetimeIsBeforeEq(a, b) && !hasSameDateTime(a, b);
    }

    Json::Value serializeArrivalTime(Graph *graph, Cache *cache, ArrivalTime arrivalTime)
    {
        Json::Value json;
        int arrival = asTimestamp(arrivalTime.arrival);
        int departure = asTimestamp(arrivalTime.departure);
        Vertice vi = cheminotc::getVerticeFromGraph(NULL, graph, cache, arrivalTime.stopId);
        json["stopId"] = arrivalTime.stopId;
        json["stopName"] = vi.name;
        json["arrival"] = arrival;
        json["departure"] = departure;
        json["tripId"] = arrivalTime.tripId;
        json["pos"] = arrivalTime.pos;
        return json;
    }

    Json::Value serializeArrivalTimes(Graph *graph, Cache *cache, std::list<ArrivalTime> arrivalTimes)
    {
        Json::Value array = Json::Value(Json::arrayValue);
        for(auto iterator = arrivalTimes.begin(), end = arrivalTimes.end(); iterator != end; ++iterator)
        {
            ArrivalTime arrivalTime = *iterator;
            array.append(serializeArrivalTime(graph, cache, arrivalTime));
        }
        return array;
    }

    Json::Value serializeStopTime(StopTime *stopTime)
    {
        Json::Value json;
        int arrival = asTimestamp(stopTime->arrival);
        int departure = asTimestamp(stopTime->departure);
        json["tripId"] = stopTime->tripId;
        json["arrival"] = arrival;
        json["departure"] = departure;
        json["pos"] = stopTime->pos;
        return json;
    }

    Json::Value serializeStopTimes(std::list<StopTime> stopTimes)
    {
        Json::Value array;
        for (auto iterator = stopTimes.begin(), end = stopTimes.end(); iterator != end; ++iterator)
        {
            array.append(serializeStopTime(&*iterator));
        }
        return array;
    }

    std::string formatStopTime(StopTime *stopTime)
    {
        Json::Value serialized = serializeStopTime(stopTime);
        return serialized.toStyledString();
    }

    std::string formatStopTimes(std::list<StopTime> stopTimes)
    {
        return serializeStopTimes(stopTimes).toStyledString();
    }

    bool isTerminus(const StopTime &a)
    {
        return hasSameTime(a.arrival, a.departure) && a.pos > 0;
    }

    std::list<std::shared_ptr<CalendarDate>> getCalendarDatesByServiceId(Cache *cache, CalendarDates *calendarDates, std::string serviceId)
    {
        auto it = cache->calendarDates.find(serviceId);
        if(it != cache->calendarDates.end())
        {
            return it->second;
        }
        else
        {
            std::list<std::shared_ptr<CalendarDate>> results;
            auto exceptions = (*calendarDates)[serviceId].calendardates();
            for (auto iterator = exceptions.begin(), end = exceptions.end(); iterator != end; ++iterator)
            {
                m::cheminot::data::CalendarDate calendarDateBuf = *iterator;
                std::shared_ptr<CalendarDate> calendarDate {new CalendarDate};
                calendarDate->serviceId = calendarDateBuf.serviceid();
                calendarDate->date = parseDate(calendarDateBuf.date());
                calendarDate->exceptionType = calendarDateBuf.exceptiontype();
                results.push_back(calendarDate);
            }
            cache->calendarDates[serviceId] = results;
            return results;
        }
    }

    StopTime parseStopTime(const tm *dateref, m::cheminot::data::StopTime stopTimeBuf)
    {
        StopTime stopTime;
        stopTime.tripId = stopTimeBuf.tripid();
        stopTime.arrival = parseTime(*dateref, stopTimeBuf.arrival());
        stopTime.departure = parseTime(*dateref, stopTimeBuf.departure());
        stopTime.pos = stopTimeBuf.pos();
        if(isSubwayTrip(stopTime.tripId))
        {
            stopTime.departure = *dateref;
            stopTime.arrival = *dateref;
        }
        else if(timeIsBeforeEq(stopTime.departure, *dateref))
        {
            stopTime.arrival = addDays(stopTime.arrival, 1);
            stopTime.departure = addDays(stopTime.departure, 1);
        }
        return stopTime;
    }

    std::list<StopTime> parseStopTimes(const tm *dateref, google::protobuf::RepeatedPtrField< ::m::cheminot::data::StopTime> stopTimesBuf)
    {
        std::list<StopTime> stopTimes;
        for(auto iterator = stopTimesBuf.begin(), end = stopTimesBuf.end(); iterator != end; ++iterator)
        {
            StopTime stopTime = parseStopTime(dateref, *iterator);
            stopTimes.push_back(stopTime);
        }
        return stopTimes;
    }

    std::list<std::string> parseTripStopIds(m::cheminot::data::TripStopIds tripStopIdsBuf)
    {
        std::list<std::string> stopIds;
        auto stopids = tripStopIdsBuf.stopids();
        for(auto iterator = stopids.begin(), end = stopids.end(); iterator != end; ++iterator)
        {
            stopIds.push_back(*iterator);
        }
        return stopIds;
    }

    std::list<std::string> parseEdges(google::protobuf::RepeatedPtrField< std::string > edgesBuf)
    {
        std::list<std::string> edges;
        for(auto iterator = edgesBuf.begin(), end = edgesBuf.end(); iterator != end; ++iterator)
        {
            edges.push_back(*iterator);
        }
        return edges;
    }

    std::unique_ptr<Calendar> parseCalendar(m::cheminot::data::Calendar calendarBuf)
    {
        std::unique_ptr<Calendar> calendar(new Calendar());

        calendar->week["monday"] = calendarBuf.monday() == "1";
        calendar->week["tuesday"] = calendarBuf.tuesday() == "1";
        calendar->week["wednesday"] = calendarBuf.wednesday() == "1";
        calendar->week["thursday"] = calendarBuf.thursday() == "1";
        calendar->week["friday"] = calendarBuf.friday() == "1";
        calendar->week["saturday"] = calendarBuf.saturday() == "1";
        calendar->week["sunday"] = calendarBuf.sunday() == "1";

        calendar->serviceId = calendarBuf.serviceid();
        calendar->startDate = parseDate(calendarBuf.startdate());
        calendar->endDate = parseDate(calendarBuf.enddate());
        return calendar;
    }

    std::list<StopTime> orderStopTimesBy(const std::list<StopTime> &stopTimes, const tm &t, bool byDeparture = true)
    {
        std::list<StopTime> stopTimesAt;
        for (auto iterator = stopTimes.begin(), end = stopTimes.end(); iterator != end; ++iterator)
        {
            StopTime stopTime = *iterator;
            stopTime.departure.tm_mday = t.tm_mday;
            stopTime.departure.tm_wday = t.tm_wday;
            stopTime.departure.tm_yday = t.tm_yday;
            stopTime.departure.tm_mon = t.tm_mon;
            stopTime.departure.tm_year = t.tm_year;
            stopTime.arrival.tm_mday = t.tm_mday;
            stopTime.arrival.tm_wday = t.tm_wday;
            stopTime.arrival.tm_yday = t.tm_yday;
            stopTime.arrival.tm_mon = t.tm_mon;
            stopTime.arrival.tm_year = t.tm_year;
            if(isSubwayTrip(stopTime.tripId))
            {
                stopTime.departure = t;
                stopTime.arrival = t;
            }
            else if(datetimeIsBeforeNotEq(byDeparture ? stopTime.departure : stopTime.arrival , t))
            {
                stopTime.departure = addDays(stopTime.departure, 1);
                stopTime.arrival = addDays(stopTime.arrival, 1);
            }
            stopTimesAt.push_back(stopTime);
        };

        stopTimesAt.sort([](const StopTime &a, const StopTime &b)
        {
            return datetimeIsBeforeEq(a.departure, b.departure);
        });

        return stopTimesAt;
    }

    Vertice getVerticeFromGraph(const tm *dateref, Graph *graph, Cache *cache, std::string id, bool byDeparture)
    {
        auto it = cache->vertices.find(id);
        if(it != cache->vertices.end())
        {
            Vertice vertice = *it->second;
            if(dateref != NULL)
            {
                vertice.stopTimes = orderStopTimesBy(vertice.stopTimes, *dateref, byDeparture);
            }
            return vertice;
        }
        else
        {
            std::shared_ptr<Vertice> vertice {new Vertice};
            auto verticeBuf = (*graph)[id];
            vertice->id = verticeBuf.id();
            vertice->name = verticeBuf.name();
            vertice->edges = parseEdges(verticeBuf.edges());
            if(dateref != NULL)
            {
                vertice->stopTimes = parseStopTimes(dateref, verticeBuf.stoptimes());
            }
            cache->vertices[id] = vertice;
            return *vertice;
        }
    }

    std::shared_ptr<Trip> parseTripRow(std::list< std::unordered_map<std::string, const void*> >::const_iterator it)
    {
        std::unordered_map<std::string, const void*> row = *it;
        std::shared_ptr<Trip> trip {new Trip};
        trip->id = (const char*)row["id"];
        trip->direction = (const char*)row["direction"];

        const char* stopIds = (const char*)row["stopIds"];
        if(stopIds != NULL)
        {
            m::cheminot::data::TripStopIds tripStopIdsBuf;
            tripStopIdsBuf.ParseFromString(stopIds);
            trip->stopIds = parseTripStopIds(tripStopIdsBuf);
        }

        const char *calendar = (const char*)row["calendar"];
        if(calendar != NULL)
        {
            m::cheminot::data::Calendar calendarBuf;
            calendarBuf.ParseFromString(calendar);
            trip->calendar = parseCalendar(calendarBuf);
        }
        return trip;
    }

    std::unordered_map< std::string, const void*> parseRow(sqlite3_stmt *stmt)
    {
        int cols = sqlite3_column_count(stmt);
        std::unordered_map<std::string, const void*> row;
        for(int col=0 ; col<cols; col++)
        {
            std::string name(sqlite3_column_name(stmt, col));
            const char *value = (const char *)sqlite3_column_text(stmt, col);
            if(value != NULL)
            {
                row[name] = strdup(value);
            }
        }
        return row;
    }

    void executeUpdate(sqlite3 *handle, std::string query)
    {
        sqlite3_stmt *stmt;
        sqlite3_exec(handle, query.c_str(), NULL, NULL, NULL);
    }

    std::list< std::unordered_map<std::string, const void*> > executeQuery(sqlite3 *handle, std::string query)
    {
        std::list< std::unordered_map <std::string, const void*> > results;
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(handle, query.c_str(),-1, &stmt, 0);
        int retval;
        while(1)
        {
            retval = sqlite3_step(stmt);
            if(retval == SQLITE_ROW)
            {
                results.push_back(parseRow(stmt));
            }
            else if(retval == SQLITE_DONE)
            {
                return results;
            }
            else
            {
                oops("Unexpected error while executing this SQL query: " + query);
            }
        }
    }

    sqlite3* openConnection(std::string path)
    {
        sqlite3 *handle;
        sqlite3_open_v2(path.c_str(), &handle, SQLITE_OPEN_READWRITE, NULL);
        return handle;
    }

    void parseGraph(std::string path, Graph *graph)
    {
        std::ifstream in(path);
        if(in.is_open())
        {
            m::cheminot::data::Graph graphBuf;
            graphBuf.ParseFromIstream(&in);
            *graph = *graphBuf.mutable_vertices();
            in.close();
        }
        else
        {
            throw std::runtime_error("Unexpected error while reading: " + path);
        }
    }

    void parseCalendarDates(std::string path, CalendarDates *calendarDates)
    {
        std::ifstream in(path);
        if(in.is_open())
        {
            m::cheminot::data::CalendarDates calendarDatesBuf;
            calendarDatesBuf.ParseFromIstream(&in);
            *calendarDates = calendarDatesBuf.exceptionsbyserviceid();
            in.close();
        }
        else
        {
            throw std::runtime_error("Unexpected error while reading: " + path);
        }
    }

    void lock(sqlite3 *handle)
    {
        std::string query = "UPDATE META SET VALUE = 1 WHERE key = 'aborted'";
        executeUpdate(handle, query);
    }

    void unlock(sqlite3 *handle)
    {
        std::string query = "UPDATE META SET VALUE = 0 WHERE key = 'aborted'";
        executeUpdate(handle, query);
    }

    bool isLocked(sqlite3 *handle, bool *locked)
    {
        std::string query = "SELECT value FROM META WHERE key = 'aborted'";
        std::list< std::unordered_map<std::string, const void*> > results = executeQuery(handle, query);
        bool x = false;
        if(!results.empty())
        {
            x = strncmp((char *)results.front()["value"], "1", 1) == 0;
        }
        if(locked != NULL)
        {
            *locked = x;
        }
        return x;
    }

    std::string getVersion(sqlite3 *handle)
    {
        std::string query = "SELECT value FROM META WHERE key = 'version'";
        std::list< std::unordered_map<std::string, const void*> > results = executeQuery(handle, query);
        return (char *)results.front()["value"];
    }

    tm getCreationDate(sqlite3 *handle)
    {
        std::string query = "SELECT value FROM META WHERE key = 'createdAt'";
        std::list< std::unordered_map<std::string, const void*> > results = executeQuery(handle, query);
        std::string date = (char *)results.front()["value"];
        return parseDate(date);
    }

    Json::Value getMeta(sqlite3 *handle)
    {
        std::string version = getVersion(handle);
        int createdAt = asTimestamp(getCreationDate(handle));
        Json::Value json;
        json["version"] = version;
        json["createdAt"] = createdAt;
        return json;
    }

    std::string parisStopIdsQuery()
    {
        std::string subQueryParisIds = "";
        for (auto iterator = parisStopIds.begin(), end = parisStopIds.end(); iterator != end; ++iterator)
        {
            std::string stopId = *iterator;
            if(subQueryParisIds == "")
            {
                subQueryParisIds += " b.stopId = '" + stopId + "'";
            }
            else
            {
                subQueryParisIds += " OR b.stopId = '" + stopId + "'";
            }
        }
        return subQueryParisIds;
    }

    std::list<std::shared_ptr<Trip>> getDirectTrips(sqlite3 *handle, Cache *cache, std::string vsId, std::string veId)
    {
        std::string vsIdQuery = isParis(vsId) ? parisStopIdsQuery() : "b.stopId = '" + vsId + "'";
        std::string veIdQuery = isParis(veId) ? parisStopIdsQuery() : "b.stopId = '" + veId + "'";
        std::string query = "SELECT a.* FROM TRIPS "
                            "a INNER JOIN TRIPS_STOPS b ON a.id = b.tripId "
                            "WHERE " + vsIdQuery + " OR " + veIdQuery + " GROUP BY b.tripId HAVING COUNT(*) = 2";
        std::list<std::shared_ptr<Trip>> trips;
        auto results = executeQuery(handle, query);
        for (auto iterator = results.begin(), end = results.end(); iterator != end; ++iterator)
        {
            std::shared_ptr<Trip> trip = parseTripRow(iterator);
            if(cache->trips.find(trip->id) == cache->trips.end())
            {
                cache->trips[trip->id] = trip;
            }
            trips.push_back(trip);
        }
        return trips;
    }

    std::list<std::shared_ptr<Trip>> getTripsByIds(sqlite3 *handle, Cache *cache, std::list<std::string> ids)
    {
        std::list<std::shared_ptr<Trip>> results;
        std::list<std::string> toCache;
        for (auto iterator = ids.begin(), end = ids.end(); iterator != end; ++iterator)
        {
            std::string id = *iterator;
            auto it = cache->trips.find(id);
            if(it != cache->trips.end())
            {
                results.push_back(it->second);
            }
            else
            {
                toCache.push_back(id);
            }
        }

        if(!toCache.empty())
        {
            std::string values = "";
            for (auto iterator = toCache.begin(), end = toCache.end(); iterator != end; ++iterator)
            {
                std::string id = *iterator;
                std::string quoted = "'" + id + "'";
                values = (values == "") ? quoted : (values + ", " + quoted);
            }

            std::string query = "SELECT * FROM TRIPS WHERE id IN (" + values + ")";
            auto fromSqlite = executeQuery(handle, query);
            for (auto iterator = fromSqlite.begin(), end = fromSqlite.end(); iterator != end; ++iterator)
            {
                std::shared_ptr<Trip> trip = parseTripRow(iterator);
                cache->trips[trip->id] = trip;
                results.push_back(trip);
            }
        }

        return results;
    }

    class CompareArrivalTime
    {
    public:
        bool operator()(const ArrivalTime& gi, const ArrivalTime& gj)
        {
            return datetimeIsBeforeEq(gj.departure, gi.departure);
        }
    };

    bool isTripRemovedOn(Cache *cache, std::shared_ptr<Trip> trip, CalendarDates *calendarDates, const tm &when)
    {
        auto exceptions = getCalendarDatesByServiceId(cache, calendarDates, trip->calendar->serviceId);
        auto it = std::find_if(exceptions.begin(), exceptions.end(), [&when](std::shared_ptr<CalendarDate> calendarDate)
        {
            return hasSameDate(calendarDate->date, when) && (calendarDate->exceptionType == 2);
        });
        return it != exceptions.end();
    }

    bool isTripAddedOn(Cache *cache, std::shared_ptr<Trip> trip, CalendarDates *calendarDates, const tm &when)
    {
        auto exceptions = getCalendarDatesByServiceId(cache, calendarDates, trip->calendar->serviceId);
        auto it = std::find_if(exceptions.begin(), exceptions.end(), [&when](std::shared_ptr<CalendarDate> exception)
        {
            return hasSameDate(exception->date, when) && (exception->exceptionType == 1);
        });
        return it != exceptions.end();
    }

    static std::unordered_map<int, std::string> week { {1, "monday"}, {2, "tuesday"}, {3, "wednesday"}, {4, "thursday"}, {5, "friday"}, {6, "saturday"}, {0, "sunday"}};

    bool isTripValidToday(std::shared_ptr<Trip> trip, const tm &when)
    {
        return trip->calendar->week[week[when.tm_wday]];
    }

    bool isTripInPeriod(std::shared_ptr<Trip> trip, const tm &when)
    {
        tm startDate = trip->calendar->startDate;
        tm endDate = trip->calendar->endDate;
        bool before = dateIsBeforeEq(startDate, when);
        bool after = dateIsBeforeEq(when, endDate);
        return before && after;
    }

    bool isTripValidOn(Cache *cache, std::shared_ptr<Trip> trip, CalendarDates *calendarDates, const tm &when)
    {
        if(trip->calendar != NULL)
        {
            bool removed = isTripRemovedOn(cache, trip, calendarDates, when);
            bool added = isTripAddedOn(cache, trip, calendarDates, when);
            bool availableToday = isTripValidToday(trip, when);
            bool inPeriod = isTripInPeriod(trip, when);
            return (!removed && inPeriod && availableToday) || added;
        }
        else
        {
            return true;
        }
        return false;
    }

    std::unordered_map<std::string, bool> tripsAvailability(sqlite3 *handle, Cache *cache, std::list<std::string> ids, CalendarDates *calendarDates, const tm &when)
    {
        std::unordered_map<std::string, bool> availablities;
        std::list<std::shared_ptr<Trip>> trips = getTripsByIds(handle, cache, ids);
        for (auto iterator = trips.begin(), end = trips.end(); iterator != end; ++iterator)
        {
            std::shared_ptr<Trip> trip = *iterator;
            availablities[trip->id] = isTripValidOn(cache, trip, calendarDates, when);
        }
        return availablities;
    }

    std::list<ArrivalTime> orderArrivalTimesBy(std::list<ArrivalTime> arrivalTimes, const tm &t)
    {
        std::list<ArrivalTime> arrivalTimesAt;
        for (auto iterator = arrivalTimes.begin(), end = arrivalTimes.end(); iterator != end; ++iterator)
        {
            ArrivalTime arrivalTime = *iterator;
            arrivalTime.departure.tm_mday = t.tm_mday;
            arrivalTime.departure.tm_wday = t.tm_wday;
            arrivalTime.departure.tm_yday = t.tm_yday;
            arrivalTime.departure.tm_mon = t.tm_mon;
            arrivalTime.departure.tm_year = t.tm_year;
            arrivalTime.arrival.tm_mday = t.tm_mday;
            arrivalTime.arrival.tm_wday = t.tm_wday;
            arrivalTime.arrival.tm_yday = t.tm_yday;
            arrivalTime.arrival.tm_mon = t.tm_mon;
            arrivalTime.arrival.tm_year = t.tm_year;

            if(datetimeIsBeforeNotEq(arrivalTime.departure, t))
            {
                arrivalTime.departure = addDays(arrivalTime.departure, 1);
                arrivalTime.arrival = addDays(arrivalTime.arrival, 1);
            }
            arrivalTimesAt.push_back(arrivalTime);
        };

        arrivalTimesAt.sort([](const ArrivalTime &a, const ArrivalTime &b)
        {
            return datetimeIsBeforeEq(a.arrival, b.arrival);
        });

        return arrivalTimesAt;
    }

    std::list<StopTime> getAvailableDepartures(sqlite3 *handle, Cache *cache, CalendarDates *calendarDates, tm arrivalTime, const Vertice *vi)
    {
        std::list<StopTime> departures(vi->stopTimes);
        departures.remove_if([&arrivalTime] (const StopTime &stopTime)
        {
            return !(isSubwayTrip(stopTime.tripId) || (datetimeIsBeforeEq(arrivalTime, stopTime.departure) && !isTerminus(stopTime)));
        });

        std::list<std::string> tripIds;
        for (auto iterator = departures.begin(), end = departures.end(); iterator != end; ++iterator)
        {
            if(!isSubwayTrip(iterator->tripId))
            {
                tripIds.push_back(iterator->tripId);
            }
        }

        std::unordered_map<std::string, bool> availablities = tripsAvailability(handle, cache, tripIds, calendarDates, arrivalTime);
        departures.remove_if([&availablities, &arrivalTime] (StopTime& stopTime)
        {
            return !(isSubwayTrip(stopTime.tripId) || availablities[stopTime.tripId]);
        });

        departures.sort([](const StopTime &a, const StopTime &b)
        {
            return datetimeIsBeforeEq(a.departure, b.departure);
        });

        return departures;
    }

    struct QueueItem
    {
        std::string stopId;
        std::string tripId;
        tm ti;
        tm gi;
    };

    class CompareQueueItem
    {
    public:
        bool operator()(const std::shared_ptr<QueueItem> &itemA, const std::shared_ptr<QueueItem> &itemB)
        {
            return datetimeIsBeforeEq(itemB->gi, itemA->gi);
        }
    };

    typedef std::priority_queue<std::shared_ptr<QueueItem>, std::vector<std::shared_ptr<QueueItem>>, CompareQueueItem> Queue;

    static tm INFINITE = {
        INT_MAX,
        INT_MAX,
        INT_MAX,
        INT_MAX,
        INT_MAX,
        INT_MAX,
        INT_MAX,
        INT_MAX,
        INT_MAX
    };

    std::unordered_map<std::string, std::shared_ptr<QueueItem>> initTimeRefinement(sqlite3 *handle, Graph *graph, ArrivalTimesFunc *arrivalTimesFunc, CalendarDates *calendarDates, Queue *queue, const Vertice *vs, tm ts, std::list<tm> startingPeriod)
    {

        std::unordered_map<std::string, std::shared_ptr<QueueItem>> items;

        ArrivalTimeFunc gsFunc;
        for (auto iterator = startingPeriod.begin(), end = startingPeriod.end(); iterator != end; ++iterator)
        {
            tm departureTime = *iterator;
            ArrivalTime gs;
            gs.stopId = vs->id;
            gs.departure = departureTime;
            gs.arrival = departureTime;
            gsFunc[asTimestamp(departureTime)] = gs;
        }

        (*arrivalTimesFunc)[vs->id] = gsFunc;

        std::shared_ptr<QueueItem> qs {new QueueItem};
        qs->stopId = vs->id;
        qs->gi = ts;
        qs->tripId = "<trip>";
        qs->ti = ts;
        queue->push(qs);
        items[vs->id] = qs;

        for(auto iterator = graph->begin(), end = graph->end(); iterator != end; ++iterator)
        {
            std::string stopId = iterator->first;
            if(stopId != vs->id)
            {
                std::shared_ptr<QueueItem> qi {new QueueItem};
                qi->stopId = stopId;
                qi->gi = INFINITE;
                qi->tripId = "<trip>";
                qi->ti = ts;
                queue->push(qi);
                items[stopId] = qi;
            }
        }
        return items;
    }

    tm enlargeStartingTime(sqlite3 *handle, Cache *cache, CalendarDates *calendarDates, Graph *graph, ArrivalTimeFunc &giFunc, std::shared_ptr<QueueItem> qi, std::shared_ptr<QueueItem> qk, const Vertice *vi, std::string vsId, const tm &ts, const tm &te)
    {
        tm t = minusHours(qk->gi, 2);
        if(qi->stopId == vsId)
        {
            return te;
        }
        else
        {
            time_t wfi = LONG_MAX;
            for (auto iterator = vi->edges.begin(), end = vi->edges.end(); iterator != end; ++iterator)
            {
                std::string edge = *iterator;
                Vertice vf = getVerticeFromGraph(&ts, graph, cache, edge);
                std::list<StopTime> vfDepartureTimes = getAvailableDepartures(handle, cache, calendarDates, t, &vf);
                for (auto iterator = vfDepartureTimes.begin(), end = vfDepartureTimes.end(); iterator != end; ++iterator)
                {
                    StopTime vfDepartureTime = *iterator;
                    for (auto iterator = vi->stopTimes.begin(), end = vi->stopTimes.end(); iterator != end; ++iterator)
                    {
                        StopTime viArrivalTime = *iterator;
                        if((vfDepartureTime.tripId == viArrivalTime.tripId) && (vfDepartureTime.pos == (viArrivalTime.pos - 1)) && datetimeIsBeforeNotEq(vfDepartureTime.departure, viArrivalTime.arrival))
                        {
                            time_t travelTime = difftime(asTimestamp(viArrivalTime.arrival), asTimestamp(vfDepartureTime.departure));
                            wfi = travelTime < wfi ? travelTime : wfi;
                        }
                    }
                }
            }

            if(wfi == LONG_MAX)
            {
                oops("Unable to compute travel time between vf to vi");
            }

            tm nextEarliestArrivalTime = asDateTime(asTimestamp(qk->gi) + wfi);

            std::pair<tm, tm> enlarged = { qi->ti, qi->gi };
            for (auto iterator = giFunc.begin(), end = giFunc.end(); iterator != end; ++iterator)
            {
                time_t ti = iterator->first;
                tm gi = iterator->second.arrival;
                if(datetimeIsBeforeEq(gi, nextEarliestArrivalTime) && datetimeIsBeforeEq(enlarged.second, gi))
                {
                    enlarged = { asDateTime(ti), gi };
                }
            }

            if(hasSameDateTime(enlarged.second, qi->gi))
            {
                auto last = std::prev(giFunc.end());
                return asDateTime(last->first);
            }

            return enlarged.first;
        }
    }

    ArrivalTime stopTime2ArrivalTime(std::string stopId, const StopTime *stopTime)
    {
        ArrivalTime arrivalTime;
        arrivalTime.stopId = stopId;
        arrivalTime.arrival = stopTime->arrival;
        arrivalTime.departure = stopTime->departure;
        arrivalTime.tripId = stopTime->tripId;
        arrivalTime.pos = stopTime->pos;
        return arrivalTime;
    }

    std::list<tm> getStartingPeriod(sqlite3 *handle, Cache *cache, CalendarDates *calendarDates, const Vertice *vs, tm ts, tm te, int max)
    {
        auto departures = getAvailableDepartures(handle, cache, calendarDates, ts, vs);

        std::list<tm> startingPeriod;
        for (auto iterator = departures.begin(), end = departures.end(); iterator != end; ++iterator)
        {
            StopTime departureTime = *iterator;
            if(datetimeIsBeforeEq(departureTime.departure, te))
            {
                if(startingPeriod.size() < max)
                {
                    startingPeriod.push_back(departureTime.departure);
                }
                else
                {
                    break;
                }
            }
        }

        return startingPeriod;
    }

    bool isQueueItemOutdated(std::unordered_map<std::string, tm> *uptodate, std::shared_ptr<QueueItem> item)
    {
        auto last = uptodate->find(item->stopId);
        if(last != uptodate->end())
        {
            return !datetimeIsBeforeEq(item->gi, last->second);
        }
        else
        {
            return false;
        }
    }

    StopTime getEarliestArrivalTime(sqlite3 *handle, Cache *cache, CalendarDates *calendarDates, const Vertice *vi, const Vertice *vj, ArrivalTime *gi)
    {
        std::list<StopTime> viDepartures = getAvailableDepartures(handle, cache, calendarDates, gi->arrival, vi);
        StopTime earliestArrivalTime;
        earliestArrivalTime.arrival = INFINITE;
        for(auto iterator = vj->stopTimes.begin(), end = vj->stopTimes.end(); iterator != end; ++iterator)
        {
            StopTime vjStopTime = *iterator;
            for(auto iterator = viDepartures.begin(), end = viDepartures.end(); iterator != end; ++iterator)
            {
                StopTime viDepartureTime = *iterator;
                if(viDepartureTime.tripId == vjStopTime.tripId && datetimeIsBeforeEq(viDepartureTime.departure, vjStopTime.arrival) && datetimeIsBeforeEq(gi->arrival, viDepartureTime.departure))
                {
                    if((viDepartureTime.pos == vjStopTime.pos - 1) && datetimeIsBeforeNotEq(vjStopTime.arrival, earliestArrivalTime.arrival))
                    {
                        return vjStopTime;
                    }
                }
            }
        }
        return earliestArrivalTime;
    }

    void updateArrivalTimeFunc(sqlite3 *handle, Graph *graph, Cache *cache, CalendarDates *calendarDates, ArrivalTimesFunc *arrivalTimesFunc, const Vertice *vi, ArrivalTime *gi, std::string vjId, const tm &startingTime, std::function<void(StopTime)> done)
    {
        Vertice vj = getVerticeFromGraph(&gi->arrival, graph, cache, vjId, false);
        StopTime vjStopTime = getEarliestArrivalTime(handle, cache, calendarDates, vi, &vj, gi);
        if(!hasSameDateTime(vjStopTime.arrival, INFINITE))   // MAYBE TODAY, ONE EDGE ISN'T AVAILABLE
        {
            ArrivalTimeFunc gjFunc;
            time_t t = asTimestamp(startingTime);
            auto it = arrivalTimesFunc->find(vjId);
            if(it != arrivalTimesFunc->end())
            {
                gjFunc = it->second;
                auto currentGj = gjFunc.find(t);
                if(currentGj != gjFunc.end())
                {
                    if(datetimeIsBeforeNotEq(vjStopTime.arrival, currentGj->second.arrival))   // UPDATING IF BETTER FOUND
                    {
                        gjFunc[t] = stopTime2ArrivalTime(vjId, &vjStopTime);
                        done(vjStopTime);
                    }
                }
                else
                {
                    gjFunc[t] = stopTime2ArrivalTime(vjId, &vjStopTime); // NEW VALUE
                    done(vjStopTime);
                }
            }
            else
            {
                gjFunc[t] = stopTime2ArrivalTime(vjId, &vjStopTime); // NEW FUNC
                (*arrivalTimesFunc)[vjId] = gjFunc;
                done(vjStopTime);
            }
            (*arrivalTimesFunc)[vjId] = gjFunc;
        }
    }

    std::tuple<bool, ArrivalTimesFunc, std::string> refineArrivalTimes(sqlite3 *handle, Graph *graph, Cache *cache, CalendarDates *calendarDates, std::string vsId, std::string veId, tm ts, tm te, int max)
    {
        Queue queue;
        ArrivalTimesFunc arrivalTimesFunc;
        std::unordered_map<std::string, tm> uptodate;
        Vertice vs = getVerticeFromGraph(&ts, graph, cache, vsId);
        std::list<tm> startingPeriod = getStartingPeriod(handle, cache, calendarDates, &vs, ts, te, max);
        if(startingPeriod.empty())
        {
            return std::make_tuple(false, arrivalTimesFunc, veId);
        }

        ts = *startingPeriod.begin();
        te = *(std::prev(startingPeriod.end()));

        std::unordered_map<std::string, std::shared_ptr<QueueItem>> items = initTimeRefinement(handle, graph, &arrivalTimesFunc, calendarDates, &queue, &vs, ts, startingPeriod);

        bool locked = false;
        while(!isLocked(handle, &locked) && queue.size() >= 2)
        {
            std::shared_ptr<QueueItem> qi = queue.top();
            Vertice vi = getVerticeFromGraph(&qi->gi, graph, cache, qi->stopId);
            queue.pop();

            if(!isQueueItemOutdated(&uptodate, qi))
            {
                std::shared_ptr<QueueItem> qk = queue.top();
                ArrivalTimeFunc giFunc = arrivalTimesFunc[vi.id];
                tm enlargedStartingTime = ts;
                if(!hasSameDateTime(ts, te))
                {
                    enlargedStartingTime = enlargeStartingTime(handle, cache, calendarDates, graph, giFunc, qi, qk, &vi, vsId, ts, te);
                }
                for (auto iterator = vi.edges.begin(), end = vi.edges.end(); iterator != end; ++iterator)
                {
                    std::string vjId = *iterator;
                    if(vjId != vsId)
                    {
                        for (auto iterator = startingPeriod.begin(), end = startingPeriod.end(); iterator != end; ++iterator)
                        {
                            tm startingTime = *iterator;
                            if(datetimeIsBeforeEq(qi->ti, startingTime) && datetimeIsBeforeEq(startingTime, enlargedStartingTime))
                            {
                                ArrivalTime gi = giFunc[asTimestamp(startingTime)];
                                updateArrivalTimeFunc(handle, graph, cache, calendarDates, &arrivalTimesFunc, &vi, &gi, vjId, startingTime, [&vjId, &queue, &uptodate, &items, &startingTime](StopTime vjStopTime)
                                {
                                    std::shared_ptr<QueueItem> updatedQj {new QueueItem};
                                    updatedQj->stopId = vjId;
                                    updatedQj->ti = items[vjId]->ti;
                                    updatedQj->gi = vjStopTime.arrival;
                                    updatedQj->tripId = vjStopTime.tripId;
                                    queue.push(updatedQj);
                                    uptodate[vjId] = vjStopTime.arrival;
                                });
                            }
                        }
                    }
                }

                if(datetimeIsBeforeEq(te, enlargedStartingTime))
                {
                    if(vi.id == veId || (isParis(veId) && isParis(vi.id)))
                    {
                        return std::make_tuple(false, arrivalTimesFunc, vi.id);
                    }
                }
                else
                {
                    qi->ti = enlargedStartingTime;
                    qi->gi = giFunc[asTimestamp(enlargedStartingTime)].arrival;
                    queue.push(qi);
                }
            }
        };

        return std::make_tuple(locked, arrivalTimesFunc, veId);
    }

    time_t getOptimalStartingTime(ArrivalTimeFunc *geFunc, std::string veId)
    {
        std::pair<time_t, cheminotc::ArrivalTime> best = *(geFunc->begin());
        for (auto iterator = std::next(geFunc->begin()), end = geFunc->end(); iterator != end; ++iterator)
        {
            std::pair<time_t, ArrivalTime> point = *iterator;
            if(datetimeIsBeforeEq(point.second.arrival, best.second.arrival))
            {
                best = point;
            }
        }
        return best.first;
    }

    std::list<ArrivalTime> pathSelection(Graph *graph, Cache *cache, ArrivalTimesFunc *arrivalTimesFunc, const tm &ts, std::string vsId, std::string veId)
    {
        std::list<ArrivalTime> path;
        if(arrivalTimesFunc->empty())
        {
            return path;
        }

        ArrivalTimeFunc geFunc = (*arrivalTimesFunc)[veId];
        time_t optimalStartingTime = getOptimalStartingTime(&geFunc, veId);
        Vertice vj = getVerticeFromGraph(&ts, graph, cache, veId);

        std::function<bool (std::string, ArrivalTime*)> getArrivalTimeAt = [&arrivalTimesFunc, &optimalStartingTime](std::string viId, ArrivalTime *arrivalTime)
        {
            auto it = arrivalTimesFunc->find(viId);
            if(it != arrivalTimesFunc->end())
            {
                ArrivalTimeFunc arrivalTimeFunc = (*arrivalTimesFunc)[viId];
                auto it = arrivalTimeFunc.find(optimalStartingTime);
                if(it != arrivalTimeFunc.end())
                {
                    *arrivalTime = it->second;
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        };

        ArrivalTime ge;
        if(getArrivalTimeAt(veId, &ge))
        {
            path.push_back(ge);
        }

        std::function<bool (std::string, std::string)> running = [](std::string vjId, std::string vsId)
        {
            return !(vjId == vsId || (vsId == parisStopId && isParis(vjId)));
        };

        while(running(vj.id, vsId))
        {
            ArrivalTime gj;
            if(getArrivalTimeAt(vj.id, &gj))
            {
                for (auto iterator = vj.edges.begin(), end = vj.edges.end(); iterator != end; ++iterator)
                {
                    std::string viId = *iterator;
                    ArrivalTime gi;
                    if(getArrivalTimeAt(viId, &gi))
                    {
                        Vertice vi = getVerticeFromGraph(&ts, graph, cache, viId);
                        auto it = std::find_if(vi.stopTimes.begin(), vi.stopTimes.end(), [&gj](const StopTime &viStopTime)
                        {
                            return (viStopTime.tripId == gj.tripId) && (viStopTime.pos == (gj.pos - 1));
                        });
                        if(it != vi.stopTimes.end())
                        {
                            if(viId == vsId || (vsId == parisStopId && isParis(viId)))   // RECOVER DEPARTURE
                            {
                                gi.tripId = gj.tripId;
                                gi.departure = it->departure;
                                gi.arrival = it->arrival;
                                if(dateIsBeforeNotEq(gi.departure, gj.arrival) && timeIsBeforeNotEq(gi.departure, gj.arrival))
                                {
                                    gi.departure.tm_mday = gj.arrival.tm_mday;
                                }
                                if(dateIsBeforeNotEq(gi.arrival, gi.departure) && timeIsBeforeEq(gi.departure, gj.arrival))
                                {
                                    gi.arrival.tm_mday = gi.departure.tm_mday;
                                }
                            }
                            path.push_front(gi);
                            vj = getVerticeFromGraph(&ts, graph, cache, gi.stopId);
                            break;
                        }
                    }
                }
            }
        };

        return path;
    }

    std::pair<bool, std::list<ArrivalTime>> lookForBestDirectTrip(sqlite3 *handle, Graph *graph, Cache *cache, CalendarDates *calendarDates, std::string vsId, std::string veId, tm ts, tm te)
    {
        Vertice vs = getVerticeFromGraph(&ts, graph, cache, vsId);
        Vertice ve = getVerticeFromGraph(&ts, graph, cache, veId);
        std::list<std::shared_ptr<Trip>> trips = getDirectTrips(handle, cache, vsId, veId);
        std::pair<std::shared_ptr<Trip>, tm> bestTrip;
        bool hasBestTrip = false;
        for(auto iterator = trips.begin(), end = trips.end(); iterator != end; ++iterator)
        {
            std::shared_ptr<Trip> trip = *iterator;

            auto veIt = std::find_if(ve.stopTimes.begin(), ve.stopTimes.end(), [&trip](const StopTime &stopTime)
            {
                return stopTime.tripId == trip->id;
            });

            auto vsIt = std::find_if(vs.stopTimes.begin(), vs.stopTimes.end(), [&trip](const StopTime &stopTime)
            {
                return stopTime.tripId == trip->id;
            });

            if(vsIt != vs.stopTimes.end() && veIt != ve.stopTimes.end())
            {
                StopTime stopTimeVs = *vsIt;
                StopTime stopTimeVe = *veIt;
                tm departureVs = stopTimeVs.departure;
                tm arrivalVe = stopTimeVe.arrival;
                if(isTripValidOn(cache, trip, calendarDates, departureVs))
                {
                    if(stopTimeVs.pos < stopTimeVe.pos && datetimeIsBeforeEq(ts, departureVs) && datetimeIsBeforeEq(departureVs, te))
                    {
                        if(!hasBestTrip || datetimeIsBeforeNotEq(arrivalVe, bestTrip.second))
                        {
                            bestTrip = {trip, arrivalVe};
                            hasBestTrip = true;
                        }
                    }
                }
            }
        }

        std::function<bool (std::string, std::shared_ptr<Trip>, ArrivalTime*)> getArrivalTime = [&ts, &graph, &cache](std::string viId, std::shared_ptr<Trip> trip, ArrivalTime *arrivalTime)
        {
            Vertice vi = getVerticeFromGraph(&ts, graph, cache, viId);
            auto it = std::find_if(vi.stopTimes.begin(), vi.stopTimes.end(), [&trip](const StopTime &stopTime)
            {
                return stopTime.tripId == trip->id;
            });
            if(it != vi.stopTimes.end())
            {
                StopTime stopTime = *it;
                *arrivalTime = stopTime2ArrivalTime(viId, &stopTime);
                return true;
            }
            return false;
        };

        std::list<ArrivalTime> arrivalTimes;

        if(hasBestTrip)
        {
            std::shared_ptr<Trip> trip = bestTrip.first;
            for(auto iterator = trip->stopIds.begin(), end = trip->stopIds.end(); iterator != end; ++iterator)
            {
                std::string stopId = *iterator;
                if(arrivalTimes.empty())
                {
                    if(stopId == vsId || (vsId == parisStopId && isParis(stopId)))
                    {
                        ArrivalTime arrivalTime;
                        if(getArrivalTime(stopId, trip, &arrivalTime))
                        {
                            arrivalTimes.push_back(arrivalTime);
                        }
                    }
                }
                else
                {
                    ArrivalTime arrivalTime;
                    if(getArrivalTime(stopId, trip, &arrivalTime))
                    {
                        arrivalTimes.push_back(arrivalTime);
                    }
                    if(stopId == veId || (veId == parisStopId && isParis(stopId)))
                    {
                        break;
                    }
                }
            }
        }

        return { trips.size() > 0, orderArrivalTimesBy(arrivalTimes, ts) };
    }

    std::pair<bool, std::list<ArrivalTime>> lookForBestTrip(sqlite3 *handle, Graph *graph, Cache *cache, CalendarDates *calendarDates, std::string vsId, std::string veId, tm ts, tm te, int max)
    {
        auto result = refineArrivalTimes(handle, graph, cache, calendarDates, vsId, veId, ts, te, max);
        ArrivalTimesFunc arrivalTimes = std::get<1>(result);
        bool locked = std::get<0>(result);
        veId = std::get<2>(result);
        if(locked)
        {
            return { locked, {} };
        }
        else
        {
            return { false, pathSelection(graph, cache,  &arrivalTimes, ts, vsId, veId) };
        }
    }
}

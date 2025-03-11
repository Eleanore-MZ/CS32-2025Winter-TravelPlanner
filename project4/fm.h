#ifndef FM_INCLUDED
#define FM_INCLUDED

#include "provided.h"
#include "bstset.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>



class FlightManager:public FlightManagerBase
{
private:
	struct FlightSegmentWrapper
	{
		FlightSegment m_fs;
		FlightSegmentWrapper(FlightSegment fs) : m_fs(fs) {}
		bool operator< (const FlightSegmentWrapper& other) const
			{ return m_fs.departure_time < other.m_fs.departure_time; }
		bool operator==(const FlightSegmentWrapper& other) const
		{ return m_fs.departure_time == other.m_fs.departure_time; }
	};

	std::unordered_map<std::string, BSTSet<FlightSegmentWrapper>> m_flightMap;
	//departure air port->a set of flightSegment departing from that airport, sorted by departure time

	
public:
	FlightManager() {}
	~FlightManager() {}
	bool load_flight_data(std::string filename);
	std::vector<FlightSegment> find_flights
	(std::string source_airport, int start_time, int end_time) const;

};

bool FlightManager::load_flight_data(std::string filename)
{
	std::ifstream flightFile(filename);
	if (!flightFile) {
		std::cerr << "Error: Unable to open file " << filename << std::endl;
		return false;
	}
	std::string line;
	while (std::getline(flightFile, line))
	{
		std::stringstream ss(line);
		std::string airline, departAirport, arriveAirport;
		int flightNum, departureTime, arrivalTime, duration;

		if (!(std::getline(ss, airline, ',') &&
			(ss >> flightNum) && ss.ignore() &&
			std::getline(ss, departAirport, ',') &&
			std::getline(ss, arriveAirport, ',') &&
			(ss >> departureTime) && ss.ignore() &&
			(ss >> arrivalTime) && ss.ignore() &&
			(ss >> duration)))
		{
			std::cerr << "Error: Invalid line format: " << line << std::endl;
			return false;
		}

		m_flightMap[departAirport].insert(FlightSegmentWrapper(FlightSegment(airline, flightNum, departAirport, arriveAirport, departureTime, duration)));
	}
	// std::cerr << "Load Successful!" << std::endl;
	flightFile.close();
	return true;
}

std::vector<FlightSegment> FlightManager::find_flights
(std::string source_airport, int start_time, int end_time) const
{
	auto airport = m_flightMap.find(source_airport);
	std::vector<FlightSegment> flights;
	if (airport != m_flightMap.end())
	{
		auto& flightSet = airport->second;
		BSTSet<FlightSegmentWrapper>::SetIterator it 
			= flightSet.find_first_not_smaller(FlightSegmentWrapper(FlightSegment("", 0, "", "", start_time, 0)));
		const FlightSegmentWrapper* p;
		while ((p = it.get_and_advance()) != nullptr && p->m_fs.departure_time < end_time)
		{
			flights.push_back(p->m_fs);
		}
	}
	return flights;
}

#endif
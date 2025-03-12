#ifndef TP_INCLUDED
#define TP_INCLUDED

#include "provided.h"
#include "fm.h"
#include <queue>
#include <unordered_map>
#include <vector>
#include <set>

using namespace std;

class TravelPlanner :public TravelPlannerBase
{
private:
	set<string> preferred; // preferred airlines if any

	struct Node
	{
		string airport;
		int arrival_time;
		int total_time;
		Node(string source_airport, int arrivalTime, int totalTime)
			:airport(source_airport), arrival_time(arrivalTime),total_time(totalTime) {}
	};

	struct NodeComparator
	{
		bool operator()(const Node* lhs, const Node* rhs) const
		{
			return lhs->arrival_time > rhs->arrival_time;
		}
	};

public:
	TravelPlanner(const FlightManagerBase& flight_manager, const AirportDB& airport_db)
		:TravelPlannerBase(flight_manager, airport_db) {}
	~TravelPlanner() {}
	void add_preferred_airline(string airline);
	bool plan_travel(string source_airport, string
		destination_airport, int start_time, Itinerary& itinerary) const;
	vector<FlightSegment>getPossibleFlights(string currentAirport, int currentTime) const; // move back to private
};


void TravelPlanner::add_preferred_airline(string airline)
{
	preferred.insert(airline);
}

bool TravelPlanner::plan_travel(string source_airport, string
	destination_airport, int start_time, Itinerary& itinerary) const
{
	// Clear the itinerary
    itinerary.source_airport = source_airport;
    itinerary.destination_airport = destination_airport;
    itinerary.flights.clear();
    itinerary.total_duration = 0;
	
	priority_queue<Node*, vector<Node*>, NodeComparator> pq; // open set sorted by arrival time
	unordered_map<string, int> earliestArrival; // Keeps track of the earliest arrival time at each airport
	unordered_map<string, FlightSegment*> previousFlights; // Keeps track of the previous flight segment to each airport
	vector<Node*> nodesToDelete;  // For memory management
	vector<FlightSegment*> flightSegmentsToDelete; // For memory management

	pq.push(new Node(source_airport, start_time, 0));
	earliestArrival[source_airport] = start_time;

	while (!pq.empty())
	{
		Node* curr = pq.top();
		cerr << "Got to " << curr->airport << endl;
		pq.pop();
		nodesToDelete.push_back(curr);

		// If we already found the destination airport, we can stop
		if (curr->airport == destination_airport)
		{
			cerr << "Arrived at " << curr->airport << endl;
			itinerary.source_airport = source_airport;
			itinerary.destination_airport = destination_airport;

			// Path reconstruction
			vector<FlightSegment> path;
			string current = destination_airport;
			while (previousFlights.find(current) != previousFlights.end())
			{
				const FlightSegment* segment = previousFlights[current];
				path.push_back(*segment); // Dereference pointer to store the actual FlightSegment
				cerr << "Pushed flight segment" << segment->flight_no << " from " << segment->source_airport << " to " << segment->destination_airport << endl;
				current = segment->source_airport; // Move to the previous airport
				if (current == source_airport) // Stop if we have reached the starting airport
					break;
			}

			if (path.empty() || current != source_airport)
			{
				for (auto node : nodesToDelete) delete node;
				for(auto fs: flightSegmentsToDelete) delete fs;
				return false;
			}

			reverse(path.begin(), path.end());
			itinerary.flights = path;
			itinerary.total_duration = curr->arrival_time - start_time;
			for (Node* node : nodesToDelete) 
			{
				delete node;
			}
			return true;
		}

		// for each possible flights from the current airport
		vector<FlightSegment> possibleFlights = getPossibleFlights(curr->airport, curr->arrival_time);
		if (possibleFlights.empty())
			continue;
		for (const auto& it : possibleFlights)
		{
			cerr << "Find flight " << it.flight_no << " from " << it.source_airport << " to " << it.destination_airport << endl;
			int nextArrivalTime = it.departure_time + it.duration_sec;
			if (nextArrivalTime > start_time + get_max_duration())
				continue;
			int waitTime = it.departure_time - curr->arrival_time;
			if(waitTime>get_max_layover())
				continue;
			if (!preferred.empty() && preferred.find(it.airline) == preferred.end())
				continue;

			string nextAirport = it.destination_airport;
			int travelTime = it.duration_sec;
			int newTotalTime = nextArrivalTime-start_time;

			
			// check if it gives us a earlier arrival time at the next airport
			if (earliestArrival.find(nextAirport) == earliestArrival.end() || nextArrivalTime < earliestArrival[nextAirport])
			{
				earliestArrival[nextAirport] = nextArrivalTime;
				FlightSegment* newFlight = new FlightSegment(it);
				flightSegmentsToDelete.push_back(newFlight);
				auto i = previousFlights.find(nextAirport);
				if (i != previousFlights.end())
				{
					previousFlights[nextAirport] = newFlight;
				}
				else
				{
					previousFlights.insert({ nextAirport, newFlight });
				}
				pq.push(new Node(nextAirport, nextArrivalTime, newTotalTime));
			}
		}
	}

	cerr << "Exited while loop" << endl;
	
	for (auto node : nodesToDelete) delete node;
	for (auto fs : flightSegmentsToDelete) delete fs;
	return false;
	
}

vector<FlightSegment> TravelPlanner::getPossibleFlights(string currentAirport, int currentTime) const
{
	vector<FlightSegment> possibleFlights 
		= get_flight_manager().find_flights(currentAirport, 
			currentTime + get_min_connection_time(), currentTime + get_max_layover());
	return possibleFlights;
}

#endif

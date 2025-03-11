#ifndef TP_INCLUDED
#define TP_INCLUDED

#include "provided.h"
#include "fm.h"
#include <queue>
#include <unordered_map>
#include <vector>

using namespace std;

class TravelPlanner :public TravelPlannerBase
{
private:

	vector<string> preferred;

	struct Node
	{
		string airport;
		int arrival_time;
		int total_time;
		Node* prevNode;
		vector<FlightSegment> flights;
		Node(string source_airport, int arrivalTime, int totalTime, Node* prev=nullptr)
			:airport(source_airport), arrival_time(arrivalTime), total_time(totalTime), prevNode(prev) {}
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

	vector<FlightSegment>getPossibleFlights(string currentAirport, int currentTime, const vector<string>& preferredAirline) const; // move back to private
};


void TravelPlanner::add_preferred_airline(string airline)
{
	preferred.push_back(airline);
}

bool TravelPlanner::plan_travel(string source_airport, string
	destination_airport, int start_time, Itinerary& itinerary) const
{
	priority_queue<Node*, vector<Node*>, NodeComparator> pq; // open set sorted by arrival time
	pq.push(new Node(source_airport, start_time, 0));
	
	unordered_map<string, int> arrival; // Keeps track of the earliest arrival time at each airport
	arrival[source_airport] = start_time;


	while (!pq.empty())
	{
		Node* curr = pq.top();
		pq.pop();

		if (curr->airport == destination_airport) //path found
		{
			// construct the path
			cerr << "Path found" << endl;

			itinerary.source_airport = source_airport;
			itinerary.destination_airport = destination_airport;
			itinerary.total_duration = curr->flights.back().departure_time + curr->flights.back().duration_sec - start_time;


			for (auto it:curr->flights)
			{
				itinerary.flights.push_back(it);
				// itinerary.total_duration += it.duration_sec;
			}

			/*
			while (n != nullptr)
			{
				if (n->prevNode != nullptr)
				{
					FlightSegment flight = n->flights.back();
					itinerary.flights.insert(itinerary.flights.begin(), flight);
					itinerary.total_duration += flight.duration_sec;
				}
				n = n->prevNode;
			}
			*/
			
			// clear up
			while (!pq.empty())
			{
				delete pq.top();
				pq.pop();
			}
			delete curr;
			
			return true;
		}

		// for each possible flights from the current airport
		vector<FlightSegment> possibleFlights = getPossibleFlights(curr->airport, curr->arrival_time, preferred);
		for (auto it : possibleFlights)
		{
			string nextAirport = it.destination_airport;
			int nextArrivalTime = it.departure_time + it.duration_sec;
			int travelTime = it.duration_sec;
			int totalTime=curr->total_time + travelTime; // not accounting for laying over?

			cout << "here!! "<<it.flight_no << endl;

			// check if it gives us a earlier arrival time at the next airport
			if (arrival.find(nextAirport) == arrival.end() || nextArrivalTime < arrival[nextAirport])
			{
				arrival[nextAirport] = nextArrivalTime;
				Node* nextNode = new Node(nextAirport, nextArrivalTime, totalTime, curr);
				// nextNode->flights = curr->flights;
				nextNode->flights.push_back(it);
				pq.push(nextNode);
			}
		}
	}

	// if pq is empty and we haven't reach the destination
	return false;
}

vector<FlightSegment> TravelPlanner::getPossibleFlights(string currentAirport, int currentTime, const vector<string>& preferredAirline) const
{
	vector<FlightSegment> possibleFlights 
		= get_flight_manager().find_flights(currentAirport, 
			currentTime + get_min_connection_time(), currentTime + get_max_layover());
	if (!preferred.empty())
	{
		for (auto it = possibleFlights.begin(); it != possibleFlights.end(); )
		{
			if (find(preferred.begin(), preferred.end(), it->airline) == preferred.end())
				it = possibleFlights.erase(it);
			else
				it++;
		}
	}
	return possibleFlights;
}

#endif

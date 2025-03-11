#ifndef TP_INCLUDED
#define TP_INCLUDED

#include "provided.h"
#include "fm.h"
#include <queue>
#include <unordered_map>

class TravelPlanner :public TravelPlannerBase
{
private:

	std::vector<std::string> preferred;

	struct Node
	{
		std::string airport;
		int arrival_time;
		int total_time;
		std::vector<FlightSegment*> flights;
		Node(std::string source_airport, int arrivalTime, int totalTime)
			:airport(source_airport), arrival_time(arrivalTime), total_time(totalTime) {}
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

	void add_preferred_airline(std::string airline);

	bool plan_travel(std::string source_airport, std::string
		destination_airport, int start_time, Itinerary& itinerary) const;
};


void TravelPlanner::add_preferred_airline(std::string airline)
{
	preferred.push_back(airline);
}

bool TravelPlanner::plan_travel(std::string source_airport, std::string
	destination_airport, int start_time, Itinerary& itinerary) const
{
	std::priority_queue<Node, std::vector<Node>, NodeComparator> pq; // open set sorted by arrival time
	pq.push(Node(source_airport, start_time, 0));
	
	std::unordered_map<std::string, int> arrival; // Keeps track of the earliest arrival time at each airport
	arrival[source_airport] = start_time;

	while (!pq.empty())
	{
		Node curr = pq.top();
		pq.pop();
		if (curr.airport == destination_airport) //path found
		{
			// construct the path
			return true;
		}

		// get next possible flights

	}
}

#endif

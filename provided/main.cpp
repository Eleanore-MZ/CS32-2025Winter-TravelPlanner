#include "tp.h"
#include "fm.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <limits>
#include <ctime>
#include <cmath>

  // Helper function to format a UNIX timestamp.
std::string format_time(int unix_time)
{
    std::time_t time = unix_time;
    std::tm* tm_ptr = std::gmtime(&time);
    std::ostringstream oss;
    oss << std::put_time(tm_ptr, "%Y-%m-%d %H:%M UTC (") << unix_time << ")";
    return oss.str();
}

  // Function that prints an itinerary.
void print_itinerary(int start_time, const Itinerary& itinerary) {
    std::cout << "Source: " << itinerary.source_airport
              << ", Destination: " << itinerary.destination_airport;
    std::cout << ", Total Duration: " << itinerary.total_duration / 3600.0 << " hours\n";
    std::cout << "Arriving at source airport at: " << format_time(start_time) << "\n";
    std::cout << "Wait time at initial airport: "
              << (itinerary.flights[0].departure_time - start_time) / 3600.0
              << " hours\n";
    std::cout << "Flights:\n";
    for (size_t i = 0; i < itinerary.flights.size(); ++i) {
        const auto& flight = itinerary.flights[i];
        std::cout << "  " << flight.source_airport << " -> " << flight.destination_airport
                  << ", Airline: " << flight.airline
                  << ", Departure: " << format_time(flight.departure_time)
                  << ", Arrival: " << format_time(flight.departure_time + flight.duration_sec)
                  << ", Duration: " << flight.duration_sec / 3600.0 << " hours\n";
        if (i < itinerary.flights.size() - 1) {
            const auto& next_flight = itinerary.flights[i + 1];
            int layover_time = next_flight.departure_time
                               - (flight.departure_time + flight.duration_sec);
            std::cout << "  Layover: " << layover_time / 3600.0 << " hours\n";
        }
    }
}

  // Main entry point
int main(int argc, char* argv[]) {
      // We expect either one argument, the test parameters file, or no arguments,
      // meaning we'll prompt for the test parameters file
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [ testParametersFile ]\n";
        return 1;
    }

    std::string param_filename;
    if (argc > 1)
        param_filename = argv[1];
    else
    {
        std::cout << "Enter test parameters file name: ";
        std::getline(std::cin, param_filename);
    }
    std::ifstream infile(param_filename);
    if (!infile)
    {
        std::cerr << "Error: Failed to open " << param_filename << "\n";
        return 1;
    }

      // Read parameters from the test input file

      // These lines should match the exact format specified:
      // 1) flight database file
      // 2) source airport
      // 3) destination airport
      // 4) arrival time at source airport (unix sec)
      // 5) max travel duration (hours)
      // 6) min connection time (hours)
      // 7) max layover time (hours)
      // Then zero or more lines eeach with a preferred airline
    std::string flight_data_file;
    std::string source_airport;
    std::string destination_airport;
    int start_time;
    double max_duration_hours;
    double min_connection_hours;
    double max_layover_hours;

    std::vector<std::string> preferred_airlines;

      // Read the required lines/values
    if (!std::getline(infile, flight_data_file) ||
        !std::getline(infile, source_airport) ||
        !std::getline(infile, destination_airport) ||
        !(infile >> start_time) ||
        !(infile >> max_duration_hours) ||
        !(infile >> min_connection_hours) ||
        !(infile >> max_layover_hours) )
    {
        std::cerr << "Error: Invalid test parameters file format.\n";
        return 1;
    }

    // Consume any trailing newline after reading preferred_airlines_count
    infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

      // Read the preferred airline names
    std::string airline;
    while (std::getline(infile, airline))
        preferred_airlines.push_back(airline);

      // Initialize the flight manager and travel planner
    FlightManager flight_manager;
    AirportDB airport_db;
    if (!airport_db.load_airport_data("airports.txt")) {
        std::cerr << "Failed to load airport data.\n";
        return 1;
    }

      // Attempt to load the flights data
    if (!flight_manager.load_flight_data(flight_data_file)) {
        std::cerr << "Failed to load flight data from " << flight_data_file << ".\n";
        return 1;
    }

    TravelPlanner travel_planner(flight_manager, airport_db);

      // Convert hours to seconds
    int max_duration_sec    = static_cast<int>(std::round(max_duration_hours * 3600.0));
    int min_connection_sec  = static_cast<int>(std::round(min_connection_hours * 3600.0));
    int max_layover_sec     = static_cast<int>(std::round(max_layover_hours * 3600.0));

      // Set the travel-planner constraints
    travel_planner.set_max_duration(max_duration_sec);
    travel_planner.set_min_connection_time(min_connection_sec);
    travel_planner.set_max_layover(max_layover_sec);

      // Add preferred airlines, if any
    for (const auto& airline : preferred_airlines) {
        travel_planner.add_preferred_airline(airline);
    }

      // Now plan the itinerary
    Itinerary itinerary;
    bool result = travel_planner.plan_travel(
        source_airport, destination_airport, start_time, itinerary);

      // Print results
    if (!result) {
        std::cout << "No itineraries found matching your criteria.\n";
    }
    else {
          // Validate the itinerary
        std::string error_message;
        if (!validate_itinerary(
                flight_manager,
                itinerary,
                travel_planner.get_min_connection_time(),
                travel_planner.get_max_layover(),
                error_message)) {
            std::cerr << "Invalid itinerary: " << error_message << std::endl;
            return 1;
        }
        print_itinerary(start_time, itinerary);
    }
}

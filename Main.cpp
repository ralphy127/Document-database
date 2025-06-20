#include "Database.hpp"
#include "Seeder.hpp"

int main() {
    // Initialize the database from the example folder
    Database database("../example_database");

    // Seed the database with example data (optional)
    Seeder::seedDatabase(database);

    // Database is available in the main project folder
}
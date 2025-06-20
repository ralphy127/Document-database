#pragma once

class Database;

/// @brief Represents data seeder
class Seeder {
public:
    /// @brief Seed database with collections
    /// @param database Database which will to be seeded if it is empty
    static void seedDatabase(Database& database);
};
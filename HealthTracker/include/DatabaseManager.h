#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <vector>
#include <optional>
#include <sqlite3.h>
#include "User.h"
#include "HealthRecord.h"
/**
 * @file DatabaseManager.h
 * @brief Declaración de la clase DatabaseManager para manejo de base de datos SQLite.
 */

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    bool init(const std::string& filename = "health.db");

    bool create_user(const std::string& username, const std::string& password, std::string& out_err);
    std::optional<User> verify_user(const std::string& username, const std::string& password, std::string& out_err);

    bool add_record(const HealthRecord& rec, std::string& out_err);
    std::vector<HealthRecord> get_ruser(int user_id, std::string& out_err);
    bool delete_record(int record_id, int user_id, std::string& out_err);

    bool export_csv(int user_id, const std::string& filename, std::string& out_err);

    // Nuevo: generar análisis en texto
    bool generate_analysis(int user_id, std::string& out_text, std::string& out_err);

    std::optional<std::string> get_username(int user_id, std::string& out_err);

private:
    sqlite3* db_;
    bool execute_sql(const std::string& sql, std::string& out_err);

    static std::string generate_salt(size_t bytes = 16);
    static std::string sha256_hex(const std::string& input);
    static std::string h_password(const std::string& password, const std::string& salt);
};

#endif

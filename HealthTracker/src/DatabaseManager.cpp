#include "DatabaseManager.h"
#include "CSVExporter.h"

#include <openssl/sha.h>
#include <random>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>

using namespace std;

DatabaseManager::DatabaseManager() : db_(nullptr) {}

DatabaseManager::~DatabaseManager() {
    if (db_) sqlite3_close(db_);
}

bool DatabaseManager::init(const string& filename) {
    if (sqlite3_open(filename.c_str(), &db_) != SQLITE_OK) {
        return false;
    }

    string err;
    execute_sql("PRAGMA foreign_keys = ON;", err);

    const char* users_sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password_hash TEXT NOT NULL,"
        "salt TEXT NOT NULL"
        ");";

    const char* records_sql =
        "CREATE TABLE IF NOT EXISTS health_records ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "datetime TEXT NOT NULL,"
        "weight REAL,"
        "systolic INTEGER,"
        "diastolic INTEGER,"
        "glucose REAL,"
        "FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE"
        ");";

    if (!execute_sql(users_sql, err)) {
        cerr << "Error creando tabla users: " << err << "\n";
        return false;
    }

    if (!execute_sql(records_sql, err)) {
        cerr << "Error creando tabla health_records: " << err << "\n";
        return false;
    }

    return true;
}

bool DatabaseManager::execute_sql(const string& sql, string& out_err) {
    char* err = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err);

    if (rc != SQLITE_OK) {
        if (err) {
            out_err = err;
            sqlite3_free(err);
        } else {
            out_err = "Error desconocido en sqlite3_exec";
        }
        return false;
    }
    return true;
}

string DatabaseManager::generate_salt(size_t bytes) {
    random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<uint8_t> dist(0, 255);

    ostringstream oss;
    oss << hex << setfill('0');

    for (size_t i = 0; i < bytes; ++i) {
        oss << setw(2) << static_cast<int>(dist(gen));
    }

    return oss.str();
}

string DatabaseManager::sha256_hex(const string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.data()), input.size(), hash);

    ostringstream oss;
    oss << hex << setfill('0');

    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        oss << setw(2) << static_cast<int>(hash[i]);
    }

    return oss.str();
}

string DatabaseManager::h_password(const string& password, const string& salt) {
    return sha256_hex(password + salt);
}

bool DatabaseManager::create_user(const string& username, const string& password, string& out_err) {
    if (username.empty()) {
        out_err = "usuario vacío";
        return false;
    }
    if (password.empty()) {
        out_err = "contraseña vacía";
        return false;
    }

    string salt = generate_salt(16);
    string phash = h_password(password, salt);

    const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        out_err = sqlite3_errmsg(db_);
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, phash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, salt.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        out_err = sqlite3_errmsg(db_);
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

optional<User> DatabaseManager::verify_user(const string& username, const string& password, string& out_err) {
    if (username.empty() || password.empty()) {
        out_err = "username o password vacío";
        return nullopt;
    }

    const char* sql =
        "SELECT id, password_hash, salt FROM users WHERE username = ? LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        out_err = sqlite3_errmsg(db_);
        return nullopt;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        out_err = "usuario no encontrado";
        return nullopt;
    }

    int id = sqlite3_column_int(stmt, 0);
    string db_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    string db_salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

    sqlite3_finalize(stmt);

    if (h_password(password, db_salt) == db_hash) {
        return User(id, username, db_hash, db_salt);
    } else {
        out_err = "credenciales inválidas";
        return nullopt;
    }
}

bool DatabaseManager::add_record(const HealthRecord& rec, string& out_err) {
    const char* sql =
        "INSERT INTO health_records (user_id, datetime, weight, systolic, diastolic, glucose) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        out_err = sqlite3_errmsg(db_);
        return false;
    }

    sqlite3_bind_int(stmt, 1, rec.user_id);
    sqlite3_bind_text(stmt, 2, rec.datetime.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, rec.weight);
    sqlite3_bind_int(stmt, 4, rec.systolic);
    sqlite3_bind_int(stmt, 5, rec.diastolic);
    sqlite3_bind_double(stmt, 6, rec.glucose);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        out_err = sqlite3_errmsg(db_);
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

vector<HealthRecord> DatabaseManager::get_ruser(int user_id, string& out_err) {
    vector<HealthRecord> results;

    const char* sql =
        "SELECT id, datetime, weight, systolic, diastolic, glucose "
        "FROM health_records WHERE user_id = ? ORDER BY datetime ASC;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        out_err = sqlite3_errmsg(db_);
        return results;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        HealthRecord r;
        r.id = sqlite3_column_int(stmt, 0);
        r.datetime = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        r.weight = sqlite3_column_double(stmt, 2);
        r.systolic = sqlite3_column_int(stmt, 3);
        r.diastolic = sqlite3_column_int(stmt, 4);
        r.glucose = sqlite3_column_double(stmt, 5);
        r.user_id = user_id;

        results.push_back(r);
    }

    sqlite3_finalize(stmt);
    return results;
}

bool DatabaseManager::delete_record(int record_id, int user_id, string& out_err) {
    const char* sql =
        "DELETE FROM health_records WHERE id = ? AND user_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        out_err = sqlite3_errmsg(db_);
        return false;
    }

    sqlite3_bind_int(stmt, 1, record_id);
    sqlite3_bind_int(stmt, 2, user_id);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        out_err = sqlite3_errmsg(db_);
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

optional<string> DatabaseManager::get_username(int user_id, string& out_err) {
    const char* sql = "SELECT username FROM users WHERE id = ? LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        out_err = sqlite3_errmsg(db_);
        return nullopt;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        out_err = "usuario no encontrado";
        return nullopt;
    }

    string username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

    sqlite3_finalize(stmt);
    return username;
}

bool DatabaseManager::export_csv(int user_id, const string& filename, string& out_err) {
    auto records = get_ruser(user_id, out_err);

    if (!out_err.empty())
        return false;

    auto username_opt = get_username(user_id, out_err);
    if (!username_opt) {
        if (out_err.empty()) out_err = "usuario no encontrado";
        return false;
    }

    if (!CSVExporter::export_file(records, filename, *username_opt, out_err, false)) {
        return false;
    }

    return true;
}

bool DatabaseManager::generate_analysis(int user_id, string& out_text, string& out_err) {
    auto records = get_ruser(user_id, out_err);

    if (!out_err.empty())
        return false;

    if (records.empty()) {
        out_err = "No hay registros para analizar";
        return false;
    }

    double sumWeight = 0.0;
    double sumSys = 0.0;
    double sumDia = 0.0;
    double sumGlu = 0.0;
    int countWeight = 0;
    int countSys = 0;
    int countDia = 0;
    int countGlu = 0;

    for (const auto& r : records) {
        // treat zero as missing for bp; treat nan as missing for doubles
        if (!std::isnan(r.weight) && r.weight != 0.0) { sumWeight += r.weight; ++countWeight; }
        if (r.systolic != 0) { sumSys += r.systolic; ++countSys; }
        if (r.diastolic != 0) { sumDia += r.diastolic; ++countDia; }
        if (!std::isnan(r.glucose) && r.glucose != 0.0) { sumGlu += r.glucose; ++countGlu; }
    }

    ostringstream oss;
    oss << fixed << setprecision(2);
    oss << "ANÁLISIS DE PROMEDIOS\n\n";

    if (countWeight > 0)
        oss << "Promedio peso: " << (sumWeight / countWeight) << " kg\n";
    else
        oss << "Promedio peso: sin datos\n";

    if (countSys > 0 && countDia > 0)
        oss << "Promedio presión: " << (sumSys / countSys) << " / " << (sumDia / countDia) << " mmHg\n";
    else
        oss << "Promedio presión: sin datos\n";

    if (countGlu > 0)
        oss << "Promedio glucosa: " << (sumGlu / countGlu) << " mg/dL\n";
    else
        oss << "Promedio glucosa: sin datos\n";

    out_text = oss.str();
    return true;
}



#include "MediaRepository.h"

#include "../../utils/PathUtf8.h"

#include <sqlite3.h>

#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>

namespace fs = std::filesystem;

namespace
{
std::string kindToStorageValue(MediaKind kind)
{
    switch (kind)
    {
    case MediaKind::Video:
        return "video";
    case MediaKind::Audio:
        return "audio";
    case MediaKind::Image:
        return "image";
    }
    throw std::logic_error("Unsupported media kind");
}

MediaKind kindFromStorageValue(const unsigned char *value)
{
    const std::string kind = value ? reinterpret_cast<const char *>(value) : "";
    if (kind == "video")
        return MediaKind::Video;
    if (kind == "audio")
        return MediaKind::Audio;
    if (kind == "image")
        return MediaKind::Image;
    throw std::runtime_error("Unknown media kind in database: " + kind);
}

std::string extensionNoDot(const fs::path &path)
{
    auto ext = path.extension().string();
    if (!ext.empty() && ext.front() == '.')
        ext.erase(ext.begin());
    return ext;
}

std::int64_t fileTimeToken(const fs::path &path)
{
    return fs::last_write_time(path).time_since_epoch().count();
}

std::string canonicalUtf8(const fs::path &path)
{
    return pathToUtf8(fs::weakly_canonical(path));
}

class Statement
{
public:
    Statement(sqlite3 *db, const char *sql) : db_(db)
    {
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt_, nullptr) != SQLITE_OK)
            throw std::runtime_error(sqlite3_errmsg(db_));
    }

    ~Statement()
    {
        if (stmt_)
            sqlite3_finalize(stmt_);
    }

    Statement(const Statement &) = delete;
    Statement &operator=(const Statement &) = delete;

    sqlite3_stmt *get() const noexcept { return stmt_; }

    void bindText(int index, const std::string &value)
    {
        if (sqlite3_bind_text(stmt_, index, value.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
            throw std::runtime_error(sqlite3_errmsg(db_));
    }

    void bindInt64(int index, sqlite3_int64 value)
    {
        if (sqlite3_bind_int64(stmt_, index, value) != SQLITE_OK)
            throw std::runtime_error(sqlite3_errmsg(db_));
    }

    bool stepRow()
    {
        const int rc = sqlite3_step(stmt_);
        if (rc == SQLITE_ROW)
            return true;
        if (rc == SQLITE_DONE)
            return false;
        throw std::runtime_error(sqlite3_errmsg(db_));
    }

    void stepDone()
    {
        const int rc = sqlite3_step(stmt_);
        if (rc != SQLITE_DONE)
            throw std::runtime_error(sqlite3_errmsg(db_));
    }

private:
    sqlite3 *db_{};
    sqlite3_stmt *stmt_{};
};
} // namespace

MediaRepository::MediaRepository(fs::path databasePath)
    : databasePath_(std::move(databasePath))
{
    if (databasePath_.has_parent_path())
        fs::create_directories(databasePath_.parent_path());

    const auto dbPath = pathToUtf8(databasePath_);
    if (sqlite3_open(dbPath.c_str(), &db_) != SQLITE_OK)
    {
        const std::string message = db_ ? sqlite3_errmsg(db_) : "Cannot open SQLite database";
        if (db_)
            sqlite3_close(db_);
        db_ = nullptr;
        throw std::runtime_error(message);
    }
}

MediaRepository::~MediaRepository()
{
    if (db_)
        sqlite3_close(db_);
}

void MediaRepository::execute(const char *sql) const
{
    char *error = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &error) != SQLITE_OK)
    {
        std::string message = error ? error : sqlite3_errmsg(db_);
        sqlite3_free(error);
        throw std::runtime_error(message);
    }
}

void MediaRepository::initialize()
{
    execute("PRAGMA foreign_keys = ON;");
    execute("CREATE TABLE IF NOT EXISTS media_files ("
            "id TEXT PRIMARY KEY,"
            "root_path TEXT NOT NULL,"
            "path TEXT NOT NULL UNIQUE,"
            "kind TEXT NOT NULL CHECK(kind IN ('video', 'audio', 'image')),"
            "title TEXT NOT NULL,"
            "extension TEXT NOT NULL,"
            "mime_type TEXT NOT NULL,"
            "file_size INTEGER NOT NULL,"
            "modified_time INTEGER NOT NULL,"
            "last_scan_token TEXT NOT NULL,"
            "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"
            ");");
    execute("CREATE INDEX IF NOT EXISTS idx_media_files_root_kind_title "
            "ON media_files(root_path, kind, title);");
}

void MediaRepository::synchronize(const fs::path &root,
                                  const std::vector<MediaRecord> &records)
{
    const auto rootPath = canonicalUtf8(root);
    const auto scanToken = std::to_string(
        std::chrono::system_clock::now().time_since_epoch().count());

    execute("BEGIN IMMEDIATE TRANSACTION;");
    try
    {
        Statement upsert(db_,
                         "INSERT INTO media_files "
                         "(id, root_path, path, kind, title, extension, mime_type, "
                         "file_size, modified_time, last_scan_token) "
                         "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
                         "ON CONFLICT(path) DO UPDATE SET "
                         "id = excluded.id,"
                         "root_path = excluded.root_path,"
                         "kind = excluded.kind,"
                         "title = excluded.title,"
                         "extension = excluded.extension,"
                         "mime_type = excluded.mime_type,"
                         "file_size = excluded.file_size,"
                         "modified_time = excluded.modified_time,"
                         "last_scan_token = excluded.last_scan_token,"
                         "updated_at = CURRENT_TIMESTAMP;");

        for (const auto &record : records)
        {
            const auto path = canonicalUtf8(record.path);
            const auto kind = kindToStorageValue(record.kind);
            const auto title = pathToUtf8(record.path.stem());
            const auto extension = extensionNoDot(record.path);

            sqlite3_reset(upsert.get());
            sqlite3_clear_bindings(upsert.get());
            upsert.bindText(1, record.id);
            upsert.bindText(2, rootPath);
            upsert.bindText(3, path);
            upsert.bindText(4, kind);
            upsert.bindText(5, title);
            upsert.bindText(6, extension);
            upsert.bindText(7, record.mimeType);
            upsert.bindInt64(8, static_cast<sqlite3_int64>(record.size));
            upsert.bindInt64(9, fileTimeToken(record.path));
            upsert.bindText(10, scanToken);
            upsert.stepDone();
        }

        Statement removeMissing(db_,
                                "DELETE FROM media_files "
                                "WHERE root_path = ? AND last_scan_token <> ?;");
        removeMissing.bindText(1, rootPath);
        removeMissing.bindText(2, scanToken);
        removeMissing.stepDone();

        execute("COMMIT;");
    }
    catch (...)
    {
        execute("ROLLBACK;");
        throw;
    }
}

std::vector<MediaRecord> MediaRepository::listByRoot(const fs::path &root) const
{
    const auto rootPath = canonicalUtf8(root);
    Statement query(db_,
                    "SELECT id, path, kind, file_size, mime_type "
                    "FROM media_files "
                    "WHERE root_path = ? "
                    "ORDER BY CASE kind "
                    "WHEN 'video' THEN 0 "
                    "WHEN 'audio' THEN 1 "
                    "WHEN 'image' THEN 2 "
                    "ELSE 3 END, title COLLATE NOCASE;");
    query.bindText(1, rootPath);

    std::vector<MediaRecord> records;
    while (query.stepRow())
    {
        MediaRecord record;
        record.id = reinterpret_cast<const char *>(sqlite3_column_text(query.get(), 0));
        record.path = fs::u8path(reinterpret_cast<const char *>(sqlite3_column_text(query.get(), 1)));
        record.kind = kindFromStorageValue(sqlite3_column_text(query.get(), 2));
        record.size = static_cast<std::uintmax_t>(sqlite3_column_int64(query.get(), 3));
        record.mimeType = reinterpret_cast<const char *>(sqlite3_column_text(query.get(), 4));
        records.push_back(std::move(record));
    }
    return records;
}

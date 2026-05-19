#include "MediaRepository.h"

#include <pqxx/pqxx>

#include <chrono>
#include <stdexcept>
#include <string>
#include <utility>

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

MediaKind kindFromStorageValue(const std::string &kind)
{
    if (kind == "video")
        return MediaKind::Video;
    if (kind == "audio")
        return MediaKind::Audio;
    if (kind == "image")
        return MediaKind::Image;
    throw std::runtime_error("Unknown media kind in database: " + kind);
}

std::string extensionNoDot(const std::string &path)
{
    const auto slash = path.find_last_of('/');
    const auto dot = path.find_last_of('.');
    if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
        return {};
    return path.substr(dot + 1);
}

std::string titleFromObjectPath(const std::string &path)
{
    const auto slash = path.find_last_of('/');
    const auto begin = slash == std::string::npos ? 0 : slash + 1;
    const auto dot = path.find_last_of('.');
    const auto end = dot == std::string::npos || dot < begin ? path.size() : dot;
    return path.substr(begin, end - begin);
}
} // namespace

MediaRepository::MediaRepository(std::string databaseUrl)
    : databaseUrl_(std::move(databaseUrl))
{
    if (databaseUrl_.empty())
        throw std::runtime_error("DATABASE_URL is not set");
}

void MediaRepository::initialize()
{
    pqxx::connection conn(databaseUrl_);
    pqxx::work tx(conn);
    tx.exec(R"SQL(
        CREATE TABLE IF NOT EXISTS media_files (
            id TEXT PRIMARY KEY,
            bucket TEXT NOT NULL,
            object_path TEXT NOT NULL UNIQUE,
            storage_url TEXT NOT NULL DEFAULT '',
            kind TEXT NOT NULL CHECK(kind IN ('video', 'audio', 'image')),
            title TEXT NOT NULL,
            extension TEXT NOT NULL,
            mime_type TEXT NOT NULL,
            file_size BIGINT NOT NULL,
            last_scan_token TEXT NOT NULL,
            created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
            updated_at TIMESTAMPTZ NOT NULL DEFAULT now()
        );
    )SQL");
    tx.exec("ALTER TABLE media_files ADD COLUMN IF NOT EXISTS storage_url TEXT NOT NULL DEFAULT '';");
    tx.exec("CREATE INDEX IF NOT EXISTS idx_media_files_bucket_kind_title "
            "ON media_files(bucket, kind, title);");
    tx.commit();
}

void MediaRepository::synchronize(const std::string &bucket,
                                  const std::vector<MediaRecord> &records)
{
    pqxx::connection conn(databaseUrl_);
    pqxx::work tx(conn);
    const auto scanToken = std::to_string(
        std::chrono::system_clock::now().time_since_epoch().count());

    for (const auto &record : records)
    {
        tx.exec_params(
            R"SQL(
                INSERT INTO media_files
                    (id, bucket, object_path, storage_url, kind, title, extension, mime_type,
                     file_size, last_scan_token)
                VALUES
                    ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)
                ON CONFLICT(object_path) DO UPDATE SET
                    id = EXCLUDED.id,
                    bucket = EXCLUDED.bucket,
                    storage_url = EXCLUDED.storage_url,
                    kind = EXCLUDED.kind,
                    title = EXCLUDED.title,
                    extension = EXCLUDED.extension,
                    mime_type = EXCLUDED.mime_type,
                    file_size = EXCLUDED.file_size,
                    last_scan_token = EXCLUDED.last_scan_token,
                    updated_at = now();
            )SQL",
            record.id,
            bucket,
            record.objectPath,
            record.storageUrl,
            kindToStorageValue(record.kind),
            titleFromObjectPath(record.objectPath),
            extensionNoDot(record.objectPath),
            record.mimeType,
            static_cast<long long>(record.size),
            scanToken);
    }

    tx.exec_params("DELETE FROM media_files WHERE bucket = $1 AND last_scan_token <> $2;",
                   bucket,
                   scanToken);
    tx.commit();
}

std::vector<MediaRecord> MediaRepository::listByBucket(const std::string &bucket) const
{
    pqxx::connection conn(databaseUrl_);
    pqxx::read_transaction tx(conn);
    const auto rows = tx.exec_params(
        R"SQL(
            SELECT id, object_path, storage_url, kind, file_size, mime_type
            FROM media_files
            WHERE bucket = $1
            ORDER BY CASE kind
                WHEN 'video' THEN 0
                WHEN 'audio' THEN 1
                WHEN 'image' THEN 2
                ELSE 3
            END, title COLLATE "C";
        )SQL",
        bucket);

    std::vector<MediaRecord> records;
    records.reserve(rows.size());
    for (const auto &row : rows)
    {
        records.push_back(MediaRecord{
            row["id"].as<std::string>(),
            row["object_path"].as<std::string>(),
            row["storage_url"].as<std::string>(),
            kindFromStorageValue(row["kind"].as<std::string>()),
            static_cast<std::uintmax_t>(row["file_size"].as<long long>()),
            row["mime_type"].as<std::string>(),
        });
    }
    return records;
}

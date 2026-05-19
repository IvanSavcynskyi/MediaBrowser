#include "SupabaseStorageScanner.h"

#include "../domain/MediaKind.h"

#include <curl/curl.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <tinyxml2.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
constexpr const char *emptyPayloadSha256 =
    "e3b0c44298fc1c149afbf4c8996fb924"
    "27ae41e4649b934ca495991b7852b855";

std::string trimTrailingSlash(std::string value)
{
    while (!value.empty() && value.back() == '/')
        value.pop_back();
    return value;
}

std::string endpointHost(const std::string &endpoint)
{
    const auto scheme = endpoint.find("://");
    const auto hostStart = scheme == std::string::npos ? 0 : scheme + 3;
    const auto pathStart = endpoint.find('/', hostStart);
    return endpoint.substr(hostStart, pathStart - hostStart);
}

std::string endpointPath(const std::string &endpoint)
{
    const auto scheme = endpoint.find("://");
    const auto hostStart = scheme == std::string::npos ? 0 : scheme + 3;
    const auto pathStart = endpoint.find('/', hostStart);
    if (pathStart == std::string::npos)
        return {};
    return endpoint.substr(pathStart);
}

std::string hex(const unsigned char *data, unsigned int len)
{
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < len; ++i)
        out << std::setw(2) << static_cast<int>(data[i]);
    return out.str();
}

std::string sha256Hex(const std::string &value)
{
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    EVP_Digest(value.data(), value.size(), digest, &len, EVP_sha256(), nullptr);
    return hex(digest, len);
}

std::string hmacSha256(const std::string &key, const std::string &message)
{
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    HMAC(EVP_sha256(),
         key.data(),
         static_cast<int>(key.size()),
         reinterpret_cast<const unsigned char *>(message.data()),
         message.size(),
         digest,
         &len);
    return std::string(reinterpret_cast<char *>(digest), len);
}

std::string hmacSha256Hex(const std::string &key, const std::string &message)
{
    const auto raw = hmacSha256(key, message);
    return hex(reinterpret_cast<const unsigned char *>(raw.data()),
               static_cast<unsigned int>(raw.size()));
}

std::tm gmtimeUtc(std::time_t t)
{
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    return tm;
}

std::string formatTime(const char *fmt)
{
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const auto tm = gmtimeUtc(now);
    std::ostringstream out;
    out << std::put_time(&tm, fmt);
    return out.str();
}

bool isUnreserved(unsigned char ch)
{
    return std::isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~';
}

std::string uriEncode(const std::string &value, bool encodeSlash)
{
    std::ostringstream out;
    out << std::uppercase << std::hex << std::setfill('0');
    for (const unsigned char ch : value)
    {
        if (isUnreserved(ch) || (!encodeSlash && ch == '/'))
            out << static_cast<char>(ch);
        else
            out << '%' << std::setw(2) << static_cast<int>(ch);
    }
    return out.str();
}

std::string mimeFromPath(const std::string &path)
{
    auto dot = path.find_last_of('.');
    std::string ext = dot == std::string::npos ? "" : path.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char ch)
                   { return static_cast<char>(std::tolower(ch)); });
    if (ext == "mp4")
        return "video/mp4";
    if (ext == "mp3")
        return "audio/mpeg";
    if (ext == "jpg" || ext == "jpeg")
        return "image/jpeg";
    if (ext == "png")
        return "image/png";
    return "application/octet-stream";
}

bool matchesKind(MediaKind kind, const std::string &path)
{
    const auto mime = mimeFromPath(path);
    switch (kind)
    {
    case MediaKind::Video:
        return mime == "video/mp4";
    case MediaKind::Audio:
        return mime == "audio/mpeg";
    case MediaKind::Image:
        return mime == "image/jpeg" || mime == "image/png";
    }
    return false;
}

std::string kindValue(MediaKind kind)
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
    return "media";
}

std::string makeId(const std::string &bucket, const std::string &objectPath)
{
    return std::to_string(std::hash<std::string>{}(bucket + "/" + objectPath));
}

size_t appendBody(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    auto *body = static_cast<std::string *>(userdata);
    body->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string curlGet(const std::string &url, const std::vector<std::string> &headers)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("Cannot initialize curl");

    curl_slist *headerList = nullptr;
    for (const auto &header : headers)
        headerList = curl_slist_append(headerList, header.c_str());

    std::string body;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, appendBody);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);

    const auto rc = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);

    curl_slist_free_all(headerList);
    curl_easy_cleanup(curl);

    if (rc != CURLE_OK)
        throw std::runtime_error(std::string("S3 request failed: ") + curl_easy_strerror(rc));
    if (status < 200 || status >= 300)
        throw std::runtime_error("S3 request returned HTTP " + std::to_string(status) + ": " + body);
    return body;
}
} // namespace

SupabaseStorageScanner::SupabaseStorageScanner(std::string endpoint,
                                               std::string region,
                                               std::string accessId,
                                               std::string accessKey,
                                               std::string bucket)
    : endpoint_(trimTrailingSlash(std::move(endpoint))),
      region_(std::move(region)),
      accessId_(std::move(accessId)),
      accessKey_(std::move(accessKey)),
      bucket_(std::move(bucket))
{
    if (endpoint_.empty() || region_.empty() || accessId_.empty() ||
        accessKey_.empty() || bucket_.empty())
    {
        throw std::runtime_error("Supabase S3 environment variables are not fully configured");
    }
}

std::vector<MediaRecord> SupabaseStorageScanner::scan() const
{
    std::vector<MediaRecord> records;
    for (const auto prefix : {std::string(mediaKindDirectory(MediaKind::Video)) + "/",
                              std::string(mediaKindDirectory(MediaKind::Audio)) + "/",
                              std::string(mediaKindDirectory(MediaKind::Image)) + "/"})
    {
        auto prefixed = scanPrefix(prefix);
        records.insert(records.end(),
                       std::make_move_iterator(prefixed.begin()),
                       std::make_move_iterator(prefixed.end()));
    }
    return records;
}

std::vector<MediaRecord> SupabaseStorageScanner::scanPrefix(const std::string &prefix) const
{
    std::vector<MediaRecord> records;
    std::string continuationToken;
    const auto host = endpointHost(endpoint_);
    const auto canonicalUri = endpointPath(endpoint_) + "/" + uriEncode(bucket_, true);

    do
    {
        std::string query;
        if (!continuationToken.empty())
            query += "continuation-token=" + uriEncode(continuationToken, true) + "&";
        query += "list-type=2&prefix=" + uriEncode(prefix, true);

        const auto amzDate = formatTime("%Y%m%dT%H%M%SZ");
        const auto shortDate = amzDate.substr(0, 8);
        const auto canonicalHeaders = "host:" + host + "\n" +
                                      "x-amz-content-sha256:" + emptyPayloadSha256 + "\n" +
                                      "x-amz-date:" + amzDate + "\n";
        const auto signedHeaders = "host;x-amz-content-sha256;x-amz-date";
        const auto canonicalRequest = "GET\n" + canonicalUri + "\n" + query + "\n" +
                                      canonicalHeaders + "\n" + signedHeaders + "\n" +
                                      emptyPayloadSha256;
        const auto scope = shortDate + "/" + region_ + "/s3/aws4_request";
        const auto stringToSign = "AWS4-HMAC-SHA256\n" + amzDate + "\n" + scope + "\n" +
                                  sha256Hex(canonicalRequest);

        const auto kDate = hmacSha256("AWS4" + accessKey_, shortDate);
        const auto kRegion = hmacSha256(kDate, region_);
        const auto kService = hmacSha256(kRegion, "s3");
        const auto kSigning = hmacSha256(kService, "aws4_request");
        const auto signature = hmacSha256Hex(kSigning, stringToSign);
        const auto authorization = "AWS4-HMAC-SHA256 Credential=" + accessId_ + "/" + scope +
                                   ", SignedHeaders=" + signedHeaders +
                                   ", Signature=" + signature;

        const auto body = curlGet(endpoint_ + "/" + uriEncode(bucket_, true) + "?" + query,
                                  {"Host: " + host,
                                   "x-amz-content-sha256: " + std::string(emptyPayloadSha256),
                                   "x-amz-date: " + amzDate,
                                   "Authorization: " + authorization});

        tinyxml2::XMLDocument doc;
        if (doc.Parse(body.c_str()) != tinyxml2::XML_SUCCESS)
            throw std::runtime_error("Cannot parse S3 ListBucketResult XML");

        auto *root = doc.FirstChildElement("ListBucketResult");
        if (!root)
            throw std::runtime_error("S3 response does not contain ListBucketResult");

        for (auto *contents = root->FirstChildElement("Contents");
             contents;
             contents = contents->NextSiblingElement("Contents"))
        {
            auto *keyEl = contents->FirstChildElement("Key");
            auto *sizeEl = contents->FirstChildElement("Size");
            if (!keyEl || !keyEl->GetText() || !sizeEl || !sizeEl->GetText())
                continue;

            const std::string objectPath = keyEl->GetText();
            MediaKind kind = MediaKind::Image;
            if (objectPath.rfind(std::string(mediaKindDirectory(MediaKind::Video)) + "/", 0) == 0)
                kind = MediaKind::Video;
            else if (objectPath.rfind(std::string(mediaKindDirectory(MediaKind::Audio)) + "/", 0) == 0)
                kind = MediaKind::Audio;
            else if (objectPath.rfind(std::string(mediaKindDirectory(MediaKind::Image)) + "/", 0) == 0)
                kind = MediaKind::Image;

            if (!matchesKind(kind, objectPath))
                continue;

            records.push_back(MediaRecord{
                makeId(bucket_, objectPath),
                objectPath,
                endpoint_ + "/" + uriEncode(bucket_, true) + "/" + uriEncode(objectPath, false),
                kind,
                static_cast<std::uintmax_t>(std::stoull(sizeEl->GetText())),
                mimeFromPath(objectPath),
            });
        }

        auto *truncatedEl = root->FirstChildElement("IsTruncated");
        const bool truncated = truncatedEl && truncatedEl->GetText() &&
                               std::string(truncatedEl->GetText()) == "true";
        auto *nextEl = root->FirstChildElement("NextContinuationToken");
        continuationToken = truncated && nextEl && nextEl->GetText() ? nextEl->GetText() : "";
    } while (!continuationToken.empty());

    return records;
}

std::string SupabaseStorageScanner::presignedGetUrl(const std::string &objectPath,
                                                    int expiresSeconds) const
{
    const auto host = endpointHost(endpoint_);
    const auto canonicalUri = endpointPath(endpoint_) + "/" +
                              uriEncode(bucket_, true) + "/" +
                              uriEncode(objectPath, false);
    const auto amzDate = formatTime("%Y%m%dT%H%M%SZ");
    const auto shortDate = amzDate.substr(0, 8);
    const auto scope = shortDate + "/" + region_ + "/s3/aws4_request";

    std::string query = "X-Amz-Algorithm=AWS4-HMAC-SHA256";
    query += "&X-Amz-Credential=" + uriEncode(accessId_ + "/" + scope, true);
    query += "&X-Amz-Date=" + amzDate;
    query += "&X-Amz-Expires=" + std::to_string(expiresSeconds);
    query += "&X-Amz-SignedHeaders=host";

    const auto canonicalRequest = "GET\n" + canonicalUri + "\n" + query + "\n" +
                                  "host:" + host + "\n\nhost\nUNSIGNED-PAYLOAD";
    const auto stringToSign = "AWS4-HMAC-SHA256\n" + amzDate + "\n" + scope + "\n" +
                              sha256Hex(canonicalRequest);

    const auto kDate = hmacSha256("AWS4" + accessKey_, shortDate);
    const auto kRegion = hmacSha256(kDate, region_);
    const auto kService = hmacSha256(kRegion, "s3");
    const auto kSigning = hmacSha256(kService, "aws4_request");
    const auto signature = hmacSha256Hex(kSigning, stringToSign);

    return endpoint_ + "/" + uriEncode(bucket_, true) + "/" +
           uriEncode(objectPath, false) + "?" + query +
           "&X-Amz-Signature=" + signature;
}

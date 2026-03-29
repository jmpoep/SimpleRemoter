#include "stdafx.h"
#include "resource.h"
#include "2015Remote.h"
#include "LicenseFile.h"
#include "pwd_gen.h"
#include "2015RemoteDlg.h"
#include "CPasswordDlg.h"
#include "LangManager.h"
#include "jsoncpp/json.h"
#include <fstream>
#include <sstream>
#include <ctime>
#include <cctype>
#include <cstdio>
#include <memory>

#ifndef _WIN64
#ifdef _DEBUG
#pragma comment(lib, "jsoncpp/jsoncppd.lib")
#else
#pragma comment(lib, "jsoncpp/jsoncpp.lib")
#endif
#else
#ifdef _DEBUG
#pragma comment(lib, "jsoncpp/jsoncpp_x64d.lib")
#else
#pragma comment(lib, "jsoncpp/jsoncpp_x64.lib")
#endif
#endif

// Get current time string
static std::string GetCurrentTimeString() {
    time_t now = time(nullptr);
    struct tm t;
    localtime_s(&t, &now);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
    return buf;
}

// Build checksum content (includes magic, version, createTime, and license fields)
static std::string BuildChecksumContent(const std::string& magic,
                                         int version,
                                         const std::string& createTime,
                                         const std::string& sn,
                                         const std::string& password,
                                         const std::string& pwdHmac,
                                         const std::string& authorization) {
    std::string content;
    content += magic + "|";
    content += std::to_string(version) + "|";
    content += createTime + "|";
    content += sn + "|";
    content += password + "|";
    content += pwdHmac + "|";
    content += authorization;
    return content;
}

bool IsIPv4Format(const std::string& sn) {
    // IPv4 format: 4 segments (0-255) separated by dots, no dashes
    if (sn.find('-') != std::string::npos) return false;
    if (sn.empty()) return false;

    int segmentCount = 0;
    int currentValue = 0;
    int digitCount = 0;

    for (size_t i = 0; i <= sn.size(); ++i) {
        if (i == sn.size() || sn[i] == '.') {
            if (digitCount == 0) return false;  // Empty segment
            if (currentValue > 255) return false;  // Value out of range
            segmentCount++;
            currentValue = 0;
            digitCount = 0;
        } else if (isdigit(sn[i])) {
            digitCount++;
            if (digitCount > 3) return false;  // Too many digits (prevents overflow)
            currentValue = currentValue * 10 + (sn[i] - '0');
        } else {
            return false;  // Invalid character
        }
    }

    return segmentCount == 4;  // Must have exactly 4 segments
}

SNMatchResult ValidateLicenseSN(const std::string& licenseSN) {
    if (IsIPv4Format(licenseSN)) {
        // IP binding: check if matches current machine's public IP
        // For now, we check against configured master IP
        // In real implementation, should get actual public IP
        std::string currentIP = CMy2015RemoteDlg::GetHardwareID(1);  // Use IP mode
        if (licenseSN == currentIP) {
            return SNMatchResult::Match;
        }
        return SNMatchResult::IPMismatch;
    } else {
        // Hardware binding: check if matches current device ID
        std::string hardwareID = getHardwareID();
        std::string hashedID = hashSHA256(hardwareID);
        std::string currentDeviceID = getFixedLengthID(hashedID);
        if (licenseSN == currentDeviceID) {
            return SNMatchResult::Match;
        }
        return SNMatchResult::HardwareMismatch;
    }
}

bool ExportLicenseFile(const std::string& filePath,
                       const std::string& sn,
                       const std::string& password,
                       const std::string& pwdHmac,
                       const std::string& authorization) {
    // 1. Generate create time
    std::string createTime = GetCurrentTimeString();

    // 2. Calculate checksum
    std::string checksumContent = BuildChecksumContent(
        LICENSE_MAGIC, LICENSE_FILE_VERSION, createTime,
        sn, password, pwdHmac, authorization);
    std::string checksum = "sha256:" + hashSHA256(checksumContent);

    // 3. Build JSON using jsoncpp
    Json::Value root;
    root["magic"] = LICENSE_MAGIC;
    root["version"] = LICENSE_FILE_VERSION;
    root["createTime"] = createTime;

    Json::Value license;
    license["sn"] = sn;
    license["password"] = password;
    license["pwdHmac"] = pwdHmac;
    license["authorization"] = authorization;
    root["license"] = license;

    root["checksum"] = checksum;

    // 4. Write to temp file first (atomic write)
    std::string tempPath = filePath + ".tmp";
    {
        std::ofstream file(tempPath);
        if (!file.is_open()) return false;
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "    ";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(root, &file);
        if (!file.good()) {
            remove(tempPath.c_str());
            return false;
        }
    }

    // 5. Remove existing file and rename temp to target
    remove(filePath.c_str());
    if (rename(tempPath.c_str(), filePath.c_str()) != 0) {
        remove(tempPath.c_str());
        return false;
    }

    return true;
}

LicenseImportResult ImportLicenseFile(const std::string& filePath,
                                       LicenseFileData& outData,
                                       std::string& outError) {
    // 1. Check file exists and read content
    std::ifstream file(filePath);
    if (!file.is_open()) {
        outError = GetImportErrorMessage(LicenseImportResult::FileNotFound);
        return LicenseImportResult::FileNotFound;
    }

    // 2. Parse JSON using jsoncpp
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(file, root)) {
        file.close();
        outError = GetImportErrorMessage(LicenseImportResult::InvalidFormat);
        return LicenseImportResult::InvalidFormat;
    }
    file.close();

    // 3. Extract and validate magic
    std::string magic = root.get("magic", "").asString();
    if (magic != LICENSE_MAGIC) {
        outError = GetImportErrorMessage(LicenseImportResult::InvalidMagic);
        return LicenseImportResult::InvalidMagic;
    }

    // 4. Extract and validate version
    int version = root.get("version", 0).asInt();
    if (version <= 0) {
        outError = GetImportErrorMessage(LicenseImportResult::InvalidFormat);
        return LicenseImportResult::InvalidFormat;
    }
    if (version > LICENSE_FILE_VERSION) {
        outError = GetImportErrorMessage(LicenseImportResult::VersionTooHigh);
        return LicenseImportResult::VersionTooHigh;
    }

    // 5. Extract createTime
    std::string createTime = root.get("createTime", "").asString();

    // 6. Extract license object
    if (!root.isMember("license") || !root["license"].isObject()) {
        outError = GetImportErrorMessage(LicenseImportResult::InvalidFormat);
        return LicenseImportResult::InvalidFormat;
    }

    const Json::Value& license = root["license"];
    std::string sn = license.get("sn", "").asString();
    std::string password = license.get("password", "").asString();
    std::string pwdHmac = license.get("pwdHmac", "").asString();
    std::string authorization = license.get("authorization", "").asString();

    // 7. Check required fields
    if (sn.empty() || password.empty() || pwdHmac.empty()) {
        outError = GetImportErrorMessage(LicenseImportResult::IncompleteData);
        return LicenseImportResult::IncompleteData;
    }

    // 8. Verify checksum
    std::string storedChecksum = root.get("checksum", "").asString();
    std::string expectedContent = BuildChecksumContent(
        magic, version, createTime, sn, password, pwdHmac, authorization);
    std::string expectedChecksum = "sha256:" + hashSHA256(expectedContent);

    if (storedChecksum != expectedChecksum) {
        outError = GetImportErrorMessage(LicenseImportResult::ChecksumMismatch);
        return LicenseImportResult::ChecksumMismatch;
    }

    // 9. Validate SN
    SNMatchResult snResult = ValidateLicenseSN(sn);
    if (snResult == SNMatchResult::HardwareMismatch) {
        outError = GetImportErrorMessage(LicenseImportResult::SNMismatchHardware);
        return LicenseImportResult::SNMismatchHardware;
    } else if (snResult == SNMatchResult::IPMismatch) {
        outError = GetImportErrorMessage(LicenseImportResult::SNMismatchIP);
        return LicenseImportResult::SNMismatchIP;
    }

    // 10. Fill output data
    outData.sn = sn;
    outData.password = password;
    outData.pwdHmac = pwdHmac;
    outData.authorization = authorization;
    outData.createTime = createTime;
    outData.version = version;

    return LicenseImportResult::Success;
}

bool ApplyLicenseData(const LicenseFileData& data) {
    // Save to settings
    THIS_CFG.SetStr("settings", "SN", data.sn);
    THIS_CFG.SetStr("settings", "Password", data.password);
    THIS_CFG.SetStr("settings", "PwdHmac", data.pwdHmac);

    // Always set Authorization (clear old value if empty)
    THIS_CFG.SetStr("settings", "Authorization", data.authorization);

    return true;
}

std::string GetImportErrorMessage(LicenseImportResult result) {
    switch (result) {
    case LicenseImportResult::Success:
        return "";
    case LicenseImportResult::FileNotFound:
        return std::string(CT2A(_L("文件不存在")));
    case LicenseImportResult::InvalidFormat:
        return std::string(CT2A(_L("文件格式错误")));
    case LicenseImportResult::InvalidMagic:
        return std::string(CT2A(_L("不是有效的YAMA授权文件")));
    case LicenseImportResult::VersionTooHigh:
        return std::string(CT2A(_L("授权文件版本过高，请升级程序")));
    case LicenseImportResult::ChecksumMismatch:
        return std::string(CT2A(_L("授权文件已损坏或被篡改")));
    case LicenseImportResult::SNMismatchHardware:
        return std::string(CT2A(_L("此授权不适用于当前设备")));
    case LicenseImportResult::SNMismatchIP:
        return std::string(CT2A(_L("此授权不适用于当前公网IP")));
    case LicenseImportResult::IncompleteData:
        return std::string(CT2A(_L("授权信息不完整")));
    default:
        return std::string(CT2A(_L("未知错误")));
    }
}

#pragma once
#include <esp_log.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <driver/sdmmc_host.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <system_error>

#define TAG "SDCardFilesystem"

class SDCardFilesystem {
public:
    // Get the singleton instance
    static SDCardFilesystem& getInstance() {
        static SDCardFilesystem instance;
        return instance;
    }

    bool mount() {
        esp_err_t ret;

        // Options for mounting the filesystem.
        esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = false,
            .max_files = 5,
            .allocation_unit_size = 16 * 1024
        };

        // Use SDMMC peripheral
        sdmmc_host_t host = SDMMC_HOST_DEFAULT();

        // This initializes the slot without card detect (CD) and write protect (WP) signals.
        sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

        // Enable internal pullups on the SD card signals
        slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

        ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

        if (ret != ESP_OK) {
            if (ret == ESP_FAIL) {
                ESP_LOGE(TAG, "Failed to mount filesystem. "
                         "If you want the card to be formatted, set format_if_mount_failed = true.");
            } else {
                ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                         "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
            }
            return false;
        }

        // Card has been initialized, print its properties
        sdmmc_card_print_info(stdout, card);
        return true;
    }

    void unmount() {
        if (fs) {
            esp_vfs_fat_sdmmc_unmount();
            ESP_LOGI(TAG, "Card unmounted");
            fs = WL_INVALID_HANDLE;
            card = nullptr;
        }
    }

    bool writeFile(const std::string& path, const std::string& data) {
        std::ofstream file(path);
        if (!file.is_open()) {
            ESP_LOGE(TAG, "Failed to open file for writing: %s", path.c_str());
            return false;
        }
        file << data;
        if (file.fail()) {
            ESP_LOGE(TAG, "Failed to write data to file: %s", path.c_str());
            file.close();
            return false;
        }
        file.close();
        return true;
    }

    std::string readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            ESP_LOGE(TAG, "Failed to open file for reading: %s", path.c_str());
            return "";
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        if (file.fail() && !file.eof()) {
            ESP_LOGE(TAG, "Failed to read data from file: %s", path.c_str());
            file.close();
            return "";
        }
        file.close();
        return content;
    }

    bool deleteFile(const std::string& path) {
        if (remove(path.c_str()) != 0) {
            ESP_LOGE(TAG, "Failed to delete file: %s", path.c_str());
            return false;
        }
        return true;
    }

    // Append a line to the bottom of the file
    bool appendLine(const std::string& path, const std::string& line) {
        std::ofstream file(path, std::ios::app); // Open in append mode
        if (!file.is_open()) {
            ESP_LOGE(TAG, "Failed to open file for appending: %s", path.c_str());
            return false;
        }
        file << line << "\n"; // Append the line with a newline character
        if (file.fail()) {
            ESP_LOGE(TAG, "Failed to append data to file: %s (possibly out of space)", path.c_str());
            file.close();
            return false;
        }
        file.close();
        return true;
    }

    // Read the first line from the top of the file
    std::string readFirstLine(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            ESP_LOGE(TAG, "Failed to open file for reading: %s", path.c_str());
            return "";
        }
        std::string line;
        std::getline(file, line); // Read the first line
        if (file.fail() && !file.eof()) {
            ESP_LOGE(TAG, "Failed to read first line from file: %s", path.c_str());
            file.close();
            return "";
        }
        file.close();
        return line;
    }

    // Remove the first line from the file
    bool removeFirstLine(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            ESP_LOGE(TAG, "Failed to open file for reading: %s", path.c_str());
            return false;
        }

        // Read all lines except the first
        std::vector<std::string> lines;
        std::string line;
        std::getline(file, line); // Skip the first line
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        if (file.fail() && !file.eof()) {
            ESP_LOGE(TAG, "Failed to read lines from file: %s", path.c_str());
            file.close();
            return false;
        }
        file.close();

        // Write the remaining lines back to the file
        std::ofstream outFile(path);
        if (!outFile.is_open()) {
            ESP_LOGE(TAG, "Failed to open file for writing: %s", path.c_str());
            return false;
        }
        for (const auto& l : lines) {
            outFile << l << "\n";
            if (outFile.fail()) {
                ESP_LOGE(TAG, "Failed to write data to file: %s (possibly out of space)", path.c_str());
                outFile.close();
                return false;
            }
        }
        outFile.close();
        return true;
    }

private:
    SDCardFilesystem() : card(nullptr), fs(WL_INVALID_HANDLE) {}

    ~SDCardFilesystem() {
        unmount();
    }

    sdmmc_card_t* card;
    wl_handle_t fs;
};

void exampleMain() {
    SDCardFilesystem& sdcard = SDCardFilesystem::getInstance();

    if (sdcard.mount()) {
        std::string filePath = "/sdcard/test.txt";

        // Append lines to the file
        if (!sdcard.appendLine(filePath, "First line")) {
            ESP_LOGE(TAG, "Failed to append line to file");
        }
        if (!sdcard.appendLine(filePath, "Second line")) {
            ESP_LOGE(TAG, "Failed to append line to file");
        }
        if (!sdcard.appendLine(filePath, "Third line")) {
            ESP_LOGE(TAG, "Failed to append line to file");
        }

        // Read the first line
        std::string firstLine = sdcard.readFirstLine(filePath);
        if (firstLine.empty()) {
            ESP_LOGE(TAG, "Failed to read first line from file");
        } else {
            ESP_LOGI(TAG, "First line: %s", firstLine.c_str());
        }

        // Remove the first line
        if (!sdcard.removeFirstLine(filePath)) {
            ESP_LOGE(TAG, "Failed to remove first line from file");
        } else {
            ESP_LOGI(TAG, "First line removed");
        }

        // Read the updated first line
        firstLine = sdcard.readFirstLine(filePath);
        if (firstLine.empty()) {
            ESP_LOGE(TAG, "Failed to read new first line from file");
        } else {
            ESP_LOGI(TAG, "New first line: %s", firstLine.c_str());
        }

        sdcard.unmount();
    }
}
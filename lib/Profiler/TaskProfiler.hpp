#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <esp_partition.h>
#include <esp_heap_caps.h>
#include <map>
#include <string>
#include <chrono>

class TaskProfilerMonitor {
public:
    TaskProfilerMonitor();
    ~TaskProfilerMonitor();

    void start_profiling(TaskHandle_t task_handle, const std::string& task_name);
    void stop_profiling(TaskHandle_t task_handle);
    void print_results();

    // Differ utility for single task
    void save_single_task_snapshot(TaskHandle_t task_handle);
    void print_single_task_diff(TaskHandle_t task_handle);

    // Differ utility for overall application
    void save_app_snapshot();
    void print_app_diff();

    // Flash monitoring
    void print_flash_info();

    // Heap optimization
    void monitor_heap();
    void optimize_heap();

    // Stack optimization
    void monitor_stack(TaskHandle_t task_handle);
    void optimize_stack(TaskHandle_t task_handle);

private:
    struct TaskStats {
        std::string name;
        std::chrono::microseconds execution_time{0};
        uint32_t cpu_usage{0};
        uint32_t stack_high_water_mark{0};
        size_t heap_free{0}; // Free heap size at the time of profiling
    };

    std::map<TaskHandle_t, TaskStats> task_stats_map;
    std::map<TaskHandle_t, TaskStats> task_snapshot_map; // For single task diff
    TaskStats app_snapshot; // For overall app diff
    std::chrono::high_resolution_clock::time_point start_time;

    uint32_t calculate_cpu_usage(TaskHandle_t task_handle);
    uint32_t get_stack_high_water_mark(TaskHandle_t task_handle);
    size_t get_free_heap_size();

    void print_table_header();
    void print_table_row(const TaskStats& stats);
    void print_table_footer();
};
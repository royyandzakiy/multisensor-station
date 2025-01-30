#include <TaskProfiler.hpp>

static const char* TAG = "TaskProfilerMonitor";

TaskProfilerMonitor::TaskProfilerMonitor() {
    ESP_LOGI(TAG, "Initialized Task Profiler Monitor");
}

TaskProfilerMonitor::~TaskProfilerMonitor() {
    ESP_LOGI(TAG, "Destroyed Task Profiler Monitor");
}

void TaskProfilerMonitor::start_profiling(TaskHandle_t task_handle, const std::string& task_name) {
    if (task_stats_map.find(task_handle) == task_stats_map.end()) {
        TaskStats stats;
        stats.name = task_name;
        task_stats_map[task_handle] = stats;
    }
    start_time = std::chrono::high_resolution_clock::now();
}

void TaskProfilerMonitor::stop_profiling(TaskHandle_t task_handle) {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    if (task_stats_map.find(task_handle) != task_stats_map.end()) {
        task_stats_map[task_handle].execution_time = duration;
        task_stats_map[task_handle].cpu_usage = calculate_cpu_usage(task_handle);
        task_stats_map[task_handle].stack_high_water_mark = get_stack_high_water_mark(task_handle);
        task_stats_map[task_handle].heap_free = get_free_heap_size();
    }
}

void TaskProfilerMonitor::print_results() {
    print_table_header();

    for (const auto& [handle, stats] : task_stats_map) {
        print_table_row(stats);
    }

    print_table_footer();

    // Print flash information
    print_flash_info();
}

uint32_t TaskProfilerMonitor::calculate_cpu_usage(TaskHandle_t task_handle) {
    TaskStatus_t task_status;
    vTaskGetInfo(task_handle, &task_status, pdTRUE, eRunning);
    return task_status.ulRunTimeCounter / 1000; // Convert to percentage
}

uint32_t TaskProfilerMonitor::get_stack_high_water_mark(TaskHandle_t task_handle) {
    return uxTaskGetStackHighWaterMark(task_handle);
}

size_t TaskProfilerMonitor::get_free_heap_size() {
    return esp_get_free_heap_size();
}

void TaskProfilerMonitor::print_table_header() {
    ESP_LOGI(TAG, "+----------------------+----------------+----------------+----------------+----------------+");
    ESP_LOGI(TAG, "| Task Name            | Exec Time (us) | CPU Usage (%%)  | Stack Free (B) | Heap Free (B)  |");
    ESP_LOGI(TAG, "+----------------------+----------------+----------------+----------------+----------------+");
}

void TaskProfilerMonitor::print_table_row(const TaskStats& stats) {
    ESP_LOGI(TAG, "| %-20s | %-14lld | %-14lu | %-14lu | %-14zu |",
             stats.name.c_str(), stats.execution_time.count(), stats.cpu_usage, stats.stack_high_water_mark, stats.heap_free);
}

void TaskProfilerMonitor::print_table_footer() {
    ESP_LOGI(TAG, "+----------------------+----------------+----------------+----------------+----------------+");
}

// Differ utility for single task
void TaskProfilerMonitor::save_single_task_snapshot(TaskHandle_t task_handle) {
    if (task_stats_map.find(task_handle) != task_stats_map.end()) {
        task_snapshot_map[task_handle] = task_stats_map[task_handle];
    }
}

void TaskProfilerMonitor::print_single_task_diff(TaskHandle_t task_handle) {
    if (task_stats_map.find(task_handle) == task_stats_map.end() ||
        task_snapshot_map.find(task_handle) == task_snapshot_map.end()) {
        ESP_LOGE(TAG, "No snapshot found for the specified task");
        return;
    }

    const TaskStats& current = task_stats_map[task_handle];
    const TaskStats& snapshot = task_snapshot_map[task_handle];

    ESP_LOGI(TAG, "Task: %s", current.name.c_str());
    ESP_LOGI(TAG, "  Execution Time Diff: %lld us", current.execution_time.count() - snapshot.execution_time.count());
    ESP_LOGI(TAG, "  CPU Usage Diff: %lu%%", current.cpu_usage - snapshot.cpu_usage);
    ESP_LOGI(TAG, "  Stack Free Diff: %lu bytes", current.stack_high_water_mark - snapshot.stack_high_water_mark);
    ESP_LOGI(TAG, "  Heap Free Diff: %zu bytes", current.heap_free - snapshot.heap_free);
}

// Differ utility for overall application
void TaskProfilerMonitor::save_app_snapshot() {
    app_snapshot = TaskStats{};
    for (const auto& [handle, stats] : task_stats_map) {
        app_snapshot.execution_time += stats.execution_time;
        app_snapshot.cpu_usage += stats.cpu_usage;
        app_snapshot.stack_high_water_mark += stats.stack_high_water_mark;
        app_snapshot.heap_free = stats.heap_free; // Use the latest heap value
    }
}

void TaskProfilerMonitor::print_app_diff() {
    TaskStats current{};
    for (const auto& [handle, stats] : task_stats_map) {
        current.execution_time += stats.execution_time;
        current.cpu_usage += stats.cpu_usage;
        current.stack_high_water_mark += stats.stack_high_water_mark;
        current.heap_free = stats.heap_free; // Use the latest heap value
    }

    ESP_LOGI(TAG, "Application Diff:");
    ESP_LOGI(TAG, "  Execution Time Diff: %lld us", current.execution_time.count() - app_snapshot.execution_time.count());
    ESP_LOGI(TAG, "  CPU Usage Diff: %lu%%", current.cpu_usage - app_snapshot.cpu_usage);
    ESP_LOGI(TAG, "  Stack Free Diff: %lu bytes", current.stack_high_water_mark - app_snapshot.stack_high_water_mark);
    ESP_LOGI(TAG, "  Heap Free Diff: %zu bytes", current.heap_free - app_snapshot.heap_free);
}

// Flash monitoring
void TaskProfilerMonitor::print_flash_info() {
    const esp_partition_t* partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
    if (partition == nullptr) {
        ESP_LOGE(TAG, "Failed to find application partition");
        return;
    }

    size_t total_flash_size = partition->size;
    size_t used_flash_size = partition->address + partition->size - partition->size;
    size_t free_flash_size = total_flash_size - used_flash_size;

    ESP_LOGI(TAG, "Flash Info:");
    ESP_LOGI(TAG, "  Total Flash Size: %zu bytes", total_flash_size);
    ESP_LOGI(TAG, "  Used Flash Size: %zu bytes", used_flash_size);
    ESP_LOGI(TAG, "  Free Flash Size: %zu bytes", free_flash_size);
}

// Heap optimization
void TaskProfilerMonitor::monitor_heap() {
    size_t free_heap = esp_get_free_heap_size();
    ESP_LOGI(TAG, "Free Heap Size: %zu bytes", free_heap);

    heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);
    ESP_LOGI(TAG, "Largest Free Block: %zu bytes", info.largest_free_block);
}

void TaskProfilerMonitor::optimize_heap() {
    ESP_LOGI(TAG, "Optimizing Heap...");
    // Example: Minimize dynamic allocations by using static memory pools
    ESP_LOGI(TAG, "Avoid frequent malloc/free calls. Use static or pre-allocated memory pools.");
}

// Stack optimization
void TaskProfilerMonitor::monitor_stack(TaskHandle_t task_handle) {
    UBaseType_t stack_high_water_mark = uxTaskGetStackHighWaterMark(task_handle);
    ESP_LOGI(TAG, "Stack High Water Mark: %u bytes", stack_high_water_mark * sizeof(StackType_t));
}

void TaskProfilerMonitor::optimize_stack(TaskHandle_t task_handle) {
    ESP_LOGI(TAG, "Optimizing Stack...");
    // Example: Adjust stack size based on high water mark
    UBaseType_t stack_high_water_mark = uxTaskGetStackHighWaterMark(task_handle);
    size_t required_stack_size = stack_high_water_mark * sizeof(StackType_t) * 2; // Add some margin
    ESP_LOGI(TAG, "Adjust stack size to: %zu bytes", required_stack_size);
}
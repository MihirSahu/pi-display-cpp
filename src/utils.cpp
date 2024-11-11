#include <curl/curl.h>
#include <string>
#include "json.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <sstream>

using json = nlohmann::json;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  //((std::string*)userdata)->push_back(*ptr);
  ((std::string*)userdata)->append(ptr, size * nmemb);

  return size * nmemb;
}

std::string get_temperature() {
  CURLcode ret;
  CURL *hnd;
  std::string response;

  hnd = curl_easy_init();
  curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
  curl_easy_setopt(hnd, CURLOPT_URL, "https://api.tomorrow.io/v4/weather/realtime?location=29.734063834504845%2C%20-95.48287469505175&units=imperial&apikey={key here}");
  curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/8.7.1");
  curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
  curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
  curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

  curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &response);

  ret = curl_easy_perform(hnd);

  if (ret != CURLE_OK) {
    response = "CURL error: " + std::string(curl_easy_strerror(ret));
  }

  curl_easy_cleanup(hnd);
  hnd = NULL;

  response.push_back('\0');
  std::string temperature = (json::parse(response)["data"]).dump();
  temperature = (json::parse(temperature)["values"]).dump();
  temperature = (json::parse(temperature)["temperature"]).dump();

  return temperature;
}

std::string get_cat_fact() {
  CURLcode ret;
  CURL *hnd;
  std::string response;

  hnd = curl_easy_init();
  curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
  curl_easy_setopt(hnd, CURLOPT_URL, "https://catfact.ninja/fact");
  curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/8.7.1");
  curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
  curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
  curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

  curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &response);

  ret = curl_easy_perform(hnd);

  if (ret != CURLE_OK) {
    response = "CURL error: " + std::string(curl_easy_strerror(ret));
  }

  curl_easy_cleanup(hnd);
  hnd = NULL;

  response.push_back('\0');
  return json::parse(response)["fact"].dump();
}

// Parse CPU statistics from /proc/stat and calculate CPU usage
std::string getCPUUsage() {
    std::ifstream file("/proc/stat");
    std::string line;
    std::getline(file, line);  // Read the first line

    // Parse the CPU stats
    std::istringstream iss(line);
    std::string cpu;
    long user, nice, system, idle;
    iss >> cpu >> user >> nice >> system >> idle;

    static long prev_user = 0, prev_nice = 0, prev_system = 0, prev_idle = 0;
    long delta_user = user - prev_user;
    long delta_nice = nice - prev_nice;
    long delta_system = system - prev_system;
    long delta_idle = idle - prev_idle;

    prev_user = user;
    prev_nice = nice;
    prev_system = system;
    prev_idle = idle;

    // Calculate usage as the fraction of non-idle CPU time
    double total = delta_user + delta_nice + delta_system + delta_idle;
    return std::to_string((delta_user + delta_nice + delta_system) / total * 100.0);
}

std::string getMemoryUsage() {
    std::ifstream file("/proc/meminfo");
    std::string line;
    long total_memory = 0, free_memory = 0;

    while (std::getline(file, line)) {
        if (line.find("MemTotal:") == 0) {
            total_memory = std::stol(line.substr(line.find_first_of("0123456789")));
        } else if (line.find("MemAvailable:") == 0) {
            free_memory = std::stol(line.substr(line.find_first_of("0123456789")));
            break;
        }
    }

    return std::to_string(((total_memory - free_memory) / (double)total_memory) * 100.0);
}

std::string getUptime() {
    std::ifstream file("/proc/uptime");
    double uptime;
    file >> uptime;
    std::cout << "uptime: " << uptime << std::endl;
    return std::to_string(uptime);
}

std::string getCPUTemperature() {
    std::ifstream file("/sys/class/thermal/thermal_zone0/temp");
    double temperature;
    file >> temperature;
    return std::to_string(temperature / 1000.0);  // Convert from millidegree to degree
}
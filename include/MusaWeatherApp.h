#ifndef MUSAWEATHERAPP_H
#define MUSAWEATHERAPP_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <atomic>
#include <mutex>
#include <thread>
#include <map>
#include "imgui.h"
#include <imgui/backend/imgui_impl_glfw.h>
#include <imgui/backend/imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <json.hpp>
#include <httplib.h>

// Constants: These define constant values used throughout the program.
extern const std::string base_url;
extern const std::string favorites_file;
extern std::string api_key;

// Struct Definition: Defines a data structure to hold information about a city.
struct City {
    std::string name;
    double lon;
    double lat;
    bool selected;
    nlohmann::json weatherData;
};

// Initial List of Cities
extern std::vector<City> cities;

// Global Variables for Threading
extern std::mutex weatherDataMutex;
extern std::atomic<int> threadsFinished;

// Function Prototypes
std::string readApiKeyFromFile(const std::string& filePath);
void fetchWeatherDataForCity(City& city);
bool validateCity(const std::string& cityName, double& lon, double& lat);
void loadFavorites(std::vector<City>& cities, std::set<std::string>& favorites);
void saveFavorites(const std::set<std::string>& favorites);
void addFavorites(const std::vector<City>& cities, std::set<std::string>& favorites);
void removeFavorites(const std::vector<City>& cities, std::set<std::string>& favorites);
std::vector<City> filterFavorites(const std::vector<City>& cities, const std::set<std::string>& favorites);
std::string unixToHHMM(int unixTime);
void uncheckAllCities(std::vector<City>& cities);
void addNewPlace(const std::string& cityName); // Updated function to prevent duplicates

#endif // MUSAWEATHERAPP_H

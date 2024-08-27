#include "MusaWeatherApp.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <map>  // Include map to store icons

int main(int, char**) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Musa's Weather Channel", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Load and set the window icon
    int iconWidth, iconHeight, iconChannels;
    unsigned char* iconPixels = stbi_load("assets/weather_icon.png", &iconWidth, &iconHeight, &iconChannels, STBI_rgb_alpha);
    if (iconPixels) {
        GLFWimage images[1];
        images[0].width = iconWidth;
        images[0].height = iconHeight;
        images[0].pixels = iconPixels;
        glfwSetWindowIcon(window, 1, images);
        stbi_image_free(iconPixels);
    }
    else {
        std::cerr << "Failed to load icon image" << std::endl;
    }

    // Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Customizable Style
    ImGui::StyleColorsDark(); // Using Dark Theme as Default
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 5.0f;
    style.Colors[ImGuiCol_WindowBg].w = 0.9f;
    style.WindowPadding = ImVec2(20.0f, 20.0f); // Add padding to the window

    // Setup ImGui binding for GLFW and OpenGL
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Read API key from file
    api_key = readApiKeyFromFile("assets/key.txt");

    // Variables to manage application state
    bool showWeatherPopup = false;
    bool showAddPlacePopup = false;
    bool showWarningPopup = false;
    bool showNoSelectionPopup = false;
    std::vector<std::thread> threads;
    std::set<std::string> favorites;
    std::map<std::string, bool> selectedFavorites; // Map to track selection in My List
    loadMyCityList(cities, favorites); // Load favorite cities from file
    char cityNameBuffer[128] = ""; // Buffer for new city input
    char addCityBuffer[128] = "";  // Buffer for the "Add Place" popup
    bool selectAllCities = false;
    bool selectAllFavorites = false;

    // Load weather icons only once and reuse them
    std::map<std::string, GLuint> weatherIcons;
    const std::map<std::string, std::string> weatherIconPaths = {
        {"Clear", "assets/sunny.png"},
        {"Clouds", "assets/cloudy.png"},
        {"Rain", "assets/rainy.png"}
    };

    auto loadIcon = [&](const std::string& iconPath) -> GLuint {
        int width, height, channels;
        unsigned char* data = stbi_load(iconPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (data) {
            GLuint textureID;
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            stbi_image_free(data);
            return textureID;
        }
        return 0;
        };

    for (const auto& icon : weatherIconPaths) {
        GLuint iconID = loadIcon(icon.second);
        if (iconID != 0) {
            weatherIcons[icon.first] = iconID;
        }
        else {
            std::cerr << "Failed to load icon: " << icon.second << std::endl;
        }
    }

    // Function to add a random city using the weather API
    auto addRandomCity = [&]() { // lambda function
        // Generate random latitude and longitude
        double randomLat = (static_cast<double>(rand()) / RAND_MAX) * 180.0 - 90.0;  // Latitude between -90 and 90
        double randomLon = (static_cast<double>(rand()) / RAND_MAX) * 360.0 - 180.0; // Longitude between -180 and 180

        // Use the OpenWeatherMap API to get a city near these random coordinates
        httplib::Client cli("http://api.openweathermap.org");
        std::string url = "/geo/1.0/reverse?lat=" + std::to_string(randomLat) + "&lon=" + std::to_string(randomLon) + "&limit=1&appid=" + api_key;

        auto res = cli.Get(url.c_str());
        if (res && res->status == 200) {
            auto cityList = nlohmann::json::parse(res->body);
            if (!cityList.empty()) {
                std::string cityName = cityList[0]["name"];
                double lon = cityList[0]["lon"];
                double lat = cityList[0]["lat"];

                // Check if the city is already in the main list or My List
                bool cityExists = std::any_of(cities.begin(), cities.end(), [&](const City& c) {
                    return c.name == cityName;
                    }) || favorites.find(cityName) != favorites.end();

                // If the city is unique, add it to the list
                if (!cityExists) {
                    cities.push_back({ cityName, lon, lat, false, nullptr });
                }
                else {
                    std::cerr << "City " << cityName << " is already in the list." << std::endl;
                }
            }
            else {
                std::cerr << "No city found at the random coordinates." << std::endl;
            }
        }
        else {
            std::cerr << "Failed to fetch random city from API" << std::endl;
        }
        };

    // Main application loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); // Process all pending events

        // Start a new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Retrieve the size of the GLFW window
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // Fit ImGui window to GLFW window size with padding
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(display_w), static_cast<float>(display_h)));

        // Create ImGui window with padding
        ImGui::Begin("Musa's Weather Channel", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        // Header: Application title with padding
        ImGui::Text("Enjoy exploring the weather !!");
        ImGui::Separator();

        // Layout: 3 Columns with padding
        ImGui::Columns(3, NULL, false);

        // Column 1: City selection list (excluding those in My List)
        ImGui::Text("Select cities to get weather:");
        if (ImGui::Checkbox("Select All Cities", &selectAllCities)) {
            for (auto& city : cities) {
                if (favorites.find(city.name) == favorites.end()) {
                    city.selected = selectAllCities;
                }
            }
        }
        ImGui::BeginChild("City Selection", ImVec2(0, display_h * 0.7f), true);  // Limit height to avoid scrolling
        for (auto& city : cities) {
            if (favorites.find(city.name) == favorites.end()) {  // Only show cities not in My List
                ImGui::Checkbox(city.name.c_str(), &city.selected);
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), city.selected ? "Selected" : " ");
            }
        }
        ImGui::EndChild();

        // Column 2: My List (Favorites) with selection for weather fetching or removal
        ImGui::NextColumn();
        ImGui::Text("My List (Can not get weather from here):");
        if (ImGui::Checkbox("Select All MyList", &selectAllFavorites)) {
            for (auto& fav : favorites) {
                selectedFavorites[fav] = selectAllFavorites;
            }
        }
        ImGui::BeginChild("My List", ImVec2(0, display_h * 0.7f), true);  // Limit height to avoid scrolling
        for (std::set<std::string>::iterator it = favorites.begin(); it != favorites.end(); ++it) {
            bool selected = selectedFavorites[*it]; // Track selection state in the map
            if (ImGui::Checkbox(it->c_str(), &selected)) {
                selectedFavorites[*it] = selected; // Update the selection state
                if (selected) {
                    auto itCity = std::find_if(cities.begin(), cities.end(), [&](City& city) {
                        return city.name == *it;
                        });
                    if (itCity == cities.end()) {
                        cities.push_back({ *it, 0.0, 0.0, true, nullptr });
                    }
                }
            }
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), selectedFavorites[*it] ? "Selected" : " ");
        }
        ImGui::EndChild();

        // Column 3: Action Buttons
        ImGui::NextColumn();

        // Calculate button size and spacing with padding
        int buttonCount = 6; // Number of buttons
        float totalButtonHeight = 40.0f * buttonCount;
        float availableHeight = (display_h * 0.7f); // Adjusted for padding
        float buttonSpacing = (availableHeight - totalButtonHeight) / (buttonCount - 1);
        ImVec2 buttonSize = ImVec2(ImGui::GetContentRegionAvail().x, 40.0f);

        // Set button colors and render buttons with padding
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));  // Blue
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.3f, 0.7f, 1.0f));
        if (ImGui::Button("See Weather", buttonSize)) {
            // Check if any city is selected in either list
            bool anySelected = std::any_of(cities.begin(), cities.end(), [](const City& city) {
                return city.selected;
                }) || std::any_of(selectedFavorites.begin(), selectedFavorites.end(), [](const std::pair<std::string, bool>& pair) {
                    return pair.second;
                    });

                // Show warning if no city is selected
                if (!anySelected) {
                    showNoSelectionPopup = true;
                }
                else {
                    showWeatherPopup = true;
                    threadsFinished = 0;
                    for (auto& city : cities) {
                        city.weatherData = nullptr; // Clear previous weather data
                    }
                    // Fetch weather for cities in both main list and My List
                    for (auto& city : cities) {
                        if (city.selected) {
                            threads.emplace_back(getWeatherDataForEach, std::ref(city)); // Fetch weather data in separate threads
                        }
                    }
                    for (auto& fav : favorites) {
                        if (selectedFavorites[fav]) {
                            auto itCity = std::find_if(cities.begin(), cities.end(), [&](City& city) {
                                return city.name == fav;
                                });
                            if (itCity != cities.end()) {
                                threads.emplace_back(getWeatherDataForEach, std::ref(*itCity));
                            }
                        }
                    }
                    uncheckAllCities(cities); // Uncheck all cities after fetching data
                    for (auto& fav : selectedFavorites) {
                        fav.second = false; // Uncheck all cities in My List after fetching data
                    }
                }
        }
        ImGui::PopStyleColor(3);  // Revert button color changes
        ImGui::Dummy(ImVec2(0.0f, buttonSpacing));  // Add spacing

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.4f, 1.0f));  // Green
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.3f, 1.0f));
        if (ImGui::Button("Add to My List", buttonSize)) {
            std::vector<City> toRemove;
            for (auto& city : cities) {
                if (city.selected && favorites.find(city.name) == favorites.end()) {
                    favorites.insert(city.name);
                    toRemove.push_back(city);
                    selectedFavorites[city.name] = false; // Initialize the selection state for My List
                }
            }
            // Remove cities added to My List from the main city list
            for (auto& city : toRemove) {
                cities.erase(std::remove_if(cities.begin(), cities.end(), [&](const City& c) {
                    return c.name == city.name;
                    }), cities.end());
            }
            saveMyCityList(favorites);
            uncheckAllCities(cities);
        }
        ImGui::PopStyleColor(3);
        ImGui::Dummy(ImVec2(0.0f, buttonSpacing));  // Add spacing

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));  // Red
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("Remove from My List", buttonSize)) {
            std::vector<std::string> toRemove;
            for (std::map<std::string, bool>::iterator it = selectedFavorites.begin(); it != selectedFavorites.end(); ++it) {
                if (it->second) {
                    toRemove.push_back(it->first);
                }
            }
            // Remove cities from My List and add back to main city list
            for (std::vector<std::string>::iterator it = toRemove.begin(); it != toRemove.end(); ++it) {
                favorites.erase(*it);
                // Check if city is already in the main list before adding
                if (std::find_if(cities.begin(), cities.end(), [&](const City& city) {
                    return city.name == *it;
                    }) == cities.end()) {
                    cities.push_back({ *it, 0.0, 0.0, false, nullptr });  // Re-add to main city list
                }
                selectedFavorites.erase(*it); // Remove from the selection state map
            }
            saveMyCityList(favorites);
        }
        ImGui::PopStyleColor(3);
        ImGui::Dummy(ImVec2(0.0f, buttonSpacing));  // Add spacing

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.6f, 0.2f, 1.0f));  // Orange
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.7f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.5f, 0.1f, 1.0f));
        if (ImGui::Button("Add Place", buttonSize)) {
            showAddPlacePopup = true;
            strcpy(addCityBuffer, ""); // Clear buffer
        }
        ImGui::PopStyleColor(3);
        ImGui::Dummy(ImVec2(0.0f, buttonSpacing));  // Add spacing

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.4f, 0.8f, 1.0f));  // Purple
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.5f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.3f, 0.7f, 1.0f));
        if (ImGui::Button("Add Random City", buttonSize)) {
            addRandomCity();
        }
        ImGui::PopStyleColor(3);
        ImGui::Dummy(ImVec2(0.0f, buttonSpacing));  // Add spacing

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));  // Dark Red
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.0f, 0.0f, 1.0f));
        if (ImGui::Button("Delete City", buttonSize)) {
            std::vector<std::string> toDelete;
            bool canDelete = true;
            for (auto& city : cities) {
                if (city.selected) {
                    if (favorites.find(city.name) != favorites.end()) {
                        canDelete = false; // Found a city in MyList that cannot be deleted
                        break;
                    }
                    toDelete.push_back(city.name);
                }
            }
            if (canDelete) {
                for (auto& cityName : toDelete) {
                    cities.erase(std::remove_if(cities.begin(), cities.end(), [&](const City& city) {
                        return city.name == cityName;
                        }), cities.end());
                }
            }
            else {
                showWarningPopup = true;
            }
        }
        ImGui::PopStyleColor(3);

        // Handle fetching weather data in background threads
        if (showWeatherPopup) {
            if (threadsFinished == threads.size()) {
                for (auto& thread : threads) {
                    if (thread.joinable()) {
                        thread.join(); // Wait for all threads to finish
                    }
                }
                threads.clear();
                showWeatherPopup = false;

                // Show a popup window with the weather data
                ImGui::OpenPopup("Weather Data");
            }
        }

        // Popup window to display weather data
        if (ImGui::BeginPopupModal("Weather Data", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            for (auto& city : cities) {
                if (!city.weatherData.is_null()) {
                    // Determine the weather icon based on the weather type
                    const std::string& weatherType = city.weatherData["weather"][0]["main"].get<std::string>();
                    GLuint weatherIcon = weatherIcons[weatherType];

                    if (weatherIcon) {
                        // Ensure all icons are displayed with the same size
                        ImGui::Image((void*)(intptr_t)weatherIcon, ImVec2(64, 64));  // Fixed size of 64x64 pixels
                    }

                    ImGui::Text("%s:", city.name.c_str());
                    ImGui::Text("Weather: %s", city.weatherData["weather"][0]["description"].get<std::string>().c_str());
                    ImGui::Text("Temperature: %.2f°C", city.weatherData["main"]["temp"].get<double>() - 273.15);
                    ImGui::Text("Humidity: %d%%", city.weatherData["main"]["humidity"].get<int>());
                    ImGui::Text("Wind Speed: %.2f m/s", city.weatherData["wind"]["speed"].get<double>());
                    ImGui::Text("Sunrise: %s", unixToHHMM(city.weatherData["sys"]["sunrise"].get<int>()).c_str());
                    ImGui::Text("Sunset: %s", unixToHHMM(city.weatherData["sys"]["sunset"].get<int>()).c_str());
                    ImGui::Separator();
                }
            }
            if (ImGui::Button("Close", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Popup window for warnings (e.g., cannot delete city in MyList)
        if (showWarningPopup) {
            ImGui::OpenPopup("Warning");
            showWarningPopup = false;
        }

        if (ImGui::BeginPopupModal("Warning", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Cannot delete a city that is in 'My List'.");
            if (ImGui::Button("Close", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Popup window for no selection warning
        if (showNoSelectionPopup) {
            ImGui::OpenPopup("No Cities Selected");
            showNoSelectionPopup = false;
        }

        if (ImGui::BeginPopupModal("No Cities Selected", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Please select at least one city to see the weather.");
            if (ImGui::Button("Close", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Popup window for adding a new place
        if (showAddPlacePopup) {
            ImGui::OpenPopup("Add New Place");
            showAddPlacePopup = false;
        }

        if (ImGui::BeginPopupModal("Add New Place", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter the name of the city to add:");
            ImGui::InputText("##AddCityName", addCityBuffer, sizeof(addCityBuffer));

            if (ImGui::Button("Add", ImVec2(120, 0))) {
                addNewPlace(addCityBuffer);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::End(); // End the main window

        // Render the ImGui frame
        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.25f, 0.25f, 0.30f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window); // Swap front and back buffers
    }

    // Clean up and terminate the application
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}


# 🌦️ MusaWeatherApp

**MusaWeatherApp** is a weather forecast application that allows users to view weather data for cities worldwide. The application is built using C++, OpenGL, and ImGui for the user interface, and it fetches weather data from the OpenWeatherMap API.

## ✨ Features

- **City Selection**: Choose from a list of predefined cities or add new cities manually.
- **My List**: Save your favorite cities to "My List" for quick access.
- **Weather Data**: Fetch and display weather data, including temperature, humidity, wind speed, and more.
- **Custom Layout**: A modern and customizable UI layout with evenly spaced buttons and adjustable window size.
- **Weather Icons**: Display weather icons corresponding to the current weather conditions.
- **Add Random City**: Automatically add a random city based on geographical coordinates.
- **Duplicate Prevention**: Ensure that no city is added more than once.
- **Multi-City Weather Display**: View weather information for cities from both the main list and "My List" simultaneously.

## 🛠️ Prerequisites

Before you begin, ensure you have the following software installed on your machine:

- **C++ Compiler**: Compatible with C++17 or later.
- **CMake**: For building the project.
- **OpenGL**: Graphics library for rendering the UI.
- **GLFW**: Library for handling window creation and input.
- **GLEW**: OpenGL Extension Wrangler Library.
- **ImGui**: Immediate Mode GUI library.

## 🚀 Linux Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/yourusername/MusaWeatherApp.git
   cd MusaWeatherApp
   ```

2. **Build the Project**:
   - Ensure CMake is installed.
   - Run the following commands:
     ```bash
     mkdir build
     cd build
     cmake ..
     make
     ```

3. **Run the Application**:
   - After the build is complete, you can run the application using:
     ```bash
     ./MusaWeatherApp
     ```


## 🚀 Linux Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/yourusername/MusaWeatherApp.git
   cd MusaWeatherApp
   ```

2. **Run the Solution**:
   - After cloning the repo, access the main folder and loate **MusaWeatherApp.sln**:
   - Run the Solution

3. **Notes**:
   - Please make sure you have **Visual Studio Code 2022** installed
   - Please note that this project could be incompatable with C++ 17



## 💻 Usage

1. **Adding Cities**:
   - Use the **Add Place** button to add a city by name.
   - Add random cities using the **Add Random City** button.

2. **Viewing Weather**:
   - Select cities from either the main list or "My List" and click **See Weather** to fetch and display weather data.

3. **Managing My List**:
   - Add selected cities to "My List" using the **Add to My List** button.
   - Remove cities from "My List" with the **Remove from My List** button.

4. **Deleting Cities**:
   - Use the **Delete City** button to remove cities from the main list. Note that cities in "My List" cannot be deleted.

## ⚙️ Configuration

### API Key

The application requires an API key from OpenWeatherMap to fetch weather data. Ensure your API key is stored in the `assets/key.txt` file.

- **API Key Format**: 
  ```
  your_openweathermap_api_key
  ```

## 📁 File Structure

- **`src/`**: Contains the source code for the application.
- **`assets/`**: Contains resources such as icons and the API key file.
- **`build/`**: Directory for the compiled binaries.
- **`CMakeLists.txt`**: CMake configuration file.
- **`README.md`**: Project documentation.

## 🤝 Contributing

Contributions are welcome! Please fork the repository and create a pull request with your changes.


## 🤝 ScreenShots

![Alt text](assets/Capture.jpg?raw=true "App ScreenSHot")

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [ImGui](https://github.com/ocornut/imgui) - Immediate Mode GUI library.
- [OpenWeatherMap](https://openweathermap.org/) - Weather API.
- [GLFW](https://www.glfw.org/) - Multi-platform library for OpenGL, OpenGL ES, and Vulkan development on the desktop.

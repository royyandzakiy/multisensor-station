# Unit Testing on Platform IO based ESP IDF

This codebase is a simple example on how to do automated testing directly in PlatformIO, for ESP IDF based development.

> The `main.c` is there to emphasize that it is not used at all in making Unit Tests. This is due to the spirit of testing is to test each individual component, regardless of the main logic. Hence, here we want to test out AppLight.hpp, which consists of an abstraction of the LED_BUILTIN in which by using the DOIT ESP32 DEVKIT V1, it will be on GPIO2.

## Get Started
- Install VSCode
- Install Platform IO (PIO) Extension in VSCode
- Open the platform.ini file, let PIO to load and install the required platform
    - This will take time, because (if not yet) you need to download the whole ESP IDF SDK, through the choice of platform, in this case is `espressif32@5.3.0`. I had specifically specified the version 5.3.0, because I tried the current latest version, which is 6.2.0, and it keeps on failing to compile with a certain error. Hence, I specified an older version which actually works
    - After the PIO Task of Downloading the platform is done, usually it does another task, to "Configure project"
- Open the PlatformIO tab, the image like below shall show up
    ![](docs/project-tasks.png)
    - `General` > `Build` Just wait until it successfully to Builds
        ![](docs/success-build.png)
    - `Advanced` > `Test`
        -  Just wait until this text shows up
        ```
            Testing...
            If you don't see any output for the first 10 secs, please reset board (press reset button)
        ```
        - Press the reset button on the ESP32, then the Unit Test will run, the `LED_BUILTIN` will flash really briefly, and this final success test message will popup
        ![](docs/success-test.png)

---
## Disclaimer
I did not write this code by my own, it is from [PacktPublishing - Developing IoT Projects with ESP32 2nd edition](https://github.com/PacktPublishing/Developing-IoT-Projects-with-ESP32-2nd-edition) :octocat:. I made minor changes mainly to divert from the initial target which is directed for ESP32S3 to a ESP32
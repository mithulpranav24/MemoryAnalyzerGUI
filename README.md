# Memory Analyzer for Linux


A graphical, real-time system and process monitor for Linux, built with C++ and Qt. This application provides a multi-tabbed interface to view detailed hardware information and live process data.

---

## Features

* **Detailed System Overview**: Displays comprehensive hardware information, including:
    * **CPU**: Model name, core/thread count, and L1/L2/L3 cache sizes.
    * **GPU**: Lists all detected graphics controllers (both integrated and discrete).
    * **RAM**: Shows total installed memory, currently available memory, and (when run with `sudo`) detailed information like RAM type, speed, and populated slot count.
* **Real-time Process Monitor**: A live, auto-updating table of all running processes, sorted by memory usage.
    * **Search/Filter Bar**: Instantly filter the process list by name (case-insensitive).
    * **Resizable Columns**: Adjust the column widths to your preference.
* **Process Inspector**: Look up a single process by its PID to see its memory usage, or compare the memory usage of two different processes.
* **Threshold Alert**: Set a custom memory usage percentage (e.g., 80%). The application will show a desktop notification if system memory usage exceeds this threshold.
* **Save Report**: Generate and save a full system report, including hardware specs and a snapshot of all running processes, to a text file.
* **Modern UI**: A clean, multi-page user interface with a sidebar and icons, built programmatically with C++ and Qt.

---

## How to Run (Quick Start with AppImage)

The easiest way to run this application is by downloading the pre-packaged AppImage from the latest release.

**1. Download**

Go to the [**Releases Page**](https://github.com/mithulpranav24/MemoryAnalyzerGUI/releases/latest) and download the `Memory_Analyzer-x86_64.AppImage` file from the "Assets" section.

**2. Make it Executable**

Open your terminal, navigate to the directory where you downloaded the file (e.g., `cd ~/Downloads`), and run the following command:
```bash
chmod +x Memory_Analyzer-x86_64.AppImage
```

**3. Run the Application**

You can now run the application from the terminal:
```bash
./Memory_Analyzer-x86_64.AppImage
```

**Important:** To view detailed RAM information (Type, Speed, Slots), you must run the application with `sudo`:
```bash
sudo ./Memory_Analyzer-x86_64.AppImage
```

---



## License

This project is licensed under the MIT License.

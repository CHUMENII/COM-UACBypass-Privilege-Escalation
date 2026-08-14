# COM BypassUAC - UAC Bypass Implementation

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](https://www.microsoft.com/windows)
[![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)](https://isocpp.org/)

A Windows UAC (User Account Control) bypass implementation using COM (Component Object Model) elevation monikers. This project demonstrates how to leverage the `ICMLuaUtil` interface to execute processes with elevated privileges without triggering the standard UAC prompt.

## Table of Contents

- [Overview](#overview)
- [How It Works](#how-it-works)
- [Architecture](#architecture)
- [Project Structure](#project-structure)
- [Building the Project](#building-the-project)
- [Usage](#usage)
- [Technical Details](#technical-details)
- [Limitations](#limitations)
- [License](#license)

## Overview

This project implements a UAC bypass technique that exploits Windows COM elevation monikers. Instead of using traditional methods that trigger UAC prompts, this implementation uses the undocumented `ICMLuaUtil` COM interface to execute processes with administrator privileges.

### Features

- **Silent Elevation**: Executes processes with elevated privileges without user interaction
- **COM-Based**: Uses Windows COM elevation monikers for privilege escalation
- **Clean Code**: Well-structured, optimized, and readable C++ implementation
- **RAII Pattern**: Automatic resource management with smart guards
- **Error Handling**: Comprehensive error checking and validation

## How It Works

The bypass technique works by:

1. **COM Initialization**: Initializes the COM library in apartment-threaded mode
2. **Moniker Creation**: Constructs an elevation moniker string in the format: `Elevation:Administrator!new:{CLSID}`
3. **Object Resolution**: Uses `CoGetObject` to resolve the moniker and obtain an elevated COM object
4. **Interface Access**: Retrieves the `ICMLuaUtil` interface from the elevated object
5. **Process Execution**: Calls `ShellExec` method to launch the target executable with elevated privileges

### The Elevation Moniker

The elevation moniker is a special COM moniker format that Windows recognizes:
```
Elevation:Administrator!new:{3E5FC7F9-9A51-4367-9063-A120244FBEC7}
```

This tells Windows to create an instance of the specified CLSID with administrator privileges, bypassing the standard UAC prompt.

## Project Structure

```
BypassUAC/
│
├── BypassUAC/                    # Main DLL project
│   ├── BypassUAC.h               # Interface definitions and declarations
│   ├── BypassUAC.cpp             # Core implementation
│   ├── BypassUAC2.cpp            # Rundll32 entry point
│   ├── BypassUAC.def             # DLL export definitions
│   ├── dllmain.cpp               # DLL entry point
│   ├── stdafx.h/cpp              # Precompiled headers
│   └── targetver.h               # Windows version targeting
│
├── Test/                         # Test application (optional)
│   └── Test.cpp                  # Test harness
│
└── BypassUAC.sln                 # Visual Studio solution file
```

### File Descriptions

#### `BypassUAC.h`
Contains:
- **Interface Definitions**: `ICMLuaUtil` COM interface structure with virtual function table
- **Constants**: CLSID and IID definitions for the CMSTPLUA component
- **Function Declarations**: Public API functions

#### `BypassUAC.cpp`
Core implementation with:
- **`CoCreateInstanceAsAdmin()`**: Creates COM objects with elevated privileges
  - Uses RAII pattern with `CoInitGuard` for automatic COM cleanup
  - Validates all parameters before use
  - Constructs elevation moniker dynamically
- **`CMLuaUtilBypassUAC()`**: Main bypass function
  - Validates input parameters
  - Converts string CLSID/IID to binary format
  - Creates elevated COM object
  - Executes target process via `ShellExec`

#### `BypassUAC2.cpp`
Rundll32 entry point that:
- Exports `BypassUAC` function for rundll32.exe invocation
- Currently hardcoded to launch `cmd.exe` (can be modified)

## Building the Project

### Prerequisites

- **Visual Studio 2022**
- **Windows SDK** (included with Visual Studio)
- **Windows 7 or later** (for testing)

### Build Steps

1. **Open the Solution**
   ```bash
   # Open BypassUAC.sln in Visual Studio
   ```

2. **Build the Project**
   - Press `Ctrl+Shift+B` or use Build → Build Solution
   - The DLL will be generated in `BypassUAC/[Configuration]/[Platform]/`

4. **Verify Output**
   - Check for `BypassUAC2.dll` in the output directory
   - Ensure no compilation errors or warnings

### Build Configuration

The project uses:
- **Runtime Library**: Multi-threaded DLL (`/MD` or `/MDd`)
- **Character Set**: Unicode
- **Platform Toolset**: v140 or later

## Usage

### Method 1: Using Rundll32 (Current Implementation)

```cmd
rundll32.exe BypassUAC2.dll,BypassUAC
```

This will execute the hardcoded command (`cmd.exe`) with elevated privileges.

### Method 2: Programmatic Usage

To use the bypass function in your own code:

```cpp
#include "BypassUAC.h"

// Execute a program with elevated privileges
BOOL success = CMLuaUtilBypassUAC(L"C:\\Windows\\System32\\notepad.exe");

if (success)
{
    // Process launched successfully with admin rights
}
else
{
    // Bypass failed - check error codes
}
```

### Method 3: Customizing the Entry Point

Modify `BypassUAC2.cpp` to accept command-line arguments:

```cpp
void CALLBACK BypassUAC(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    // Parse lpszCmdLine to get target executable
    // Convert to wide string and call CMLuaUtilBypassUAC()
}
```

## Technical Details

### COM Interface: ICMLuaUtil

The `ICMLuaUtil` interface is an undocumented Windows COM interface used internally by the Windows UAC system. Key methods:

- **`ShellExec`**: Executes a process with elevated privileges
  ```cpp
  HRESULT ShellExec(
      LPCWSTR lpFile,        // Path to executable
      LPCTSTR lpParameters,  // Command-line parameters (optional)
      LPCTSTR lpDirectory,   // Working directory (optional)
      ULONG fMask,           // Flags (usually 0)
      ULONG nShow            // Window show state (SW_SHOW, etc.)
  );
  ```

### CLSID and IID

- **CLSID_CMSTPLUA**: `{3E5FC7F9-9A51-4367-9063-A120244FBEC7}`
  - Component Manager Shell TPLUA (Trusted Platform Local User Account)
  
- **IID_ICMLuaUtil**: `{6EDD6D74-C007-4E75-B76A-E5740995E24C}`
  - Interface identifier for ICMLuaUtil

### Code Optimizations

The implementation includes several optimizations:

1. **RAII Pattern**: `CoInitGuard` automatically manages COM initialization/cleanup
2. **Parameter Validation**: All inputs are validated before use
3. **Early Returns**: Fail-fast error handling reduces nesting
4. **Memory Safety**: Proper initialization and cleanup of all buffers
5. **Error Propagation**: HRESULT values properly propagated and checked

### Key Implementation Details

#### CoInitGuard Class

```cpp
class CoInitGuard
{
    // Automatically initializes COM on construction
    // Handles RPC_E_CHANGED_MODE gracefully
    // Automatically uninitializes COM on destruction
};
```

Benefits:
- Exception-safe resource management
- No memory leaks even on early returns
- Handles already-initialized COM gracefully

#### BuildElevationMonikerName Function

```cpp
HRESULT BuildElevationMonikerName(
    REFCLSID rclsid,           // Input CLSID
    LPWSTR monikerName,        // Output buffer
    size_t monikerLength       // Buffer size
);
```

This helper function:
- Validates input parameters
- Converts CLSID to string format
- Constructs the elevation moniker string
- Uses safe string functions (`StringCchPrintfW`)

## Limitations

1. **Windows Version**: May not work on all Windows versions
   - Tested on Windows 7/8/10/11
   - Behavior may vary between versions

2. **UAC Settings**: Effectiveness depends on UAC configuration
   - May not work if UAC is completely disabled
   - May not work if UAC is set to "Always notify"

3. **Process Integrity**: The elevated process inherits the integrity level of the parent

4. **COM Dependencies**: Requires COM to be available and properly configured

## Discalimer

This project is provided for educational purposes.

## License

This project is licensed under the MIT License. For more information, see the [LICENSE file](LICENSE).

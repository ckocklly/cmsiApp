# CMSI App

## Overview

The Comprehensive Malocclusion Severity Index (CMSI) is a scoring system that evaluates a patient's malocclusion severity using a set of cephalometric and clinical indicators to determine aligner treatment suitability.

This application streamlines the workflow from image upload and analysis to CMSI index report generation.

## Prerequisites
 - C++11 or newer
 - Python 3.10+
    - requests
    - trimesh
 - CMake 3.16+
 - Qt 6.0+

## Usage
### Step 1: Clone the repo
Download the files and go to the project folder by running the following commands:
```
git clone https://github.com/ckocklly/cmsiApp.git
cd cmsiApp
```

### Step 2: Build & run
Thanks to CMake, we can build the project by just two commands
```
cmake -B build
cmake --build build
```
The executable `cmsiApp.exe` or `cmsiApp.app` should be generated in the folder `build`. Simply open it.

### Step 3: Upload the images & wait for results
*(unfinished)*

## Acknowledgements
 - Matthew Chew from R&D Hardware, LuxCreo for mentorship
 - Chohotech for analyses
 - Basil Chen for decision framework
 - Dr. Jean-Marc Retrouvey for CMSI
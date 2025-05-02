This repository contains a simple C++ implementation of a decoder for GRIB (GRIdded Binary) weather data messages.

Project Structure:
    - Utils.h – Header file containing the full implementation of the Decoder class, responsible for parsing and interpreting GRIB messages and Section class being a data type for each section of the GRIB message.
    - main.cpp – Example usage of the GribDecoder class. It demonstrates how to initialize the decoder and process a sample GRIB message.

Features:
    - Reads and interprets meteorological data from GRIB-format messages
    - Extracts relevant weather parameters (e.g., temperature, pressure, wind)
    - Modular class structure for easy reuse and integration

About GRIB:
GRIB is a concise data format commonly used in meteorology to store historical and forecast weather data. It's widely used by meteorological organizations around the world.

License:
This project is released under the MIT License.

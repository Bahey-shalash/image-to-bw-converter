
# Image to BW Altium Converter

A powerful utility to convert color images (PNG, JPEG, BMP, etc.) into high-quality 1-bit black-and-white PNGs using Floyd–Steinberg error diffusion. This repository provides both a simple CLI tool and a modern GUI frontend.

---

## Features

- ⚙️ Command-line tool for fast batch conversions  
- 🖼️ GUI with live input/output preview and adjustable threshold  
- 🧠 Accurate Floyd–Steinberg dithering  
- ⚡ Fast C backend with optional verbose output  

---


## Project Structure

```
.
├── NO_GUI.c                   # Command-line image converter
├── bw_converter.h/.c          # Shared C backend for conversion
├── gui_app.py                 # PySide6-based desktop GUI
├── Makefile                   # Build system
├── libbwconvert.so            # Shared object (built)
├── stb_image.h                # STB image loader
├── stb_image_write.h          # STB image writer
└── README.md                  # This file
```

---

## Prerequisites

### CLI version (NO\_GUI)

- GCC or Clang

### GUI version

- Python 3.7+
- PySide6

Install GUI requirements:

```bash
pip install PySide6
```

---

## Build Instructions

To build the CLI and shared library:

```bash
make
```

To clean all compiled artifacts:

```bash
make clean
```

---

## CLI Usage

First, build the CLI tool:

```bash
gcc -O3 NO_GUI.c -o image_bw_converter
```

Then run it using:

```bash
./image_bw_converter [options] <input_image> <output.png>
```

### Options:

- `-t <threshold>`  Brightness cutoff (0–255, default: 128)
- `-i`              Invert black/white after dithering
- `-v`              Enable verbose output
- `-h`              Show help message
- `--version`       Show version information

### Examples:

```bash
./image_bw_converter input.jpg output.png
./image_bw_converter -t 100 -i -v photo.jpg result.png
```

---

## GUI Usage

After building, launch the graphical interface:

```bash
python gui_app.py
```

### GUI Features:

- Browse and select **Input Image**
- Specify **Output PNG** file path
- Adjust **Threshold** using slider
- Toggle **Invert Output** and **Verbose Mode**
- View **Input** and **Output** previews side by side
- Click **Convert** to process

---

## License

MIT License. See `LICENSE` or apply your own.

---

Happy converting! 🎨
<img width="800" alt="Screenshot 2025-04-19 at 7 28 30 PM" src="https://github.com/user-attachments/assets/45e53448-bb4e-4ece-84f6-18afa286c52b" />


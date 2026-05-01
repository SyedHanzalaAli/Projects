# ⟨/⟩ Unit Converter

> A fast, interactive desktop utility for converting values across 10 measurement categories — built with Python and CustomTkinter.

![Python](https://img.shields.io/badge/Python-3.10%2B-blue?style=flat-square&logo=python)
![CustomTkinter](https://img.shields.io/badge/GUI-CustomTkinter-4F8EF7?style=flat-square)
![License](https://img.shields.io/badge/license-MIT-green?style=flat-square)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey?style=flat-square)

---

## Features

- **Live conversion** — result updates instantly as you type, no button click needed
- **10 categories, 82 units** — from everyday Length and Mass to Energy, Pressure, and Digital Storage
- **⇅ Swap** — reverse the conversion direction in one click
- **Smart formatting** — up to 10 significant figures; scientific notation for extreme values
- **Keyboard friendly** — press `Enter` to convert, full mouse-free workflow
- **Dark, modern UI** — built with CustomTkinter, runs natively on Windows, macOS, and Linux
- **Offline** — zero network requests, no telemetry, no dependencies beyond the GUI library

---

## Screenshots

> _Run the app locally to see it in action — `python app.py`_

---

## Supported Unit Categories

| # | Category | Example Units |
|---|----------|---------------|
| 1 | **Length** | Meter, Kilometer, Mile, Foot, Inch, Nautical Mile, Light Year |
| 2 | **Mass / Weight** | Kilogram, Pound, Ounce, Stone, Metric Ton, Short Ton |
| 3 | **Temperature** | Celsius, Fahrenheit, Kelvin |
| 4 | **Area** | Square Meter, Acre, Hectare, Square Mile, Square Foot |
| 5 | **Volume** | Liter, Gallon (US/UK), Cup, Fluid Ounce, Tablespoon |
| 6 | **Speed** | m/s, km/h, mph, Knot, Mach |
| 7 | **Time** | Second, Minute, Hour, Day, Week, Month, Year |
| 8 | **Digital Storage** | Bit → Petabyte, plus KiB / MiB / GiB |
| 9 | **Pressure** | Pascal, Bar, Atmosphere, PSI, Torr |
| 10 | **Energy** | Joule, Calorie, kWh, BTU, Electronvolt |

---

## Getting Started

### Prerequisites

- Python **3.10** or higher
- pip

### Installation

```bash
# 1. Clone the repository
git clone https://github.com/your-username/unit-converter.git
cd unit-converter

# 2. Install the only dependency
pip install customtkinter

# 3. Launch the app
python app.py
```

> **Windows users:** double-click `app.py` if Python is associated with `.py` files, or use the command above in PowerShell / Command Prompt.

---

## Project Structure

```
unit-converter/
│
├── app.py               # GUI layer — CustomTkinter window & all widgets
├── unit_converter.py    # Conversion engine — pure logic, no UI code
└── README.md
```

### Architecture

The project follows a clean **separation of concerns**:

- **`unit_converter.py`** is a standalone library. It holds the full unit registry (a nested dictionary mapping each unit to its SI base factor), the temperature converter (non-linear, handled separately), and the `format_result()` formatter. It can be imported and used independently in scripts or other projects.

- **`app.py`** is purely presentational. It imports `convert()`, `format_result()`, and `get_units()` from `unit_converter.py` and never contains any mathematical logic itself.

---

## How Conversions Work

All linear conversions are resolved in two steps:

```
result = value × factor(from_unit) ÷ factor(to_unit)
```

Each unit stores its **factor relative to the SI base unit** for that category (e.g., `1 km = 1000 m`, so `factor = 1000`). This makes adding new units trivial — one line in the registry.

**Temperature** is the only non-linear category. It converts through Celsius as an intermediate step:

```
°F → °C  :  (°F − 32) × 5/9
°C → K   :  °C + 273.15
```

---

## Adding New Units

Open `unit_converter.py` and add an entry to the relevant category dictionary:

```python
"Length": {
    ...
    "Furlong": 201.168,   # ← 1 furlong = 201.168 meters
}
```

That's it — the GUI picks it up automatically on next launch.

---

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Enter` / `Numpad Enter` | Trigger conversion |
| `Tab` | Move between fields |

---

## Dependencies

| Package | Purpose | Version |
|---------|---------|---------|
| [customtkinter](https://github.com/TomSchimansky/CustomTkinter) | Modern Tkinter-based GUI widgets | `≥ 5.0` |

All other modules (`tkinter`, `typing`) are part of the Python standard library.

---

## Contributing

Contributions are welcome! To add a unit category or fix a conversion factor:

1. Fork the repository
2. Create a feature branch: `git checkout -b feat/add-force-units`
3. Make your changes in `unit_converter.py`
4. Open a Pull Request with a brief description

Please keep `unit_converter.py` free of any UI imports so it stays independently usable.

---

## License

Distributed under the **MIT License**. See [`LICENSE`](LICENSE) for details.

---

<p align="center">Built with Python · CustomTkinter · ⟨/⟩</p>

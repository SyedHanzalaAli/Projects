from typing import Optional

# UNIT REGISTRY 
# Each category maps unit_name → factor_to_base_unit
# Special handling for Temperature (non-linear) is done separately.

UNITS: dict[str, dict[str, float]] = {
    "Length": {
        "Meter (m)":         1.0,
        "Kilometer (km)":    1_000.0,
        "Centimeter (cm)":   0.01,
        "Millimeter (mm)":   0.001,
        "Mile (mi)":         1_609.344,
        "Yard (yd)":         0.9144,
        "Foot (ft)":         0.3048,
        "Inch (in)":         0.0254,
        "Nautical Mile":     1_852.0,
        "Light Year (ly)":   9.461e15,
    },
    "Mass / Weight": {
        "Kilogram (kg)":     1.0,
        "Gram (g)":          0.001,
        "Milligram (mg)":    1e-6,
        "Metric Ton (t)":    1_000.0,
        "Pound (lb)":        0.453592,
        "Ounce (oz)":        0.0283495,
        "Stone (st)":        6.35029,
        "Short Ton (US)":    907.185,
        "Long Ton (UK)":     1_016.05,
    },
    "Temperature": {
        "Celsius (°C)":    None,   # handled specially
        "Fahrenheit (°F)": None,
        "Kelvin (K)":      None,
    },
    "Area": {
        "Square Meter (m²)":    1.0,
        "Square Kilometer (km²)": 1e6,
        "Square Centimeter (cm²)": 1e-4,
        "Square Mile (mi²)":    2.59e6,
        "Square Yard (yd²)":    0.836127,
        "Square Foot (ft²)":    0.092903,
        "Square Inch (in²)":    6.4516e-4,
        "Hectare (ha)":         10_000.0,
        "Acre":                 4_046.86,
    },
    "Volume": {
        "Liter (L)":            1.0,
        "Milliliter (mL)":      0.001,
        "Cubic Meter (m³)":     1_000.0,
        "Cubic Centimeter (cm³)": 0.001,
        "Gallon (US)":          3.78541,
        "Gallon (UK)":          4.54609,
        "Quart (US)":           0.946353,
        "Pint (US)":            0.473176,
        "Cup (US)":             0.236588,
        "Fluid Ounce (US fl oz)": 0.0295735,
        "Tablespoon (tbsp)":    0.0147868,
        "Teaspoon (tsp)":       0.00492892,
    },
    "Speed": {
        "Meter/Second (m/s)":   1.0,
        "Kilometer/Hour (km/h)": 0.277778,
        "Mile/Hour (mph)":      0.44704,
        "Knot (kn)":            0.514444,
        "Foot/Second (ft/s)":   0.3048,
        "Mach (at sea level)":  340.29,
    },
    "Time": {
        "Second (s)":           1.0,
        "Millisecond (ms)":     0.001,
        "Microsecond (μs)":     1e-6,
        "Minute (min)":         60.0,
        "Hour (h)":             3_600.0,
        "Day (d)":              86_400.0,
        "Week (wk)":            604_800.0,
        "Month (avg)":          2_629_800.0,
        "Year (avg)":           31_557_600.0,
    },
    "Digital Storage": {
        "Bit (b)":              1.0,
        "Byte (B)":             8.0,
        "Kilobyte (KB)":        8_000.0,
        "Megabyte (MB)":        8e6,
        "Gigabyte (GB)":        8e9,
        "Terabyte (TB)":        8e12,
        "Petabyte (PB)":        8e15,
        "Kibibyte (KiB)":       8_192.0,
        "Mebibyte (MiB)":       8_388_608.0,
        "Gibibyte (GiB)":       8_589_934_592.0,
    },
    "Pressure": {
        "Pascal (Pa)":          1.0,
        "Kilopascal (kPa)":     1_000.0,
        "Bar":                  100_000.0,
        "Atmosphere (atm)":     101_325.0,
        "PSI (lb/in²)":         6_894.76,
        "Torr (mmHg)":          133.322,
    },
    "Energy": {
        "Joule (J)":            1.0,
        "Kilojoule (kJ)":       1_000.0,
        "Calorie (cal)":        4.184,
        "Kilocalorie (kcal)":   4_184.0,
        "Watt-hour (Wh)":       3_600.0,
        "Kilowatt-hour (kWh)":  3_600_000.0,
        "BTU":                  1_055.06,
        "Electronvolt (eV)":    1.60218e-19,
    },
}

CATEGORIES: list[str] = list(UNITS.keys())


def get_units(category: str) -> list[str]:
    """Return sorted list of unit names for a given category."""
    return list(UNITS[category].keys())


def _convert_temperature(value: float, from_unit: str, to_unit: str) -> float:
    """Convert temperature between Celsius, Fahrenheit, and Kelvin."""
    # Step 1: normalise to Celsius
    if "Celsius" in from_unit:
        celsius = value
    elif "Fahrenheit" in from_unit:
        celsius = (value - 32) * 5 / 9
    elif "Kelvin" in from_unit:
        celsius = value - 273.15
    else:
        raise ValueError(f"Unknown temperature unit: {from_unit}")

    # Step 2: convert from Celsius to target
    if "Celsius" in to_unit:
        return celsius
    elif "Fahrenheit" in to_unit:
        return celsius * 9 / 5 + 32
    elif "Kelvin" in to_unit:
        return celsius + 273.15
    else:
        raise ValueError(f"Unknown temperature unit: {to_unit}")


def convert(
    value: float,
    category: str,
    from_unit: str,
    to_unit: str,
) -> float:
    """
    Convert `value` from `from_unit` to `to_unit` within `category`.

    Returns the converted float.
    Raises ValueError for unknown units or categories.
    """
    if category not in UNITS:
        raise ValueError(f"Unknown category: {category}")

    if from_unit == to_unit:
        return value

    if category == "Temperature":
        return _convert_temperature(value, from_unit, to_unit)

    cat = UNITS[category]
    if from_unit not in cat:
        raise ValueError(f"Unknown unit '{from_unit}' in category '{category}'")
    if to_unit not in cat:
        raise ValueError(f"Unknown unit '{to_unit}' in category '{category}'")

    # Convert to base unit, then to target
    base_value = value * cat[from_unit]
    return base_value / cat[to_unit]


def format_result(value: float) -> str:
    """
    Smart formatting:
    - Very large / very small → scientific notation
    - Otherwise up to 10 significant figures, trailing zeros stripped
    """
    if value == 0:
        return "0"
    abs_val = abs(value)
    if abs_val >= 1e15 or (abs_val < 1e-6 and abs_val != 0):
        return f"{value:.6e}"
    # Use up to 10 sig-figs, strip trailing zeros
    formatted = f"{value:.10g}"
    return formatted

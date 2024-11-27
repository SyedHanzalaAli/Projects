def length_conversion(value, from_unit, to_unit):
    length_units = {
        "m": 1,
        "cm": 0.01,
        "mm": 0.001,
        "km": 1000,
        "inch": 0.0254,
        "ft": 0.3048,
        "yd": 0.9144,
        "mile": 1609.34
    }
    result = value * length_units[from_unit] / length_units[to_unit]
    print(f"{value} {from_unit} is equal to {result} {to_unit}")
    return result

def mass_conversion(value, from_unit, to_unit):
    mass_units = {
        "kg": 1,
        "g": 0.001,
        "mg": 1e-6,
        "lb": 0.453592,
        "oz": 0.0283495
    }
    result = value * mass_units[from_unit] / mass_units[to_unit]
    print(f"{value} {from_unit} is equal to {result} {to_unit}")
    return result

def time_conversion(value, from_unit, to_unit):
    time_units = {
        "s": 1,
        "min": 60,
        "hr": 3600,
        "day": 86400
    }
    result = value * time_units[from_unit] / time_units[to_unit]
    print(f"{value} {from_unit} is equal to {result} {to_unit}")
    return result

def electric_current_conversion(value, from_unit, to_unit):
    current_units = {
        "A": 1,
        "mA": 0.001,
        "μA": 1e-6
    }
    result = value * current_units[from_unit] / current_units[to_unit]
    print(f"{value} {from_unit} is equal to {result} {to_unit}")
    return result

def temperature_conversion(value, from_unit, to_unit):
    if from_unit == "K" and to_unit == "C":
        result = value - 273.15
    elif from_unit == "C" and to_unit == "K":
        result = value + 273.15
    elif from_unit == "C" and to_unit == "F":
        result = (value * 9/5) + 32
    elif from_unit == "F" and to_unit == "C":
        result = (value - 32) * 5/9
    elif from_unit == "K" and to_unit == "F":
        result = (value - 273.15) * 9/5 + 32
    elif from_unit == "F" and to_unit == "K":
        result = (value - 32) * 5/9 + 273.15
    else:
        result = value
    print(f"{value} {from_unit} is equal to {result} {to_unit}")
    return result

def amount_of_substance_conversion(value, from_unit, to_unit):
    substance_units = {
        "mol": 1,
        "mmol": 1e-3,
        "μmol": 1e-6
    }
    result = value * substance_units[from_unit] / substance_units[to_unit]
    print(f"{value} {from_unit} is equal to {result} {to_unit}")
    return result

def luminous_intensity_conversion(value, from_unit, to_unit):
    intensity_units = {
        "cd": 1,
        "mcd": 1e-3,
        "μcd": 1e-6
    }
    result = value * intensity_units[from_unit] / intensity_units[to_unit]
    print(f"{value} {from_unit} is equal to {result} {to_unit}")
    return result

def convert(value, quantity_type, from_unit, to_unit):
    if quantity_type == "length":
        return length_conversion(value, from_unit, to_unit)
    elif quantity_type == "mass":
        return mass_conversion(value, from_unit, to_unit)
    elif quantity_type == "time":
        return time_conversion(value, from_unit, to_unit)
    elif quantity_type == "current":
        return electric_current_conversion(value, from_unit, to_unit)
    elif quantity_type == "temperature":
        return temperature_conversion(value, from_unit, to_unit)
    elif quantity_type == "substance":
        return amount_of_substance_conversion(value, from_unit, to_unit)
    elif quantity_type == "intensity":
        return luminous_intensity_conversion(value, from_unit, to_unit)
    else:
        print("Invalid quantity type.")
        return None

# User input
value = float(input("Enter the value to convert: "))
quantity_type = input("Enter the quantity type (length, mass, time, current, temperature, substance, intensity): ").lower()
from_unit = input(f"Enter the unit you are converting from (e.g., m, kg, s): ").lower()
to_unit = input(f"Enter the unit you are converting to (e.g., cm, g, min): ").lower()

# Perform conversion
convert(value, quantity_type, from_unit, to_unit)

import tkinter as tk
from tkinter import ttk
import customtkinter as ctk
from unit_converter import CATEGORIES, convert, format_result, get_units


# THEME CONFIGURATION 
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

PALETTE = {
    "bg":           "#0D0F14",
    "surface":      "#151820",
    "surface_2":    "#1C2030",
    "accent":       "#4F8EF7",
    "accent_dim":   "#2A4A8A",
    "accent_glow":  "#6FA8FF",
    "text_primary": "#E8ECF5",
    "text_secondary":"#7A8BA6",
    "text_muted":   "#3D4F6A",
    "success":      "#3DD68C",
    "error":        "#F7604F",
    "border":       "#232B3E",
    "separator":    "#1A2235",
}

FONT_MONO   = ("Courier New", 11)
FONT_BODY   = ("Segoe UI", 11)
FONT_SMALL  = ("Segoe UI", 9)
FONT_LABEL  = ("Segoe UI Semibold", 10)
FONT_TITLE  = ("Segoe UI Black", 20)
FONT_RESULT = ("Courier New", 28, "bold")
FONT_BADGE  = ("Segoe UI", 9, "bold")


# MAIN APPLICATION 
class UnitConverterApp(ctk.CTk):

    def __init__(self):
        super().__init__()

        self.title("Unit Converter")
        self.geometry("620x700")
        self.minsize(540, 620)
        self.resizable(True, True)
        self.configure(fg_color=PALETTE["bg"])

        # State
        self._current_category = ctk.StringVar(value=CATEGORIES[0])
        self._from_unit        = ctk.StringVar()
        self._to_unit          = ctk.StringVar()
        self._input_value      = ctk.StringVar()
        self._result_text      = ctk.StringVar(value="—")
        self._status_text      = ctk.StringVar(value="")
        self._swap_anim_running= False

        self._build_ui()
        self._on_category_change()   # populate dropdowns
        self._bind_events()

    # UI CONSTRUCTION 

    def _build_ui(self):
        """Assemble all UI layers."""
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(3, weight=1)   # result area expands

        #  Header 
        self._build_header()

        # Category selector
        self._build_category_row()

        # Input / From
        self._build_input_section()

        # Result / To
        self._build_result_section()

        # Status bar
        self._build_status_bar()

    def _build_header(self):
        hdr = ctk.CTkFrame(self, fg_color=PALETTE["surface"], corner_radius=0)
        hdr.grid(row=0, column=0, sticky="ew", padx=0, pady=0)
        hdr.grid_columnconfigure(1, weight=1)

        # Accent bar
        accent_bar = tk.Frame(hdr, bg=PALETTE["accent"], height=3)
        accent_bar.grid(row=0, column=0, columnspan=2, sticky="ew")

        # Logo mark
        logo_mark = ctk.CTkLabel(
            hdr, text="⟨/⟩", font=("Courier New", 22, "bold"),
            text_color=PALETTE["accent"], fg_color="transparent"
        )
        logo_mark.grid(row=1, column=0, padx=(24, 8), pady=(16, 14), sticky="w")

        # Title + subtitle stack
        text_frame = ctk.CTkFrame(hdr, fg_color="transparent")
        text_frame.grid(row=1, column=1, sticky="w", pady=(16, 14))

        ctk.CTkLabel(
            text_frame, text="UNIT CONVERTER",
            font=FONT_TITLE, text_color=PALETTE["text_primary"],
            fg_color="transparent"
        ).grid(row=0, column=0, sticky="w")

        ctk.CTkLabel(
            text_frame, text="fast · precise · offline",
            font=FONT_SMALL, text_color=PALETTE["text_secondary"],
            fg_color="transparent"
        ).grid(row=1, column=0, sticky="w")

        # Version badge
        badge = ctk.CTkLabel(
            hdr, text=" v1.0 ", font=FONT_BADGE,
            text_color=PALETTE["accent"],
            fg_color=PALETTE["accent_dim"],
            corner_radius=4
        )
        badge.grid(row=1, column=2, padx=(0, 24), pady=(16, 14), sticky="e")

    def _build_category_row(self):
        frame = ctk.CTkFrame(self, fg_color=PALETTE["surface_2"], corner_radius=0)
        frame.grid(row=1, column=0, sticky="ew", padx=0, pady=(1, 0))
        frame.grid_columnconfigure(1, weight=1)

        ctk.CTkLabel(
            frame, text="CATEGORY", font=FONT_BADGE,
            text_color=PALETTE["text_muted"], fg_color="transparent"
        ).grid(row=0, column=0, padx=(24, 10), pady=12, sticky="w")

        self._cat_menu = ctk.CTkOptionMenu(
            frame,
            variable=self._current_category,
            values=CATEGORIES,
            command=lambda _: self._on_category_change(),
            fg_color=PALETTE["surface"],
            button_color=PALETTE["accent_dim"],
            button_hover_color=PALETTE["accent"],
            dropdown_fg_color=PALETTE["surface"],
            dropdown_hover_color=PALETTE["accent_dim"],
            text_color=PALETTE["text_primary"],
            font=FONT_BODY,
            anchor="w",
            width=260,
        )
        self._cat_menu.grid(row=0, column=1, padx=(0, 24), pady=10, sticky="w")

    def _build_input_section(self):
        outer = ctk.CTkFrame(self, fg_color="transparent")
        outer.grid(row=2, column=0, sticky="ew", padx=20, pady=(18, 0))
        outer.grid_columnconfigure(0, weight=1)

        # Section label
        ctk.CTkLabel(
            outer, text="INPUT", font=FONT_BADGE,
            text_color=PALETTE["text_muted"], fg_color="transparent"
        ).grid(row=0, column=0, sticky="w", padx=4, pady=(0, 6))

        card = ctk.CTkFrame(
            outer, fg_color=PALETTE["surface"],
            corner_radius=12,
            border_width=1, border_color=PALETTE["border"]
        )
        card.grid(row=1, column=0, sticky="ew")
        card.grid_columnconfigure(0, weight=1)

        # Value entry
        self._value_entry = ctk.CTkEntry(
            card,
            textvariable=self._input_value,
            placeholder_text="Enter a number…",
            font=("Courier New", 22, "bold"),
            text_color=PALETTE["text_primary"],
            placeholder_text_color=PALETTE["text_muted"],
            fg_color="transparent",
            border_width=0,
            height=54,
        )
        self._value_entry.grid(row=0, column=0, sticky="ew", padx=20, pady=(14, 4))

        # Divider
        tk.Frame(card, bg=PALETTE["separator"], height=1).grid(
            row=1, column=0, sticky="ew", padx=16
        )

        # From-unit selector
        from_row = ctk.CTkFrame(card, fg_color="transparent")
        from_row.grid(row=2, column=0, sticky="ew", padx=16, pady=(8, 14))
        from_row.grid_columnconfigure(1, weight=1)

        ctk.CTkLabel(
            from_row, text="FROM", font=FONT_BADGE,
            text_color=PALETTE["text_muted"], fg_color="transparent", width=44
        ).grid(row=0, column=0, sticky="w")

        self._from_menu = ctk.CTkOptionMenu(
            from_row,
            variable=self._from_unit,
            values=[],
            fg_color=PALETTE["surface_2"],
            button_color=PALETTE["accent_dim"],
            button_hover_color=PALETTE["accent"],
            dropdown_fg_color=PALETTE["surface"],
            dropdown_hover_color=PALETTE["accent_dim"],
            text_color=PALETTE["text_primary"],
            font=FONT_BODY,
            anchor="w",
            dynamic_resizing=False,
            width=380,
        )
        self._from_menu.grid(row=0, column=1, padx=(8, 0), sticky="ew")

    def _build_result_section(self):
        outer = ctk.CTkFrame(self, fg_color="transparent")
        outer.grid(row=3, column=0, sticky="nsew", padx=20, pady=(12, 0))
        outer.grid_columnconfigure(0, weight=1)
        outer.grid_rowconfigure(1, weight=1)

        # Section label + swap button on same row
        label_row = ctk.CTkFrame(outer, fg_color="transparent")
        label_row.grid(row=0, column=0, sticky="ew", padx=4, pady=(0, 6))
        label_row.grid_columnconfigure(0, weight=1)

        ctk.CTkLabel(
            label_row, text="RESULT", font=FONT_BADGE,
            text_color=PALETTE["text_muted"], fg_color="transparent"
        ).grid(row=0, column=0, sticky="w")

        self._swap_btn = ctk.CTkButton(
            label_row, text="⇅ Swap", font=FONT_BADGE,
            width=72, height=24,
            fg_color=PALETTE["surface_2"],
            hover_color=PALETTE["accent_dim"],
            text_color=PALETTE["text_secondary"],
            corner_radius=6,
            command=self._swap_units,
        )
        self._swap_btn.grid(row=0, column=1, sticky="e")

        # Result card
        card = ctk.CTkFrame(
            outer, fg_color=PALETTE["surface"],
            corner_radius=12,
            border_width=1, border_color=PALETTE["border"]
        )
        card.grid(row=1, column=0, sticky="nsew")
        card.grid_columnconfigure(0, weight=1)
        card.grid_rowconfigure(1, weight=1)

        # Result display
        self._result_label = ctk.CTkLabel(
            card,
            textvariable=self._result_text,
            font=FONT_RESULT,
            text_color=PALETTE["accent"],
            fg_color="transparent",
            anchor="center",
            wraplength=520,
        )
        self._result_label.grid(row=0, column=0, sticky="nsew", padx=24, pady=(24, 10))

        # Divider
        tk.Frame(card, bg=PALETTE["separator"], height=1).grid(
            row=1, column=0, sticky="ew", padx=16
        )

        # To-unit selector
        to_row = ctk.CTkFrame(card, fg_color="transparent")
        to_row.grid(row=2, column=0, sticky="ew", padx=16, pady=(8, 14))
        to_row.grid_columnconfigure(1, weight=1)

        ctk.CTkLabel(
            to_row, text="TO", font=FONT_BADGE,
            text_color=PALETTE["text_muted"], fg_color="transparent", width=44
        ).grid(row=0, column=0, sticky="w")

        self._to_menu = ctk.CTkOptionMenu(
            to_row,
            variable=self._to_unit,
            values=[],
            fg_color=PALETTE["surface_2"],
            button_color=PALETTE["accent_dim"],
            button_hover_color=PALETTE["accent"],
            dropdown_fg_color=PALETTE["surface"],
            dropdown_hover_color=PALETTE["accent_dim"],
            text_color=PALETTE["text_primary"],
            font=FONT_BODY,
            anchor="w",
            dynamic_resizing=False,
            width=380,
        )
        self._to_menu.grid(row=0, column=1, padx=(8, 0), sticky="ew")

        # Convert button
        self._convert_btn = ctk.CTkButton(
            card,
            text="  CONVERT  →",
            font=("Segoe UI Black", 13),
            height=46,
            fg_color=PALETTE["accent"],
            hover_color=PALETTE["accent_glow"],
            text_color="#FFFFFF",
            corner_radius=8,
            command=self._do_convert,
        )
        self._convert_btn.grid(row=3, column=0, padx=16, pady=(10, 16), sticky="ew")

    def _build_status_bar(self):
        bar = ctk.CTkFrame(self, fg_color=PALETTE["surface_2"], corner_radius=0, height=28)
        bar.grid(row=4, column=0, sticky="ew", padx=0, pady=(16, 0))
        bar.grid_propagate(False)
        bar.grid_columnconfigure(0, weight=1)

        self._status_lbl = ctk.CTkLabel(
            bar, textvariable=self._status_text,
            font=FONT_SMALL, text_color=PALETTE["text_muted"],
            fg_color="transparent", anchor="w"
        )
        self._status_lbl.grid(row=0, column=0, sticky="w", padx=16)

    # EVENT BINDINGS

    def _bind_events(self):
        """Keyboard shortcuts and live-update bindings."""
        self.bind("<Return>", lambda _: self._do_convert())
        self.bind("<KP_Enter>", lambda _: self._do_convert())
        self._input_value.trace_add("write", lambda *_: self._do_convert())
        self._from_unit.trace_add("write", lambda *_: self._do_convert())
        self._to_unit.trace_add("write", lambda *_: self._do_convert())

    #  LOGIC CALLBACKS 

    def _on_category_change(self):
        cat = self._current_category.get()
        units = get_units(cat)
        self._from_menu.configure(values=units)
        self._to_menu.configure(values=units)
        # sensible defaults: first and second unit
        self._from_unit.set(units[0])
        self._to_unit.set(units[1] if len(units) > 1 else units[0])
        self._result_text.set("—")
        self._status_text.set(f"Category: {cat}  ·  {len(units)} units available")
        self._set_result_color(PALETTE["accent"])

    def _do_convert(self):
        raw = self._input_value.get().strip()
        if not raw:
            self._result_text.set("—")
            self._status_text.set("Waiting for input…")
            self._set_result_color(PALETTE["text_muted"])
            return
        try:
            value = float(raw)
        except ValueError:
            self._result_text.set("!")
            self._status_text.set("⚠  Enter a valid number")
            self._set_result_color(PALETTE["error"])
            return

        cat      = self._current_category.get()
        from_u   = self._from_unit.get()
        to_u     = self._to_unit.get()

        try:
            result = convert(value, cat, from_u, to_u)
            display = format_result(result)
            self._result_text.set(display)
            self._status_text.set(
                f"{format_result(value)} {from_u}  =  {display} {to_u}"
            )
            self._set_result_color(PALETTE["success"] if from_u != to_u else PALETTE["accent"])
        except Exception as exc:
            self._result_text.set("Err")
            self._status_text.set(f"Error: {exc}")
            self._set_result_color(PALETTE["error"])

    def _swap_units(self):
        """Swap From ↔ To units and re-convert."""
        from_val = self._from_unit.get()
        to_val   = self._to_unit.get()
        self._from_unit.set(to_val)
        self._to_unit.set(from_val)
        # Animate button flash
        self._swap_btn.configure(fg_color=PALETTE["accent_dim"])
        self.after(120, lambda: self._swap_btn.configure(fg_color=PALETTE["surface_2"]))

    def _set_result_color(self, color: str):
        self._result_label.configure(text_color=color)


#  ENTRY POINT
if __name__ == "__main__":
    app = UnitConverterApp()
    app.mainloop()

import docx
from docx import Document
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT, WD_ALIGN_VERTICAL
from docx.oxml import parse_xml, OxmlElement
from docx.oxml.ns import nsdecls, qn

def create_block_diagram_doc():
    doc = Document()

    # Color Palette Definitions
    PRIMARY_COLOR = RGBColor(15, 76, 129)     # Deep Navy Blue (ESP32)
    SECONDARY_COLOR = RGBColor(211, 84, 0)    # Amber / Orange (GPRS Modem)
    TERTIARY_COLOR = RGBColor(22, 160, 133)   # Teal / Emerald (Nuvoton UI)
    DARK_TEXT = RGBColor(44, 62, 80)          # Charcoal
    MUTED_TEXT = RGBColor(127, 140, 141)      # Muted Gray
    WHITE = RGBColor(255, 255, 255)

    HEX_PRIMARY = "0F4C81"
    HEX_SECONDARY = "D35400"
    HEX_TERTIARY = "16A085"
    HEX_LIGHT_PRIMARY = "EBF5FB"
    HEX_LIGHT_SECONDARY = "FDFEFE"
    HEX_LIGHT_TERTIARY = "E8F8F5"
    HEX_BOX_BG = "F4F6F7"
    HEX_BORDER_COLOR = "BDC3C7"

    # Margins
    for section in doc.sections:
        section.top_margin = Inches(0.7)
        section.bottom_margin = Inches(0.7)
        section.left_margin = Inches(0.7)
        section.right_margin = Inches(0.7)

    # Style Helpers
    def set_cell_background(cell, hex_color):
        shading = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{hex_color}"/>')
        cell._tc.get_or_add_tcPr().append(shading)

    def set_cell_margins(cell, top=100, bottom=100, left=150, right=150):
        tcPr = cell._tc.get_or_add_tcPr()
        tcMar = OxmlElement('w:tcMar')
        for m, val in [('top', top), ('bottom', bottom), ('left', left), ('right', right)]:
            node = OxmlElement(f'w:{m}')
            node.set(qn('w:w'), str(val))
            node.set(qn('w:type'), 'dxa')
            tcMar.append(node)
        tcPr.append(tcMar)

    def set_cell_borders(cell, color="CCCCCC", sz="4", val="single"):
        tcPr = cell._tc.get_or_add_tcPr()
        borders = parse_xml(
            f'<w:tcBorders {nsdecls("w")}>\n'
            f'  <w:top w:val="{val}" w:sz="{sz}" w:space="0" w:color="{color}"/>\n'
            f'  <w:left w:val="{val}" w:sz="{sz}" w:space="0" w:color="{color}"/>\n'
            f'  <w:bottom w:val="{val}" w:sz="{sz}" w:space="0" w:color="{color}"/>\n'
            f'  <w:right w:val="{val}" w:sz="{sz}" w:space="0" w:color="{color}"/>\n'
            f'</w:tcBorders>'
        )
        tcPr.append(borders)

    def add_block_cell(cell, title, subtitle, hex_header_bg, hex_body_bg, items):
        set_cell_margins(cell, top=80, bottom=80, left=120, right=120)
        set_cell_background(cell, hex_body_bg)
        set_cell_borders(cell, color=hex_header_bg, sz="12")
        
        # Header text
        p_hdr = cell.paragraphs[0]
        p_hdr.alignment = WD_ALIGN_PARAGRAPH.CENTER
        p_hdr.paragraph_format.space_after = Pt(2)
        r_title = p_hdr.add_run(f"■ {title}\n")
        r_title.bold = True
        r_title.font.size = Pt(10)
        r_title.font.color.rgb = PRIMARY_COLOR if hex_header_bg == HEX_PRIMARY else (SECONDARY_COLOR if hex_header_bg == HEX_SECONDARY else TERTIARY_COLOR)

        if subtitle:
            r_sub = p_hdr.add_run(f"({subtitle})\n")
            r_sub.italic = True
            r_sub.font.size = Pt(8.5)
            r_sub.font.color.rgb = MUTED_TEXT

        # Bullet list
        for item in items:
            p_item = cell.add_paragraph()
            p_item.paragraph_format.space_after = Pt(2)
            p_item.paragraph_format.line_spacing = 1.05
            r_bullet = p_item.add_run("• ")
            r_bullet.bold = True
            r_bullet.font.size = Pt(8.5)
            r_bullet.font.color.rgb = DARK_TEXT
            r_txt = p_item.add_run(item)
            r_txt.font.size = Pt(8.5)
            r_txt.font.color.rgb = DARK_TEXT

    # -------------------------------------------------------------
    # Document Header Title Block
    # -------------------------------------------------------------
    title_p = doc.add_paragraph()
    title_p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    title_run = title_p.add_run("AIO9 System Hardware Block Diagram Document")
    title_run.font.size = Pt(22)
    title_run.font.bold = True
    title_run.font.color.rgb = PRIMARY_COLOR
    title_p.paragraph_format.space_after = Pt(4)

    sub_p = doc.add_paragraph()
    sub_p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    sub_run = sub_p.add_run("Detailed Subsystem Block Diagrams & Inter-Board Signals: ESP32 Board, GPRS Modem Board & Nuvoton UI Board")
    sub_run.font.size = Pt(12)
    sub_run.font.italic = True
    sub_run.font.color.rgb = MUTED_TEXT
    sub_p.paragraph_format.space_after = Pt(18)

    # Overview text
    doc.add_paragraph(
        "This document provides the complete hardware block diagrams and signal interface maps for the three primary circuit boards in the AIO9 system architecture: "
        "the ESP32 Core Controller Board, the SIMCom GPRS Modem Board, and the Nuvoton UI Board."
    )

    # -------------------------------------------------------------
    # Section 1: System Level Inter-Board Flow Diagram
    # -------------------------------------------------------------
    h1 = doc.add_heading(level=1)
    r1 = h1.add_run("1. Top-Level Multi-Board Architecture & Flow Diagram")
    r1.font.color.rgb = PRIMARY_COLOR

    p_intro = doc.add_paragraph(
        "The AIO9 architecture decouples main processing, cellular communication, and screen/keypad user interaction across three dedicated hardware boards:"
    )
    p_intro.paragraph_format.space_after = Pt(8)

    # 3-Column Top Level Block Layout Table
    top_table = doc.add_table(rows=1, cols=3)
    top_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    top_cells = top_table.rows[0].cells

    # Column 1: ESP32 Board
    add_block_cell(
        top_cells[0],
        "ESP32 Core Board",
        "Master Controller",
        HEX_PRIMARY,
        HEX_LIGHT_PRIMARY,
        [
            "Dual-Core Xtensa LX6 (240MHz)",
            "Core 0: Network & GPRS Tasks",
            "Core 1: Scheduler & Sensors",
            "ULP Coprocessor (Pulse Counter)",
            "RTC Slow RAM (State Memory)",
            "SPIFFS (16MB Flash) & SD Card",
            "DS1307 Real Time Clock (I2C)",
            "HDC1080 / BME280 Sensors"
        ]
    )

    # Column 2: GPRS Modem Board
    add_block_cell(
        top_cells[1],
        "GPRS Modem Board",
        "Cellular Subsystem",
        HEX_SECONDARY,
        HEX_LIGHT_SECONDARY,
        [
            "SIMCom A7672S / A7672M6",
            "LTE Cat-1 4G + 2G GSM Fallback",
            "Direct Layer-4 TCP Engine",
            "Multi-Carrier SIM Slot (Airtel/BSNL)",
            "RF Front-End & Antenna",
            "4.0V Power Regulator",
            "2000uF Peak Current Capacitors"
        ]
    )

    # Column 3: Nuvoton UI Board
    add_block_cell(
        top_cells[2],
        "Nuvoton UI Board",
        "HMI Subsystem",
        HEX_TERTIARY,
        HEX_LIGHT_TERTIARY,
        [
            "Nuvoton MG51 MCU (8051-Core)",
            "16x2 Alphanumeric LCD Display",
            "6-Key Tactile Keypad Scan",
            "UART Display Protocol Engine",
            "HD44780 Command Parser",
            "EXT INT Keypress Generator",
            "Continuous 5V Sleep Power"
        ]
    )

    # Inter-board Bus Table / Connectors
    doc.add_paragraph().paragraph_format.space_after = Pt(6)
    conn_table = doc.add_table(rows=3, cols=3)
    conn_table.alignment = WD_TABLE_ALIGNMENT.CENTER

    c_cells0 = conn_table.rows[0].cells
    c_cells0[0].text = "◄── UART1 (GPIO 14 RX / GPIO 4 TX @ 115200 Baud) ──►"
    c_cells0[1].text = "ESP32 ◄──► Nuvoton UI Data Line"
    c_cells0[2].text = "Bidirectional screen drawing & remapped keypress stream"
    
    c_cells1 = conn_table.rows[1].cells
    c_cells1[0].text = "◄── UART2 (GPIO 16 RX / GPIO 17 TX @ 115200 Baud) ──►"
    c_cells1[1].text = "ESP32 ◄──► GPRS Modem Line"
    c_cells1[2].text = "AT command interface & Direct TCP socket payload stream"

    c_cells2 = conn_table.rows[2].cells
    c_cells2[0].text = "──► EXT INT (GPIO 27 ext0 LOW) & PWR (GPIO 32) ──►"
    c_cells2[1].text = "ESP32 ◄──► Nuvoton Sleep & Wake"
    c_cells2[2].text = "Nuvoton pulls INT LOW on keypress to wake ESP32 from Deep Sleep"

    for row in conn_table.rows:
        for cell in row.cells:
            set_cell_background(cell, "F2F4F4")
            set_cell_margins(cell, top=60, bottom=60, left=100, right=100)
            for p in cell.paragraphs:
                for r in p.runs:
                    r.font.size = Pt(8.5)

    doc.add_paragraph().paragraph_format.space_after = Pt(12)

    # -------------------------------------------------------------
    # Section 2: ESP32 Board Block Diagram
    # -------------------------------------------------------------
    h1 = doc.add_heading(level=1)
    r1 = h1.add_run("2. ESP32 Core Controller Board Block Diagram")
    r1.font.color.rgb = PRIMARY_COLOR

    esp_table = doc.add_table(rows=2, cols=3)
    esp_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    r0 = esp_table.rows[0].cells
    r1_cells = esp_table.rows[1].cells

    add_block_cell(
        r0[0],
        "Dual-Core Processing Engine",
        "ESP32 Microcontroller",
        HEX_PRIMARY,
        HEX_LIGHT_PRIMARY,
        [
            "Core 0: Dedicated GPRS stack, TCP sockets, AT parsing",
            "Core 1: Scheduler loop, sensor sampling, UI rendering",
            "FreeRTOS preemptive multitasking & semaphore locks"
        ]
    )

    add_block_cell(
        r0[1],
        "ULP Coprocessor & RTC RAM",
        "Low Power Subsystem",
        HEX_PRIMARY,
        HEX_LIGHT_PRIMARY,
        [
            "ULP Coprocessor running 1ms sampling loop during sleep",
            "Pulse counting for Rain Gauge & Wind Anemometer",
            "RTC Slow Memory preserving state across deep sleep"
        ]
    )

    add_block_cell(
        r0[2],
        "Storage & Filesystem",
        "Non-Volatile Memory",
        HEX_PRIMARY,
        HEX_LIGHT_PRIMARY,
        [
            "16MB Flash memory with SPIFFS filesystem",
            "Stores cur_file, unsent.txt queue, APN configs",
            "SPI SD Card slot for offline log backups & SD OTA"
        ]
    )

    add_block_cell(
        r1_cells[0],
        "I2C Sensor Bus (GPIO 21/22)",
        "Hardware Sensors",
        HEX_PRIMARY,
        HEX_LIGHT_PRIMARY,
        [
            "DS1307 Real Time Clock (0x68) for system timestamping",
            "HDC1080 / HDC2022 (0x40) Temperature & Humidity sensor",
            "BME280 Barometric Pressure sensor (Sea level correction)"
        ]
    )

    add_block_cell(
        r1_cells[1],
        "Power Control & ADCs",
        "Power Management",
        HEX_PRIMARY,
        HEX_LIGHT_PRIMARY,
        [
            "3.7V - 4.2V Li-ion Battery & Solar Input channels",
            "Calibrated Battery Voltage sense via ADC pins",
            "GPIO 32: Nuvoton 5V Power Gate (Held HIGH in sleep)",
            "GPIO 26: GPRS Modem Power Enable MOSFET Gate"
        ]
    )

    add_block_cell(
        r1_cells[2],
        "External I/O Interfaces",
        "Peripherals",
        HEX_PRIMARY,
        HEX_LIGHT_PRIMARY,
        [
            "GPIO 27: ext0 External Interrupt input from Nuvoton",
            "GPIO 14/4: UART1 serial interface to Nuvoton UI",
            "GPIO 16/17: UART2 serial interface to SIMCom modem",
            "GPIO 33: Hardware Reset line to SIMCom modem"
        ]
    )

    doc.add_paragraph().paragraph_format.space_after = Pt(12)

    # -------------------------------------------------------------
    # Section 3: GPRS Board Block Diagram
    # -------------------------------------------------------------
    h1 = doc.add_heading(level=1)
    r1 = h1.add_run("3. GPRS Modem Board Block Diagram")
    r1.font.color.rgb = SECONDARY_COLOR

    gprs_table = doc.add_table(rows=2, cols=2)
    gprs_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    g0 = gprs_table.rows[0].cells
    g1 = gprs_table.rows[1].cells

    add_block_cell(
        g0[0],
        "Cellular Core Engine",
        "SIMCom Module",
        HEX_SECONDARY,
        HEX_LIGHT_SECONDARY,
        [
            "SIMCom A7672S / A7670 / A7672M6 Chipset",
            "LTE Cat-1 4G Network Engine with 2G GSM Fallback",
            "115200 Baud UART AT Command interface",
            "PWRKEY & RESET Hardware control inputs"
        ]
    )

    add_block_cell(
        g0[1],
        "Direct Layer-4 TCP Engine",
        "Firmware Socket Stack",
        HEX_SECONDARY,
        HEX_LIGHT_SECONDARY,
        [
            "Bypasses modem high-level HTTP parser defects",
            "Layer-4 Sockets (AT+NETOPEN, AT+CIPOPEN, AT+CIPSEND)",
            "Direct IP connection to server (No DNS delay)",
            "Instant '>' buffer prompt response (< 5ms)"
        ]
    )

    add_block_cell(
        g1[0],
        "SIM Interface & Discovery",
        "Carrier Engine",
        HEX_SECONDARY,
        HEX_LIGHT_SECONDARY,
        [
            "Dual/Single SIM Slot supporting 1.8V / 3.0V SIM cards",
            "Auto Carrier Discovery via ICCID (899116 / 899145)",
            "Supports Airtel M2M, Airtel Commercial, BSNL, Jio",
            "Automatic APN selection (airteliot.com / bsnlnet)"
        ]
    )

    add_block_cell(
        g1[1],
        "Power & RF Front-End",
        "Hardware Power Rail",
        HEX_SECONDARY,
        HEX_LIGHT_SECONDARY,
        [
            "Dedicated 4.0V Voltage Regulator rail",
            "2000uF Tantalum/Bulk capacitors for 2A peak GPRS bursts",
            "Main Cellular Antenna connector & RF Matching Network",
            "Signal quality RSSI / CSQ detection circuit"
        ]
    )

    doc.add_paragraph().paragraph_format.space_after = Pt(12)

    # -------------------------------------------------------------
    # Section 4: Nuvoton UI Board Block Diagram
    # -------------------------------------------------------------
    h1 = doc.add_heading(level=1)
    r1 = h1.add_run("4. Nuvoton UI Board Block Diagram")
    r1.font.color.rgb = TERTIARY_COLOR

    nuv_table = doc.add_table(rows=2, cols=2)
    nuv_table.alignment = WD_TABLE_ALIGNMENT.CENTER
    n0 = nuv_table.rows[0].cells
    n1 = nuv_table.rows[1].cells

    add_block_cell(
        n0[0],
        "Microcontroller Core",
        "Nuvoton MG51 MCU",
        HEX_TERTIARY,
        HEX_LIGHT_TERTIARY,
        [
            "Nuvoton MG51 8051-core high-speed MCU",
            "Dedicated firmware for LCD control & Keypad scanning",
            "115200 Baud UART Interface to ESP32 (GPIO 14/4)",
            "Receives display update commands from ESP32"
        ]
    )

    add_block_cell(
        n0[1],
        "Alphanumeric LCD Module",
        "Display Engine",
        HEX_TERTIARY,
        HEX_LIGHT_TERTIARY,
        [
            "16x2 Character Alphanumeric LCD display",
            "HD44780 Command Parser emulation",
            "Supports display clear (0x01), cursor position (0xC0)",
            "Display ON/OFF control (0x0C / 0x08) for power saving"
        ]
    )

    add_block_cell(
        n1[0],
        "Tactile Matrix Keypad",
        "User Inputs",
        HEX_TERTIARY,
        HEX_LIGHT_TERTIARY,
        [
            "6-Button Tactile Keypad (UP, DOWN, LEFT, RIGHT, SET, CLR)",
            "Continuous key Matrix scanning & hardware debouncing",
            "Remaps key output to AIO9 internal key codes",
            "Transmits ASCII key codes over UART to ESP32"
        ]
    )

    add_block_cell(
        n1[1],
        "Power & Wakeup Control",
        "Power Subsystem",
        HEX_TERTIARY,
        HEX_LIGHT_TERTIARY,
        [
            "Powered continuously via 5V rail (ESP32 GPIO 32 HIGH)",
            "Dedicated INT pin tied to ESP32 GPIO 27 (ext0)",
            "Pulls INT LOW on any keypress to wake ESP32 from Deep Sleep",
            "Allows zero keypress loss during deep sleep"
        ]
    )

    doc.add_paragraph().paragraph_format.space_after = Pt(12)

    # -------------------------------------------------------------
    # Section 5: Complete Signal & Pinout Matrix
    # -------------------------------------------------------------
    h1 = doc.add_heading(level=1)
    r1 = h1.add_run("5. Signal & Interconnection Pinout Matrix")
    r1.font.color.rgb = PRIMARY_COLOR

    pin_headers = ["Subsystem", "Signal Name", "ESP32 Pin", "Target Board Pin", "Type / Protocol", "Description & Logic"]
    pin_data = [
        ["Nuvoton UI", "UI_TX", "GPIO 4", "Nuvoton RX", "UART Out (115200 Baud)", "Transmits screen drawing & cursor commands from ESP32 to Nuvoton."],
        ["Nuvoton UI", "UI_RX", "GPIO 14", "Nuvoton TX", "UART In (115200 Baud)", "Receives remapped keypress byte stream from Nuvoton keypad."],
        ["Nuvoton UI", "UI_INT", "GPIO 27", "Nuvoton INT", "Digital Input (ext0 LOW)", "Keypress trigger line that wakes ESP32 from Deep Sleep mode."],
        ["Nuvoton UI", "UI_PWR", "GPIO 32", "Nuvoton 5V Gate", "Digital Output (MOSFET)", "Kept HIGH in sleep mode so Nuvoton remains powered for keypress wakeup."],
        ["GPRS Modem", "GPRS_TX", "GPIO 17", "SIMCom RXD", "UART Out (115200 Baud)", "Sends AT commands and raw Layer-4 TCP socket HTTP POST payloads."],
        ["GPRS Modem", "GPRS_RX", "GPIO 16", "SIMCom TXD", "UART In (115200 Baud)", "Receives modem responses, TCP connection status, and HTTP 200 OK."],
        ["GPRS Modem", "GPRS_PWR", "GPIO 26 / 5", "SIMCom PWRKEY", "Digital Output", "Power pulse control line to turn SIMCom modem ON/OFF."],
        ["GPRS Modem", "GPRS_RST", "GPIO 33 / 4", "SIMCom RESET", "Digital Output", "Hardware emergency reset trigger for modem recovery."],
        ["Sensors & RTC", "I2C_SDA", "GPIO 21", "RTC / Sensors SDA", "I2C Data (400kHz)", "Shared I2C data bus for DS1307 RTC, HDC1080/2022, and BME280."],
        ["Sensors & RTC", "I2C_SCL", "GPIO 22", "RTC / Sensors SCL", "I2C Clock (400kHz)", "Shared I2C clock bus for all onboard environmental sensors."],
        ["Rain / Wind", "RAIN_IN", "GPIO (ULP)", "Tipping Bucket", "Digital Pulse Input", "Rain gauge tipping bucket pulses counted by ULP coprocessor."],
        ["Rain / Wind", "WIND_IN", "GPIO (ULP)", "Anemometer", "Digital Pulse Input", "Wind speed reed switch pulses counted by ULP coprocessor."],
        ["Power Sense", "BAT_SENSE", "GPIO 34-39", "Battery Divider", "Analog ADC Input", "Calibrated battery voltage divider measurement input."]
    ]

    pin_table = doc.add_table(rows=len(pin_data) + 1, cols=6)
    pin_table.alignment = WD_TABLE_ALIGNMENT.CENTER

    hdr_cells = pin_table.rows[0].cells
    for i, title in enumerate(pin_headers):
        hdr_cells[i].text = title
        set_cell_background(hdr_cells[i], HEX_PRIMARY)
        for p in hdr_cells[i].paragraphs:
            for r in p.runs:
                r.font.bold = True
                r.font.color.rgb = WHITE
                r.font.size = Pt(8.5)

    for row_idx, row_vals in enumerate(pin_data):
        row_cells = pin_table.rows[row_idx + 1].cells
        bg = "F4F6F7" if row_idx % 2 == 0 else "FFFFFF"
        for col_idx, val in enumerate(row_vals):
            row_cells[col_idx].text = val
            set_cell_background(row_cells[col_idx], bg)
            set_cell_margins(row_cells[col_idx], top=60, bottom=60, left=80, right=80)
            for p in row_cells[col_idx].paragraphs:
                for r in p.runs:
                    r.font.size = Pt(8.0)
                    if col_idx in [1, 2, 3]:
                        r.font.bold = True
                        r.font.name = "Consolas"

    # Save document
    doc_path = "AIO9_ESP32_GPRS_Nuvoton_Block_Diagrams.docx"
    doc.save(doc_path)
    print(f"Successfully generated Word Document: {doc_path}")

if __name__ == "__main__":
    create_block_diagram_doc()

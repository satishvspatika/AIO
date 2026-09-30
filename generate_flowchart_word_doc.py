import os
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

import docx
from docx import Document
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml import parse_xml, OxmlElement
from docx.oxml.ns import nsdecls, qn

def generate_flowchart_images():
    print("Generating flowchart images...")
    
    # -----------------------------------------------------------------
    # Flowchart 1: Top-Level System Flowchart Diagram
    # -----------------------------------------------------------------
    fig, ax = plt.subplots(figsize=(12, 14), dpi=300)
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 140)
    ax.axis('off')

    # Color Palette
    c_blue = '#1F618D'      # ESP32 Core
    c_orange = '#D35400'    # GPRS Modem
    c_teal = '#117A65'      # Nuvoton UI
    c_green = '#27AE60'     # Success / Network
    c_gray = '#5D6D7E'      # Flow boxes
    c_yellow = '#F39C12'    # Decision boxes

    def draw_box(x, y, w, h, text, color, boxstyle="round,pad=0.5", fontsize=9, bold=True, textcolor="white"):
        rect = mpatches.FancyBboxPatch((x, y), w, h, boxstyle=boxstyle, ec="black", fc=color, lw=1.5)
        ax.add_patch(rect)
        weight = 'bold' if bold else 'normal'
        ax.text(x + w/2.0, y + h/2.0, text, color=textcolor, fontsize=fontsize,
                ha='center', va='center', weight=weight, multialignment='center')

    def draw_arrow(x1, y1, x2, y2, label=""):
        ax.annotate('', xy=(x2, y2), xytext=(x1, y1),
                    arrowprops=dict(facecolor='black', edgecolor='black', arrowstyle="->", lw=2, mutation_scale=15))
        if label:
            mx, my = (x1 + x2)/2.0, (y1 + y2)/2.0
            ax.text(mx + 1.5, my, label, fontsize=8, weight='bold', color='#2C3E50')

    # Diagram Title Header
    ax.text(50, 136, "AIO9 System-Wide Operational Flowchart", fontsize=14, weight='bold', ha='center', color='#1A5276')

    # Column 1: Power & Boot Flow (x=5)
    draw_box(5, 120, 26, 8, "Power On / Sleep Wakeup\n(RTC Timer / GPIO 27 INT)", c_blue)
    draw_arrow(18, 120, 18, 110)
    
    draw_box(5, 102, 26, 8, "Hardware & RTC Init\nCheck Reset Reason & RAM", c_blue)
    draw_arrow(18, 102, 18, 92)

    draw_box(5, 84, 26, 8, "Sensor Sampling Task\n(Read HDC1080, RTC, ADCs)", c_blue)
    draw_arrow(18, 84, 18, 74)

    draw_box(5, 66, 26, 8, "ULP Pulse Counter Read\n(Rain Gauge & Wind Speed)", c_blue)
    draw_arrow(18, 66, 18, 56)

    draw_box(5, 48, 26, 8, "Log Telemetry to SPIFFS\nWrite Record & Update Queue", c_blue)

    # Column 2: Inter-Board UI Flow (x=37)
    draw_box(37, 120, 26, 8, "Nuvoton MG51 Power Gate\nGPIO 32 HIGH (Continuous 5V)", c_teal)
    draw_arrow(50, 120, 50, 110)

    draw_box(37, 102, 26, 8, "Keypad Scan Engine\n6-Key Matrix Debounce", c_teal)
    draw_arrow(50, 102, 50, 92)

    draw_box(37, 84, 26, 8, "UART1 Bus (GPIO 14/4)\nDisplay Commands & Keys", c_teal)
    draw_arrow(50, 84, 50, 74)

    draw_box(37, 66, 26, 8, "LCD Refresh Engine\n16x2 Display (HD44780)", c_teal)
    draw_arrow(50, 66, 50, 56)

    draw_box(37, 48, 26, 8, "Keypress Interrupt\nGPIO 27 INT LOW -> ESP32", c_teal)

    # Column 3: Cellular GPRS & Layer-4 TCP Flow (x=69)
    draw_box(69, 120, 26, 8, "GPRS Modem Power On\nGPIO 26 High Pulse", c_orange)
    draw_arrow(82, 120, 82, 110)

    draw_box(69, 102, 26, 8, "SIM ICCID Auto-Discovery\nAirtel / BSNL / Jio APN", c_orange)
    draw_arrow(82, 102, 82, 92)

    draw_box(69, 84, 26, 8, "Network Registration\n4G CEREG / 2G CREG Attach", c_green)
    draw_arrow(82, 84, 82, 74)

    draw_box(69, 66, 26, 8, "Direct Layer-4 TCP Socket\nAT+CIPOPEN & AT+CIPSEND", c_green)
    draw_arrow(82, 66, 82, 56)

    draw_box(69, 48, 26, 8, "Transmit HTTP POST & Backlog\nReceive 200 OK Response", c_green)

    # Connecting Arrows across Subsystems
    draw_arrow(31, 52, 37, 52, "State")
    draw_arrow(31, 88, 37, 88, "Data")
    draw_arrow(31, 124, 69, 124, "Trigger")
    draw_arrow(31, 52, 69, 52, "Payload")

    # Bottom Merged Shutdown & Sleep Box
    draw_arrow(18, 48, 18, 34)
    draw_arrow(82, 48, 82, 34)
    
    draw_box(20, 22, 60, 10, "Graceful GPRS Shutdown & Deep Sleep Entry\n(GPRS Power Off -> Calculate Sleep Seconds -> ESP32 Deep Sleep)", c_blue, fontsize=10)

    plt.tight_layout()
    img_path1 = "flowchart_system.png"
    plt.savefig(img_path1, bbox_inches='tight', dpi=300)
    plt.close()
    print("Saved flowchart_system.png")

    # -----------------------------------------------------------------
    # Flowchart 2: ESP32 Internal Functional Flowchart Diagram
    # -----------------------------------------------------------------
    fig, ax = plt.subplots(figsize=(12, 10), dpi=300)
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 100)
    ax.axis('off')

    ax.text(50, 96, "ESP32 Core Controller Board Internal Flowchart", fontsize=14, weight='bold', ha='center', color='#1A5276')

    # Flow Steps
    draw_box(35, 82, 30, 8, "ESP32 Boot / Reset Evaluator", c_blue)
    draw_arrow(50, 82, 50, 74)

    draw_box(15, 64, 30, 8, "Core 0: Network & GPRS Tasks\n(AT Parser, Socket, Backlog)", c_blue)
    draw_box(55, 64, 30, 8, "Core 1: Scheduler & Sensors\n(I2C Read, ULP Fetch, UI)", c_blue)

    draw_arrow(50, 74, 30, 64)
    draw_arrow(50, 74, 70, 64)

    draw_box(15, 46, 30, 8, "SPIFFS & SD Storage Engine\n(Record Locking & Queuing)", c_blue)
    draw_box(55, 46, 30, 8, "ULP 1ms Pulse Coprocessor\n(Rain & Wind Speed Counters)", c_blue)

    draw_arrow(30, 64, 30, 54)
    draw_arrow(70, 64, 70, 54)

    draw_box(35, 28, 30, 8, "Power & Sleep Manager\n(Battery ADC, GPIO 32 Hold)", c_blue)

    draw_arrow(30, 46, 45, 28)
    draw_arrow(70, 46, 55, 28)

    draw_box(35, 10, 30, 8, "ESP32 Deep Sleep State", c_gray)
    draw_arrow(50, 28, 50, 18)

    plt.tight_layout()
    img_path2 = "flowchart_esp32.png"
    plt.savefig(img_path2, bbox_inches='tight', dpi=300)
    plt.close()
    print("Saved flowchart_esp32.png")

    # -----------------------------------------------------------------
    # Flowchart 3: GPRS Cellular Subsystem Flowchart
    # -----------------------------------------------------------------
    fig, ax = plt.subplots(figsize=(12, 10), dpi=300)
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 100)
    ax.axis('off')

    ax.text(50, 96, "GPRS Modem Board Execution Flowchart", fontsize=14, weight='bold', ha='center', color='#D35400')

    draw_box(35, 84, 30, 8, "GPRS Power On Trigger\n(GPIO 26 High Pulse)", c_orange)
    draw_arrow(50, 84, 50, 74)

    draw_box(35, 66, 30, 8, "SIM Card & APN Discovery\n(AT+CPIN?, AT+ICCID)", c_orange)
    draw_arrow(50, 66, 50, 56)

    draw_box(35, 48, 30, 8, "4G/2G Network Attach\n(AT+CEREG? / AT+CREG?)", c_green)
    draw_arrow(50, 48, 50, 38)

    draw_box(35, 30, 30, 8, "Direct Layer-4 TCP Connection\n(AT+CIPOPEN -> AT+CIPSEND)", c_green)
    draw_arrow(50, 30, 50, 20)

    draw_box(35, 10, 30, 8, "HTTP POST Transmission & 200 OK\n(Backlog Queue Flush)", c_green)

    plt.tight_layout()
    img_path3 = "flowchart_gprs.png"
    plt.savefig(img_path3, bbox_inches='tight', dpi=300)
    plt.close()
    print("Saved flowchart_gprs.png")

    # -----------------------------------------------------------------
    # Flowchart 4: Nuvoton UI Subsystem Flowchart
    # -----------------------------------------------------------------
    fig, ax = plt.subplots(figsize=(12, 10), dpi=300)
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 100)
    ax.axis('off')

    ax.text(50, 96, "Nuvoton UI Board Subsystem Flowchart", fontsize=14, weight='bold', ha='center', color='#117A65')

    draw_box(35, 84, 30, 8, "Continuous 5V Power Active\n(GPIO 32 HIGH during sleep)", c_teal)
    draw_arrow(50, 84, 50, 74)

    draw_box(35, 66, 30, 8, "Matrix Keypad Scanning\n(UP, DOWN, SET, CLR)", c_teal)
    draw_arrow(50, 66, 50, 56)

    draw_box(15, 44, 30, 10, "Keypress Detected?\nPull GPIO 27 INT LOW\n(Wake ESP32)", c_yellow, boxstyle="square,pad=0.5", textcolor="black")
    draw_box(55, 44, 30, 10, "UART Packet Engine\nReceive LCD Commands\n(0x01 Clear, 0xC0 Cursor)", c_teal)

    draw_arrow(50, 56, 30, 54)
    draw_arrow(50, 56, 70, 54)

    draw_box(35, 20, 30, 10, "16x2 LCD Display Refresh\n(HD44780 Screen Draw)", c_teal)

    draw_arrow(30, 44, 45, 30)
    draw_arrow(70, 44, 55, 30)

    plt.tight_layout()
    img_path4 = "flowchart_nuvoton.png"
    plt.savefig(img_path4, bbox_inches='tight', dpi=300)
    plt.close()
    print("Saved flowchart_nuvoton.png")

def create_word_doc():
    print("Creating Word document with flowcharts...")
    doc = Document()

    PRIMARY_COLOR = RGBColor(15, 76, 129)     # Deep Blue
    SECONDARY_COLOR = RGBColor(211, 84, 0)    # Orange
    TERTIARY_COLOR = RGBColor(17, 122, 101)   # Teal
    DARK_TEXT = RGBColor(44, 62, 80)
    MUTED_TEXT = RGBColor(127, 140, 141)
    WHITE = RGBColor(255, 255, 255)

    # Set Margins
    for section in doc.sections:
        section.top_margin = Inches(0.7)
        section.bottom_margin = Inches(0.7)
        section.left_margin = Inches(0.7)
        section.right_margin = Inches(0.7)

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

    # Document Header Title
    title_p = doc.add_paragraph()
    title_p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    title_run = title_p.add_run("AIO9 System Flowchart & Detailed Block Diagram Manual")
    title_run.font.size = Pt(22)
    title_run.font.bold = True
    title_run.font.color.rgb = PRIMARY_COLOR
    title_p.paragraph_format.space_after = Pt(4)

    sub_p = doc.add_paragraph()
    sub_p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    sub_run = sub_p.add_run("Complete Operational Flowcharts & Subsystem Hardware Diagrams for ESP32, GPRS & Nuvoton Boards")
    sub_run.font.size = Pt(12)
    sub_run.font.italic = True
    sub_run.font.color.rgb = MUTED_TEXT
    sub_p.paragraph_format.space_after = Pt(18)

    doc.add_paragraph(
        "This document presents the complete system-wide operational flowchart alongside detailed hardware block diagrams for the AIO9 station architecture. "
        "Each section contains visual flowchart diagrams, block-by-block functional specifications, inter-board communication protocols, and signal descriptions."
    )

    # -------------------------------------------------------------
    # Section 1: System-Wide Operational Flowchart
    # -------------------------------------------------------------
    h1 = doc.add_heading(level=1)
    r1 = h1.add_run("1. System-Wide Operational Flowchart & Multi-Board Execution")
    r1.font.color.rgb = PRIMARY_COLOR

    doc.add_paragraph("The flowchart below details the complete execution lifecycle across the ESP32 Board, GPRS Board, and Nuvoton UI Board:")
    
    # Insert Diagram 1 Image
    p_img1 = doc.add_paragraph()
    p_img1.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_img1.add_run().add_picture("flowchart_system.png", width=Inches(6.5))

    doc.add_paragraph().paragraph_format.space_after = Pt(8)

    # Flowchart Step Descriptions
    steps_data = [
        ("Phase 1: Boot & Reset Reason Check", "ESP32 wakes from Deep Sleep via RTC timer or Nuvoton keypress (GPIO 27 INT LOW). Inspects RTC RAM variables and initializes SPIFFS and I2C sensors."),
        ("Phase 2: Sensor Sampling & ULP Read", "Polled readings from HDC1080/2022 (Temp/Hum), DS1307 RTC, and BME280. ULP Coprocessor pulse counts for Rain Gauge and Wind Anemometer are retrieved."),
        ("Phase 3: Inter-Board UI Interface", "Nuvoton MG51 MCU scans tactile keypad continuously. Display updates and menu screens are transmitted over UART1 (115200 baud)."),
        ("Phase 4: Cellular & Direct TCP Socket", "Modem powered via GPIO 26. SIM ICCID auto-discovery selects APN (airteliot.com / bsnlnet). Direct Layer-4 TCP engine connects and posts HTTP raw payload."),
        ("Phase 5: Backlog Flush & Deep Sleep", "Transmits up to 15 queued backlog records from SPIFFS. Performs graceful GPRS shutdown (AT+CPOWD), blanks UI display, and enters ESP32 Deep Sleep.")
    ]

    for title, desc in steps_data:
        p_step = doc.add_paragraph()
        p_step.paragraph_format.space_after = Pt(4)
        r_t = p_step.add_run(f"• {title}: ")
        r_t.bold = True
        r_t.font.color.rgb = PRIMARY_COLOR
        r_d = p_step.add_run(desc)
        r_d.font.color.rgb = DARK_TEXT

    doc.add_paragraph().paragraph_format.space_after = Pt(12)

    # -------------------------------------------------------------
    # Section 2: ESP32 Board Flowchart & Block Diagram
    # -------------------------------------------------------------
    h1 = doc.add_heading(level=1)
    r1 = h1.add_run("2. ESP32 Core Controller Board Flowchart & Architecture")
    r1.font.color.rgb = PRIMARY_COLOR

    p_img2 = doc.add_paragraph()
    p_img2.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_img2.add_run().add_picture("flowchart_esp32.png", width=Inches(6.2))

    doc.add_paragraph("The ESP32 Board operates as the central system master with the following hardware blocks:")

    esp_blocks = [
        ("Dual-Core Processor", "Xtensa LX6 @ 240MHz. Core 0 manages GPRS UART, Layer-4 TCP sockets, and AT commands. Core 1 manages system scheduling, sensors, and UI rendering."),
        ("ULP Coprocessor", "Runs 1ms sampling loop during Deep Sleep. Counts tipping bucket rain pulses and wind anemometer pulses without waking the main CPU."),
        ("RTC Slow Memory", "512-byte reserved RTC RAM preserving pulse accumulators, APN settings, unsent backlog indices, and carrier states across deep sleep."),
        ("Flash & SPIFFS Storage", "16MB Flash with SPIFFS filesystem storing telemetry logs (cur_file, unsent.txt) and station settings. Includes SPI SD card slot for backups."),
        ("I2C Bus (GPIO 21/22)", "Hardware I2C bus communicating with DS1307 RTC (0x68), HDC1080/2022 (0x40), and BME280 barometric pressure sensor."),
        ("Power Management & ADCs", "Battery voltage divider sense via ADC. GPIO 32 controls Nuvoton 5V power gate; GPIO 26 controls GPRS modem power MOSFET gate.")
    ]

    for b_title, b_desc in esp_blocks:
        p_b = doc.add_paragraph()
        p_b.paragraph_format.space_after = Pt(4)
        r_bt = p_b.add_run(f"■ {b_title}: ")
        r_bt.bold = True
        r_bt.font.color.rgb = PRIMARY_COLOR
        r_bd = p_b.add_run(b_desc)
        r_bd.font.color.rgb = DARK_TEXT

    doc.add_paragraph().paragraph_format.space_after = Pt(12)

    # -------------------------------------------------------------
    # Section 3: GPRS Board Flowchart & Block Diagram
    # -------------------------------------------------------------
    h1 = doc.add_heading(level=1)
    r1 = h1.add_run("3. GPRS Modem Board Flowchart & Architecture")
    r1.font.color.rgb = SECONDARY_COLOR

    p_img3 = doc.add_paragraph()
    p_img3.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_img3.add_run().add_picture("flowchart_gprs.png", width=Inches(6.2))

    gprs_blocks = [
        ("SIMCom A7672 Core", "LTE Cat-1 4G cellular engine with 2G GSM fallback. Controlled over UART2 (GPIO 16/17 @ 115200 baud)."),
        ("Direct Layer-4 TCP Engine", "Hardware transport layer sockets (AT+CIPOPEN, AT+CIPSEND). Bypasses internal modem HTTP stack bugs and provides instant '>' prompt in < 5ms."),
        ("SIM Interface & Discovery", "Supports Airtel M2M, Airtel Commercial, BSNL, and Jio. Auto-discovers carrier via ICCID prefix and configures matching APN (airteliot.com / bsnlnet)."),
        ("Power & RF Front-End", "Dedicated 4.0V voltage regulator rail backed by 2000µF bulk capacitors to support 2A peak current bursts during GPRS/LTE transmissions.")
    ]

    for b_title, b_desc in gprs_blocks:
        p_b = doc.add_paragraph()
        p_b.paragraph_format.space_after = Pt(4)
        r_bt = p_b.add_run(f"■ {b_title}: ")
        r_bt.bold = True
        r_bt.font.color.rgb = SECONDARY_COLOR
        r_bd = p_b.add_run(b_desc)
        r_bd.font.color.rgb = DARK_TEXT

    doc.add_paragraph().paragraph_format.space_after = Pt(12)

    # -------------------------------------------------------------
    # Section 4: Nuvoton UI Board Flowchart & Block Diagram
    # -------------------------------------------------------------
    h1 = doc.add_heading(level=1)
    r1 = h1.add_run("4. Nuvoton UI Board Flowchart & Architecture")
    r1.font.color.rgb = TERTIARY_COLOR

    p_img4 = doc.add_paragraph()
    p_img4.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_img4.add_run().add_picture("flowchart_nuvoton.png", width=Inches(6.2))

    nuv_blocks = [
        ("Nuvoton MG51 MCU", "8051-core microcontroller running dedicated HMI firmware. Communicates with ESP32 via UART1 (GPIO 14/4 @ 115200 baud)."),
        ("16x2 LCD Display Engine", "HD44780 character LCD module. Parses screen clear (0x01), cursor positioning (0xC0), and display backlight control commands."),
        ("Tactile Keypad Scanner", "6-button matrix scanner (UP, DOWN, LEFT, RIGHT, SET, CLR) with hardware debouncing and automatic key code remapping."),
        ("Continuous Power & Wakeup", "Powered on continuous 5V rail (GPIO 32 HIGH). Pulls INT line LOW (GPIO 27 ext0) on any keypress to wake ESP32 from Deep Sleep instantly.")
    ]

    for b_title, b_desc in nuv_blocks:
        p_b = doc.add_paragraph()
        p_b.paragraph_format.space_after = Pt(4)
        r_bt = p_b.add_run(f"■ {b_title}: ")
        r_bt.bold = True
        r_bt.font.color.rgb = TERTIARY_COLOR
        r_bd = p_b.add_run(b_desc)
        r_bd.font.color.rgb = DARK_TEXT

    doc.add_paragraph().paragraph_format.space_after = Pt(12)

    # -------------------------------------------------------------
    # Section 5: Pinout & Signal Reference Matrix
    # -------------------------------------------------------------
    h1 = doc.add_heading(level=1)
    r1 = h1.add_run("5. Signal & Interconnection Pinout Reference Matrix")
    r1.font.color.rgb = PRIMARY_COLOR

    pin_headers = ["Subsystem", "Signal Name", "ESP32 Pin", "Target Board Pin", "Protocol", "Signal Function"]
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
        ["Sensors & RTC", "I2C_SCL", "GPIO 22", "RTC / Sensors SCL", "I2C Clock (400kHz)", "Shared I2C clock bus for all onboard environmental sensors."]
    ]

    pin_table = doc.add_table(rows=len(pin_data) + 1, cols=6)
    pin_table.alignment = WD_TABLE_ALIGNMENT.CENTER

    hdr_cells = pin_table.rows[0].cells
    for i, title in enumerate(pin_headers):
        hdr_cells[i].text = title
        set_cell_background(hdr_cells[i], "0F4C81")
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

    doc_path = "AIO9_System_Flowchart_and_Block_Diagrams.docx"
    doc.save(doc_path)
    print(f"Successfully generated Word Document with embedded flowcharts: {doc_path}")

if __name__ == "__main__":
    generate_flowchart_images()
    create_word_doc()

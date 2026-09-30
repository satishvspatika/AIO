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

def generate_hardware_diagram_images():
    print("Generating clean, non-overlapping Hardware Block Diagrams...")

    # Color Tokens
    C_ESP32 = '#1A5276'      # Navy Blue for ESP32 Main PCB
    C_GPRS = '#BA4A00'       # Burnt Orange for GPRS Modem PCB
    C_NUVOTON = '#117864'    # Teal for Nuvoton UI PCB
    C_CHIP = '#2C3E50'       # Dark Slate for ICs
    C_POWER = '#C0392B'      # Red for Power Rails
    C_SIGNAL = '#27AE60'     # Green for Signals
    C_BG_ESP32 = '#EBF5FB'
    C_BG_GPRS = '#FBEEE6'
    C_BG_NUVOTON = '#E8F8F5'
    C_BOX = '#FFFFFF'

    def draw_rounded_rect(ax, x, y, w, h, title, subtitle, bg_color, border_color, title_color="white"):
        # Draw outer rectangle
        rect = mpatches.FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0.3,rounding_size=0.4",
                                       ec=border_color, fc=bg_color, lw=2)
        ax.add_patch(rect)
        
        # Header banner
        header_h = max(2.5, h * 0.22)
        header_y = y + h - header_h
        banner = mpatches.FancyBboxPatch((x, header_y), w, header_h, boxstyle="round,pad=0.1,rounding_size=0.2",
                                         ec=border_color, fc=border_color, lw=1)
        ax.add_patch(banner)

        # Title text inside header
        ax.text(x + w/2.0, header_y + header_h/2.0, title, color=title_color, fontsize=9.5,
                ha='center', va='center', weight='bold')

        # Body Subtitle / Items text below header
        if subtitle:
            ax.text(x + w/2.0, (y + header_y)/2.0, subtitle, color='#2C3E50', fontsize=8,
                    ha='center', va='center', weight='normal', multialignment='center')

    def draw_orthogonal_arrow(ax, x1, y1, x2, y2, label="", color="black", linestyle="-", lw=1.8):
        # Draw orthogonal line path so arrows NEVER cross over text or boxes
        ax.annotate('', xy=(x2, y2), xytext=(x1, y1),
                    arrowprops=dict(arrowstyle="->", color=color, lw=lw, linestyle=linestyle, mutation_scale=14))
        if label:
            mx, my = (x1 + x2)/2.0, (y1 + y2)/2.0
            # Offset label slightly above/beside line
            offset_y = 1.2 if y1 == y2 else 0.0
            offset_x = 1.2 if x1 == x2 else 0.0
            ax.text(mx + offset_x, my + offset_y, label, fontsize=7.5, weight='bold', color=color,
                    ha='center', va='center', bbox=dict(boxstyle="round,pad=0.2", fc="white", ec=color, lw=1))

    # =================================================================
    # DIAGRAM 1: Top-Level Hardware Boards Interconnection Schematic
    # =================================================================
    fig, ax = plt.subplots(figsize=(14, 10), dpi=300)
    ax.set_xlim(0, 140)
    ax.set_ylim(0, 100)
    ax.axis('off')

    ax.text(70, 96, "AIO9 Hardware Multi-Board System Interconnection Schematic",
            fontsize=13, weight='bold', ha='center', color='#1B4F72')

    # PCB Board Outline 1: ESP32 Main Controller PCB
    board1 = mpatches.FancyBboxPatch((5, 10), 42, 78, boxstyle="round,pad=0.5,rounding_size=0.5",
                                     ec=C_ESP32, fc=C_BG_ESP32, lw=2.5, linestyle="--")
    ax.add_patch(board1)
    ax.text(26, 85, "BOARD 1: ESP32 CORE MAIN PCB", fontsize=10, weight='bold', color=C_ESP32, ha='center')

    # Components inside Board 1
    draw_rounded_rect(ax, 10, 62, 32, 16, "ESP32-WROOM-32 MCU", "Dual-Core LX6 @ 240MHz\n520KB SRAM | ULP Core", C_BOX, C_ESP32)
    draw_rounded_rect(ax, 10, 42, 15, 14, "16MB Flash IC", "SPIFFS File OS", C_BOX, C_CHIP)
    draw_rounded_rect(ax, 27, 42, 15, 14, "DS1307 RTC IC", "I2C (0x68) | 32.768kHz", C_BOX, C_CHIP)
    draw_rounded_rect(ax, 10, 22, 15, 14, "HDC1080 IC", "Temp/Hum Sensor", C_BOX, C_CHIP)
    draw_rounded_rect(ax, 27, 22, 15, 14, "Power MOSFETs", "GPIO 32 (UI) / 26 (GPRS)", C_BOX, C_POWER)

    # PCB Board Outline 2: GPRS Modem PCB
    board2 = mpatches.FancyBboxPatch((93, 52), 42, 36, boxstyle="round,pad=0.5,rounding_size=0.5",
                                     ec=C_GPRS, fc=C_BG_GPRS, lw=2.5, linestyle="--")
    ax.add_patch(board2)
    ax.text(114, 85, "BOARD 2: GPRS MODEM PCB", fontsize=10, weight='bold', color=C_GPRS, ha='center')

    # Components inside Board 2
    draw_rounded_rect(ax, 98, 68, 32, 14, "SIMCom A7672 Module", "LTE Cat-1 4G / 2G Engine", C_BOX, C_GPRS)
    draw_rounded_rect(ax, 98, 55, 15, 10, "SIM Slot Socket", "Airtel / BSNL SIM", C_BOX, C_CHIP)
    draw_rounded_rect(ax, 115, 55, 15, 10, "4.0V Buck + Caps", "2000uF Bulk Cap Bank", C_BOX, C_POWER)

    # PCB Board Outline 3: Nuvoton UI PCB
    board3 = mpatches.FancyBboxPatch((93, 10), 42, 36, boxstyle="round,pad=0.5,rounding_size=0.5",
                                     ec=C_NUVOTON, fc=C_BG_NUVOTON, lw=2.5, linestyle="--")
    ax.add_patch(board3)
    ax.text(114, 43, "BOARD 3: NUVOTON UI PCB", fontsize=10, weight='bold', color=C_NUVOTON, ha='center')

    # Components inside Board 3
    draw_rounded_rect(ax, 98, 26, 32, 14, "Nuvoton MG51 MCU", "8051 Core @ 24MHz | UART", C_BOX, C_NUVOTON)
    draw_rounded_rect(ax, 98, 13, 15, 10, "16x2 LCD Module", "HD44780 Driver", C_BOX, C_CHIP)
    draw_rounded_rect(ax, 115, 13, 15, 10, "6-Key Matrix", "Tactile Switches", C_BOX, C_CHIP)

    # Inter-Board Signals & Buses (Clear non-overlapping routing)
    # 1. ESP32 -> GPRS UART2 Bus
    draw_orthogonal_arrow(ax, 42, 73, 98, 73, "UART2 (GPIO 16/17 @ 115200)", C_GPRS, lw=2)

    # 2. ESP32 -> GPRS PWRKEY / RESET
    draw_orthogonal_arrow(ax, 42, 66, 98, 66, "PWRKEY/RESET (GPIO 26/33)", C_POWER, lw=1.8, linestyle="--")

    # 3. ESP32 -> Nuvoton UART1 Bus
    draw_orthogonal_arrow(ax, 42, 33, 98, 33, "UART1 (GPIO 14/4 @ 115200)", C_NUVOTON, lw=2)

    # 4. Nuvoton -> ESP32 EXT INT Wakeup
    draw_orthogonal_arrow(ax, 98, 24, 42, 24, "INT LOW (GPIO 27 ext0 Wake)", C_SIGNAL, lw=2)

    # 5. ESP32 -> Nuvoton UI 5V Power Gate
    draw_orthogonal_arrow(ax, 42, 17, 98, 17, "5V VCC Rail (GPIO 32 MOSFET)", C_POWER, lw=1.8, linestyle="--")

    plt.tight_layout()
    img_hw1 = "hw_board_system.png"
    plt.savefig(img_hw1, bbox_inches='tight', dpi=300)
    plt.close()
    print("Saved hw_board_system.png")

    # =================================================================
    # DIAGRAM 2: ESP32 Core Main PCB Hardware Component Layout
    # =================================================================
    fig, ax = plt.subplots(figsize=(12, 10), dpi=300)
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 100)
    ax.axis('off')

    ax.text(50, 96, "BOARD 1: ESP32 Core Main PCB Hardware Circuit Diagram", fontsize=13, weight='bold', ha='center', color=C_ESP32)

    # Core Chip
    draw_rounded_rect(ax, 35, 68, 30, 20, "ESP32-WROOM-32", "Microcontroller Core\n(Xtensa Dual-Core 240MHz)\nSRAM: 520KB | ULP Engine", C_BG_ESP32, C_ESP32)

    # Subsystem Components around ESP32
    draw_rounded_rect(ax, 5, 70, 22, 14, "16MB Flash IC", "SPI NOR Flash\n(SPI Bus)", C_BOX, C_CHIP)
    draw_rounded_rect(ax, 5, 50, 22, 14, "MicroSD Slot", "SPI Interface\n(CS, CLK, MOSI, MISO)", C_BOX, C_CHIP)

    draw_rounded_rect(ax, 73, 70, 22, 14, "DS1307 RTC IC", "I2C Bus (0x68)\n+ 32.768kHz Crystal", C_BOX, C_CHIP)
    draw_rounded_rect(ax, 73, 50, 22, 14, "HDC1080 / BME280", "I2C Bus (0x40 / 0x76)\nTemp, Hum, Pressure", C_BOX, C_CHIP)

    draw_rounded_rect(ax, 5, 20, 24, 16, "Sensors & Pulse Inputs", "Rain Gauge (Optocoupler)\nWind Anemometer (Reed)\nWind Vane (Analog ADC)", C_BOX, C_SIGNAL)
    draw_rounded_rect(ax, 38, 20, 24, 16, "Power Management", "3.7V Li-ion & Solar In\n3.3V LDO Regulator\nADC Volt Sense Dividers", C_BOX, C_POWER)
    draw_rounded_rect(ax, 71, 20, 24, 16, "Power Switches", "GPIO 32: UI Power MOSFET\nGPIO 26: GPRS Power MOSFET", C_BOX, C_POWER)

    # Connections
    draw_orthogonal_arrow(ax, 27, 77, 35, 77, "SPI Bus", C_CHIP)
    draw_orthogonal_arrow(ax, 27, 57, 35, 57, "SPI Bus", C_CHIP)
    draw_orthogonal_arrow(ax, 73, 77, 65, 77, "I2C (21/22)", C_CHIP)
    draw_orthogonal_arrow(ax, 73, 57, 65, 57, "I2C (21/22)", C_CHIP)

    draw_orthogonal_arrow(ax, 17, 36, 42, 68, "ULP / ADC Pins", C_SIGNAL)
    draw_orthogonal_arrow(ax, 50, 36, 50, 68, "3.3V Rail & ADC", C_POWER)
    draw_orthogonal_arrow(ax, 83, 36, 58, 68, "MOSFET Gates", C_POWER)

    plt.tight_layout()
    img_hw2 = "hw_board_esp32.png"
    plt.savefig(img_hw2, bbox_inches='tight', dpi=300)
    plt.close()
    print("Saved hw_board_esp32.png")

    # =================================================================
    # DIAGRAM 3: GPRS Modem PCB Hardware Diagram
    # =================================================================
    fig, ax = plt.subplots(figsize=(12, 9), dpi=300)
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 100)
    ax.axis('off')

    ax.text(50, 94, "BOARD 2: GPRS Modem PCB Hardware Circuit Diagram", fontsize=13, weight='bold', ha='center', color=C_GPRS)

    draw_rounded_rect(ax, 35, 55, 30, 24, "SIMCom A7672 Module", "Cellular Core Engine\nLTE Cat-1 4G / 2G GSM\nBaseband & Layer-4 TCP", C_BG_GPRS, C_GPRS)

    draw_rounded_rect(ax, 5, 60, 22, 16, "SIM Card Socket", "Push-Push Slot\nESD Protection Array\nAirtel / BSNL SIM", C_BOX, C_CHIP)
    draw_rounded_rect(ax, 73, 60, 22, 16, "RF Front-End", "SMA Antenna Port\n50 Ohm Match Filter\n4G High Gain Antenna", C_BOX, C_GPRS)

    draw_rounded_rect(ax, 15, 20, 30, 18, "4.0V Power Supply", "Buck DC-DC Converter\n2000uF Tantalum/Cap Bank\nSupplies 2A Burst Current", C_BOX, C_POWER)
    draw_rounded_rect(ax, 55, 20, 30, 18, "Control Interface", "PWRKEY Transistor (GPIO 26)\nRESET Transistor (GPIO 33)\nUART2 TX/RX (GPIO 16/17)", C_BOX, C_CHIP)

    # Connections
    draw_orthogonal_arrow(ax, 27, 68, 35, 68, "SIM Lines", C_CHIP)
    draw_orthogonal_arrow(ax, 65, 68, 73, 68, "50 Ohm RF", C_GPRS)
    draw_orthogonal_arrow(ax, 30, 38, 43, 55, "4.0V VCC Rail", C_POWER)
    draw_orthogonal_arrow(ax, 70, 38, 57, 55, "Control / UART", C_CHIP)

    plt.tight_layout()
    img_hw3 = "hw_board_gprs.png"
    plt.savefig(img_hw3, bbox_inches='tight', dpi=300)
    plt.close()
    print("Saved hw_board_gprs.png")

    # =================================================================
    # DIAGRAM 4: Nuvoton UI PCB Hardware Diagram
    # =================================================================
    fig, ax = plt.subplots(figsize=(12, 9), dpi=300)
    ax.set_xlim(0, 100)
    ax.set_ylim(0, 100)
    ax.axis('off')

    ax.text(50, 94, "BOARD 3: Nuvoton UI PCB Hardware Circuit Diagram", fontsize=13, weight='bold', ha='center', color=C_NUVOTON)

    draw_rounded_rect(ax, 35, 55, 30, 24, "Nuvoton MG51 MCU", "HMI Microcontroller\n8051 Core @ 24MHz\n16KB Flash | 1KB SRAM", C_BG_NUVOTON, C_NUVOTON)

    draw_rounded_rect(ax, 5, 60, 22, 16, "16x2 Character LCD", "HD44780 Parallel Bus\nContrast Potentiometer\nBacklight Driver", C_BOX, C_CHIP)
    draw_rounded_rect(ax, 73, 60, 22, 16, "6-Key Switch Array", "Tactile Push Buttons\nRC Debounce Filter\nMatrix Scan Lines", C_BOX, C_CHIP)

    draw_rounded_rect(ax, 15, 20, 30, 18, "5V Power Terminal", "5V Input Header\n(Sourced from ESP32 Board\nvia GPIO 32 MOSFET)", C_BOX, C_POWER)
    draw_rounded_rect(ax, 55, 20, 30, 18, "Inter-Board Signals", "UART1 TX/RX (115200 Baud)\nOpen-Collector INT Out\n(Pulls GPIO 27 LOW)", C_BOX, C_SIGNAL)

    # Connections
    draw_orthogonal_arrow(ax, 35, 68, 27, 68, "4-bit LCD Bus", C_CHIP)
    draw_orthogonal_arrow(ax, 73, 68, 65, 68, "Key Matrix", C_CHIP)
    draw_orthogonal_arrow(ax, 30, 38, 43, 55, "5V VCC Power", C_POWER)
    draw_orthogonal_arrow(ax, 70, 38, 57, 55, "UART & INT", C_SIGNAL)

    plt.tight_layout()
    img_hw4 = "hw_board_nuvoton.png"
    plt.savefig(img_hw4, bbox_inches='tight', dpi=300)
    plt.close()
    print("Saved hw_board_nuvoton.png")

def create_hardware_word_doc():
    print("Creating Hardware-Focused Word Document...")
    doc = Document()

    PRIMARY_COLOR = RGBColor(26, 82, 118)     # Deep Navy
    SECONDARY_COLOR = RGBColor(186, 74, 0)    # Orange
    TERTIARY_COLOR = RGBColor(17, 120, 100)   # Teal
    DARK_TEXT = RGBColor(44, 62, 80)
    MUTED_TEXT = RGBColor(127, 140, 141)
    WHITE = RGBColor(255, 255, 255)

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

    # Document Title Block
    title_p = doc.add_paragraph()
    title_p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    title_run = title_p.add_run("AIO9 Hardware PCB Architecture & Signal Block Diagram Manual")
    title_run.font.size = Pt(22)
    title_run.font.bold = True
    title_run.font.color.rgb = PRIMARY_COLOR
    title_p.paragraph_format.space_after = Pt(4)

    sub_p = doc.add_paragraph()
    sub_p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    sub_run = sub_p.add_run("Detailed PCB Circuit Diagrams, Hardware Chips, Power Rails & Pinout Interfaces")
    sub_run.font.size = Pt(12)
    sub_run.font.italic = True
    sub_run.font.color.rgb = MUTED_TEXT
    sub_p.paragraph_format.space_after = Pt(18)

    doc.add_paragraph(
        "This document details the hardware board architecture, physical PCB component layouts, IC chipsets, power regulation circuits, "
        "and signal interconnections for the three main circuit boards in the AIO9 system: Board 1 (ESP32 Core Main PCB), Board 2 (GPRS Modem PCB), and Board 3 (Nuvoton UI PCB)."
    )

    # -------------------------------------------------------------
    # Section 1: Multi-Board Interconnection Diagram
    # -------------------------------------------------------------
    h1 = doc.add_heading(level=1)
    r1 = h1.add_run("1. Multi-Board Hardware System Interconnection Diagram")
    r1.font.color.rgb = PRIMARY_COLOR

    doc.add_paragraph("The schematic below illustrates the three physical PCB boards, their onboard microcontrollers, power gates, and inter-board signal header lines:")

    p_img1 = doc.add_paragraph()
    p_img1.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_img1.add_run().add_picture("hw_board_system.png", width=Inches(6.6))

    doc.add_paragraph().paragraph_format.space_after = Pt(8)

    # Inter-board Connections Detail Table
    doc.add_paragraph("Inter-Board Header Connections & Bus Protocols:", style='List Bullet')
    
    conn_headers = ["Header Name", "From Board", "To Board", "Signals & Pins", "Protocol / Power Specs"]
    conn_data = [
        ["J1 UI Serial Header", "Board 1 (ESP32)", "Board 3 (Nuvoton)", "TX (GPIO 4) ──► RX\nRX (GPIO 14) ◄── TX", "UART1 @ 115200 Baud, 3.3V Logic. Transmits screen drawing commands & receives remapped key codes."],
        ["J2 UI Control Header", "Board 3 (Nuvoton)", "Board 1 (ESP32)", "INT (Nuvoton) ──► GPIO 27\n5V VCC ◄── GPIO 32 MOSFET", "Open-Collector INT pulls GPIO 27 LOW on keypress to wake ESP32. 5V rail supplied via GPIO 32 P-FET."],
        ["J3 Modem Data Header", "Board 1 (ESP32)", "Board 2 (GPRS)", "TX (GPIO 17) ──► RXD\nRX (GPIO 16) ◄── TXD", "UART2 @ 115200 Baud. AT command interface & Layer-4 TCP socket HTTP POST payload stream."],
        ["J4 Modem Power Header", "Board 1 (ESP32)", "Board 2 (GPRS)", "PWRKEY ◄── GPIO 26 / 5\nRESET ◄── GPIO 33 / 4", "NPN Transistor driven pulse lines. Controls modem power state & emergency hardware reset."]
    ]

    c_table = doc.add_table(rows=len(conn_data) + 1, cols=5)
    c_table.alignment = WD_TABLE_ALIGNMENT.CENTER

    hdr_cells = c_table.rows[0].cells
    for i, title in enumerate(conn_headers):
        hdr_cells[i].text = title
        set_cell_background(hdr_cells[i], "1A5276")
        for p in hdr_cells[i].paragraphs:
            for r in p.runs:
                r.font.bold = True
                r.font.color.rgb = WHITE
                r.font.size = Pt(8.5)

    for row_idx, row_vals in enumerate(conn_data):
        row_cells = c_table.rows[row_idx + 1].cells
        bg = "F4F6F7" if row_idx % 2 == 0 else "FFFFFF"
        for col_idx, val in enumerate(row_vals):
            row_cells[col_idx].text = val
            set_cell_background(row_cells[col_idx], bg)
            set_cell_margins(row_cells[col_idx], top=60, bottom=60, left=80, right=80)
            for p in row_cells[col_idx].paragraphs:
                for r in p.runs:
                    r.font.size = Pt(8.0)
                    if col_idx in [0, 3]:
                        r.font.bold = True
                        r.font.name = "Consolas"

    doc.add_paragraph().paragraph_format.space_after = Pt(12)

    # -------------------------------------------------------------
    # Section 2: Board 1 - ESP32 Core Main PCB Block Diagram
    # -------------------------------------------------------------
    h1 = doc.add_heading(level=1)
    r1 = h1.add_run("2. Board 1: ESP32 Core Main PCB Hardware Architecture")
    r1.font.color.rgb = PRIMARY_COLOR

    p_img2 = doc.add_paragraph()
    p_img2.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_img2.add_run().add_picture("hw_board_esp32.png", width=Inches(6.3))

    doc.add_paragraph("Hardware Component Specifications for Board 1:")

    esp_comps = [
        ("ESP32-WROOM-32 Module", "Dual-Core Xtensa 32-bit LX6 MCU @ 240MHz. Contains 520KB SRAM, ULP Coprocessor for low-power pulse counting, and internal RTC Controller."),
        ("16MB SPI NOR Flash IC", "High-density flash memory interfaced via SPI bus (CS, CLK, MOSI, MISO). Hosts the SPIFFS filesystem storing cur_file, unsent.txt queue, and APN configuration."),
        ("DS1307 Real Time Clock IC", "Dedicated hardware RTC IC connected to I2C bus at address 0x68. Backed by a 32.768kHz crystal oscillator and CR2032 3V coin cell battery."),
        ("HDC1080 / BME280 Sensors", "Precision relative humidity & temperature digital sensor (I2C 0x40) and barometric pressure sensor (I2C 0x76) for MSLP sea-level calculation."),
        ("Optocouplers & Pulse Conditioning", "Optically isolated input filtering circuits for Tipping Bucket Rain Gauge and Reed Switch Anemometer. Connected directly to ESP32 ULP coprocessor pins."),
        ("Power Management & MOSFET Gates", "3.3V LDO regulator powering core ICs. High-side P-FET power switches for UI 5V rail (GPIO 32) and GPRS modem power rail (GPIO 26). Resistor ADC voltage dividers for battery & solar sense.")
    ]

    for title, desc in esp_comps:
        p_c = doc.add_paragraph()
        p_c.paragraph_format.space_after = Pt(4)
        r_t = p_c.add_run(f"■ {title}: ")
        r_t.bold = True
        r_t.font.color.rgb = PRIMARY_COLOR
        r_d = p_c.add_run(desc)
        r_d.font.color.rgb = DARK_TEXT

    doc.add_paragraph().paragraph_format.space_after = Pt(12)

    # -------------------------------------------------------------
    # Section 3: Board 2 - GPRS Modem PCB Block Diagram
    # -------------------------------------------------------------
    h1 = doc.add_heading(level=1)
    r1 = h1.add_run("3. Board 2: GPRS Modem PCB Hardware Architecture")
    r1.font.color.rgb = SECONDARY_COLOR

    p_img3 = doc.add_paragraph()
    p_img3.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_img3.add_run().add_picture("hw_board_gprs.png", width=Inches(6.3))

    gprs_comps = [
        ("SIMCom A7672 Module", "Multi-band LTE Cat-1 4G module with 2G GSM/EDGE fallback. Contains integrated Layer-4 TCP socket engine operating over 115200 baud UART."),
        ("4.0V Buck Converter & Capacitor Bank", "High-efficiency DC-DC step-down converter providing dedicated 4.0V power. Parallel 2000µF Tantalum/Low-ESR capacitor bank supplies 2A peak current during RF transmission bursts."),
        ("SIM Card Socket & ESD Protection", "Push-push SIM card socket with multi-channel TVS diode ESD protection array. Supports 1.8V/3.0V SIM cards for Airtel M2M, Airtel Commercial, BSNL, and Jio."),
        ("RF Front-End & SMA Connector", "50Ω microstrip transmission line with LC pi-matching network leading to standard SMA female connector and external high-gain cellular antenna.")
    ]

    for title, desc in gprs_comps:
        p_c = doc.add_paragraph()
        p_c.paragraph_format.space_after = Pt(4)
        r_t = p_c.add_run(f"■ {title}: ")
        r_t.bold = True
        r_t.font.color.rgb = SECONDARY_COLOR
        r_d = p_c.add_run(desc)
        r_d.font.color.rgb = DARK_TEXT

    doc.add_paragraph().paragraph_format.space_after = Pt(12)

    # -------------------------------------------------------------
    # Section 4: Board 3 - Nuvoton UI PCB Block Diagram
    # -------------------------------------------------------------
    h1 = doc.add_heading(level=1)
    r1 = h1.add_run("4. Board 3: Nuvoton UI PCB Hardware Architecture")
    r1.font.color.rgb = TERTIARY_COLOR

    p_img4 = doc.add_paragraph()
    p_img4.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_img4.add_run().add_picture("hw_board_nuvoton.png", width=Inches(6.3))

    nuv_comps = [
        ("Nuvoton MG51FB9AE MCU", "High-speed 8051-core 8-bit MCU (24MHz, 16KB Flash, 1KB SRAM). Runs custom UI packet parser and key matrix scanner."),
        ("16x2 Character LCD Module", "Alphanumeric liquid crystal display with HD44780 parallel controller interface, contrast trimpot adjustment, and transistor-driven LED backlight."),
        ("6-Key Tactile Switch Array", "Matrix tactile switches (UP, DOWN, LEFT, RIGHT, SET, CLR) with hardware RC filter debouncing. Scanned continuously by Nuvoton GPIOs."),
        ("Open-Collector Interrupt Circuit", "NPN transistor driver pulling the INT line LOW (tied to ESP32 GPIO 27) whenever any key is pressed, waking ESP32 from Deep Sleep instantly.")
    ]

    for title, desc in nuv_comps:
        p_c = doc.add_paragraph()
        p_c.paragraph_format.space_after = Pt(4)
        r_t = p_c.add_run(f"■ {title}: ")
        r_t.bold = True
        r_t.font.color.rgb = TERTIARY_COLOR
        r_d = p_c.add_run(desc)
        r_d.font.color.rgb = DARK_TEXT

    doc.add_paragraph().paragraph_format.space_after = Pt(12)

    # -------------------------------------------------------------
    # Section 5: Hardware Component & IC Summary Table
    # -------------------------------------------------------------
    h1 = doc.add_heading(level=1)
    r1 = h1.add_run("5. Master Hardware Component & IC Lookup Table")
    r1.font.color.rgb = PRIMARY_COLOR

    master_headers = ["PCB Board", "Component / IC", "Part Number / Model", "Interface / Bus", "Hardware Function"]
    master_data = [
        ["Board 1 (ESP32)", "Main MCU Module", "ESP32-WROOM-32", "Internal SPI / I2C / UART", "Master dual-core processor, ULP pulse counting, data logging."],
        ["Board 1 (ESP32)", "Flash Storage IC", "W25Q128JV (16MB)", "SPI (CS, CLK, MOSI, MISO)", "Non-volatile SPIFFS filesystem for logs & unsent backlog queue."],
        ["Board 1 (ESP32)", "Real Time Clock IC", "DS1307ZN", "I2C (0x68) @ 400kHz", "Hardware real-time clock backed by 32.768kHz crystal & 3V battery."],
        ["Board 1 (ESP32)", "Temp & Humidity IC", "HDC1080DMBR", "I2C (0x40) @ 400kHz", "Digital temperature & relative humidity sensor."],
        ["Board 1 (ESP32)", "Pressure Sensor IC", "BME280", "I2C (0x76) @ 400kHz", "Barometric pressure sensor for sea-level pressure calculation."],
        ["Board 1 (ESP32)", "Power MOSFET Gates", "AO4407A (P-FET)", "GPIO 32 / GPIO 26", "High-side power switches for UI 5V rail and GPRS modem rail."],
        ["Board 2 (GPRS)", "Cellular Engine", "SIMCom A7672S", "UART2 @ 115200 Baud", "LTE Cat-1 4G / 2G modem with Layer-4 TCP socket engine."],
        ["Board 2 (GPRS)", "Buck Regulator", "LM2596 / MP1584", "4.0V VCC Power Rail", "DC-DC step down converter supplying 4.0V to SIMCom modem."],
        ["Board 2 (GPRS)", "Bulk Cap Array", "2000µF Tantalum", "4.0V Power Bus", "Energy reservoir providing 2A burst current during RF transmit."],
        ["Board 3 (Nuvoton)", "HMI MCU", "Nuvoton MG51FB9AE", "UART1 @ 115200 Baud", "Keypad matrix scanning, LCD packet parsing, INT generation."],
        ["Board 3 (Nuvoton)", "LCD Character Display", "1602A (HD44780)", "4-bit Parallel Bus", "16x2 alphanumeric LCD display with LED backlight driver."]
    ]

    m_table = doc.add_table(rows=len(master_data) + 1, cols=5)
    m_table.alignment = WD_TABLE_ALIGNMENT.CENTER

    hdr_cells = m_table.rows[0].cells
    for i, title in enumerate(master_headers):
        hdr_cells[i].text = title
        set_cell_background(hdr_cells[i], "1A5276")
        for p in hdr_cells[i].paragraphs:
            for r in p.runs:
                r.font.bold = True
                r.font.color.rgb = WHITE
                r.font.size = Pt(8.5)

    for row_idx, row_vals in enumerate(master_data):
        row_cells = m_table.rows[row_idx + 1].cells
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

    doc_path = "AIO9_Hardware_PCB_Architecture_and_Block_Diagrams.docx"
    doc.save(doc_path)
    print(f"Successfully generated Hardware Word Document: {doc_path}")

if __name__ == "__main__":
    generate_hardware_diagram_images()
    create_hardware_word_doc()

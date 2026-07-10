# 🚗 CAN-Based Automotive Parameter Monitoring System

---

## 📖 Overview

This project implements a **real-time automotive parameter monitoring system** using the **CAN (Controller Area Network) protocol**.

It simulates communication between multiple Electronic Control Units (ECUs) to monitor key vehicle parameters such as:

* Engine temperature
* Fuel level
* Indicator status

The system is divided into multiple nodes, each responsible for specific functionality, and all nodes communicate over a **CAN bus** for reliable and efficient data exchange.

---

##  Block Diagram

<img width="921" height="629" alt="Screenshot 2026-05-06 191816" src="https://github.com/user-attachments/assets/1b430a46-33e1-42a1-a4ea-4ac1c386247c" />

---

## 🛠️ Hardware Components

| Component                       | Quantity | Function                                      |
| ------------------------------- | -------- | --------------------------------------------- |
| LPC2129 Microcontroller         | 3        | Controls each node (Main, Fuel, Indicator)    |
| CAN Bus (CAN_H, CAN_L)          | 1        | Communication network connecting all nodes    |
| DS18B20 Temperature Sensor      | 1        | Measures engine temperature                   |
| Fuel Gauge Sensor               | 1        | Provides analog fuel level input              |
| LCD (20x4)                      | 1        | Displays real-time vehicle parameters         |
| LEDs                            | 8        | Represent vehicle indicators                  |
| Switches                        | 2        | Trigger interrupts for indicator control      |
| USB to UART Converter           | 1        | Used for programming/debugging                |

---

## ⚙️ Working Principle

Each node in the system performs a specific function and communicates using the CAN protocol:

* The **Fuel Node** reads fuel level using ADC and sends data over CAN
* The **Main Node** reads temperature from DS18B20, receives fuel data, and updates the LCD
* The **Indicator Node** receives CAN messages and controls LEDs

All nodes are connected via the CAN bus, ensuring **real-time, reliable communication**.
Interrupts are used for fast response in indicator control.

---

## 📷 Output Images

### 🔧 Hardware Setup

<img width="1076" height="690" alt="HardWare_SetUp" src="https://github.com/user-attachments/assets/69a379a5-4438-4387-ae45-3e8b6c17cf30" />

---

## 📟 LCD Display

<img width="889" height="1600" alt="WhatsApp Image 2026-07-10 at 1 52 08 PM" src="https://github.com/user-attachments/assets/b6f195de-d2b2-486b-bff0-38ed5bbdc628" />

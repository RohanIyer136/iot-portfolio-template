# 📡 IoT Module 04: Smart Home Automation System

## 🔍 Project Overview

This project demonstrates a basic smart home automation setup using IoT hardware and platforms. The system allows users to remotely control home appliances and monitor environmental conditions such as temperature and humidity. Control is enabled via the Blynk IoT app and physical switches.

---

## 🛠️ Components Used

- **NodeMCU (ESP8266)** – Wi-Fi-enabled microcontroller  
- **DHT11 Sensor** – Temperature and humidity monitoring  
- **Relay Module** – Switching home appliances  
- **Blynk IoT App** – Mobile control interface  
- **Arduino IDE** – Programming environment  

---

## ⚙️ Core Functionalities

- ✅ Remote control of home appliances (fan, light, etc.)  
- 🌡️ Real-time sensor data (temperature & humidity)  
- 📲 Smartphone-based interface (Blynk)  
- 🔄 Physical switch override  
- 🎙️ Voice control enabled via Google Assistant (optional)  

---

## 🔧 Setup Instructions

1. **Hardware Connections**
   - Connect DHT11 to NodeMCU  
   - Wire relay to appliances (fan, bulb, etc.)  
   - Set up physical switches in parallel  

2. **Software Setup**
   - Install Blynk library on Arduino IDE  
   - Upload code to NodeMCU using Arduino IDE  
   - Configure Blynk project with correct auth token and device pin mappings  

3. **Deployment**
   - Power the NodeMCU using USB or adapter  
   - Open Blynk app to monitor and control devices  
   - Test local manual switches and verify cloud control works seamlessly  

---

## 📘 Learning Outcomes

- 🧠 Grasp of IoT system architecture  
- 🔗 Integration of sensors and actuators  
- ☁️ Understanding cloud-based device control  
- 📡 Wi-Fi communication with microcontrollers  
- 🛠️ Combining physical and virtual controls in home automation  

---

## 📸 Project Preview

![Smart Home IoT Project](https://tse4.mm.bing.net/th?id=OIP.KaH5nXVN0lbFqmFzjIt_pQHaEK&pid=Api)

---

## 📝 Notes

- Make sure to use a stable Wi-Fi network for smooth Blynk operation  
- Use proper insulation and safety protocols when connecting appliances  
- Customize the Blynk UI to suit additional controls if needed  

---

## 📚 Credits

Project inspired by introductory IoT automation exercises for smart homes using NodeMCU and Blynk.

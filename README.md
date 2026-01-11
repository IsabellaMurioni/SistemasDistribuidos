# Sistemas Distribuidos

## 📌 Desarrollado por
**Isabella Murioni** y **Bautista Crocco**

---

## 🧠 Descripción del Proyecto
Este proyecto corresponde a un trabajo práctico de la materia **Sistemas Distribuidos** y consiste en la implementación de la **lógica de un agente dentro de un entorno de juego distribuido**.

El sistema modela un **estado global del juego** que es compartido entre distintos agentes. Cada agente procesa turnos y toma decisiones en función de la información disponible.  
La lógica está desarrollada en **C++**, con un diseño modular que separa claramente el estado del juego de la lógica del agente, facilitando la escalabilidad y el mantenimiento.

---

## 🎮 ¿Qué es el software?
El software implementa:

- Un **estado global del juego (`GameState`)** que contiene información sobre:
  - Agentes
  - Bases
  - Turnos
  - Estado final del juego
  - Equipo ganador

- Un **agente simple (`SimpleAgent`)** que:
  - Se inicializa con un ID y un equipo
  - Procesa cada turno del juego
  - Ejecuta acciones simples según reglas predefinidas

Este tipo de arquitectura es común en **sistemas distribuidos, simulaciones y sistemas multi-agente**.

---

## ▶️ Ejecución del Proyecto

El proyecto se ejecuta utilizando **tres terminales**, simulando un entorno distribuido con servidor y agentes.

---

### 🖥️ Terminal 1 – Build inicial del proyecto

```bash
cd tp4-murioni-crocco
code .
mkdir build && cd build
cmake ..
make

---

### 🤖 Terminal 2 – Ejecución del agente
cd tp4-murioni-crocco
cd build
./agent

---

🌐 Terminal 3 – Ejecución del servidor
cd tp4-murioni-crocco
cd build
./server




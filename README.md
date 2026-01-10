# Sistemas Distribuidos

## 📌 Desarrollado por
**Isabella Murioni** y **Bautista Crocco**

---

## 🧠 Descripción del Proyecto
Este proyecto corresponde a un trabajo práctico de **Sistemas Distribuidos** y consiste en la implementación de la **lógica de un agente dentro de un entorno de juego distribuido**.

El sistema modela un estado global del juego compartido entre agentes, donde cada uno procesa turnos y toma decisiones en función de la información disponible. La lógica está desarrollada en **C++**, con un diseño modular que separa claramente el estado del juego de la lógica del agente.

---

## 🎮 ¿Qué es el software?
El software implementa:

- Un **estado global del juego (GameState)** que contiene información sobre:
  - Agentes
  - Bases
  - Turnos
  - Estado final del juego
  - Equipo ganador
- Un **agente simple (SimpleAgent)** que:
  - Se inicializa con un ID y un equipo
  - Procesa cada turno del juego
  - Ejecuta acciones simples según reglas predefinidas

Este tipo de arquitectura es común en **sistemas distribuidos, simulaciones y sistemas multi-agente**.

---

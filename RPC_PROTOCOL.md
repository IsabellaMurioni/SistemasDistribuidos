# Documentación del Protocolo RPC

Este documento describe el protocolo de RPC utilizado para la comunicación entre el coordinador del juego y los agentes en el sistema de juego. Todos los mensajes se intercambian a través de conexiones TCP utilizando formato JSON.

## Estructura de Mensajes

Todos los mensajes RPC siguen una estructura JSON común con los siguientes campos:

```json
{
    "id": "identificador_unico_de_llamada",
    "type": "tipo_de_mensaje"
    // ... campos adicionales específicos del tipo de mensaje
}
```

### Campos Comunes

-   **`id`** (string): Identificador único para la llamada, utilizado para emparejar solicitudes con respuestas
-   **`type`** (string): Especifica el tipo de mensaje RPC

## Mensajes de Solicitud

### 1. Solicitud de Registro de Agente

Utilizada por los agentes para registrarse con el coordinador del juego al conectarse.

```json
{
    "id": "id_de_llamada",
    "type": "register_agent",
    "agent_id": "identificador_unico_del_agente"
}
```

**Campos:**

-   `agent_id` (string): Identificador único para el agente

### 2. Solicitud de Recepción de Inteligencia

Enviada por el coordinador para notificar a los agentes información de inteligencia.

```json
{
    "id": "id_de_llamada",
    "type": "receive_intel",
    "intel": "cadena_de_datos_de_inteligencia"
}
```

**Campos:**

-   `intel` (string): Información de inteligencia que debe ser procesada por el agente

### 3. Solicitud de Notificación de Muerte

Enviada por el coordinador para informar a un agente que ha sido eliminado del juego.

```json
{
    "id": "id_de_llamada",
    "type": "notify_death"
}
```

**Campos:** Ninguno (solo campos comunes)

### 4. Solicitud de Notificación de Fin de Juego

Enviada por el coordinador para informar a los agentes que el juego ha terminado.

```json
{
    "id": "id_de_llamada",
    "type": "notify_game_over",
    "winning_team": 1
}
```

**Campos:**

-   `winning_team` (integer): Identificador del equipo ganador (0, 1, o -1 para empate)

### 5. Solicitud de Jugar Turno

Enviada por el coordinador para solicitar a un agente que haga un movimiento durante su turno. Incluye el estado completo del juego para permitir decisiones informadas.

```json
{
    "id": "id_de_llamada",
    "type": "play_turn",
    "agent_id": "identificador_del_agente",
    "state": {
        "agents": [
            {
                "id": "agent_1",
                "team": "red",
                "position": { "x": 5, "y": 3 },
                "facing": "east",
                "hp": 80,
                "max_hp": 100,
                "is_alive": true
            },
            {
                "id": "agent_2",
                "team": "blue",
                "position": { "x": 15, "y": 12 },
                "facing": "north",
                "hp": 90,
                "max_hp": 100,
                "is_alive": true
            }
        ],
        "bases": [
            {
                "team": "red",
                "position": { "x": 2, "y": 2 },
                "hp": 450,
                "max_hp": 500,
                "is_destroyed": false
            },
            {
                "team": "blue",
                "position": { "x": 18, "y": 18 },
                "hp": 500,
                "max_hp": 500,
                "is_destroyed": false
            }
        ],
        "current_turn": 42,
        "game_over": false,
        "winner": "",
        "config": {
            "map_width": 20,
            "map_height": 20,
            "max_turns": 500,
            "spawn_cooldown": 5
        }
    }
}
```

**Campos:**

-   `agent_id` (string): Identificador del agente cuyo turno es
-   `state` (object): Estado completo del juego con la siguiente información:

#### Estado del Juego (`state`)

**`agents`** (array): Lista de todos los agentes en el juego

-   `id` (string): Identificador único del agente
-   `team` (string): Equipo al que pertenece el agente
-   `position` (object): Posición actual en el mapa
    -   `x` (integer): Coordenada X
    -   `y` (integer): Coordenada Y
-   `facing` (string): Dirección hacia la que mira ("north", "south", "east", "west")
-   `hp` (integer): Puntos de vida actuales
-   `max_hp` (integer): Puntos de vida máximos
-   `is_alive` (boolean): Si el agente está vivo

**`bases`** (array): Lista de todas las bases en el juego

-   `team` (string): Equipo propietario de la base
-   `position` (object): Posición de la base en el mapa
-   `hp` (integer): Puntos de vida actuales de la base
-   `max_hp` (integer): Puntos de vida máximos de la base
-   `is_destroyed` (boolean): Si la base ha sido destruida

**`current_turn`** (integer): Número del turno actual

**`game_over`** (boolean): Si el juego ha terminado

**`winner`** (string): Equipo ganador (vacío si el juego continúa)

**`config`** (object): Configuración del juego

-   `map_width` (integer): Ancho del mapa
-   `map_height` (integer): Alto del mapa
-   `max_turns` (integer): Número máximo de turnos
-   `spawn_cooldown` (integer): Tiempo de espera para generar nuevos agentes

## Mensajes de Respuesta

### 1. Respuesta Vacía

Una respuesta genérica de éxito utilizada para solicitudes que no requieren datos específicos en el retorno (register_agent, receive_intel, notify_death, notify_game_over).

```json
{
    "id": "id_de_llamada",
    "status": "ok"
}
```

**Campos:**

-   `status` (string): Siempre "ok" indicando procesamiento exitoso

### 2. Respuesta de Jugar Turno

Respuesta de un agente indicando su acción elegida para su turno.

```json
{
    "id": "id_de_llamada",
    "action": "move north"
}
```

**Campos:**

-   `action` (string): La acción que el agente quiere realizar

**Acciones Comunes:**

-   **Movimiento:** `"move_north"` / `"move_south"` / `"move_east"` / `"move_west"` - Comandos de movimiento
-   **Ataque:** `"attack_north"` / `"attack_south"` / `"attack_east"` / `"attack_west"` - Comandos de ataque
-   **Defensa:** `"defend_north"` / `"defend_south"` / `"defend_east"` / `"defend_west"` - Comandos de defensa
-   **Comunicación:** `"send_message:mensaje"` - Enviar mensaje a otros agentes del equipo

## Flujo del Protocolo

### 1. Registro del Agente

1. El agente se conecta al servidor TCP del coordinador
2. El agente envía `RegisterAgentRequest`
3. El coordinador responde con `VoidResponse` si es exitoso
4. La conexión permanece abierta para la comunicación del juego

### 2. Comunicación del Juego

1. El coordinador envía solicitudes a los agentes (inteligencia, notificaciones de fin de juego, solicitudes de turno)
2. Los agentes responden apropiadamente:
    - `VoidResponse` para notificaciones
    - `PlayTurnResponse` para solicitudes de turno

### 3. Manejo de Errores

-   Los mensajes inválidos resultan en cierre de conexión
-   Timeouts (5 segundos) para solicitudes

## Ejemplo de Sesión de Comunicación

```
1. El agente se conecta y se registra:
   Agente -> Servidor: {"id":"1","type":"register_agent","agent_id":"blue_agent_001"}
   Servidor -> Agente: {"id":"1","status":"ok"}

2. El juego comienza, el agente recibe inteligencia:
   Servidor -> Agente: {"id":"2","type":"receive_intel","intel":"Enemigo avistado en (10,5)"}
   Agente -> Servidor: {"id":"2","status":"ok"}

3. Turno del agente para jugar:
   Servidor -> Agente: {"id":"3","type":"play_turn","agent_id":"blue_agent_001","state":{"agents":[...],"bases":[...],"current_turn":15,"game_over":false,"winner":"","config":{...}}}
   Agente -> Servidor: {"id":"3","action":"move north"}

4. El juego termina:
   Servidor -> Agente: {"id":"4","type":"notify_game_over","winning_team":1}
   Agente -> Servidor: {"id":"4","status":"ok"}
```

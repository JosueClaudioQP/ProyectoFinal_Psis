# ADMIN LINUX

## Programación de Sistemas

Proyecto Final desarrollado para el curso de **Programación de Sistemas** de la **Universidad Nacional de San Agustín de Arequipa (UNSA)**.

Carrera:

**Ingeniería de Sistemas**

---

# Descripción

ADMIN LINUX es una herramienta de administración para sistemas Linux desarrollada en lenguaje C.

El proyecto integra diferentes módulos que permiten administrar procesos, archivos, ejecutar comandos del sistema, realizar respaldos, analizar scripts Bash y administrar una cola de descargas desde una única aplicación.

El objetivo es ofrecer una herramienta modular, ligera y fácil de utilizar para tareas básicas de administración en sistemas Linux.

---

# Funcionalidades

Actualmente el proyecto está organizado en los siguientes módulos:

- Administrador de procesos
- Shell de archivos
- Comandos Linux
- Sistema de respaldos
- Analizador de scripts Bash
- Cola de descargas

Cada módulo se implementa de manera independiente para facilitar el desarrollo colaborativo.

---

# Estructura del proyecto

```
admin_linux
│
├── include/
├── src/
├── backups/
├── docs/
├── downloads/
├── historial/
├── logs/
├── scripts/
├── Makefile
└── README.md
```

---

# Requisitos

- Ubuntu 22.04 o superior
- GCC
- GNU Make

Las demás dependencias se encuentran en el archivo:

```
requirements.sh
```

---

# Compilación

Desde la carpeta del proyecto ejecutar:

```bash
make
```

---

# Ejecución

```bash
./admin_linux
```

o

```bash
make run
```

---

# Limpieza

```bash
make clean
```

---

# Programadores

Este proyecto fue desarrollado por:

- Carrillo Villalta Gustavo Alonso
- Choque Sanchez Alejandra Camila
- Mamani Cespedes Jhonatan Benjamin
- Quispe Pauccar Josue CLaudio

Universidad Nacional de San Agustín

Escuela Profesional de Ingeniería de Sistemas

---

# Licencia

Proyecto desarrollado únicamente con fines académicos para el curso de Programación de Sistemas.
# ADMIN LINUX

## Programación de Sistemas

Proyecto Final desarrollado para el curso de **Programación de Sistemas** de la **Universidad Nacional de San Agustín de Arequipa (UNSA)**.

Carrera:

**Ingeniería de Sistemas**

---

# Descripción

ADMIN LINUX es una herramienta de administración para sistemas Linux desarrollada en lenguaje C.

El proyecto integra diferentes módulos que permiten administrar procesos, archivos, ejecutar comandos del sistema, realizar respaldos, analizar scripts Bash y administrar una cola de descargas desde una única aplicación.

El objetivo es ofrecer una herramienta modular, ligera y fácil de utilizar para tareas básicas de administración en sistemas Linux.

---

# Funcionalidades

Actualmente el proyecto está organizado en los siguientes módulos:

- Administrador de procesos
- Shell de archivos
- Comandos Linux
- Sistema de respaldos
- Analizador de scripts Bash
- Cola de descargas

Cada módulo se implementa de manera independiente para facilitar el desarrollo colaborativo.

---

# Estructura del proyecto

```
admin_linux
│
├── include/
├── src/
├── backups/
├── docs/
├── downloads/
├── historial/
├── logs/
├── scripts/
├── Makefile
└── README.md
```

---

# Requisitos

- Ubuntu 22.04 o superior
- GCC
- GNU Make

Las demás dependencias se encuentran en el archivo:

```
requirements.sh
```

---

# Compilación

Desde la carpeta del proyecto ejecutar:

```bash
make
```

---

# Ejecución

```bash
./admin_linux
```

o

```bash
make run
```

---

# Uso de módulos

## Shell de archivos

Desde el menú principal seleccionar:

```bash
2
```

Opciones disponibles dentro del módulo:

- `1` para listar el contenido de una ruta
- `2` para copiar archivos o directorios
- `3` para mover archivos o directorios
- `4` para eliminar archivos o directorios
- `5` para buscar por nombre
- `6` para ver estadísticas de tamaño y elementos

## Comandos Linux

Desde el menú principal seleccionar:

```bash
3
```

Opciones disponibles dentro del módulo:

- `1` para ejecutar un comando Linux
- `2` para ver el historial de comandos ejecutados
- `3` para limpiar el historial

El historial se guarda en:

```bash
historial/comandos.log
```

---

# Limpieza

```bash
make clean
```

---

# Programadores

Este proyecto fue desarrollado por:

- Carrillo Villalta Gustavo Alonso
- Choque Sanchez Alejandra Camila
- Mamani Cespedes Jhonatan Benjamin
- Quispe Pauccar Josue CLaudio

Universidad Nacional de San Agustín

Escuela Profesional de Ingeniería de Sistemas

---

# Licencia

Proyecto desarrollado únicamente con fines académicos para el curso de Programación de Sistemas.

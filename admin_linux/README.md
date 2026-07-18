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

- Integrante 1
- Integrante 2
- Integrante 3
- Integrante 4

Universidad Nacional de San Agustín

Escuela Profesional de Ingeniería de Sistemas

---

# Licencia

Proyecto desarrollado únicamente con fines académicos para el curso de Programación de Sistemas.

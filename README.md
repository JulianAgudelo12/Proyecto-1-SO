# Proyecto-1-S.O.
---
# Paralelismo en un Ambiente Controlado

Este proyecto fue desarrollado como parte del curso de Sistemas Operativos (ST0257) en la Universidad EAFIT. Su objetivo es explorar y analizar el comportamiento del paralelismo en diferentes sistemas operativos, utilizando implementaciones en C tanto para Windows como para Linux.

## Tabla de Contenidos

1. Descripción del Proyecto
2. Requisitos
3. Instalación
4. Uso
5. Resultados y Análisis
6. Problemas Conocidos
7. Conclusiones

## Descripción del Proyecto

Este proyecto implementa diferentes algoritmos de paralelismo en C para evaluar el rendimiento en sistemas operativos Windows y Linux. Se enfoca en la optimización del uso de memoria, la distribución de tareas entre múltiples núcleos, y la medición de tiempos de ejecución.

## Requisitos

- **Lenguaje de Programación:** C
- **Compilador:** GCC (Linux) / MinGW o Visual Studio (Windows)
- **Sistema Operativo:** Linux o Windows
- **Herramientas Adicionales:**
    - Canva (para la presentación)
    - Herramientas de monitoreo del sistema (por ejemplo, `htop` en Linux, el `administrador de tareas` en Windows)

## Instalación

1. **Clonar el repositorio:**
    
    ```bash
    ...
    
    ```
    
2. **Compilar el código:**
    - En Linux:
        
        ```bash
        ...
        
        ```
        
    - En Windows (MinGW):
        
        ```bash
        ...
        
        ```
        

## Uso

Para ejecutar el programa, simplemente corre el ejecutable generado:

```bash
...

```

Puedes modificar los parámetros de entrada directamente en el código fuente para ajustar el número de hilos o el tamaño de las tareas.

## Resultados y Análisis

El proyecto proporciona información detallada sobre los tiempos de ejecución en distintos escenarios (single-core vs multi-core). En nuestra investigación, observamos que en ciertos casos, el modo multicore mostró un rendimiento inesperadamente bajo, lo que resaltó la importancia de una correcta distribución de tareas y afinidad de núcleos.

## Problemas Conocidos

- **Desbalance en la Carga de Trabajo:** Como se menciona en el análisis, algunos procesos no se distribuyen equitativamente entre los núcleos, lo que causa ineficiencia.
- **Afinidad y Asignación de Procesos:** La asignación de procesos a núcleos específicos no siempre se realiza de manera óptima, lo que afecta el rendimiento.

## Conclusiones

1. La implementación y optimización del paralelismo en sistemas multicore requiere un cuidadoso balance entre tareas.
2. Factores como la afinidad de procesos y la contención de recursos pueden impactar significativamente en el rendimiento.
3. La correcta medición y análisis de tiempos de ejecución es crucial para identificar y resolver problemas en aplicaciones paralelas.

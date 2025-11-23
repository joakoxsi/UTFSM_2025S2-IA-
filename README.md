# UTFSM_2025S2-IA-
Este repositorio implementa un algoritmo de Simulated Annealing (SA) para resolver el Rotating Workforce Scheduling Problem (RWSP), un problema de planificación de turnos con restricciones de cobertura, bloques, secuencias prohibidas y fines de semana libres.


Proyecto RWSP-SA
- main.cpp
- Makefile
- parametros.txt         # Configuraciones de SA a probar
- metricas.csv           # Se genera automáticamente
- Resultados/            # Soluciones generadas
- Instancias/            # Archivos .txt del RWSP

# Informacion necesaria
- GNU Make 
- Compilador C++ (g++ o similar)
- Sistema operativo recomendado: WSL Ubuntu

# Ejecucion del Codigo
Se usa make run para ejecutar el codigo
1. Procesar todas las instancias dentro de la carpeta Instancias
2. Lee cada linea del archivo parametro para poder usarla en la ejecucion 
3. Guardar resultados en la carpeta /Resultados
4. Guardar los resultados en metricas.csv

# Explicación del Código
- Solución inicial: heurística greedy basada en la matriz de requerimientos.
- Vecindario: intercambio swap entre dos trabajadores en un día aleatorio.
- Función objetivo: suma de penalizaciones por cobertura, bloques, secuencias prohibidas y fines de semana trabajados.
- SA: aplica enfriamiento multiplicativo y criterio de Metropolis para aceptar o rechazar vecinos.

# ---------------------------------
# Makefile para Resolucion_RWSP
# ---------------------------------

# Compilador de C++
CXX = g++

# Banderas (flags) del compilador
# -std=c++17: Usar el estándar C++17 (puedes cambiarlo a c++11, c++14, c++20)
# -Wall: Mostrar todas las advertencias (Recomendado)
# -Wextra: Mostrar advertencias adicionales
# -g: Incluir símbolos de depuración (para usar gdb)
# -O2: Optimización de nivel 2 (puedes quitarla si prefieres depurar)
CXXFLAGS = -std=c++17 -Wall -Wextra -g -O2

# Nombre del archivo fuente
SRC = Resolucion_RWSP.cpp

# Nombre del ejecutable de salida
TARGET = Resolucion_RWSP

# ---------------------------------
# Objetivos (Targets)
# ---------------------------------

# El objetivo por defecto (se ejecuta con solo 'make')
# .PHONY le dice a 'make' que 'all' no es un archivo real
.PHONY: all
all: $(TARGET)

# Regla para construir el ejecutable
# Depende del archivo fuente
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Objetivo para limpiar los archivos compilados
# .PHONY le dice a 'make' que 'clean' no es un archivo real
.PHONY: clean
clean:
	rm -f $(TARGET) *.o

# Objetivo opcional para compilar y ejecutar
# .PHONY le dice a 'make' que 'run' no es un archivo real
.PHONY: run
run: all
	./$(TARGET)
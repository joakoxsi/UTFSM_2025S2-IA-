#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <sstream>
#include <random>
#include <algorithm>
#include <cmath>
#include <numeric> 

using namespace std;
namespace fs = std::filesystem;
struct TurnoInfo {
    string nombre;
    int minConsec;
    int maxConsec;
};

struct Instancia {
    int W,N,A;
    vector<vector<int>> matrizRequerimientos;
    vector<TurnoInfo> Turno;
    int minLibre,maxLibre;
    int minTrabajo,maxTrabajo;
    int cantL2,cantL3;

    vector<vector<string>> L2; 
    vector<vector<string>> L3;
};

struct ParamSA {
    double alpha;
    double tempMin;
    double tempInicial;
    unsigned int semilla;
};

Instancia cargaInstancia(string nombre){
    ifstream archivo_instancia(nombre);
    string linea;
    int espacio=0;
    Instancia instancia;
    if (!archivo_instancia.is_open()){
        throw runtime_error("Error: No se pudo abrir el archivo: " + nombre);
    }
    
    int fila = 0;
 
    stringstream ss;
    while (getline(archivo_instancia,linea)){
        if (linea.empty()){
            espacio++;
            fila = 0;
            continue;
        }
        switch (espacio)
        {
        case 0:
            instancia.W=stoi(linea);
            break;
        case 1:
            instancia.N=stoi(linea);
            break;
        
        case 2:
            instancia.A=stoi(linea);
            instancia.matrizRequerimientos.resize(instancia.A);
            instancia.Turno.resize(instancia.A);
            break;
        case 3:
            int valor;
            ss.clear(); 
            ss.str(linea);
            for(int i=0; i<instancia.W; i++ ){
                ss >> valor;
                instancia.matrizRequerimientos[fila].push_back(valor);
            }
            fila++;
            break;
        case 4:
            if (fila < instancia.A) {
                ss.clear(); 
                ss.str(linea);
                string nombre_turno;
                int min_c, max_c;               
                ss >> nombre_turno >> min_c >> max_c;
                instancia.Turno[fila].nombre = nombre_turno;
                instancia.Turno[fila].minConsec = min_c;
                instancia.Turno[fila].maxConsec = max_c;
                
                fila++; 
            }
            break;
        case 5:
            ss.clear(); 
            ss.str(linea);    
            ss>>instancia.minLibre>>instancia.maxLibre;
            break;
        case 6:
            ss.clear(); 
            ss.str(linea);    
            ss>>instancia.minTrabajo>>instancia.maxTrabajo;
            break;
        case 7:
            ss.clear(); 
            ss.str(linea);    
            ss>>instancia.cantL2>>instancia.cantL3;
            instancia.L2.resize(instancia.cantL2);
            instancia.L3.resize(instancia.cantL3);
            break;
        case 8:
            if (fila < instancia.cantL2){
                instancia.L2[fila].push_back (linea);
            }
            else{
                instancia.L3[fila-instancia.cantL2].push_back (linea);
            }
            fila++;
            break;
        default:
            break;
        }
    }
    return instancia;
};



// FUNCIONES AUXILIARES

bool esLibre(const string& turno) {
    return turno == "-"; 
}

vector<vector<string>> parsearL2strings(const Instancia& inst) {
    vector<vector<string>> FS2(inst.cantL2);

    for (int idx = 0; idx < inst.cantL2; ++idx) {
        string linea = inst.L2[idx][0];
        stringstream ss(linea);
        string token;
        while (ss >> token) {
            FS2[idx].push_back(token);
        }
    }
    return FS2;
}

vector<vector<string>> parsearL3strings(const Instancia& inst) {
    vector<vector<string>> FS3(inst.cantL3);

    for (int idx = 0; idx < inst.cantL3; ++idx) {
        string linea = inst.L3[idx][0];
        stringstream ss(linea);
        string token;
        while (ss >> token) {
            FS3[idx].push_back(token);
        }
    }
    return FS3;
}

bool esTrabajo(const string& turno) {
    return !esLibre(turno); // todo lo que no es "-" lo consideramos trabajo
}

pair<int,int> nextPos(int i, int j, int N, int W) {
    if (j + 1 < W) return {i, j+1};
    return { (i + 1) % N, 0 };
}


bool esProhibida2(const vector<vector<string>>& FS2,
                  const string& a, const string& b)
{
    for (const auto& seq : FS2) {
        if (seq.size() == 2 && seq[0] == a && seq[1] == b)
            return true;
    }
    return false;
}

bool esProhibida3(const vector<vector<string>>& FS3,const string& a, const string& b, const string& c){
    for (const auto& seq : FS3) {
        if (seq.size() == 3 && seq[0] == a && seq[1] == b && seq[2] == c)
            return true;
    }
    return false;
}

double funcionEnfriamiento(double alpha,double tempaturaActual){
    double ajusteTemperatura = alpha*tempaturaActual;
    return ajusteTemperatura;
}

double funcionEvaluacion(
    const vector<vector<string>>& repr,const Instancia& inst,const vector<vector<string>>& FS2,const vector<vector<string>>& FS3)
{
    int N = inst.N;
    int W = inst.W;

    // Penalizaciones acumuladas
    double penWeekend       = 0.0;
    double penCobertura     = 0.0;
    double penBloqueTrabajo = 0.0;
    double penBloqueLibre   = 0.0;
    double penBloqueTurno   = 0.0;
    double penL2            = 0.0;
    double penL3            = 0.0;

    // -----------------------------
    // 1) Fin de semana
    //    Si W=7 → sábado=5, domingo=6
    // -----------------------------
    vector<int> weekend;
    if (W >= 7) {
        weekend = {5, 6};
    } else {
        // si no hay 7 días, tomo los dos últimos como "fin de semana"
        weekend = {max(0, W-2), max(0, W-1)};
    }

    for (int i = 0; i < N; ++i) {
        bool trabaja = false;
        for (int d : weekend) {
            if (d >= 0 && d < W && esTrabajo(repr[i][d])) {
                trabaja = true;
                break;
            }
        }
        if (trabaja) penWeekend += 1.0;
    }

    // -----------------------------
    // 2) Cobertura mínima
    //    matrizRequerimientos[a][j]
    // -----------------------------
    for (int j = 0; j < W; ++j) {

        // contamos cuántos trabajan en cada turno a
        vector<int> cuenta(inst.A, 0);

        for (int i = 0; i < N; ++i) {
            const string& t = repr[i][j];

            // buscar qué turno es (por nombre)
            for (int a = 0; a < inst.A; ++a) {
                if (inst.Turno[a].nombre == t) {
                    cuenta[a]++;
                    break;
                }
            }
        }

        for (int a = 0; a < inst.A; ++a) {
            int req = inst.matrizRequerimientos[a][j];
            int deficit = max(0, req - cuenta[a]);
            penCobertura += deficit;
        }
    }

    // -----------------------------
    // 3) Bloques trabajo/libre con rotatividad
    // -----------------------------
    int T = N * W;
    int i = 0, j = 0, pasos = 0;

    while (pasos < T) {
        bool actualTrab = esTrabajo(repr[i][j]);
        int len = 0;

        do {
            len++;
            pasos++;
            auto nxt = nextPos(i, j, N, W);
            i = nxt.first;
            j = nxt.second;
        } while (pasos < T && esTrabajo(repr[i][j]) == actualTrab);

        if (actualTrab) {
            if (len < inst.minTrabajo) penBloqueTrabajo += (inst.minTrabajo - len);
            if (len > inst.maxTrabajo) penBloqueTrabajo += (len - inst.maxTrabajo);
        } else {
            if (len < inst.minLibre)   penBloqueLibre += (inst.minLibre - len);
            if (len > inst.maxLibre)   penBloqueLibre += (len - inst.maxLibre);
        }
    }

    // -----------------------------
    // 4) Bloques consecutivos de cada turno específico
    // -----------------------------
    for (int a = 0; a < inst.A; ++a) {
        const string& turnoA = inst.Turno[a].nombre;

        i = 0; j = 0; pasos = 0;

        while (pasos < T) {
            if (repr[i][j] != turnoA) {
                auto nxt = nextPos(i, j, N, W);
                i = nxt.first;
                j = nxt.second;
                pasos++;
                continue;
            }

            int len = 0;
            do {
                len++;
                pasos++;
                auto nxt = nextPos(i, j, N, W);
                i = nxt.first;
                j = nxt.second;
            } while (pasos < T && repr[i][j] == turnoA);

            int minC = inst.Turno[a].minConsec;
            int maxC = inst.Turno[a].maxConsec;

            if (len < minC) penBloqueTurno += (minC - len);
            if (len > maxC) penBloqueTurno += (len - maxC);
        }
    }

    // -----------------------------
    // 5) Secuencias prohibidas L2 / L3
    // -----------------------------
    i = 0; j = 0;
    auto p1 = nextPos(i, j, N, W);
    int i1 = p1.first, j1 = p1.second;
    auto p2 = nextPos(i1, j1, N, W);
    int i2 = p2.first, j2 = p2.second;

    for (int t = 0; t < T; ++t) {
        const string& a = repr[i][j];
        const string& b = repr[i1][j1];
        const string& c = repr[i2][j2];

        if (esProhibida2(FS2, a, b)) penL2 += 1.0;
        if (esProhibida3(FS3, a, b, c)) penL3 += 1.0;

        i  = i1; j  = j1;
        i1 = i2; j1 = j2;
        auto nx = nextPos(i2, j2, N, W);
        i2 = nx.first; j2 = nx.second;
    }

    // -----------------------------
    // 6) Función objetivo total (a MINIMIZAR)
    //    Por ahora sin pesos, todos = 1
    // -----------------------------
    double fo = 0.0;
    fo += penWeekend;
    fo += 2*penCobertura;
    fo += penBloqueTrabajo;
    fo += penBloqueLibre;
    fo += 4 * penBloqueTurno;
    fo += 2* penL2;
    fo += 3* penL3;

    return fo;
}



void SolucionInicial(const Instancia& inst,vector<vector<string>>& solucion)
{
    int N = inst.N;
    int W = inst.W;
    int A = inst.A;

    solucion.assign(N, vector<string>(W, "-"));

    // Para cada día
    for (int j = 0; j < W; ++j) {

        // Lista de todos los trabajadores aún sin turno ese día
        vector<int> trabajadores(N);
        iota(trabajadores.begin(), trabajadores.end(), 0);

        int idxTrab = 0;  // índice del trabajador a asignar

        // Para cada turno a, cumplir requerimientos miope
        for (int a = 0; a < A; ++a) {

            int req = inst.matrizRequerimientos[a][j];

            for (int r = 0; r < req && idxTrab < N; ++r) {
                int i = trabajadores[idxTrab];
                solucion[i][j] = inst.Turno[a].nombre;
                idxTrab++;
            }
        }

        // El resto queda libre "-" automáticamente
    }
}


void generarVecino(const Instancia& inst,vector<vector<string>>& sol, mt19937& rng)
{
    int N = inst.N;
    int W = inst.W;

    if (N < 2) return; 
    uniform_int_distribution<int> distDia(0, W - 1);
    uniform_int_distribution<int> distTrab(0, N - 1);

    int j = distDia(rng);  
    int i1 = distTrab(rng);
    int i2 = distTrab(rng);

    int intentos = 0;
    while (i2 == i1 && intentos < 10) {
        i2 = distTrab(rng);
        intentos++;
    }
    if (i1 == i2) return; // no se pudo elegir dos distintos, salimos

    if (sol[i1][j] == sol[i2][j]) {
        j = distDia(rng);
        if (sol[i1][j] == sol[i2][j]) {
            return; // mala suerte, no cambiamos nada
        }
    }
    std::swap(sol[i1][j], sol[i2][j]);
}





double SimulatedAnneling( Instancia inst, vector<vector<string>>& solucion,double temperaturaMinima,double temperaturaActual,double alpha,int semilla,long long& numIteraciones)
{
    auto FS2 = parsearL2strings(inst);
    auto FS3 = parsearL3strings(inst);
    random_device rd;
    mt19937 rng(semilla);
    uniform_real_distribution<double> dist01(0.0, 1.0);
    double calidadSolucion = funcionEvaluacion(solucion, inst, FS2, FS3);
    vector<vector<string>> mejorSolucion = solucion;
    double mejorValor = calidadSolucion;
    numIteraciones = 0;  
    int iterPorTemp = 10000;
    while (temperaturaActual > temperaturaMinima) {
        for (int it = 0; it < iterPorTemp; ++it) {
            vector<vector<string>> vecinoSolucion = solucion;
            generarVecino(inst, vecinoSolucion, rng);

            double evaluacionVecino = funcionEvaluacion(vecinoSolucion, inst, FS2, FS3);
            numIteraciones++; 
            double delta = evaluacionVecino - calidadSolucion; // minimización

            if (delta < 0) {
                solucion = vecinoSolucion;
                calidadSolucion = evaluacionVecino;

                if (evaluacionVecino < mejorValor) {
                    mejorValor = evaluacionVecino;
                    mejorSolucion = vecinoSolucion;
                }
            } else {
                double u = dist01(rng);
                double prob = exp(-delta / temperaturaActual);

                if (u < prob) {
                    solucion = vecinoSolucion;
                    calidadSolucion = evaluacionVecino;
                }
            }
        }

        temperaturaActual = funcionEnfriamiento(alpha, temperaturaActual);
    } 
    solucion = mejorSolucion;
    //cout << "Mejor valor encontrado por SA: " << mejorValor << "\n";
    return mejorValor; 
}


int contarFinesDeSemanaLibres(const vector<vector<string>>& sol,const Instancia& inst)
{
    int N = inst.N;
    int W = inst.W;
    int dSab, dDom;
    if (W >= 7) {
        dSab = 5;
        dDom = 6;
    } else {
        dSab = max(0, W - 2);
        dDom = max(0, W - 1);
    }

    int libres = 0;
    for (int i = 0; i < N; ++i) {
        bool libreSab = (sol[i][dSab] == "-");
        bool libreDom = (sol[i][dDom] == "-");
        if (libreSab && libreDom) {
            libres++;   // este trabajador tiene el fin de semana libre
        }
    }
    return libres;
}


void mostrarSolucion(const vector<vector<string>>& solucion, Instancia insta) {
    int N = solucion.size();
    if (N == 0) return;
    int W = solucion[0].size();

    cout << "\n===== SOLUCIÓN =====\n\n";

    // Encabezado de días
    cout << "     ";
    for (int j = 0; j < W; ++j) {
        cout << "D" << j << " ";
    }
    cout << "\n";

    // Contenido de la matriz
    for (int i = 0; i < N; ++i) {
        cout << "T" << i << " | ";
        for (int j = 0; j < W; ++j) {
            cout << solucion[i][j] << "  ";
        }
        cout << "\n";
    }

    cout << "\n=====================\n";
}


void escribirSalida(const vector<vector<string>>& solucion,const Instancia& inst,const string& nombreInstancia,unsigned int semilla){
    fs::path p(nombreInstancia);
    string base = p.stem().string();   // ej: "N10-10"

    string nombreSalida = base + "_" + to_string(semilla) + ".txt";

    fs::path carpetaResultados = fs::path("Resultados");
    if (!fs::exists(carpetaResultados)) {
        fs::create_directories(carpetaResultados);
    }

    fs::path rutaSalida = carpetaResultados / nombreSalida;

    ofstream out(rutaSalida);
    if (!out.is_open()) {
        cerr << "ERROR: no se pudo abrir archivo de salida: "
             << rutaSalida << "\n";
        return;
    }

    int N = inst.N;
    int W = inst.W;

    int finesLibres = contarFinesDeSemanaLibres(solucion, inst);
    out << finesLibres << "\n";

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < W; ++j) {
            out << solucion[i][j];
            if (j + 1 < W) out << ' ';
        }
        out << '\n';
    }

    out.close();
    cout << "Archivo de salida generado en: " << rutaSalida << "\n";
}

int main() {
    // 1) Leer parámetros de SA desde archivo
    vector<ParamSA> configs;
    {
        ifstream f("parametros.txt");
        if (!f.is_open()) {
            cerr << "ERROR: no se pudo abrir parametros.txt\n";
            return 1;
        }

        double alpha, tMin, tIni;
        unsigned int seed;

        // Cada línea: alpha  tempMin  tempInicial  semilla
        while (f >> alpha >> tMin >> tIni >> seed) {
            configs.push_back(ParamSA{alpha, tMin, tIni, seed});
        }

        if (configs.empty()) {
            cerr << "ERROR: parametros.txt está vacío o mal formateado\n";
            return 1;
        }
    }

    // 2) Recorrer todas las instancias dentro de la carpeta "Instancias"
    vector<fs::path> rutasInstancias;
    fs::path raizInstancias = "Instancias";

    try {
        if (!fs::exists(raizInstancias)) {
            cerr << "ERROR: no existe la carpeta 'Instancias'\n";
            return 1;
        }

        // SOLO archivos directamente en Instancias (sin subcarpetas)
        for (const auto& entry : fs::directory_iterator(raizInstancias)) {
            if (!entry.is_regular_file()) continue;
            if (entry.path().extension() != ".txt") continue;  // solo .txt

            rutasInstancias.push_back(entry.path());
        }

        if (rutasInstancias.empty()) {
            cerr << "ERROR: no se encontraron archivos .txt en 'Instancias'\n";
            return 1;
        }
    } catch (const std::exception& e) {
        cerr << "Error recorriendo carpeta Instancias: " << e.what() << "\n";
        return 1;
    }

    // 2.5) Preparar archivo de métricas
    fs::path rutaMetricas = "metricas.csv";
    bool archivoNuevo = !fs::exists(rutaMetricas);

    ofstream met(rutaMetricas, ios::app);
    if (!met.is_open()) {
        cerr << "ERROR: no se pudo abrir metricas.csv para escritura\n";
        return 1;
    }

    // Escribir encabezado solo si el archivo no existía
    if (archivoNuevo) {
        met << "instancia,alpha,tempMin,tempInicial,semilla,"
            "fo_inicial,fo_final,fines_semana_libres,iteraciones_sa\n";
    }

    // 3) Ejecutar para cada combinación (parámetros, instancia)
    for (const auto& cfg : configs) {
        cout << "\n=== Ejecutando configuración "
             << "(alpha=" << cfg.alpha
             << ", Tmin=" << cfg.tempMin
             << ", Tini=" << cfg.tempInicial
             << ", seed=" << cfg.semilla << ") ===\n";

        for (const auto& ruta : rutasInstancias) {
            cout << "\n--- Instancia: " << ruta.string() << " ---\n";

            try {
                // Cargar instancia
                Instancia instancia = cargaInstancia(ruta.string());

                // Construir solución inicial greedy
                vector<vector<string>> solucion;
                SolucionInicial(instancia, solucion);

                // Evaluar solución inicial (para métricas)
                auto FS2 = parsearL2strings(instancia);
                auto FS3 = parsearL3strings(instancia);
                double foInicial = funcionEvaluacion(solucion, instancia, FS2, FS3);
                long long iteracionesSA = 0;
                // Ejecutar SA con esta configuración
                SimulatedAnneling(instancia,
                                  solucion,
                                  cfg.tempMin,
                                  cfg.tempInicial,
                                  cfg.alpha,
                                  cfg.semilla,
                                  iteracionesSA);

                // Evaluar solución final
                double foFinal = funcionEvaluacion(solucion, instancia, FS2, FS3);
                int finesLibres = contarFinesDeSemanaLibres(solucion, instancia);

                // Escribir salida: <nombre_instancia>_<seed>.txt en /Resultados
                escribirSalida(solucion,
                               instancia,
                               ruta.string(),
                               cfg.semilla);

                // Guardar métricas en CSV
                met << ruta.filename().string() << ","
                    << cfg.alpha << ","
                    << cfg.tempMin << ","
                    << cfg.tempInicial << ","
                    << cfg.semilla << ","
                    << foInicial << ","
                    << foFinal << ","
                    << finesLibres << ","
                    << iteracionesSA << "\n";

            } catch (const std::exception& e) {
                cerr << "Error procesando instancia " << ruta
                     << ": " << e.what() << "\n";
            }
        }
    }

    met.close();
    return 0;
}
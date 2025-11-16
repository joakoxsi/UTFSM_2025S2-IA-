#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <sstream>

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




int main(){
    cout << "Hello Word" << " \n";
    fs::path ruta="./Instancias/N10/N10-10.txt";
    try{
      Instancia instancia = cargaInstancia(ruta.string());  


        vector<vector<string>> representacion;
        representacion.resize(instancia.N);
        for(int i=0; i< instancia.N;i++){
            representacion[i].resize(instancia.W);
        }    

        cout << "Número de Filas (N): " << representacion.size() << endl;
        cout << "Número de Filas (N): " << representacion[0].size() << endl;
        
    }
    catch (exception& e) {
        cerr << "Error fatal al cargar la instancia: " << e.what() << endl;
    }

    // AHora se va a construir una se va a contruir la representacion
    return 0;
}
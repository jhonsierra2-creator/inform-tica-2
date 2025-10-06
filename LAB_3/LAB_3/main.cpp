#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm> // std::min, std::count
using namespace std;

int main() {
    string nombreEntrada, nombreSalida;
    size_t n; // tamaño del bloque (semilla)

    cout << "Ingrese el valor de n (tamaño del bloque, entero > 0): ";
    if (!(cin >> n) || n == 0) {
        cerr << "Valor de n inválido. Debe ser un entero mayor que 0.\n";
        return 1;
    }

    cout << "Nombre del archivo de entrada: ";
    cin >> nombreEntrada;
    cout << "Nombre del archivo de salida: ";
    cin >> nombreSalida;

    // Intentar abrir el archivo de entrada en modo binario (para leer bytes exactamente)
    ifstream fin(nombreEntrada, ios::binary);
    if (!fin.is_open()) {
        cout << "El archivo '" << nombreEntrada << "' no existe.\n";
        cout << "¿Deseas crearlo vacío? (s/n): ";
        char opcion;
        cin >> opcion;
        if (opcion == 's' || opcion == 'S') {
            // crearlo vacío y volver a abrir para lectura (será vacío)
            ofstream crear(nombreEntrada, ios::binary);
            if (!crear.is_open()) {
                cerr << "Error al crear el archivo.\n";
                return 1;
            }
            crear.close();
            // reabrir para lectura (será un ifstream vacío, no falla)
            fin.open(nombreEntrada, ios::binary);
            if (!fin.is_open()) {
                cerr << "Error al abrir el archivo recién creado.\n";
                return 1;
            }
            cout << "Archivo '" << nombreEntrada << "' creado correctamente (vacío).\n";
        } else {
            cout << "No se puede continuar sin archivo de entrada.\n";
            return 1;
        }
    } else {
        cout << "Archivo '" << nombreEntrada << "' encontrado correctamente.\n";
    }

    // Leer todo el archivo byte a byte y construir la cadena de bits
    string bitsTotales;
    // (Opcional) si conocemos tamaño podemos reservar memoria:
    fin.seekg(0, ios::end);
    streampos tam = fin.tellg();
    fin.seekg(0, ios::beg);
    if (tam > 0) bitsTotales.reserve(static_cast<size_t>(tam) * 8);

    char c;
    while (fin.get(c)) {
        unsigned char uc = static_cast<unsigned char>(c);
        for (int i = 7; i >= 0; --i) {
            bitsTotales += ((uc >> i) & 1) ? '1' : '0';
        }
    }
    fin.close();

    cout << "Cadena binaria generada (primeros 64 bits): "
         << bitsTotales.substr(0, min((size_t)64, bitsTotales.size()))
         << (bitsTotales.size() > 64 ? "..." : "") << endl;
    cout << "Total de bits generados: " << bitsTotales.size() << endl;

    // Si no hay bits, terminamos
    if (bitsTotales.empty()) {
        cout << "No hay datos en el archivo de entrada. No se generó archivo codificado.\n";
        return 0;
    }

    // Particionar en bloques de tamaño n
    vector<string> bloques;
    for (size_t i = 0; i < bitsTotales.size(); i += n) {
        bloques.push_back(bitsTotales.substr(i, n)); // el último bloque puede ser más corto
    }

    // Codificar bloques según reglas dadas
    vector<string> bloquesCodificados;
    for (size_t i = 0; i < bloques.size(); ++i) {
        string bloque = bloques[i];
        string nuevo = bloque;

        if (i == 0) {
            // Primer bloque: invertir todos los bits
            for (char &b : nuevo) b = (b == '0') ? '1' : '0';
        } else {
            // Analizar bloque anterior
            int count1 = static_cast<int>(count(bloques[i - 1].begin(), bloques[i - 1].end(), '1'));
            int count0 = static_cast<int>(bloques[i - 1].size()) - count1;

            if (count1 == count0) {
                // invertir todos los bits
                for (char &b : nuevo) b = (b == '0') ? '1' : '0';
            } else if (count0 > count1) {
                // swap por pares (0<->1) dentro del bloque: (0,1), (2,3), ...
                for (size_t j = 0; j + 1 < nuevo.size(); j += 2) {
                    swap(nuevo[j], nuevo[j + 1]);
                }
            } else { // count1 > count0
                // swap cada grupo de 3: (0,2), (3,5), ...
                for (size_t j = 0; j + 2 < nuevo.size(); j += 3) {
                    swap(nuevo[j], nuevo[j + 2]);
                }
            }
        }

        bloquesCodificados.push_back(nuevo);
    }

    // Juntar resultado
    string resultado;
    resultado.reserve(bitsTotales.size());
    for (const auto &b : bloquesCodificados) resultado += b;

    // Escribir resultado en el archivo de salida (modo texto, contiene '0'/'1')
    ofstream fout(nombreSalida, ios::binary);
    if (!fout.is_open()) {
        cerr << "Error al crear el archivo de salida.\n";
        return 1;
    }
    fout << resultado;
    fout.close();

    cout << "Archivo codificado correctamente en '" << nombreSalida << "'.\n";
    cout << "Bits escritos: " << resultado.size() << endl;

    return 0;
}



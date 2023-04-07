#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

struct Alumno {
    string Nombre;
    string Apellidos;
    string Carrera;
    float mensualidad;

    Alumno() = default;
    Alumno(string name, string lastname, string major, float pay){
        Nombre = name;
        Apellidos = lastname;
        Carrera = major;
        mensualidad = pay;
    }
};


struct VariableRecord {
    string archivo;

    VariableRecord(const string &file_name) {
        archivo = file_name;
    }

    vector<Alumno> load(){
        ifstream  archivoo;
        vector<Alumno> res;
        string cadena;
        int pos2;  int pos; int pos3;
        string apellidos; string nombre; string carrera; string mensualidad;
        archivoo.open(archivo, ios::in);
        if(archivoo.is_open()) {
            while (!archivoo.eof()) {
                getline(archivoo, cadena);
                pos = cadena.find_first_of('|', 0);
                pos2 = cadena.find_first_of('|', pos + 1);
                pos3 = cadena.find_first_of('|', pos2 + 1);

                nombre = cadena.substr(0, pos);
                apellidos = cadena.substr(pos + 1, pos2 - pos - 1);
                carrera = cadena.substr(pos2 + 1, pos3 - pos2 - 1);
                mensualidad = (cadena.substr(pos3 + 1));
                Alumno a(nombre, apellidos, carrera, stof(mensualidad));
                res.push_back(a);
            }
            archivoo.close();
        }
        return res;
    }

    void add(Alumno record){
        ofstream archivoo;
        archivoo.open(archivo, ios::app);
        if(archivoo.is_open()) {
            archivoo.seekp(0, std::ios_base::end);
            archivoo << '\n'
                     << record.Nombre << '|'
                     << record.Apellidos << '|'
                     << record.Carrera << '|'
                     << record.mensualidad;
            archivoo.close();
        }
    }

    Alumno readRecord(int num){
        ifstream archivo_nuevo;
        string cadena;
        int pos2;  int pos; int pos3;
        int i=1;
        string apellidos; string nombre; string carrera; string mensualidad;

        archivo_nuevo.open(archivo, ios::in);
        if(archivo_nuevo.is_open()) {
            archivo_nuevo.seekg(ios::beg);
            while (!archivo_nuevo.eof()) {
                getline(archivo_nuevo, cadena);
                if(i==num) {
                    pos = cadena.find_first_of('|', 0);
                    pos2 = cadena.find_first_of('|', pos + 1);
                    pos3 = cadena.find_first_of('|', pos2 + 1);

                    nombre = cadena.substr(0, pos);
                    apellidos = cadena.substr(pos + 1, pos2 - pos - 1);
                    carrera = cadena.substr(pos2 + 1, pos3 - pos2 - 1);
                    mensualidad = (cadena.substr(pos3 + 1));
                }
                i++;
            }
            archivo_nuevo.close();
        }
        Alumno a(nombre, apellidos, carrera, stof(mensualidad));
        return a;
    }


};

int main() {
    VariableRecord a("datos.txt");

    cout << "Imprimiendo alumnos existentes (antes de agregar):" << endl;
    vector<Alumno> res1 = a.load();
    for(auto &i : res1){
        cout << " Nombre : " << i.Nombre << endl;
        cout << " Apellidos : " << i.Apellidos << endl;
        cout << " Carrera:  " << i.Carrera << endl;
        cout << " Mensualidad: " << i.mensualidad << endl;
        cout << endl;
    };

    // Agregando 3 alumnos
    a.add({"Enzo", "Camizan Vidal", "Computer Science", 153.50});
    a.add({"Valentin", "Quezada Amour", "Computer Science", 164.50});
    a.add({"Alvaro", "Guerrero Jimenez", "Computer Science", 180.00});


    cout << "Imprimiendo alumnos existentes (despues de agregar):" << endl;
    vector<Alumno> res2 = a.load();
    for(auto &i : res2){
        cout << " Nombre : " << i.Nombre << endl;
        cout << " Apellidos : " << i.Apellidos << endl;
        cout << " Carrera:  " << i.Carrera << endl;
        cout << " Mensualidad: " << i.mensualidad << endl;
        cout << endl;
    };

    cout << "Imprimiendo alumno #4:" << endl;
    Alumno alum = a.readRecord(4);
    cout << " Nombre : " << alum.Nombre << endl;
    cout << " Apellidos : " << alum.Apellidos << endl;
    cout << " Carrera:  " << alum.Carrera << endl;
    cout << " Mensualidad: " << alum.mensualidad << endl;
    cout << endl;

    return 0;
}
#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <limits>
#include <ostream>
#include <string>
#include <vector>

#include <prettyprint.hpp>

struct Alumno
{
    char codigo[5];
    char nombre[11];
    char apellidos[20];
    char carrera[15];

    Alumno() = default;

    Alumno(
        char const* _codigo, char const* _nombre, char const* _apellidos, char const* _carrera)
    {
        std::copy_n(_codigo, 5, codigo);
        std::copy_n(_nombre, 11, nombre);
        std::copy_n(_apellidos, 20, apellidos);
        std::copy_n(_carrera, 15, carrera);
    }

    friend std::ostream& operator<<(std::ostream& os, Alumno const& al)
    {
        os << "<Student: ";
        os.write(al.codigo, 5);
        os << ", ";
        os.write(al.nombre, 11);
        os << ", ";
        os.write(al.apellidos, 20);
        os << ", ";
        os.write(al.carrera, 15);
        os << ">";

        return os;
    }
};

class FixedRecord
{
private:
    std::fstream file;

public:
    FixedRecord(std::string const& file_name)
    {
        std::ofstream(file_name, std::ofstream::app | std::fstream::binary);
        file = std::fstream{file_name, std::ios::in | std::ios::out | std::ios::binary};
    }

    auto load() -> std::vector<Alumno>
    {
        std::vector<Alumno> ret;
        file.seekg(0);

        while (!file.eof())
        {
            ret.push_back(read_student());
            file.get();
        }

        file.clear();

        return ret;
    }

    void add(Alumno const& al)
    {
        file.seekp(0, std::ios_base::end);

        file.write(al.codigo, 5);
        file.write(al.nombre, 11);
        file.write(al.apellidos, 20);
        file.write(al.carrera, 15);
        file << "\n";

        file.flush();
    }

    auto read_record(size_t pos) -> Alumno
    {
        return read_offset(pos * sizeof(Alumno) + pos);
    }

    auto read_offset(size_t offset) -> Alumno
    {
        file.seekg(offset, std::ios_base::beg);

        return read_student();
    }

    auto read_student() -> Alumno
    {
        Alumno al{};

        file.read(al.codigo, 5);
        file.read(al.nombre, 11);
        file.read(al.apellidos, 20);
        file.read(al.carrera, 15);

        return al;
    }
};

auto main() -> int
{
    FixedRecord fr{"datos1.txt"};
    std::cout << fr.load() << "\n";

    fr.add({"12345", "AAABBBCCCDD", "AAABBBCCCDDDEEEFFFGG", "AAABBBCCCDDDEEE"});

    std::cout << fr.read_record(0) << "\n";
    std::cout << fr.read_record(1) << "\n";
    std::cout << fr.read_record(2) << "\n";
    std::cout << fr.read_record(3) << "\n";
}

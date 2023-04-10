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
#include <type_traits>
#include <vector>

#include "prettyprint.hpp"

struct Alumno
{
    char codigo[5];
    char nombre[11];
    char apellidos[20];
    char carrera[15];

    int ciclo;
    float mensualidad;

    Alumno() = default;

    Alumno(
        char const* _codigo,
        char const* _nombre,
        char const* _apellidos,
        char const* _carrera,
        int _ciclo,
        float _mensualidad)
        : ciclo(_ciclo),
          mensualidad(_mensualidad)
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
        os << ", ";
        os << al.ciclo;
        os << ", ";
        os << al.mensualidad;
        os << ">";

        return os;
    }
};

class FixedRecord
{
private:
    std::fstream file;

    static size_t const record_not_deleted;
    static size_t const record_deleted_and_last;

    size_t const header_size = sizeof(size_t);

    struct AlumnoEntry
    {
        Alumno al;
        size_t deleted_p;

        auto is_deleted() -> bool
        {
            return deleted_p != record_not_deleted;
        }
    };

private:
    size_t get_header()
    {
        file.seekg(0, std::ios_base::beg);
        size_t ret;
        file.read(reinterpret_cast<char*>(&ret), sizeof(size_t));
        return ret;
    }

    void set_header(size_t header)
    {
        file.seekp(0, std::ios_base::beg);
        file.write(reinterpret_cast<char const*>(&header), sizeof(header));
    }

public:
    FixedRecord(std::string const& file_name)
    {
        std::ofstream(file_name, std::ofstream::app | std::fstream::binary);
        file = std::fstream{file_name, std::ios::in | std::ios::out | std::ios::binary};

        if (file.peek(), file.eof())
        {
            file.clear();
            file.seekp(0, std::ios_base::beg);

            file.write(
                reinterpret_cast<char const*>(&record_deleted_and_last),
                sizeof(record_not_deleted));
        }
    }

    auto load() -> std::vector<Alumno>
    {
        std::vector<Alumno> ret;
        file.seekg(header_size, std::ios_base::beg);

        while (file.peek(), !file.eof())
        {
            AlumnoEntry ae = read_record();

            if (!ae.is_deleted())
            {
                ret.push_back(ae.al);
            }
        }
        file.clear();

        return ret;
    }

    void add(Alumno const& al)
    {
        size_t header = get_header();

        if (header == record_deleted_and_last)
        {
            file.seekp(0, std::ios_base::end);
        }
        else
        {
            AlumnoEntry ae = read_alumno_entry(header);
            set_header(ae.deleted_p);
            file.seekp(calculate_offset(header), std::ios_base::beg);
        }

        file.write(al.codigo, 5);
        file.write(al.nombre, 11);
        file.write(al.apellidos, 20);
        file.write(al.carrera, 15);
        file.write(reinterpret_cast<char const*>(&al.ciclo), sizeof(al.ciclo));
        file.write(reinterpret_cast<char const*>(&al.mensualidad), sizeof(al.mensualidad));

        file.write(
            reinterpret_cast<char const*>(&record_not_deleted), sizeof(record_not_deleted));

        file.flush();
    }

    auto read_record(size_t pos) -> Alumno
    {
        return read_alumno_entry(pos).al;
    }

    auto read_alumno_entry(size_t pos) -> AlumnoEntry
    {
        return read_offset(calculate_offset(pos));
    }

    auto calculate_offset(size_t pos) -> size_t
    {
        constexpr size_t alumno_size = sizeof(Alumno::codigo) + sizeof(Alumno::nombre)
                                       + sizeof(Alumno::apellidos) + sizeof(Alumno::carrera)
                                       + sizeof(Alumno::ciclo) + sizeof(Alumno::mensualidad);
        constexpr size_t record_size = alumno_size + sizeof(size_t);

        return header_size + pos * record_size;
    }

    auto read_offset(size_t offset) -> AlumnoEntry
    {
        file.seekg(offset, std::ios_base::beg);

        return read_record();
    }

    auto read_record() -> AlumnoEntry
    {
        AlumnoEntry ret{};
        Alumno& al = ret.al;

        file.read(al.codigo, 5);
        file.read(al.nombre, 11);
        file.read(al.apellidos, 20);
        file.read(al.carrera, 15);

        file.read(reinterpret_cast<char*>(&al.ciclo), sizeof(al.ciclo));
        file.read(reinterpret_cast<char*>(&al.mensualidad), sizeof(al.mensualidad));

        file.read(reinterpret_cast<char*>(&ret.deleted_p), sizeof(ret.deleted_p));

        return ret;
    }

    auto remove(int pos) -> bool
    {
        if (read_alumno_entry(pos).is_deleted())
        {
            return false;
        }

        size_t header = get_header();

        size_t offset = calculate_offset(pos + 1) - sizeof(record_not_deleted);
        file.seekp(offset, std::ios_base::beg);
        file.write(reinterpret_cast<char const*>(&header), sizeof(header));

        set_header(pos);

        file.flush();

        return true;
    }

    auto get_deleted_positions() -> std::vector<size_t>
    {
        size_t cur_index = get_header();

        std::vector<size_t> ret;

        while (cur_index != record_deleted_and_last)
        {
            ret.push_back(cur_index);
            cur_index = read_alumno_entry(cur_index).deleted_p;
        }

        return ret;
    }
};

size_t const FixedRecord::record_not_deleted = std::numeric_limits<size_t>::max();
size_t const FixedRecord::record_deleted_and_last = std::numeric_limits<size_t>::max() - 1;

auto main(int argc, char** argv) -> int
{
    if (argc != 2)
    {
        return 1;
    }

    FixedRecord fr{argv[1]};
    std::cout << fr.load() << "\n";

    // fr.add({"12345", "AAABBBCCCDD", "AAABBBCCCDDDEEEFFFGG", "AAABBBCCCDDDEEE", 3, 1200.0});
    fr.add({"98765", "aaabbbcccdd", "aaabbbcccdddeeefffgg", "aaabbbcccdddeee", 4, 1600.0});

    // fr.remove(2);

    std::cout << fr.get_deleted_positions() << "\n";

    std::cout << fr.read_record(0) << "\n";
    std::cout << fr.read_record(1) << "\n";
    std::cout << fr.read_record(2) << "\n";
    std::cout << fr.read_record(3) << "\n";
}

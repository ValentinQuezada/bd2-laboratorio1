#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "prettyprint.hpp"

struct Alumno
{
    std::string Nombre;
    std::string Apellidos;
    std::string Carrera;
    float mensualidad;

    friend std::ostream& operator<<(std::ostream& os, Alumno const& al)
    {
        os << "<Alumno: " << std::quoted(al.Nombre) << ", " << std::quoted(al.Apellidos) << ", "
           << std::quoted(al.Carrera) << ", " << al.mensualidad << ">";

        return os;
    }
};

class VariableRecord
{
    std::fstream file;

public:
    VariableRecord(std::string const& file_name)
    {
        std::ofstream(file_name, std::ofstream::app | std::fstream::binary);
        file = std::fstream{file_name, std::ios::in | std::ios::out | std::ios::binary};
    }

    auto load() -> std::vector<Alumno>
    {
        std::vector<Alumno> ret;

        auto read_delimiter = [&](char del, std::string& s_out) -> bool {
            std::getline(file, s_out, del);
            return !file.eof();
        };

        file.seekg(0);
        while (file.peek(), !file.eof())
        {
            Alumno al;

            if (!read_delimiter('|', al.Nombre))
            {
                break;
            }
            if (!read_delimiter('|', al.Apellidos))
            {
                break;
            }
            if (!read_delimiter('|', al.Carrera))
            {
                break;
            }
            std::string temp;
            if (!read_delimiter('\n', temp))
            {
                break;
            }
            al.mensualidad = std::stof(temp);

            ret.emplace_back(std::move(al));
        }

        return ret;
    }

    auto read_alumno() -> std::optional<Alumno>
    {
        auto read_delimiter = [&](char del, std::string& s_out) -> bool {
            std::getline(file, s_out, del);
            return !file.eof();
        };

        Alumno al;

        if (!read_delimiter('|', al.Nombre) || !read_delimiter('|', al.Apellidos)
            || !read_delimiter('|', al.Carrera))
        {
            return {};
        }
        std::string temp;
        if (!read_delimiter('\n', temp))
        {
            return {};
        }
        al.mensualidad = std::stof(temp);

        return {std::move(al)};
    }

    void add(Alumno const& al)
    {
        file.seekp(0, std::ios_base::end);
        file << al.Nombre << "|" << al.Apellidos << "|" << al.Carrera << "|"
             << std::to_string(al.mensualidad) << "\n";
    }

    auto read_record(int pos) -> Alumno
    {
        file.seekg(0, std::ios_base::beg);

        while (pos > 0)
        {
            --pos;
            std::string temp;
            std::getline(file, temp, '\n');

            if (file.eof())
            {
                throw std::runtime_error("Index out of range");
            }
        }

        auto a_o = read_alumno();
        if (a_o.has_value())
        {
            return a_o.value();
        }
        throw std::runtime_error("Index out of range");
    }
};

auto main(int argc, char** argv) -> int
{
    if (argc != 2)
    {
        return 1;
    }

    VariableRecord vr{argv[1]};

    vr.add({"N1", "A1", "C1", 100.0});

    std::cout << vr.load() << "\n";
    std::cout << vr.read_record(8) << "\n";
}

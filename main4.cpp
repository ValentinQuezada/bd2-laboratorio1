#include <cstddef>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "prettyprint.hpp"

struct Matricula
{
    std::string codigo;
    int ciclo;
    float mensualidad;
    std::string observaciones;

    friend auto operator<<(std::ostream& os, Matricula const& mat) -> std::ostream&
    {
        os << "<Matricula: " << std::quoted(mat.codigo) << ", " << mat.ciclo << ", "
           << mat.mensualidad << ", " << std::quoted(mat.observaciones) << ">";

        return os;
    }
};

class VariableRecord
{
    std::fstream file;

    using size_specifier = std::string::size_type;

public:
    VariableRecord(std::string const& file_name)
    {
        std::ofstream(file_name, std::ofstream::app | std::fstream::binary);
        file = std::fstream{file_name, std::ios::in | std::ios::out | std::ios::binary};
        file.exceptions(std::fstream::eofbit);
    }

    auto load() -> std::vector<Matricula>
    {
        std::vector<Matricula> ret;

        file.seekg(0);
        auto exceptions = file.exceptions();
        while (has_remaining_characters())
        {
            ret.emplace_back(read_matricula());
        }

        return ret;
    }

    auto read_matricula() -> Matricula
    {
        auto [size_codigo, size_observaciones] = read_sizes();

        Matricula mat;

        // XXX: Should eventually use resize_and_overwrite
        mat.codigo.resize(size_codigo);
        file.read(mat.codigo.data(), size_codigo);

        file.read(reinterpret_cast<char*>(&mat.ciclo), sizeof(mat.ciclo));
        file.read(reinterpret_cast<char*>(&mat.mensualidad), sizeof(mat.mensualidad));

        mat.observaciones.resize(size_observaciones);
        file.read(mat.observaciones.data(), size_observaciones);

        return mat;
    }

    auto read_sizes() -> std::tuple<size_t, size_t>
    {
        size_specifier size_codigo = 0;
        size_specifier size_observaciones = 0;

        file.read(reinterpret_cast<char*>(&size_codigo), sizeof(size_specifier));
        file.read(reinterpret_cast<char*>(&size_observaciones), sizeof(size_specifier));

        return {size_codigo, size_observaciones};
    }

    void add(Matricula const& mat)
    {
        file.seekp(0, std::ios_base::end);

        auto codigo_size = mat.codigo.size();
        file.write(reinterpret_cast<char const*>(&codigo_size), sizeof(codigo_size));
        auto observaciones_size = mat.observaciones.size();
        file.write(
            reinterpret_cast<char const*>(&observaciones_size), sizeof(observaciones_size));

        file.write(mat.codigo.data(), mat.codigo.size());
        file.write(reinterpret_cast<char const*>(&mat.ciclo), sizeof(mat.ciclo));
        file.write(reinterpret_cast<char const*>(&mat.mensualidad), sizeof(mat.mensualidad));
        file.write(mat.observaciones.data(), mat.observaciones.size());
    }

    auto read_record(int pos) -> Matricula
    {
        file.seekg(0);

        while (pos > 0)
        {
            if (!has_remaining_characters())
            {
                throw std::runtime_error("Index out of bounds.");
            }

            --pos;

            auto [size_codigo, size_observaciones] = read_sizes();
            file.seekg(
                sizeof(Matricula::ciclo) + sizeof(Matricula::mensualidad) + size_codigo
                    + size_observaciones,
                std::ios_base::cur);
        }

        if (!has_remaining_characters())
        {
            throw std::runtime_error("Index out of bounds.");
        }

        return read_matricula();
    }

    bool has_remaining_characters()
    {
        auto exceptions = file.exceptions();
        file.exceptions(std::fstream::goodbit);
        file.peek();
        bool ret = !file.eof();
        file.clear();
        file.exceptions(exceptions);
        return ret;
    }
};

auto main(int argc, char** argv) -> int
{
    if (argc != 2)
    {
        return 1;
    }

    VariableRecord vr{argv[1]};

    vr.add({"C1", 4, 1200, "O1"});

    std::cout << vr.load() << "\n";
    std::cout << vr.read_record(3) << "\n";
}

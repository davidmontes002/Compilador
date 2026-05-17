#ifndef TABLA_SIMBOLOS_H
#define TABLA_SIMBOLOS_H

#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

// Estructura que guarda la información que consultaremos en cualquier momento
struct Simbolo
{
    string tipo;  // Ejemplo: "entero", "cadena", "booleano"
    string valor; // Ejemplo: "25"
};

class TablaSimbolos
{
private:
    // El mapa hash donde guardaremos todo. Búsqueda muy rápida.
    unordered_map<string, Simbolo> tabla;

public:
    // 1. buscar id y verificar si está duplicado
    bool existe(const string &id)
    {
        return tabla.find(id) != tabla.end();
    }

    // 2. Insertar nueva variable (solo si NO existe)
    bool insertar(const string &id, const string &tipo, const string &valor = "")
    {
        if (existe(id))
        {
            // Regla: si ya existe dicha variable NO SERÁ AÑADIDA
            return false;
        }
        tabla[id] = {tipo, valor};
        return true;
    }

    // 3. getype: obtener el tipo de dato
    string getType(const string &id)
    {
        if (!existe(id))
        {
            return "error"; // La variable no fue declarada
        }
        return tabla[id].tipo;
    }

    // 4. equaltype: para verificar operaciones y asignaciones
    bool equalType(const string &idDestino, const string &tipoExpresion)
    {
        string tipoDestino = getType(idDestino);

        if (tipoDestino == "error")
        {
            return false;
        }

        // Verificación estricta de tipos (entero con entero, cadena con cadena)
        if (tipoDestino != tipoExpresion)
        {
            return false;
        }
        return true;
    }
    // 5. Cubo de Tipos / Tabla de Operadores
    string evaluarOperacion(const string &tipoIzq, const string &op, const string &tipoDer)
    {
        // Si alguno ya trae error, la operación entera es un error
        if (tipoIzq == "error" || tipoDer == "error")
            return "error";

        // Operaciones aritméticas (+, -, *, /)
        if (op == "+" || op == "-" || op == "*" || op == "/")
        {
            if (tipoIzq == "ENTERO" && tipoDer == "ENTERO")
                return "ENTERO";

            if ((tipoIzq == "ENTERO" || tipoIzq == "DECIMAL") &&
                (tipoDer == "ENTERO" || tipoDer == "DECIMAL"))
            {
                return "DECIMAL"; // Si mezclas decimal y entero, el resultado es decimal
            }

            // Si intentan sumar/restar cadenas con números:
            return "error";
        }

        // Operaciones relacionales (<, >, ==) siempre devuelven BOOLEANO
        if (op == "<" || op == ">" || op == "==")
        {
            if (tipoIzq == tipoDer)
                return "BOOLEANO";
            return "error";
        }

        return "error";
    }
    // Añade esto en la sección public de TablaSimbolos.h
    string obtenerTablaAString()
    {
        if (tabla.empty())
            return "La tabla de símbolos está vacía.\n";

        string resultado = "ID\t\t|\tTIPO\n";
        resultado += "----------------------------------------\n";

        for (const auto &par : tabla)
        {
            resultado += par.first;
            // Tabulaciones dinámicas para que se vea alineado
            if (par.first.length() < 6)
                resultado += "\t\t|\t";
            else
                resultado += "\t|\t";

            resultado += par.second.tipo + "\n";
        }
        return resultado;
    }
};

#endif
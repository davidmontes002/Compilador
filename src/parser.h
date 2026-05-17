#pragma once
// #include "Lexer.h"
#include "Tokens.h"
#include "TablaSimbolos.h"
// #include <iostream>
#include <vector>
// creacion de la estructura de parsers
// cambio
using namespace std;
// aqui se declara
//  structura nodo del ast
//
struct Nodo
{
  string tipo;
  string valor;
  vector<Nodo *> hijos;
  Nodo *izq;
  Nodo *der;
  // constructor de la struct
  Nodo(string t, string v, Nodo *i = nullptr, Nodo *d = nullptr)
      : tipo(t), valor(v), izq(i), der(d) {}
  ~Nodo()
  {
    delete izq;
    delete der;
    for (Nodo *hijo : hijos)
      delete hijo;
  }
};

class Parser
{
public:
  // void printTree();
  vector<string> errores;
  Parser(vector<Token> toks);
  Nodo *ParseAsignacion();
  Nodo *parsearPrograma();
  TablaSimbolos tablaSimbolos;

private:
  Nodo *parsearImprimir();
  Nodo *parsearDeclaracion();
  Nodo *parsearSi();
  Nodo *parsearMientras();
  Nodo *parsearCondicion();
  Nodo *parsearBloque();
  Nodo *llamadaFuncion();
  Nodo *parsearSentencia();
  Nodo *parsearAsignacion();
  Nodo *parsearExpresion();
  Token Actual();
  Token nextToken();
  // fos = flag only show- sobrecarga
  Token nextToken(bool fos);

  vector<Token> tokens;
  int posicion = 0;
};

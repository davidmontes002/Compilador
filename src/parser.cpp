#include "parser.h"
#include "Tokens.h"
// #include <cstdlib>
#include <iostream>
#include <iterator>
// #include <iterator>
using namespace std;
Parser::Parser(vector<Token> t) : tokens(t), posicion(0) {}
// retorna el token en posicion
Token Parser::Actual()
{
  if (posicion >= tokens.size())
  {
    return tokens.back(); // regresa el último token (FIN)
  }
  return tokens[posicion];
}

Token Parser::nextToken()
{
  // funcion que muestra el actual y avanza 1
  if (posicion >= tokens.size())
  {
    return tokens.back(); // regresa el último token (FIN)
  }
  return tokens[posicion++]; // retorna token y despues de retornarlo suma 1
}
// fos = bandera de solo muestra ()
// sobrecarga con bandera
Token Parser::nextToken(bool fos)
{
  // funcion que solo muestra el siguiente, no avanza
  if (posicion >= tokens.size())
  {
    return tokens.back(); // regresa el último token (FIN)
  }
  return tokens[posicion + 1]; // retorna token en posicion +1 (no avanza)
}
// inicio
Nodo *Parser::parsearPrograma()
{
  Nodo *raiz = new Nodo("PROGRAMA", ""); // nodo : raiz "PROGRAMA"
  while (Actual().tipo != FIN)
  {
    Nodo *sentencia = parsearSentencia();
    // cout << "Llamada: " << i << endl;
    // se supone que tiene que haber una llamada
    if (sentencia != nullptr)
    {
      raiz->hijos.push_back(sentencia);
    }
  }
  return raiz;
}
// required: que entre en inicio de una sentencia
Nodo *Parser::parsearSentencia()
{
  switch (Actual().tipo)
  {
  case ID:
    return parsearAsignacion();
    break;
  case LLAVE_ABRE:
    return parsearBloque();
    break;
  case KW_CLASE:
    break;
  case KW_MIENTRAS:
    return parsearMientras();
    break;
  case KW_IMPRIMIR:
    return parsearImprimir();
    break;
  case KW_SI:
    return parsearSi();
    break;
  case KW_BOOL:
    parsearDeclaracion();
    break;
  case KW_ENTERO:
    return parsearDeclaracion();
    break;
  case KW_CADENA_TIPO:
    return parsearDeclaracion();
    break;
  case KW_CARACTER:
    return parsearDeclaracion();
    break;
  case KW_DOBLE:
    return parsearDeclaracion();
    break;
  // case KW_CADENA_TIPO:
  // parsearCadenaTipo();
  default: // se ejecuta solo si al parsearla sentencia
           // entra cuando el puntero queda intermedio en un token o en un token
           // que no es inicio de sentencia
    cout << "debug 1: " << Actual().tipo << " -> " << Actual().lexema << endl;
    errores.push_back("Error: DEB:Inicio de sentencia invalido en linea: " +
                      to_string(Actual().linea));
    // cout << "Token: " << Actual().lexema << endl;
    int lineaE = Actual().linea;
    // si encuentra un error en la linea x, avanza al token de la linea
    // diferente a esa
    while (Actual().tipo != FIN && Actual().linea == lineaE)
    {
      nextToken();
    }
    return nullptr;
  }
  return nullptr;
}
// sentencias tipo
// x= expresion
Nodo *Parser::parsearAsignacion()
{
  Token id = nextToken(); // x, avanza 1
                          // = , avanza a 1

  //========= Inicio contendio semantica ======
  if (!tablaSimbolos.existe(id.lexema))
  {
    errores.push_back("Error Semantico: La variable '" + id.lexema + "' no ha sido declarada. Linea " + to_string(id.linea));
  }
  //========== fin contenido de semantica =======

  if (Actual().tipo != ASIGNACION)
  {
    errores.push_back("Error: valor invalido o se esperaba '=' en linea " +
                      to_string(Actual().linea));
    int lineaE = Actual().linea;
    while (Actual().tipo != FIN && Actual().tipo != LLAVE_ABRE &&
           Actual().linea == lineaE)
    {
      nextToken();
    }
    return nullptr;
  }
  // igual
  // nextToken();
  // cout << "linea salida" << Actual().linea << Actual().lexema << endl;
  // si cambio de linea y no termino
  if (nextToken(0).linea != Actual().linea)
  {
    cout << "linea entrada" << Actual().linea << endl;

    cout << "token : " << Actual().lexema << endl;
    errores.push_back("Error: valor invalido despues de la asignacion linea: " +
                      to_string(Actual().linea));
    int lineaE = Actual().linea;
    while (Actual().linea == lineaE && Actual().tipo != LLAVE_ABRE &&
           Actual().tipo != FIN)
    {
      nextToken();
    }
    return nullptr;
  }
  nextToken();
  Nodo *temp = nullptr;
  switch (Actual().tipo)
  {
  case KW_LEER:
    nextToken();
    return new Nodo("ASIGNACION", "=", new Nodo("ID", id.lexema),
                    new Nodo("Lectura", "Input"));
    break;
  case KW_VERDADERO:
    nextToken();
    return new Nodo("ASIGNACION", "=", new Nodo("ID", id.lexema),
                    new Nodo("BOOl", "VERDADERO"));
    break;
  case KW_FALSO:
    nextToken();
    return new Nodo("ASIGNACION", "=", new Nodo("ID", id.lexema),
                    new Nodo("BOOl", "FALSO"));
    break;
  }

  if (Actual().tipo == DECIMAL || Actual().tipo == ENTERO ||
      Actual().tipo == ID || Actual().tipo == CADENA)
  {
    Nodo *der = nullptr;
    if (Actual().tipo == CADENA)
    {
      Nodo *temp = new Nodo("CADENA", Actual().lexema);
      nextToken();
      der = temp;
    }
    else
    {
      der = parsearExpresion();
    }

    //========= Inicio contendio semantica ======
    if (der != nullptr && tablaSimbolos.existe(id.lexema))
    {
      string tipoDestino = tablaSimbolos.getType(id.lexema);
      string tipoValorAsignado = "";

      // Determinamos el tipo de lo que se le está asignando
      if (der->tipo == "ENTERO" || der->tipo == "DECIMAL" || der->tipo == "CADENA")
      {
        tipoValorAsignado = der->tipo;
      }
      else if (der->tipo == "ID")
      {
        // Si le asignas otra variable (ej: x = y), sacamos el tipo de 'y' de la tabla
        tipoValorAsignado = tablaSimbolos.getType(der->valor);
      }

      // Si logramos determinar el tipo asignado y no coincide con el destino
      if (tipoValorAsignado != "" && !tablaSimbolos.equalType(id.lexema, tipoValorAsignado))
      {
        errores.push_back("Error Semantico: Incompatibilidad de tipos en linea " + to_string(id.linea) +
                          ". No puedes asignar " + tipoValorAsignado + " a la variable '" + id.lexema + "' que es " + tipoDestino + ".");
      }
    }
    //========== fin contenido de semantica =======

    return new Nodo("ASIGNACION", "=", new Nodo("ID", id.lexema), der);
  }
  else
  {
    errores.push_back(
        "Error: valor invalido despues en la asignacion, linea: " +
        to_string(Actual().linea));
    int lineaE = Actual().linea;
    while (Actual().linea == lineaE && Actual().tipo != LLAVE_ABRE &&
           Actual().tipo != FIN)
    {
      nextToken();
    }
    return nullptr;
  }
}
Nodo *Parser::parsearExpresion()
{
  // entra con ')'
  // aqui entra una cadena =="cadena"

  int lineaE = Actual().linea;
  Nodo *izq = nullptr;
  // cout << "tipo: " << Actual().tipo << endl;
  if (Actual().tipo == ENTERO || Actual().tipo == DECIMAL ||
      Actual().tipo == ID || Actual().tipo == CADENA)
  {

    //========= Inicio contendio semantica ======
    if (Actual().tipo == ID)
    {
      if (!tablaSimbolos.existe(Actual().lexema))
      {
        errores.push_back("Error Semantico: Uso de variable no declarada '" + Actual().lexema + "' en linea: " + to_string(Actual().linea));
      }
    }
    //========== fin contenido de semantica =======

    string tipoNodo;
    if (Actual().tipo == ENTERO)
      tipoNodo = "ENTERO"; // ENUM toString(enum)
                           // string
    else if (Actual().tipo == DECIMAL)
      tipoNodo = "DECIMAL";
    else if (Actual().tipo == CADENA)
      tipoNodo = "CADENA";
    else
      tipoNodo = "ID";
    // cout << "tipoNodo asignado: " << tipoNodo << endl;
    izq = new Nodo(tipoNodo, nextToken().lexema); // Entero , 2  -> -+/
    if (Actual().tipo == FIN || Actual().tipo == CADENA ||
        Actual().linea != lineaE)
    {
      return izq;
    } // aqui ya paso cadena y es el ')'
  }
  else
  { // x=
    errores.push_back("Error: valor invalido en linea: " +
                      to_string(Actual().linea));
    int lineaE = Actual().linea;
    // saltar a la siguiente linea

    while (Actual().tipo != FIN && Actual().linea == lineaE &&
           Actual().tipo != LLAVE_ABRE && Actual().tipo != PAREN_CIERRA)
    {
      nextToken();
    }
    return nullptr;
  }

  if (Actual().tipo == MAS || Actual().tipo == MENOS ||
      Actual().tipo == MULTIPLICACION || Actual().tipo == DIVISION)
  {

    Token operador = nextToken(); //+ avanzo 1
    Nodo *der = parsearExpresion();
    // x=2+10 :  +
    // der =
    return new Nodo(operador.lexema, "", izq, der);
  }
  else if (Actual().tipo == PAREN_CIERRA)
  {
    return izq;
  }
  else
  {
    errores.push_back("Error: valor o simbolo invalido en la linea: " +
                      to_string(Actual().linea));
    int lineaE = Actual().linea;
    while (Actual().tipo != FIN && Actual().linea == lineaE)
    {
      nextToken();
    }
    return nullptr;
  }

  return izq;
}
Nodo *Parser::parsearCondicion()
{
  // debugin si(x==){} errores
  if (Actual().tipo != PAREN_ABRE)
  {
    errores.push_back("Error: se esperaba un '(' en linea: " +
                      to_string(Actual().linea));
    int lineaE = Actual().linea;
    while (Actual().tipo != PAREN_CIERRA && Actual().tipo != FIN &&
           Actual().tipo != LLAVE_ABRE && Actual().linea == lineaE)
    {
      nextToken();
    }
    return nullptr;
  }
  nextToken();
  Nodo *izq = nullptr;

  if (Actual().tipo == PAREN_CIERRA)
  {
    errores.push_back("Error: condicion vacia '()', invalida en linea: " +
                      to_string(Actual().linea));
    return nullptr;
  }
  if (!(Actual().tipo == ID || Actual().tipo == ENTERO ||
        Actual().tipo == DECIMAL))
  {
    errores.push_back("Error: valor invalido en la condicion, linea: " +
                      to_string(Actual().linea));
    int lineaE = Actual().linea;
    while (Actual().tipo != FIN && Actual().tipo != PAREN_CIERRA &&
           Actual().tipo != LLAVE_ABRE && Actual().linea == lineaE)
    {
      nextToken();
    }
    return nullptr;
  }
  else
  {
    string tipoNodo;
    if (Actual().tipo == ENTERO)
      tipoNodo = "ENTERO"; // ENUM toString(enum)
    else if (Actual().tipo == DECIMAL)
      tipoNodo = "DECIMAL";
    // else if (Actual().tipo == CADENA) tipoNodo = "CADENA";
    else
      tipoNodo = "ID";
    // cout << "tipoNodo asignado: " << tipoNodo << endl;
    izq = new Nodo(tipoNodo, nextToken().lexema); // Entero , 2  -> -+/
  }
  if (Actual().tipo == MENOR_QUE || Actual().tipo == MAYOR_QUE ||
      Actual().tipo == IGUALDAD || Actual().tipo == OP_AND ||
      Actual().tipo == OP_OR || Actual().tipo == DESIGUALDAD ||
      Actual().tipo == MAYOR_IGUAL || Actual().tipo == MENOR_IGUAL)
  {
    Token operador = nextToken(); //+ avanzo valo
    Nodo *der = parsearExpresion();
    return new Nodo("CONDICION", operador.lexema, izq, der);
  }
  else
  {

    errores.push_back("Error: operador invalido en linea: " +
                      to_string(Actual().linea));
    while (Actual().tipo != PAREN_CIERRA && Actual().tipo != FIN &&
           Actual().tipo != LLAVE_ABRE)
    {
      nextToken();
    }
    return nullptr;
  }
  return izq;
}

Nodo *Parser::parsearBloque()
{
  // validar si se inicia en con }
  // solo coherente dentro de un if o algo asi xd
  //{}
  if (Actual().tipo != LLAVE_ABRE)
  {
    errores.push_back("Error: se esperaba un '{' en linea: " +
                      to_string(Actual().linea));
    return nullptr;
  }
  nextToken(); // validacion de que un bloque no sea vacio

  Nodo *bloque = new Nodo("BLOQUE", "{}");
  while (Actual().tipo != LLAVE_CIERRA && Actual().tipo != FIN)
  {
    Nodo *s = parsearSentencia();
    // si en algun momento s es nullptr quiere decir, que en la
    // sentencia hay un error
    if (s != nullptr)
    {
      bloque->hijos.push_back(s);
    }
  }
  if (Actual().tipo != LLAVE_CIERRA)
  {
    errores.push_back("Error: se esperaba un '}' en linea: " +
                      to_string(Actual().linea));

    return nullptr;
  }
  nextToken();
  return bloque;
}
// parseo de si
Nodo *Parser::parsearSi()
{
  // si(){}
  Nodo *si_parsed = new Nodo("SENTENCIA_SI", "<>");

  nextToken();
  Nodo *izq = parsearCondicion();

  if (izq == nullptr)
  {
    // vacio
  }
  else
  {
    if (Actual().tipo != PAREN_CIERRA)
    {
      errores.push_back("Error: se esperaba un ')' en linea: " +
                        to_string(Actual().linea));
      int lineaE = Actual().linea;
      while (Actual().tipo != FIN && Actual().tipo != LLAVE_ABRE &&
             Actual().tipo == lineaE)
      {
        nextToken();
      }
      return nullptr;
    }
    else
    {

      si_parsed->hijos.push_back(izq);
    }
  }

  nextToken();
  Nodo *der = parsearBloque();

  if (der == nullptr)
  {
    return nullptr;
  }

  si_parsed->hijos.push_back(der);
  if (Actual().tipo == KW_SINO)
  {
    nextToken();
    Nodo *bloque = parsearBloque();
    bloque->tipo = "SINO_BLOQUE";

    si_parsed->hijos.push_back(bloque);
  }

  return si_parsed;
}
// ejemplo input
// x=0
// mientras(x<10){
// x=x+1
// }
Nodo *Parser::parsearMientras()
{
  // entra en mientras
  if (Actual().tipo != KW_MIENTRAS)
  {
    errores.push_back("Error: inicio de sentencia invalido, en linea: " +
                      to_string(Actual().linea));
    return nullptr;
  }
  nextToken();
  if (Actual().tipo != PAREN_ABRE)
  {
    errores.push_back("Error: se esperaba un '(' en linea: " +
                      to_string(Actual().tipo));
    while (Actual().tipo != PAREN_CIERRA && Actual().tipo != FIN &&
           Actual().tipo != LLAVE_ABRE)
    {
      nextToken();
    }
    return nullptr;
  }
  Nodo *izq = parsearCondicion();
  if (izq == nullptr)
  {
    return nullptr;
  }
  // validacion si hay )
  if (Actual().tipo != PAREN_CIERRA)
  {
    errores.push_back("Error: se esperaba un ')' en linea: " +
                      to_string(Actual().tipo));
    return nullptr;
  }
  nextToken();
  Nodo *der = parsearBloque();
  return new Nodo("SENT_MIENTRAS", "", izq, der);
}
Nodo *Parser::parsearImprimir()
{

  Nodo *N_print = new Nodo("IMPRIMIR", "");
  if (Actual().tipo == KW_IMPRIMIR)
  {
    nextToken();

    if (Actual().tipo != PAREN_ABRE)
    {
      errores.push_back("Error: se esperaba un '(' en linea: " +
                        to_string(Actual().linea));
      int lineaE = Actual().linea;
      while (Actual().tipo != FIN && Actual().linea == lineaE)
      {
        nextToken();
      }
      return nullptr;
    }
    nextToken();
    N_print->izq = parsearExpresion();

    if (Actual().tipo != PAREN_CIERRA)
    {
      errores.push_back("Error: se esperaba un ')' en linea: " +
                        to_string(Actual().linea));
      int lineaE = Actual().linea;
      while (Actual().tipo != FIN && Actual().linea == lineaE)
      {
        nextToken();
      }
    }
    else
    {
      nextToken();
    }
  }
  else
  {
    int lineaE = Actual().linea;
    while (Actual().tipo != FIN && Actual().linea == lineaE)
    {
      nextToken();
    }
    return nullptr;
  }

  return N_print;
}
Nodo *Parser::parsearDeclaracion()
{
  Nodo *declaracion = new Nodo("DECLARACION", "");
  string tipoDato = "";

  if (Actual().tipo == KW_CADENA_TIPO || Actual().tipo == KW_ENTERO ||
      Actual().tipo == KW_DOBLE || Actual().tipo == KW_CARACTER ||
      Actual().tipo == KW_BOOL)
  {
    switch (Actual().tipo)
    {
    case KW_CADENA_TIPO:
      tipoDato = "CADENA";
      break;
    case KW_ENTERO:
      tipoDato = "ENTERO";
      break;
    case KW_DOBLE:
      tipoDato = "DOBLE";
      break;
    case KW_CARACTER:
      tipoDato = "CARACTER";
      break;
    case KW_BOOL:
      tipoDato = "BOOLEANO";
      break;
    default:
      tipoDato = "UNDENIFIED";
    }

    int lineaE = Actual().linea;
    nextToken();
    if (lineaE != Actual().linea)
    {
      errores.push_back("Error: simbolo faltante en la asignacion en linea: " +
                        to_string(lineaE));

      int lineaE = Actual().linea;
      while (Actual().tipo != FIN && Actual().linea == lineaE)
      {
        nextToken();
      }
      return nullptr;
    }
    // estado : ID (nombre de la variable)

    if (Actual().tipo != ID)
    {
      errores.push_back("Error: simbolo faltante en la asignacion en linea: " +
                        to_string(lineaE));

      int lineaE = Actual().linea;
      while (Actual().tipo != FIN && Actual().linea == lineaE)
      {
        nextToken();
      }
      return nullptr;
    }
    string lex_id = Actual().lexema;

    //========= Inicio contendio semantica ======
    if (!tablaSimbolos.insertar(lex_id, tipoDato))
    {
      errores.push_back("Error Semantico: La variable '" + lex_id + "' ya fue declarada previamente. Linea: " + to_string(Actual().linea));
    }
    //========== fin contenido de semantica =======

    // estado :
    if (nextToken(0).tipo == ASIGNACION)
    {
      declaracion->izq = new Nodo(tipoDato, "");

      declaracion->der = parsearAsignacion();
    }
    else
    {
      declaracion->izq = new Nodo(tipoDato, "");
      declaracion->der = new Nodo("ID", lex_id);
      nextToken();
      // devolio declracion
      return declaracion;
    }
  }
  return declaracion;
}
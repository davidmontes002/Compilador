#include "Lexer.h"
#include "Tokens.h"
#include "ast_viewer.h"
#include "glib-object.h"
#include "glib.h"
#include "gtk/gtkcssprovider.h"
#include "parser.h"
#include <fstream>
#include <gtk/gtk.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// Estructura actualizada para incluir el nuevo widget de la Tabla de Símbolos
typedef struct
{
  GtkWidget *EntryCode;
  GtkWidget *OutputText;
  GtkWidget *SymbolTableText; // <--- NUEVO
  GtkWidget *ThemeColorBtn;
  GtkCssProvider *Provider;
  GtkCssProvider *ProviderFont;
} AppWidgets;

int sizeFont = 16;
bool color_state = false;

char cssWindowColorNight[800] =
    "window,  scrolledwindow, viewport { background-color: #2B2B2B; } "
    "label {color: #EDDDDA;}"
    "textview { background-color: #1E1E1E; color: #EBE7E6; } "
    "textview text { background-color: #1E1E1E; color: #EBE7E6; } "
    "button { background: #3C3C3C; background-image: none; "
    "         border: none; border-radius: 4px; } "
    "button:hover { background: #505050; background-image: none; } "
    "button label { color: #EBE7E6; }";

char cssWindowColorDay[800] =
    "window,  scrolledwindow, viewport { background-color: #EBEBEB; } "
    "label {color: #2E2B2B;}"
    "textview { background-color: #FFFFFF; color: #000000; } "
    "textview text { background-color: #FFFFFF; color: #000000; } "
    "button { background: #EFEFEF; background-image: none; "
    "         border: none; border-radius: 4px; } "
    "button:hover { background: #E3D8D5; background-image: none; } "
    "button label { color: #121111; }";

char cssBuffer[200];
vector<Token> listaTokens;
vector<string> errores;
Nodo *arbol = nullptr;

GtkTextBuffer *bufferCode;
GtkTextBuffer *bufferOutput;

// === MODIFICADO: Ahora actualiza la UI automáticamente ===
void parsear(GtkWidget *btn, gpointer data)
{
  AppWidgets *widgets = (AppWidgets *)data;
  errores.clear();

  Parser parser(listaTokens);
  arbol = parser.parsearPrograma();
  errores = parser.errores;

  // 1. Mostrar Errores Semánticos y Sintácticos en el OutputText
  GtkTextBuffer *bufOut = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->OutputText));
  string textoErrores = "";
  if (!errores.empty())
  {
    for (string e : errores)
    {
      textoErrores += e + "\n";
    }
  }
  else
  {
    textoErrores = "Compilación exitosa. No se encontraron errores.\n";
  }
  gtk_text_buffer_set_text(bufOut, textoErrores.c_str(), -1);

  // 2. Mostrar la Tabla de Símbolos en tiempo real
  GtkTextBuffer *bufSym = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->SymbolTableText));
  string textoTabla = parser.tablaSimbolos.obtenerTablaAString();
  gtk_text_buffer_set_text(bufSym, textoTabla.c_str(), -1);
}

void ZoomM(GtkWidget *widget, gpointer data)
{
  AppWidgets *widgets = (AppWidgets *)data;
  GtkCssProvider *provider = widgets->ProviderFont;
  if (sizeFont < 30)
  {
    sizeFont = sizeFont + 1;
    sprintf(cssBuffer,
            ".text_output { font-family: 'Monospace'; font-size: %dpt; }"
            ".text_entry {font-family : 'Monospace'; font-size: %dpt;}",
            sizeFont, sizeFont);
    gtk_css_provider_load_from_data(provider, cssBuffer, -1, NULL);
  }
}

void ZoomL(GtkWidget *widget, gpointer data)
{
  AppWidgets *widgets = (AppWidgets *)data;
  GtkCssProvider *provider = widgets->ProviderFont;
  if (sizeFont > 10)
  {
    sizeFont = sizeFont - 1;
    sprintf(cssBuffer,
            ".text_output { font-family: 'Monospace'; font-size: %dpt; }"
            ".text_entry {font-family : 'Monospace'; font-size: %dpt;}",
            sizeFont, sizeFont);
    gtk_css_provider_load_from_data(provider, cssBuffer, -1, NULL);
  }
}

void imprimirArbol(Nodo *nodo, int nivel = 0) { ast_viewer_mostrar(arbol); }

string LimpiarLexemaParaDisplay(const string &lexema)
{
  string resultado = "";
  for (char c : lexema)
  {
    if (c == '\n')
      resultado += "\\n";
    else if (c == '\r')
      resultado += "\\r";
    else if (c == '\t')
      resultado += "\\t";
    else
      resultado += c;
  }
  return resultado;
}

void setBufferCodeBytxt(string buff_C)
{
  gtk_text_buffer_set_text(GTK_TEXT_BUFFER(bufferCode), buff_C.c_str(), -1);
}

void ImportarCode(GtkWidget *widget, gpointer data)
{
  AppWidgets *widgets = (AppWidgets *)data;
  bufferCode = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->EntryCode));
  ifstream inputFile("code_ejemplo.txt");
  if (!inputFile.is_open())
  {
    cout << "Error: no se pudo abrir el archivo" << endl;
  }
  string linea;
  string bufferC;
  bufferC.reserve(300);
  while (getline(inputFile, linea))
  {
    bufferC.append(linea);
    bufferC.append("\n");
  }
  setBufferCodeBytxt(bufferC);
}

void changeColor(GtkWidget *widget, gpointer data)
{
  AppWidgets *widgets = (AppWidgets *)data;
  GtkCssProvider *provider = widgets->Provider;
  GtkWidget *ThemeColorBtn = widgets->ThemeColorBtn;
  if (color_state)
  {
    gtk_css_provider_load_from_data(provider, cssWindowColorNight, -1, NULL);
    gtk_button_set_label(GTK_BUTTON(ThemeColorBtn), "Night");
  }
  else
  {
    gtk_css_provider_load_from_data(provider, cssWindowColorDay, -1, NULL);
    gtk_button_set_label(GTK_BUTTON(ThemeColorBtn), "Day");
  }
  color_state = !color_state;
}

void IDC_BTN_ANALIZAR(GtkWidget *btn, gpointer data)
{
  AppWidgets *widgets = (AppWidgets *)data;
  bufferOutput = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->OutputText));
  bufferCode = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->EntryCode));
  gint len = gtk_text_buffer_get_char_count(bufferCode);

  if (len > 0)
  {
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(bufferCode, &start, &end);
    gchar *codigo = gtk_text_buffer_get_text(bufferCode, &start, &end, FALSE);
    string CodigoFuente = string(codigo);
    g_free(codigo);
    try
    {
      Lexer miAnalizador(CodigoFuente, "matriz_transiciones.csv",
                         "nombres_tokens.csv", "estados_tokens.csv",
                         "char_columnas.csv", "keywords.csv");

      listaTokens = miAnalizador.generarListaTokens();
      listaTokens.push_back({FIN, "EOF", 0, 0});

      std::stringstream salida;
      for (const auto &token : listaTokens)
      {
        const std::string &nombre = NOMBRES_TOKENS[token.tipo];
        salida << "Línea: " << token.linea << " \t| Col: " << token.columna << " \t| ";
        salida << "Tipo: [" << nombre << "]";
        salida << (nombre.length() < 7 ? "\t\t" : "\t");
        salida << "| Lexema: [" << LimpiarLexemaParaDisplay(token.lexema) << "]\r\n";
      }
      gtk_text_buffer_set_text(bufferOutput, salida.str().c_str(), -1);
    }
    catch (exception &e)
    {
      string msg = e.what();
      gtk_text_buffer_set_text(bufferOutput, msg.c_str(), -1);
    }
  }
}

static void activate(GtkApplication *app, gpointer user_data)
{
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Analizador Lexico y Semántico FIM");
  gtk_window_set_default_size(GTK_WINDOW(window), 1100, 700);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  GtkWidget *box_btns = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  GtkWidget *box_header_src = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

  GtkWidget *lblinput = gtk_label_new("Codigo fuente");

  gtk_container_set_border_width(GTK_CONTAINER(box), 15);
  gtk_container_add(GTK_CONTAINER(window), box);

  GtkWidget *btnZoomM = gtk_button_new_with_label("+");
  GtkWidget *btnZoomL = gtk_button_new_with_label("-");
  GtkWidget *btnCompilar = gtk_button_new_with_label("Ejecutar Analizador (Lexer)");
  GtkWidget *btnMostrarAST = gtk_button_new_with_label("Mostrar AST");
  GtkWidget *btnGenerarArbol = gtk_button_new_with_label("Generar Arbol y Analizar");
  GtkWidget *btnGetText = gtk_button_new_with_label("Importar");
  GtkWidget *btnChangeColor = gtk_button_new_with_label("Day");

  GtkWidget *scrollCode = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_set_size_request(scrollCode, -1, 150);
  gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrollCode), 200);

  GtkWidget *EntryCode = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(EntryCode), GTK_WRAP_WORD);
  gtk_container_add(GTK_CONTAINER(scrollCode), EntryCode);

  // === MODIFICADO: Contenedor para dividir la pantalla inferior en dos ===
  GtkWidget *box_bottom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

  // Lado Izquierdo: Consola de Salida / Errores
  GtkWidget *box_out_vert = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  GtkWidget *lbloutput = gtk_label_new("Salida / Consola de Errores");
  GtkWidget *scrollOutput = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrollOutput), 200);
  GtkWidget *OutputText = gtk_text_view_new();
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(OutputText), GTK_WRAP_WORD);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(OutputText), FALSE);
  gtk_container_add(GTK_CONTAINER(scrollOutput), OutputText);
  gtk_box_pack_start(GTK_BOX(box_out_vert), lbloutput, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box_out_vert), scrollOutput, TRUE, TRUE, 0);

  // Lado Derecho: Tabla de Símbolos
  GtkWidget *box_sym_vert = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  GtkWidget *lblsymbols = gtk_label_new("Tabla de Símbolos");
  GtkWidget *scrollSymbols = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrollSymbols), 200);
  GtkWidget *SymbolTableText = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(SymbolTableText), FALSE);
  gtk_container_add(GTK_CONTAINER(scrollSymbols), SymbolTableText);
  gtk_box_pack_start(GTK_BOX(box_sym_vert), lblsymbols, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box_sym_vert), scrollSymbols, TRUE, TRUE, 0);

  // Agregamos ambas cajas al contenedor inferior
  gtk_box_pack_start(GTK_BOX(box_bottom), box_out_vert, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box_bottom), box_sym_vert, TRUE, TRUE, 0);
  // =========================================================================

  GtkStyleContext *context_out = gtk_widget_get_style_context(OutputText);
  GtkStyleContext *context_entry = gtk_widget_get_style_context(EntryCode);
  GtkStyleContext *context_sym = gtk_widget_get_style_context(SymbolTableText); // Estilo para tabla

  gtk_style_context_add_class(context_out, "text_output");
  gtk_style_context_add_class(context_entry, "text_entry");
  gtk_style_context_add_class(context_sym, "text_output"); // Usamos el mismo estilo de salida

  GtkCssProvider *provider = gtk_css_provider_new();
  GtkCssProvider *providerFont = gtk_css_provider_new();

  AppWidgets *widgets = g_new(AppWidgets, 1);
  widgets->EntryCode = EntryCode;
  widgets->OutputText = OutputText;
  widgets->SymbolTableText = SymbolTableText; // Guardamos el widget
  widgets->ThemeColorBtn = btnChangeColor;
  widgets->Provider = provider;
  widgets->ProviderFont = providerFont;

  gtk_css_provider_load_from_data(
      providerFont,
      ".text_output { font-family: 'Monospace'; font-size: 16pt; }"
      ".text_entry {font-family : 'Monospace'; font-size: 16pt;}",
      -1, NULL);
  gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                            GTK_STYLE_PROVIDER(providerFont),
                                            GTK_STYLE_PROVIDER_PRIORITY_USER);

  gtk_css_provider_load_from_data(provider, cssWindowColorDay, -1, NULL);
  gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                            GTK_STYLE_PROVIDER(provider),
                                            GTK_STYLE_PROVIDER_PRIORITY_USER);

  gtk_box_pack_start(GTK_BOX(box_btns), btnGenerarArbol, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box_btns), btnMostrarAST, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box_btns), btnCompilar, FALSE, TRUE, 0);
  gtk_box_pack_end(GTK_BOX(box_btns), btnChangeColor, FALSE, TRUE, 0);
  gtk_box_pack_end(GTK_BOX(box_btns), btnZoomM, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box_header_src), btnGetText, FALSE, TRUE, 0);
  gtk_box_pack_end(GTK_BOX(box_btns), btnZoomL, FALSE, TRUE, 0);

  // Empaquetado principal
  gtk_box_pack_start(GTK_BOX(box), box_btns, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), box_header_src, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box_header_src), lblinput, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), scrollCode, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box), box_bottom, TRUE, TRUE, 0); // Insertamos el panel dividido

  // Signals
  g_signal_connect(btnZoomL, "clicked", G_CALLBACK(ZoomL), widgets);
  g_signal_connect(btnZoomM, "clicked", G_CALLBACK(ZoomM), widgets);
  g_signal_connect(btnGetText, "clicked", G_CALLBACK(ImportarCode), widgets);
  g_signal_connect(btnMostrarAST, "clicked", G_CALLBACK(imprimirArbol), NULL);

  // MODIFICADO: Ahora pasamos la estructura 'widgets' a parsear para actualizar textos
  g_signal_connect(btnGenerarArbol, "clicked", G_CALLBACK(parsear), widgets);

  g_signal_connect(btnCompilar, "clicked", G_CALLBACK(IDC_BTN_ANALIZAR), widgets);
  g_signal_connect(btnChangeColor, "clicked", G_CALLBACK(changeColor), widgets);

  gtk_widget_show_all(window);
}

int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);
  GtkApplication *app;
  int status;
  app = gtk_application_new("org.compiler.app", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
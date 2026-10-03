#include <iostream>
#include <string>
#include <vector>
#include <cctype>

using namespace std;

// Estructuras de datos requeridas
struct Token
{
    string lexema;
    string token;
    string interpretacion;
};

struct Instruccion
{
    string operacion = "";
    vector<string> registros;
    int direccion = -1; // -1 representa null[cite: 5]
    bool valida = false;
    string errorMsg = "";
};

// Función auxiliar para identificar tokens válidos
Token identificarToken(const string &lexema, bool esperandoComa)
{
    Token t;
    t.lexema = lexema;
    if (lexema == "MOV" || lexema == "ADD" || lexema == "STO" || lexema == "END")
    {
        t.token = "MNEMONICO";
        t.interpretacion = "Mnemónico de la operación";
    }
    else if (lexema == "AL" || lexema == "BL")
    {
        t.token = "REGISTRO";
        t.interpretacion = "Registro destino/origen";
    }
    else if (lexema == ",")
    {
        t.token = "COMA";
        t.interpretacion = "Separador";
    }
    else
    {
        bool esNumero = !lexema.empty();
        for (char c : lexema)
        {
            if (!isdigit(c))
                esNumero = false;
        }
        if (esNumero)
        {
            t.token = "NUMERO";
            t.interpretacion = "Dirección de memoria"; // Operando que representa dirección[cite: 4]
        }
        else
        {
            t.token = "ERROR";
            t.interpretacion = "Desconocido";
        }
    }
    return t;
}

// Analizador Léxico y AFD: Recorre carácter por carácter[cite: 4]
void analizarInstruccion(const string &entrada, Instruccion &inst, vector<Token> &tokens)
{
    inst.valida = true;
    int n = entrada.length();
    int i = 0;
    string buffer = "";
    vector<string> lexemas;

    // Recorrido carácter por carácter[cite: 4]
    while (i < n)
    {
        if (isspace(entrada[i]))
        {
            if (!buffer.empty())
            {
                lexemas.push_back(buffer);
                buffer = "";
            }
        }
        else if (entrada[i] == ',')
        {
            if (!buffer.empty())
            {
                lexemas.push_back(buffer);
                buffer = "";
            }
            lexemas.push_back(",");
        }
        else
        {
            buffer += entrada[i];
        }
        i++;
    }
    if (!buffer.empty())
        lexemas.push_back(buffer);

    // Validación del formato mediante AFD basado en los lexemas extraídos
    if (lexemas.empty())
    {
        inst.valida = false;
        inst.errorMsg = "Instrucción vacía.";
        return;
    }

    string mnem = lexemas[0];
    Token tMnem = identificarToken(mnem, false);
    tokens.push_back(tMnem);
    inst.operacion = mnem;

    if (tMnem.token != "MNEMONICO")
    {
        inst.valida = false;
        inst.errorMsg = "Mnemónico inválido.";
        return;
    }

    if (mnem == "MOV")
    {
        if (lexemas.size() != 4)
        {
            inst.valida = false;
            inst.errorMsg = "Formato incorrecto para MOV. Falta registro, coma o dirección.";
            return;
        }
        Token tReg = identificarToken(lexemas[1], false);
        Token tComa = identificarToken(lexemas[2], true);
        Token tNum = identificarToken(lexemas[3], false);

        tokens.push_back(tReg);
        tokens.push_back(tComa);
        tokens.push_back(tNum);

        if (tReg.token == "REGISTRO" && tComa.token == "COMA" && tNum.token == "NUMERO")
        {
            inst.registros.push_back(tReg.lexema);
            inst.direccion = stoi(tNum.lexema);
        }
        else
        {
            inst.valida = false;
            inst.errorMsg = "Falta la coma entre el registro y la dirección, o registro inválido.";
        }
    }
    else if (mnem == "ADD")
    {
        if (lexemas.size() != 4)
        {
            inst.valida = false;
            inst.errorMsg = "Formato incorrecto para ADD.";
            return;
        }
        Token tReg1 = identificarToken(lexemas[1], false);
        Token tComa = identificarToken(lexemas[2], true);
        Token tReg2 = identificarToken(lexemas[3], false);

        tokens.push_back(tReg1);
        tokens.push_back(tComa);
        tokens.push_back(tReg2);

        if (tReg1.token == "REGISTRO" && tComa.token == "COMA" && tReg2.token == "REGISTRO")
        {
            inst.registros.push_back(tReg1.lexema);
            inst.registros.push_back(tReg2.lexema);
        }
        else
        {
            inst.valida = false;
            inst.errorMsg = "Operandos inválidos para ADD.";
        }
    }
    else if (mnem == "STO")
    {
        if (lexemas.size() != 2)
        {
            inst.valida = false;
            inst.errorMsg = "Formato incorrecto para STO. Falta dirección.";
            return;
        }
        Token tNum = identificarToken(lexemas[1], false);
        tokens.push_back(tNum);

        if (tNum.token == "NUMERO")
        {
            inst.direccion = stoi(tNum.lexema);
        }
        else
        {
            inst.valida = false;
            inst.errorMsg = "Operando inválido para STO. Se esperaba una dirección.";
        }
    }
    else if (mnem == "END")
    {
        if (lexemas.size() != 1)
        {
            inst.valida = false;
            inst.errorMsg = "Formato incorrecto para END. No lleva operandos.";
            return;
        }
    }
}

// Implementación de la Máquina de Moore[cite: 5]
class MaquinaMoore
{
private:
    Instruccion inst;
    int estadoActual;

public:
    MaquinaMoore(Instruccion i) : inst(i), estadoActual(1) {}

    bool siguientePaso()
    {
        if (inst.operacion == "MOV")
        {
            switch (estadoActual)
            {
            case 1:
                cout << "1. MAR <- " << inst.direccion << "\n";
                break; // MAR recibe la dirección[cite: 3]
            case 2:
                cout << "2. MBR <- M[MAR]\n";
                break;
            case 3:
                cout << "3. " << inst.registros[0] << " <- MBR\n";
                break;
            default:
                return false; // Fin[cite: 5]
            }
        }
        else if (inst.operacion == "ADD")
        {
            switch (estadoActual)
            {
            case 1:
                cout << "1. ACC <- " << inst.registros[0] << " + " << inst.registros[1] << "\n";
                break; // Se modela como una sola microoperación[cite: 3]
            default:
                return false;
            }
        }
        else if (inst.operacion == "STO")
        {
            switch (estadoActual)
            {
            case 1:
                cout << "1. MAR <- " << inst.direccion << "\n";
                break;
            case 2:
                cout << "2. MBR <- ACC\n";
                break;
            case 3:
                cout << "3. M[MAR] <- MBR\n";
                break;
            default:
                return false;
            }
        }
        else if (inst.operacion == "END")
        {
            switch (estadoActual)
            {
            case 1:
                cout << "1. HALT <- 1\n";
                break;
            default:
                return false;
            }
        }
        estadoActual++;
        return true;
    }

    void ejecutarMicrooperaciones()
    {
        while (siguientePaso())
        {
        }
        cout << "Generación terminada.\n";
    }
};

void procesarEntrada(const string &entrada)
{
    cout << "--------------------------------------\n";
    cout << "ENTRADA\n"
         << entrada << "\n\n";

    Instruccion inst;
    vector<Token> tokens;
    analizarInstruccion(entrada, inst, tokens);

    cout << "VALIDACION MEDIANTE AFD\n";
    if (!inst.valida)
    {
        cout << "Instrucción inválida: " << inst.errorMsg << "\n";
        cout << "No se construye el objeto instrucción.\nNo se generan microoperaciones.\n"; // Salida requerida para error[cite: 6]
        return;
    }
    cout << "Instrucción válida.\n\n";

    cout << "TOKENS RECONOCIDOS\n";
    for (const auto &t : tokens)
    {
        cout << t.token << " (\"" << t.lexema << "\")\n";
    }
    cout << "\n";

    cout << "COMPONENTES IDENTIFICADOS\n";
    cout << "Mnemónico: " << inst.operacion << "\n";
    if (!inst.registros.empty())
    {
        cout << "Registro(s): ";
        for (const auto &r : inst.registros)
            cout << r << " ";
        cout << "\n";
    }
    if (inst.direccion != -1)
    {
        cout << "Dirección de memoria: " << inst.direccion << "\n";
        cout << "Direccionamiento: directo\n";
    }
    cout << "\n";

    // Objeto Instrucción JSON-like[cite: 5]
    cout << "OBJETO INSTRUCCION\n{\n";
    cout << "  operacion: \"" << inst.operacion << "\",\n";
    cout << "  registros: [";
    for (size_t i = 0; i < inst.registros.size(); i++)
    {
        cout << "\"" << inst.registros[i] << "\"";
        if (i < inst.registros.size() - 1)
            cout << ", ";
    }
    cout << "],\n";
    cout << "  direccion: " << (inst.direccion == -1 ? "null" : to_string(inst.direccion)) << "\n}\n\n";

    cout << "MICROOPERACIONES GENERADAS POR MOORE\n";
    MaquinaMoore maquina(inst);
    maquina.ejecutarMicrooperaciones();
}

int main()
{
    // DECLARACION DE VARIABLES DE PRUEBA Y RESULTADOS ESPERADOS
    // Se definen las variables para verificar explícitamente los casos solicitados[cite: 6, 7]
    vector<string> pruebasValidas = {
        "MOV AL, 6",
        "MOV BL, 7",
        "ADD AL, BL",
        "STO 8",
        "END"};

    vector<string> pruebasInvalidas = {
        "MOV AX, 6",
        "MOV AL 6",
        "ADD AL,",
        "STO",
        "END 8"};

    cout << "======= PRUEBAS ESPERADAS COMO VALIDAS =======\n";
    for (const string &prueba : pruebasValidas)
    {
        procesarEntrada(prueba);
    }

    cout << "\n======= PRUEBAS ESPERADAS COMO INVALIDAS =======\n";
    for (const string &prueba : pruebasInvalidas)
    {
        procesarEntrada(prueba);
    }

    return 0;
}

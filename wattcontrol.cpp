#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

// ==========================================
// DIRECTIVAS Y CONSTANTES DEL NEGOCIO (Finca Lechera)
// ==========================================
#define HORAS_DIA 24
#define PRECIO_CARO 1000.0        // Límite de precio alto en COP/kWh
#define NIVEL_EMBALSE_ALERTA 40.0 // Porcentaje crítico de embalses
#define POTENCIA_CONTRATADA 5.0   // Potencia máxima contratada en kW
#define PRECIO_RACIONAMIENTO 1500.0 // Límite para emergencia de racionamiento (break)

// ==========================================
// 1. CLASE BASE ABSTRACTA: Carga
// ==========================================
class Carga {
protected:
    string nombre;
    double potenciaKW;
    bool encendida;
    int horasUso;
public:
    Carga(string nom, double pot) : nombre(nom), potenciaKW(pot), encendida(false), horasUso(0) {}
    virtual ~Carga() {}

    // Método virtual puro (Polimorfismo)
    virtual bool debeEncender(double precioActual, double nivelEmbalses, bool esHoraPico) = 0;

    string getNombre() const { return nombre; }
    double getPotencia() const { return potenciaKW; }
    bool estaEncendida() const { return encendida; }
    int getHorasUso() const { return horasUso; }
    
    void reiniciarHoras() { horasUso = 0; }
};

// ==========================================
// 2. CLASE HIJA: CargaCritica (Ej. Tanque de Frío / Refrigeración de Leche)
// ==========================================
class CargaCritica : public Carga {
public:
    CargaCritica(string nom, double pot) : Carga(nom, pot) {}

    // La carga crítica nunca se apaga (Regla 1)
    bool debeEncender(double precioActual, double nivelEmbalses, bool esHoraPico) override {
        encendida = true;
        horasUso++;
        return true;
    }
};

// ==========================================
// 3. CLASE HIJA: CargaAplazable (Ej. Máquina Ordeñadora / Bomba de Agua)
// ==========================================
class CargaAplazable : public Carga {
private:
    int horasMinimasRequeridas;
public:
    CargaAplazable(string nom, double pot, int hMin) : Carga(nom, pot), horasMinimasRequeridas(hMin) {}

    // Aplaza si embalses < 40%, si es hora pico, o si precio > PRECIO_CARO (Reglas 2 a 5)
    bool debeEncender(double precioActual, double nivelEmbalses, bool esHoraPico) override {
        if (nivelEmbalses < NIVEL_EMBALSE_ALERTA || esHoraPico || precioActual > PRECIO_CARO) {
            encendida = false;
        } else {
            encendida = true;
            horasUso++;
        }
        return encendida;
    }

    void apagarForzoso() {
        encendida = false;
    }

    int getHorasMinimas() const { return horasMinimasRequeridas; }
};

// ==========================================
// 4. CLASE MEDIDOR (Acumula kWh y costos)
// ==========================================
class Medidor {
private:
    double kwhAcumuladosGestor;
    double costoAcumuladoGestor;
    double kwhSinGestor;
    double costoSinGestor;
public:
    Medidor() : kwhAcumuladosGestor(0), costoAcumuladoGestor(0), kwhSinGestor(0), costoSinGestor(0) {}

    void registrarConsumo(double potencia, double tarifa, bool conGestor) {
        if (conGestor) {
            kwhAcumuladosGestor += potencia; // Asumiendo intervalos de 1 hora (kW * 1h = kWh)
            costoAcumuladoGestor += (potencia * tarifa);
        } else {
            kwhSinGestor += potencia;
            costoSinGestor += (potencia * tarifa);
        }
    }

    void reiniciar() {
        kwhAcumuladosGestor = 0;
        costoAcumuladoGestor = 0;
        kwhSinGestor = 0;
        costoSinGestor = 0;
    }

    double getCostoGestor() const { return costoAcumuladoGestor; }
    double getCostoSinGestor() const { return costoSinGestor; }
    double getKwhGestor() const { return kwhAcumuladosGestor; }
};

// ==========================================
// 5. CLASE MERCADO (Genera precios y embalses)
// ==========================================
class Mercado {
public:
    double generarPrecio() {
        // Genera un precio aleatorio entre 400 y 1400 COP/kWh
        return 400.0 + (rand() % 1001);
    }

    double generarEmbalses() {
        // Genera un nivel de embalses entre 30% y 90%
        return 30.0 + (rand() % 61);
    }
};

// ==========================================
// 6. CLASE NEGOCIO (Integra todo)
// ==========================================
class Negocio {
private:
    Carga* cargas[3]; // Arreglo de punteros Carga* (Composición/Agregación)
    Medidor medidor;
    Mercado mercado;
    double ultimoPrecio;
    double ultimosEmbalses;
    int ultimaHora;
    bool alertaProduccion;
public:
    Negocio() {
        // Inicializamos las cargas de la Finca Lechera
        cargas[0] = new CargaCritica("Tanque de Frio (Refrigeracion)", 2.5);
        cargas[1] = new CargaAplazable("Maquina Ordenadora Electrica", 1.8, 4); // Mínimo 4 horas
        cargas[2] = new CargaAplazable("Bomba de Agua y Riego", 1.5, 3);          // Mínimo 3 horas
        
        ultimoPrecio = 600.0;
        ultimosEmbalses = 65.0;
        ultimaHora = 8;
        alertaProduccion = false;
    }

    ~Negocio() {
        for(int i = 0; i < 3; i++) {
            delete cargas[i];
        }
    }

    void ingresarLecturaManual(int h, double p, double e) {
        ultimaHora = h;
        ultimoPrecio = p;
        ultimosEmbalses = e;
        cout << "\n[OK] Lectura actualizada con exito.\n";
    }

    void evaluarDecisionCargas() {
        bool horaPico = (ultimaHora >= 18 && ultimaHora <= 21); // Ejemplo: 6 PM a 9 PM es hora pico
        cout << "\n--- EVALUACION DE CARGAS (Hora: " << ultimaHora << ":00 | Precio: " << ultimoPrecio << " | Embalses: " << ultimosEmbalses << "%) ---\n";
        for(int i = 0; i < 3; i++) {
            bool estado = cargas[i]->debeEncender(ultimoPrecio, ultimosEmbalses, horaPico);
            cout << "- " << cargas[i]->getNombre() << " -> " << (estado ? "ENCENDIDA" : "APLAZADA / APAGADA") << "\n";
        }
    }

    void simularDiaCompleto() {
        medidor.reiniciar();
        for(int i = 0; i < 3; i++) {
            cargas[i]->reiniciarHoras();
        }
        alertaProduccion = false;

        cout << "\n--- INICIANDO SIMULACION DE 24 HORAS ---\n";
        srand(time(0));

        for (int hora = 0; hora < HORAS_DIA; hora++) {
            // CONTINUE: 1 de cada 20 horas falla la lectura del precio (falla simulada en hora 10 por ejemplo, o aleatoria)
            if (hora == 10) { 
                cout << "[AVISO HORA " << hora << "] Fallo en lectura de precio. Solo opera carga critica.\n";
                // Sumamos consumo solo de la carga crítica
                medidor.registrarConsumo(cargas[0]->getPotencia(), ultimoPrecio, true);
                medidor.registrarConsumo(cargas[0]->getPotencia(), ultimoPrecio, false);
                cargas[0]->debeEncender(ultimoPrecio, ultimosEmbalses, false);
                continue; // Salta a la siguiente hora
            }

            double precioHora = mercado.generarPrecio();
            double embalsesHora = mercado.generarEmbalses();
            bool horaPico = (hora >= 18 && horaPicoCheck(hora));

            // BREAK: Si precio supera 1500 y embalses < 40%, emergencia de racionamiento
            if (precioHora > PRECIO_RACIONAMIENTO && embalsesHora < NIVEL_EMBALSE_ALERTA) {
                cout << "\n[EMERGENCIA] Racionamiento declarado en la hora " << hora << "! Precio: " << precioHora << " | Embalses: " << embalsesHora << "%\n";
                break; // Termina la simulación
            }

            // Evaluar cargas con gestor
            double potenciaTotalEncendida = 0;
            for(int i = 0; i < 3; i++) {
                bool encendida = cargas[i]->debeEncender(precioHora, embalsesHora, hora >= 18);
                if(encendida) potenciaTotalEncendida += cargas[i]->getPotencia();
            }

            // Regla 6: Si la potencia encendida supera la contratada, apagar la carga aplazable de mayor potencia y mostrar alerta
            if (potenciaTotalEncendida > POTENCIA_CONTRATADA) {
                cout << "[ALERTA HORA " << hora << "] Exceso de potencia contratada (" << potenciaTotalEncendida << " kW). Apagando carga aplazable mayor.\n";
                // Apagamos la carga 1 (máquina ordeñadora por ejemplo)
                ((CargaAplazable*)cargas[1])->apagarForzoso();
            }

            // Registrar consumo con gestor y sin gestor (sin gestor las 3 prendidas todo el tiempo)
            double potenciaConGestor = 0;
            for(int i = 0; i < 3; i++) {
                if(cargas[i]->estaEncendida()) potenciaConGestor += cargas[i]->getPotencia();
            }
            medidor.registrarConsumo(potenciaConGestor, precioHora, true);

            double potenciaSinGestor = cargas[0]->getPotencia() + cargas[1]->getPotencia() + cargas[2]->getPotencia();
            medidor.registrarConsumo(potenciaSinGestor, precioHora, false);
        }

        // Verificar horas mínimas de las aplazables
        for(int i = 1; i < 3; i++) {
            CargaAplazable* cap = (CargaAplazable*)cargas[i];
            if(cap->getHorasUso() < cap->getHorasMinimas()) {
                alertaProduccion = true;
            }
        }

        cout << "[OK] Simulación de 24 horas completada con exito.\n";
    }

    bool horaPicoCheck(int h) {
        return (h >= 18 && h <= 21);
    }

    void verResumen() {
        cout << "\n========================================\n";
        cout << "        RESUMEN DE LA SIMULACION        \n";
        cout << "========================================\n";
        for(int i = 0; i < 3; i++) {
            cout << "- " << cargas[i]->getNombre() << " | Horas de uso: " << cargas[i]->getHorasUso() << " h\n";
        }
        double costoG = medidor.getCostoGestor();
        double costoSG = medidor.getCostoSinGestor();
        double ahorroPesos = costoSG - costoG;
        double ahorroPorcentaje = (costoSG > 0) ? (ahorroPesos / costoSG) * 100 : 0;

        cout << "\nCosto con Gestor WattControl: $" << costoG << " COP\n";
        cout << "Costo sin Gestor (Normal):    $" << costoSG << " COP\n";
        cout << "Ahorro obtenido:              $" << ahorroPesos << " COP (" << ahorroPorcentaje << "%)\n";

        if (alertaProduccion) {
            cout << "\n[ALERTA] produccion incompleta (No se cumplieron las horas mínimas de operación en equipos aplazables).\n";
        } else {
            cout << "\n[ESTADO] Produccion completa y exitosa.\n";
        }
        cout << "========================================\n";
    }
};

// ==========================================
// FUNCIÓN PRINCIPAL (MENU CON VALIDACION)
// ==========================================
int main() {
    Negocio negocio;
    int opcion;
    double precio, embalses;
    int hora;

    do {
        cout << "\n===== WATTCONTROL: FINCA LECHERA =====" << endl;
        cout << "1. Ingresar lectura manual (hora, precio, embalses)" << endl;
        cout << "2. Evaluar decision de cada carga" << endl;
        cout << "3. Simular un dia completo (24 h)" << endl;
        cout << "4. Ver resumen de la ultima simulacion" << endl;
        cout << "0. Salir" << endl;
        cout << "Opcion: ";
        cin >> opcion;

        // Validación con do-while de la opción del menú (por seguridad básica)
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            opcion = -1;
        }

        switch (opcion) {
            case 1: {
                // Validación con do-while para hora (0 a 23)
                do {
                    cout << "Ingrese la hora (0 - 23): ";
                    cin >> hora;
                } while (hora < 0 || hora > 23);

                // Validación con do-while para precio (> 0)
                do {
                    cout << "Ingrese el precio en COP/kWh (> 0): ";
                    cin >> precio;
                } while (precio <= 0);

                // Validación con do-while para embalses (0 a 100)
                do {
                    cout << "Ingrese el nivel de embalses % (0 - 100): ";
                    cin >> embalses;
                } while (embalses < 0 || embalses > 100);

                negocio.ingresarLecturaManual(hora, precio, embalses);
                break;
            }
            case 2:
                negocio.evaluarDecisionCargas();
                break;
            case 3:
                negocio.simularDiaCompleto();
                break;
            case 4:
                negocio.verResumen();
                break;
            case 0:
                cout << "Saliendo del sistema WattControl... Hasta luego!\n";
                break;
            default:
                cout << "Opcion no valida. Intente de nuevo.\n";
        }
    } while (opcion != 0);

    return 0;
}
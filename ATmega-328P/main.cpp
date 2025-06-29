#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>

//Posicion de la gallina y variable nivel
uint8_t gallina_x = 3;
uint8_t gallina_y = 7;
uint8_t nivel = 1;

uint8_t obstaculos[8];
int8_t direcciones[8];

// Matriz de L1
uint8_t L1[8] = {
    0x00, 0x44, 0x44, 0x44, 0x44, 0x44, 0x64, 0x00
};

// Matriz de L2
uint8_t L2[8] = {
    0x00, 0x4E, 0x42, 0x4E, 0x48, 0x48, 0x6E, 0x00
};

// Matriz de L3
uint8_t L3[8] = {
    0x00, 0x4E, 0x42, 0x4E, 0x42, 0x42, 0x6E, 0x00
};

//Muestra L1, L2 y L3. Esto se lo agrega antes de cada nivel en la funcion Reinicio()
void mostrar_letra(uint8_t letra[8], uint16_t duracion_ms) {

    uint16_t repeticiones = duracion_ms / 8;  // Duración total / tiempo por ciclo
    uint8_t desplazamiento_max = 8;            // Mover 8 veces para desplazar toda la letra fuera

    for (uint8_t d = 0; d < desplazamiento_max; d++) { 
        for (uint16_t r = 0; r < repeticiones / desplazamiento_max; r++) {
            for (uint8_t j = 0; j < 8; j++) {
                PORTD = 1 << j;
                // Desplaza los bits de la fila j hacia la derecha d veces (efecto derecha a izquierda)
                PORTB = ~(letra[j] >> d);
                _delay_ms(1);
            }
        }
    }
}

//Detectar la señal de los botones para mover la gallina con antirebote
void leer_botones() {
    static uint8_t last_state = 0x0F;
    uint8_t current = PINC & 0x0F;

    if (current != last_state) {
        _delay_ms(10); // rebote
        current = PINC & 0x0F;

        if (!(current & (1 << PC0)) && gallina_y > 0) gallina_y--;
        if (!(current & (1 << PC1)) && gallina_y < 7) gallina_y++;
        if (!(current & (1 << PC2)) && gallina_x > 0) gallina_x--;
        if (!(current & (1 << PC3)) && gallina_x < 7) gallina_x++;

        last_state = current;
    }
}

//mueve los los obstaculos, de abajo a arriba(La matriz esta en horizontal) 
void mover_obstaculos() {
    for (int i = 0; i < 8; i++) {
        if (direcciones[i] == 1)
            obstaculos[i] = (obstaculos[i] << 1) | (obstaculos[i] >> 7);
        else if (direcciones[i] == -1)
            obstaculos[i] = (obstaculos[i] >> 1) | (obstaculos[i] << 7);
    }
}

// Detecta la colision la pocision de la gallina con las de los obstaculos
uint8_t colision() {
    return (obstaculos[gallina_y] & (1 << gallina_x));
}

// Muestra los obstaculo
void mostrar() {
    for (uint8_t j = 0; j < 8; j++) {
        PORTD = 1 << j;
        uint8_t fila = obstaculos[j];
        if (j == gallina_y) fila |= (1 << gallina_x);
        PORTB = ~fila;
        _delay_ms(0.5);
    }
}

//Posicion de cada obstaculo y direccion de su movimiento
void cargar_nivel(uint8_t n) {
    
    for (int i = 0; i < 8; i++) {
        obstaculos[i] = 0;
        direcciones[i] = 0;
    }

    if (n == 1) {
        obstaculos[2] = 0b10001010;
        obstaculos[4] = 0b11001100;
        obstaculos[6] = 0b11110001;
        direcciones[2] = 1;
        direcciones[4] = -1;
        direcciones[6] = 1;
    } else if (n == 2) {
        obstaculos[1] = 0b10001001;
        obstaculos[3] = 0b01100110;
        obstaculos[5] = 0b11100011;
        obstaculos[6] = 0b10100010;
        direcciones[1] = -1;
        direcciones[3] = 1;
        direcciones[5] = -1;
        direcciones[6] = 1;
    } else if (n == 3) {
        obstaculos[0] = 0b10000001;  
        obstaculos[1] = 0b01000010;  
        obstaculos[2] = 0b00011000;  
        obstaculos[3] = 0b00111100;  
        obstaculos[4] = 0b00000000;  
        obstaculos[5] = 0b11100111;  
        obstaculos[6] = 0b00011000;  
        direcciones[0] = 1;
        direcciones[1] = -1;
        direcciones[2] = 1;
        direcciones[3] = -1;
        direcciones[4] = 1;
        direcciones[5] = -1;
        direcciones[6] = 1;
    }
}

//Reiniciar nivel
void reiniciar() {
    gallina_x = 3;
    gallina_y = 7;
    if (nivel == 1) mostrar_letra(L1, 400);
    if (nivel == 2) mostrar_letra(L2, 400);
    if (nivel == 3) mostrar_letra(L3, 400);
    cargar_nivel(nivel);
}

// Funcion Victoria, enciende todos los leds y manda señal para sonido
void victoria_final() {
    for (int i = 0; i < 5; i++) {
        PORTD = 0xFF;
        PORTB = 0x00;
        _delay_ms(100);
        PORTD = 0x00;
        PORTB = 0xFF;
        _delay_ms(200);
    }
    nivel = 1;
    PORTC |= (1 << PC4);    // PC4 = 1
    PORTC &= ~(1 << PC5);   // PC5 = 0
    _delay_ms(100);
    PORTC &= ~((1 << PC4) | (1 << PC5));

    reiniciar();
}

// Controla el tiempo del nivel 3 
void controlar_tiempo_nivel3() {
    static uint16_t tiempo_nivel3 = 0;

    if (nivel == 3) {
        tiempo_nivel3++;
        if (tiempo_nivel3 >= 200) {  
            nivel = 1;
            reiniciar();             // Reinicia el juego
            tiempo_nivel3 = 0;
        }
    } else {
        tiempo_nivel3 = 0;  // Si sale del nivel 3, reinicia el contador
    }
}

uint16_t tiempo_nivel3 = 0;

int main() {
    DDRD = 0xFF; DDRB = 0xFF;
    DDRC = 0x00; PORTC = 0x0F;

    DDRC |= (1 << PC4) | (1 << PC5); // PC4 y PC5 como salidas
    PORTC &= ~((1 << PC4) | (1 << PC5)); // Inicialmente en 0

    // sonido de inicio
    PORTC |= (1 << PC4) | (1 << PC5);
    
    _delay_ms(10);           // Pulso

    PORTC &= ~((1 << PC4) | (1 << PC5)); // Apaga la señal  

    _delay_ms(200);
    reiniciar();

    uint8_t contador_obs = 0;
    
    while (1) {
        leer_botones(); 
    
    
        // Detecta la colision, manda senal de sonido de colision y rinicia nivel.
        if (colision()) {

            PORTC &= ~(1 << PC4);   // PC4 = 0 (RB1)
            PORTC |= (1 << PC5);    // PC5 = 1 (RB0)
            _delay_ms(10);         // Pulso breve
            PORTC &= ~((1 << PC4) | (1 << PC5)); // Apaga señal
    
            nivel = 1;
            reiniciar();
        }

        // Victoria por nivel
        if (gallina_y == 0) {

            PORTC |= (1 << PC4);    // PC4 = 1 (RB1)
            PORTC &= ~(1 << PC5);   // PC5 = 0 (RB0)
            _delay_ms(10);
            PORTC &= ~((1 << PC4) | (1 << PC5));

            if (nivel < 3) {
                nivel++;
                reiniciar();
            } else {
                victoria_final();
            }
        }
    
    
        for (int i = 0; i < 4; i++) {
        
            mostrar();

            controlar_tiempo_nivel3();
        }


        // Mueve los obstaculos

        contador_obs++;
        if (contador_obs >= 10) { 
            mover_obstaculos();
            contador_obs = 0;
        }
    }
}

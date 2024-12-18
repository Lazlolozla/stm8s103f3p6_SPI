/* Creado el mar 17 de dic del 2024 a las 16:37:36 CST
 * 
 * Proyecto: 003spi
 * Descripción: [ Este programa inicializa el SPI, selecciona el chip, escribe valores en el SPI, verifica si se produjo un error y deselecciona el chip. El bucle se ejecutará siempre.]
 */

#include <stdint.h>/* proporciona definiciones para tipos de datos enteros sin signo y con signo, como uint8_t, uint16_t, etc.*/

#define F_CPU 2000000UL/*frecuencia del reloj del microcontrolador en Hz. En este caso, se establece en 2 MHz.*/

/*Esta macro se utiliza para acceder a registros de hardware del microcontrolador. El parámetro mem_addr es la dirección de memoria del registro que se desea acceder. La macro devuelve un puntero a un uint8_t que se puede utilizar para leer o escribir en el registro.*/
#define _SFR_(mem_addr)     (*(volatile uint8_t *)(0x5000 + (mem_addr)))

/*Estas constantes definen las direcciones de memoria y los bits de los registros SPI del microcontrolador.*/
/* SPI */
#define SPI_CR1     _SFR_(0x200)
#define SPE         6
#define BR0         3
#define MSTR        2
#define SPI_CR2     _SFR_(0x201)
#define SSM         1
#define SSI         0
#define SPI_SR      _SFR_(0x203)
#define BSY         7
#define TXE         1
#define RXNE        0
#define SPI_DR      _SFR_(0x204)

/*Esta constante define el número del pin que se utiliza para la selección de chip.*/
/* Chip select */
#define CS_PIN      4

#define PC_ODR  _SFR_(0x0A) /*Registro de salida del puerto C. Se utiliza para establecer el valor (HIGH o LOW) de salida */
#define PC_DDR  _SFR_(0x0C)/*El registro de dirección del puerto C. Se utiliza para configurar los pines como entradas o salidas.*/
#define PC_CR1   _SFR_(0x0D)/*El registro de control 1 del puerto C. Se utiliza para configurar las características de los pines, como la velocidad de salida y la configuración de los pines como entradas o salidas.*/


/*
 * SPI pinout:
 * SCK  -> PC5
 * MOSI -> PC6
 * MISO -> PC7
 * CS   -> PC4
 */

// Definir un valor de tiempo de espera en ciclos de reloj para las operaciones SPI
#define SPI_TIMEOUT 10000

/*Esta función inicializa el pin de selección de chip como salida y configura el maestro SPI para operar a 500 kHz.*/
void SPI_init() {
    // Inicializar el pin CS como salida
    PC_DDR |= (1 << CS_PIN);
    PC_CR1 |= (1 << CS_PIN);
    PC_ODR |= (1 << CS_PIN);

    // Inicializar el maestro SPI a 500kHz
    SPI_CR2 = (1 << SSM) | (1 << SSI);
    SPI_CR1 = (1 << MSTR) | (1 << SPE) | (1 << BR0);
}

/*Esta función escribe un byte en el registro de datos del SPI y espera a que el búfer de transmisión esté vacío. Si se produce un tiempo de espera, la función devuelve un error.*/
uint8_t SPI_write(uint8_t data) {
    SPI_DR = data;

    // Esperar a que el búfer de transmisión esté vacío con tiempo de espera
    uint16_t timeout = SPI_TIMEOUT;
    while (!(SPI_SR & (1 << TXE)) && timeout--) {
        // Si se produce un tiempo de espera, manejar el error
        if (timeout == 0) {
            // Manejo de errores: Manejar el tiempo de espera de la comunicación SPI
            return 0xFF; // Indicar error
        }
    }

    return 0; // Éxito
}

/*Esta función escribe un byte de relleno en el registro de datos del SPI y espera a que el búfer de recepción esté lleno. Si se produce un tiempo de espera, la función devuelve un error.*/
uint8_t SPI_read() {
    SPI_write(0xFF);

    // Esperar a que el búfer de recepción esté lleno con tiempo de espera
    uint16_t timeout = SPI_TIMEOUT;
    while (!(SPI_SR & (1 << RXNE)) && timeout--) {
        // Si se produce un tiempo de espera, manejar el error
        if (timeout == 0) {
            // Manejo de errores: Manejar el tiempo de espera de la comunicación SPI
            return 0xFF; // Indicar error
        }
    }

    return SPI_DR;
}

/*Esta función selecciona el chip estableciendo el pin de selección de chip en alto.*/
void chip_select() {
	while((SPI_SR & (1<< BSY)));
    PC_ODR &= ~(1 << CS_PIN);
}

/* La función  espera a que el SPI esté disponible y luego selecciona el chip esclavo estableciendo el pin de selección de chip en bajo.

Es importante destacar que la función  es utilizada para seleccionar el chip esclavo antes de realizar una transferencia de datos. Después de la transferencia de datos, se debe deseleccionar el chip esclavo utilizando la función chip_deselect() para evitar conflictos con otros dispositivos en el bus SPI..*/
void chip_deselect() {
    PC_ODR |= (1 << CS_PIN);
}

void main(void){
    SPI_init();/* Inicializa el SPI llamando a la función.*/

    while(1){
        chip_select();/*Selecciona el chip*/
        for (uint8_t i = 0xAA; i < 0xFA; i += 0x10) {/*Comienza un bucle que se ejecutará desde 0xAA hasta 0xFA con incrementos de 0x10.*/
            if (SPI_write(i) != 0) {/* Escribe el valor de i en el SPI  y verifica si se produjo un error. Si se produjo un error, se ejecutará el código dentro del if.*/
                // Manejar error de escritura SPI
            }
        }
        chip_deselect();/* Deselecciona el chip*/
    }
}


#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"
#include <stdio.h>

#define I2C_PORT i2c0
#define I2C_ADDR 0x20

#define SDA_PIN 0
#define SCL_PIN 1

#define NOTIFY_PIN 13

#define SPI_PORT spi1
#define MOSI_PIN 15
#define SCK_PIN 14
#define OE_PIN 29
#define LE_PIN 26

#define NUM_COLS 5
#define NUM_ROWS 3
#define DEBOUNCE_COUNT 5   // ~5ms if scan loop ~1ms

volatile uint8_t last_button_code = 0;
volatile bool button_pending = false;
volatile bool i2c_read_request = false;

const uint8_t col_pins[NUM_COLS] = {2, 3, 4, 5, 6};
const uint8_t row_pins[NUM_ROWS] = {7, 8, 9};

uint8_t debounce[NUM_ROWS][NUM_COLS] = {0};
bool key_state[NUM_ROWS][NUM_COLS] = {0};

void setup_spi() {
    spi_init(SPI_PORT, 500 * 1000);  // Set SPI clock speed to 500kHz
    gpio_set_function(MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SCK_PIN, GPIO_FUNC_SPI);
    gpio_init(OE_PIN);
    gpio_set_dir(OE_PIN, GPIO_OUT);
    gpio_init(LE_PIN);
    gpio_set_dir(LE_PIN, GPIO_OUT);
}

void send_data(uint8_t data1, uint8_t data2) {
    uint8_t buffer[2] = { data1, data2 }; 
    gpio_put(OE_PIN, 1); 

    gpio_put(LE_PIN, 0);  // Latch data
    spi_write_blocking(SPI_PORT, buffer, 2);  // Send 2 bytes over SPI
    gpio_put(LE_PIN, 1);  // Latch data after sending
    sleep_ms(1); 

    gpio_put(OE_PIN, 0);  // Enable output (after latching)
    sleep_ms(1);  // Small delay to allow time for the output to stabilize
}

void i2c_slave_handler(void) {
    i2c_hw_t *hw = i2c_get_hw(I2C_PORT);
    uint32_t intr_stat = hw->intr_stat;

    // --- CASE 1: MASTER READING FROM PICO ---
    // 1. Handle Read Request (Master wants a byte)
    if (intr_stat & I2C_IC_INTR_STAT_R_RD_REQ_BITS) {
        // Clear the interrupt by reading the CLR_RD_REQ register
        (void)hw->clr_rd_req; 
        
        // Push data to the FIFO
        hw->data_cmd = (uint32_t)last_button_code;
    }

    // 2. Handle Stop Condition
    if (intr_stat & I2C_IC_INTR_STAT_R_STOP_DET_BITS) {
        (void)hw->clr_stop_det;
    }

    // --- CASE 2: MASTER WRITING TO PICO ---
    if (intr_stat & I2C_IC_INTR_STAT_R_RX_FULL_BITS) {
        // Read the byte sent by the Master
        uint8_t received_byte = (uint8_t)hw->data_cmd;
        
        // Example logic: Do something with the received data
        // If you receive 2 bytes, this block will fire twice.
        printf("Received from Pi: 0x%02X\n", received_byte);
    }
}


void matrix_init(void)
{
    // Init columns as outputs (default HIGH)
    for (int c = 0; c < NUM_COLS; c++) {
        gpio_init(col_pins[c]);
        gpio_set_dir(col_pins[c], GPIO_OUT);
        gpio_put(col_pins[c], 1);
    }

    // Init rows as inputs with pull-ups
    for (int r = 0; r < NUM_ROWS; r++) {
        gpio_init(row_pins[r]);
        gpio_set_dir(row_pins[r], GPIO_IN);
        gpio_pull_up(row_pins[r]);
    }
}

/* ---------- MATRIX SCAN ---------- */
void matrix_scan(void) {
    for (int c = 0; c < NUM_COLS; c++) {
        gpio_put(col_pins[c], 0);
        sleep_us(5);

        for (int r = 0; r < NUM_ROWS; r++) {
            bool pressed = (gpio_get(row_pins[r]) == 0);

            if (pressed) {
                if (debounce[r][c] < DEBOUNCE_COUNT)
                    debounce[r][c]++;
                else
                    key_state[r][c] = true;
            } else {
                if (key_state[r][c]) {
                    // RELEASE detected
                    last_button_code = (r << 4) | c;
                    button_pending = true;

                    gpio_put(NOTIFY_PIN, 0);
                    sleep_ms(1);
                    gpio_put(NOTIFY_PIN, 1);
                    printf("read req %x\n", last_button_code);

                    key_state[r][c] = false;
                }
                debounce[r][c] = 0;
            }
        }

        gpio_put(col_pins[c], 1);
    }
}

int main() {
    stdio_init_all();
    // Wait for USB connection (optional but helpful for debugging)
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("USB Connected!\n");
    sleep_ms(1000); 

    printf("Step 1: GPIO Init\n");
    gpio_init(NOTIFY_PIN);
    gpio_set_dir(NOTIFY_PIN, GPIO_OUT);
    gpio_put(NOTIFY_PIN, 1);

    printf("Step 2: SPI/Matrix Init\n");
    setup_spi();
    matrix_init();

    printf("Step 3: I2C Init\n");
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    
    // Check if the bus is pulled high (should be)
    if (gpio_get(SDA_PIN) == 0 || gpio_get(SCL_PIN) == 0) {
        printf("WARNING: I2C Bus appears stuck LOW!\n");
    }

    i2c_set_slave_mode(I2C_PORT, true, I2C_ADDR);

    printf("Step 4: Enabling Interrupts\n");
    
    // Safety: Clear any pending hardware flags before enabling
    (void)i2c_get_hw(I2C_PORT)->clr_intr;

    // Set up the interrupt
    irq_set_exclusive_handler(I2C0_IRQ, i2c_slave_handler);
    
    // CRITICAL: Set the mask FIRST, then enable the IRQ
    i2c_get_hw(I2C_PORT)->intr_mask = (I2C_IC_INTR_MASK_M_RD_REQ_BITS | 
                                       I2C_IC_INTR_MASK_M_STOP_DET_BITS |
                                    I2C_IC_INTR_MASK_M_RX_FULL_BITS);
    
    irq_set_enabled(I2C0_IRQ, true);

    printf("Keyboard scanner active!\n");

    uint32_t value = 0x0001;
    bool flag = true;
    uint8_t incr = 0;

    while (true) {
        matrix_scan();
        sleep_ms(1);

        if(incr == 100)
        {
            //while (1) {
                send_data((uint8_t)((value >> 8) & 0xFF), (uint8_t)(value & 0xFF));  // Example: Send two bytes to control LEDs (all LEDs on for the first byte)

                //sleep_ms(50);

                if (flag)
                    value = value << 1;
                else
                    value = value >> 1;

                if (value == 0x4000 || value == 0x0001) {
                    flag = !flag;
                }
            //}
            incr = 0;
        }
        incr++;
        
    }

    return 0;
}
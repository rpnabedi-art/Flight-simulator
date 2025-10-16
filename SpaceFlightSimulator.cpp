#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

// OLED (SSD1306) I2C functions (simplified)
#define I2C_ADDRESS 0x3C
void I2C_init(void);
void I2C_start(void);
void I2C_write(uint8_t data);
void I2C_stop(void);
void OLED_init(void);
void OLED_clear(void);
void OLED_draw_pixel(uint8_t x, uint8_t y);
void OLED_update(void);

// Game objects
#define MAX_STARS 10
#define MAX_ASTEROIDS 5
#define MAX_LASERS 3

typedef struct {
    int8_t x, y;     // Position
    int8_t vx, vy;   // Velocity
    uint8_t active;  // Is object active?
} Object;

Object spaceship;              // Spaceship
Object stars[MAX_STARS];       // Collectible stars
Object asteroids[MAX_ASTEROIDS]; // Obstacles
Object lasers[MAX_LASERS];     // Lasers
uint16_t score;                // Player score
uint8_t game_over;             // Game state

// ADC for joystick
void ADC_init(void);
uint16_t ADC_read(uint8_t channel);

// PWM for buzzer
void PWM_init(void);
void buzzer_sound(uint16_t freq, uint16_t duration);

// Button inputs
#define BUTTON_SHOOT PD2
#define BUTTON_RESET PD4
void buttons_init(void);

// Timer for game loop
void Timer_init(void);

// Game functions
void init_game(void);
void update_game(void);
void draw_game(void);
void check_collisions(void);

int main(void) {
    // Initialize hardware
    ADC_init();
    I2C_init();
    OLED_init();
    PWM_init();
    buttons_init();
    Timer_init();
    
    sei(); // Enable interrupts
    
    // Start game
    init_game();
    
    while (1) {
        if (!game_over) {
            update_game();
            draw_game();
            check_collisions();
        } else {
            OLED_clear();
            char score_str[16];
            sprintf(score_str, "Game Over! %d", score);
            // Simplified: Display text (requires font library)
            _delay_ms(2000);
            if (!(PIND & (1 << BUTTON_RESET))) {
                init_game(); // Restart game
            }
        }
    }
}

// I2C Functions (simplified for SSD1306)
void I2C_init(void) {
    TWSR = 0;
    TWBR = ((F_CPU / 100000UL) - 16) / 2; // 100kHz I2C
}

void I2C_start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

void I2C_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

void I2C_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void OLED_init(void) {
    I2C_start();
    I2C_write(I2C_ADDRESS << 1);
    I2C_write(0x00); // Command mode
    // Initialization commands for SSD1306
    uint8_t init_cmds[] = {0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12, 0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF};
    for (uint8_t i = 0; i < sizeof(init_cmds); i++) {
        I2C_write(init_cmds[i]);
    }
    I2C_stop();
}

void OLED_clear(void) {
    I2C_start();
    I2C_write(I2C_ADDRESS << 1);
    I2C_write(0x40); // Data mode
    for (uint16_t i = 0; i < 128 * 8; i++) {
        I2C_write(0x00);
    }
    I2C_stop();
}

void OLED_draw_pixel(uint8_t x, uint8_t y) {
    if (x >= 128 || y >= 64) return;
    I2C_start();
    I2C_write(I2C_ADDRESS << 1);
    I2C_write(0x00); // Command mode
    I2C_write(0x21); I2C_write(x); I2C_write(x); // Set column
    I2C_write(0x22); I2C_write(y / 8); I2C_write(y / 8); // Set page
    I2C_stop();
    
    I2C_start();
    I2C_write(I2C_ADDRESS << 1);
    I2C_write(0x40); // Data mode
    I2C_write(1 << (y % 8));
    I2C_stop();
}

void OLED_update(void) {
    // Simplified: In real implementation, use a frame buffer
}

// ADC for joystick
void ADC_init(void) {
    ADMUX = (1 << REFS0); // Vref = AVcc
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC, prescaler 128
}

uint16_t ADC_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

// PWM for buzzer
void PWM_init(void) {
    DDRD |= (1 << PD3); // Buzzer on OC2B
    TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20); // Fast PWM
    TCCR2B = (1 << CS22); // Prescaler 64
}

void buzzer_sound(uint16_t freq, uint16_t duration) {
    uint8_t prescaler = 64;
    OCR2B = (F_CPU / (2UL * prescaler * freq)) - 1;
    _delay_ms(duration);
    OCR2B = 0; // Turn off
}

// Button inputs
void buttons_init(void) {
    DDRD &= ~((1 << BUTTON_SHOOT) | (1 << BUTTON_RESET)); // Inputs
    PORTD |= (1 << BUTTON_SHOOT) | (1 << BUTTON_RESET); // Pull-ups
}

// Timer for 60 FPS game loop
volatile uint8_t game_tick = 0;
void Timer_init(void) {
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // CTC, prescaler 64
    OCR1A = (F_CPU / 64 / 60) - 1; // ~60Hz
    TIMSK1 = (1 << OCIE1A); // Enable compare interrupt
}

ISR(TIMER1_COMPA_vect) {
    game_tick = 1;
}

// Game logic
void init_game(void) {
    score = 0;
    game_over = 0;
    
    // Initialize spaceship
    spaceship.x = 20;
    spaceship.y = 32;
    spaceship.vx = 0;
    spaceship.vy = 0;
    spaceship.active = 1;
    
    // Initialize stars
    for (uint8_t i = 0; i < MAX_STARS; i++) {
        stars[i].x = 50 + (i * 10);
        stars[i].y = (i * 13) % 64;
        stars[i].vx = -1;
        stars[i].active = 1;
    }
    
    // Initialize asteroids
    for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
        asteroids[i].x = 100 + (i * 20);
        asteroids[i].y = (i * 17) % 64;
        asteroids[i].vx = -2 - (i % 3);
        asteroids[i].active = 1;
    }
    
    // Initialize lasers
    for (uint8_t i = 0; i < MAX_LASERS; i++) {
        lasers[i].active = 0;
    }
    
    OLED_clear();
}

void update_game(void) {
    if (!game_tick) return;
    game_tick = 0;
    
    // Read joystick (ADC0 for X, ADC1 for Y)
    int16_t joy_x = ADC_read(0);
    int16_t joy_y = ADC_read(1);
    
    // Update spaceship velocity (simple physics)
    spaceship.vx = (joy_x - 512) / 100; // Scale joystick input
    spaceship.vy = (joy_y - 512) / 100;
    
    // Update position
    spaceship.x += spaceship.vx;
    spaceship.y += spaceship.vy;
    
    // Boundary check
    if (spaceship.x < 0) spaceship.x = 0;
    if (spaceship.x > 120) spaceship.x = 120;
    if (spaceship.y < 0) spaceship.y = 0;
    if (spaceship.y > 60) spaceship.y = 60;
    
    // Update stars
    for (uint8_t i = 0; i < MAX_STARS; i++) {
        if (stars[i].active) {
            stars[i].x += stars[i].vx;
            if (stars[i].x < 0) {
                stars[i].x = 128;
                stars[i].y = (stars[i].y + 13) % 64;
            }
        }
    }
    
    // Update asteroids
    for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].active) {
            asteroids[i].x += asteroids[i].vx;
            if (asteroids[i].x < 0) {
                asteroids[i].x = 128;
                asteroids[i].y = (asteroids[i].y + 17) % 64;
                asteroids[i].vx = -2 - (i % 3);
            }
        }
    }
    
    // Update lasers
    for (uint8_t i = 0; i < MAX_LASERS; i++) {
        if (lasers[i].active) {
            lasers[i].x += lasers[i].vx;
            if (lasers[i].x > 128) lasers[i].active = 0;
        }
    }
    
    // Check shoot button
    if (!(PIND & (1 << BUTTON_SHOOT))) {
        for (uint8_t i = 0; i < MAX_LASERS; i++) {
            if (!lasers[i].active) {
                lasers[i].x = spaceship.x + 5;
                lasers[i].y = spaceship.y;
                lasers[i].vx = 5;
                lasers[i].active = 1;
                buzzer_sound(1000, 50); // Shoot sound
                break;
            }
        }
    }
}

void draw_game(void) {
    OLED_clear();
    
    // Draw spaceship (simple triangle)
    if (spaceship.active) {
        OLED_draw_pixel(spaceship.x, spaceship.y);
        OLED_draw_pixel(spaceship.x + 1, spaceship.y - 1);
        OLED_draw_pixel(spaceship.x + 1, spaceship.y + 1);
    }
    
    // Draw stars
    for (uint8_t i = 0; i < MAX_STARS; i++) {
        if (stars[i].active) {
            OLED_draw_pixel(stars[i].x, stars[i].y);
        }
    }
    
    // Draw asteroids (simple square)
    for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].active) {
            OLED_draw_pixel(asteroids[i].x, asteroids[i].y);
            OLED_draw_pixel(asteroids[i].x + 1, asteroids[i].y);
            OLED_draw_pixel(asteroids[i].x, asteroids[i].y + 1);
            OLED_draw_pixel(asteroids[i].x + 1, asteroids[i].y + 1);
        }
    }
    
    // Draw lasers
    for (uint8_t i = 0; i < MAX_LASERS; i++) {
        if (lasers[i].active) {
            OLED_draw_pixel(lasers[i].x, lasers[i].y);
        }
    }
    
    OLED_update();
}

void check_collisions(void) {
    // Check spaceship vs asteroids
    for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].active) {
            if (abs(spaceship.x - asteroids[i].x) < 3 && abs(spaceship.y - asteroids[i].y) < 3) {
                game_over = 1;
                buzzer_sound(500, 200); // Explosion sound
            }
        }
    }
    
    // Check lasers vs asteroids
    for (uint8_t i = 0; i < MAX_LASERS; i++) {
        if (lasers[i].active) {
            for (uint8_t j = 0; j < MAX_ASTEROIDS; j++) {
                if (asteroids[j].active) {
                    if (abs(lasers[i].x - asteroids[j].x) < 3 && abs(lasers[i].y - asteroids[j].y) < 3) {
                        asteroids[j].active = 0;
                        lasers[i].active = 0;
                        score += 10;
                        buzzer_sound(800, 100); // Hit sound
                    }
                }
            }
        }
    }
    
    // Check spaceship vs stars
    for (uint8_t i = 0; i < MAX_STARS; i++) {
        if (stars[i].active) {
            if (abs(spaceship.x - stars[i].x) < 3 && abs(spaceship.y - stars[i].y) < 3) {
                stars[i].active = 0;
                score += 5;
                buzzer_sound(1200, 50); // Collect sound
            }
        }
    }
}
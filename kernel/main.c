/*
 * main.c - Punto de entrada del kernel
 * Sistema Operativo Personalizado basado en ReactOS
 *
 * Este archivo contiene el punto de entrada principal del kernel.
 * El kernel se ejecuta después de que el bootloader transfiere el control.
 */

// Definiciones básicas
#define NULL ((void*)0)
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// Colores VGA
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_BLUE 1
#define VGA_COLOR_GREEN 2
#define VGA_COLOR_CYAN 3
#define VGA_COLOR_RED 4
#define VGA_COLOR_MAGENTA 5
#define VGA_COLOR_BROWN 6
#define VGA_COLOR_LIGHT_GREY 7
#define VGA_COLOR_DARK_GREY 8
#define VGA_COLOR_LIGHT_BLUE 9
#define VGA_COLOR_LIGHT_GREEN 10
#define VGA_COLOR_LIGHT_CYAN 11
#define VGA_COLOR_LIGHT_RED 12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_YELLOW 14
#define VGA_COLOR_WHITE 15

// Variables globales para la terminal
static unsigned short* video_memory = (unsigned short*)VGA_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;
static unsigned char current_color;

/*
 * Crea un byte de color VGA a partir de colores de fondo y primer plano
 */
static inline unsigned char vga_entry_color(unsigned char fg, unsigned char bg) {
    return fg | (bg << 4);
}

/*
 * Crea una entrada VGA a partir de un carácter y un color
 */
static inline unsigned short vga_entry(unsigned char c, unsigned char color) {
    return (unsigned short)c | ((unsigned short)color << 8);
}

/*
 * Limpia la pantalla
 */
void clear_screen(void) {
    unsigned char color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int index = y * VGA_WIDTH + x;
            video_memory[index] = vga_entry(' ', color);
        }
    }
    
    cursor_x = 0;
    cursor_y = 0;
}

/*
 * Imprime un carácter en la posición actual del cursor
 */
void putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else {
        const int index = cursor_y * VGA_WIDTH + cursor_x;
        video_memory[index] = vga_entry(c, current_color);
        cursor_x++;
        
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }
    
    // Scroll si es necesario
    if (cursor_y >= VGA_HEIGHT) {
        cursor_y = VGA_HEIGHT - 1;
        // TODO: Implementar scroll
    }
}

/*
 * Imprime una cadena de texto
 */
void print(const char* str) {
    while (*str) {
        putchar(*str);
        str++;
    }
}

/*
 * Imprime una cadena de texto con un color específico
 */
void print_color(const char* str, unsigned char fg, unsigned char bg) {
    unsigned char old_color = current_color;
    current_color = vga_entry_color(fg, bg);
    print(str);
    current_color = old_color;
}

/*
 * kernel_main - Punto de entrada principal del kernel
 *
 * Esta función es llamada por el bootloader después de configurar
 * el entorno de ejecución básico.
 *
 * Responsabilidades:
 * - Inicializar la terminal/pantalla
 * - Mostrar información de arranque
 * - Inicializar subsistemas del kernel
 * - Transferir control al scheduler (futuro)
 */
void kernel_main(void) {
    // Inicializar la terminal
    current_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    clear_screen();
    
    // Mostrar banner de inicio
    print_color("==========================================\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    print_color("   Sistema Operativo Personalizado v0.1   \n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    print_color("   Basado en componentes de ReactOS      \n", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    print_color("==========================================\n", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    print("\n");
    
    // Información del sistema
    print("Iniciando kernel...\n");
    print_color("[OK]", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    print(" Terminal inicializada\n");
    
    print_color("[OK]", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    print(" Memoria de video configurada\n");
    
    print_color("[INFO]", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    print(" HAL no inicializado (pendiente)\n");
    
    print_color("[INFO]", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    print(" Drivers no cargados (pendiente)\n");
    
    print("\n");
    print("Kernel cargado exitosamente.\n");
    print("Sistema en espera...\n");
    
    // Loop infinito - el kernel está ejecutándose
    // En el futuro, aquí se transferirá el control al scheduler
    while (1) {
        // Halt CPU hasta la próxima interrupción
        __asm__ __volatile__("hlt");
    }
}

/*
 * Punto de entrada alternativo del kernel (para compatibilidad)
 */
void _start(void) {
    kernel_main();
}

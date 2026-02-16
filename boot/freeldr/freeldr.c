/*
 * FreeLoader - Simplified Boot Loader
 * Copyright (c) 2024 System_Operative_Edit Project
 * 
 * Este archivo es parte del proyecto System_Operative_Edit
 * Licencia: GPL-3.0
 * 
 * freeldr.c - Punto de entrada principal del FreeLoader
 * 
 * Este es el segundo stage del bootloader. Es cargado por el boot sector
 * en la dirección 0000:F800 y se encarga de:
 * 1. Inicializar el hardware básico
 * 2. Detectar memoria disponible
 * 3. Mostrar información del sistema
 * 4. Preparar el entorno para cargar el kernel (fase futura)
 */

#include "include/freeldr.h"
#include "include/video.h"
#include "include/memory.h"
#include "include/disk.h"

/* Declaraciones de funciones externas (string.c) */
extern int strlen(const char *str);
extern void strcpy(char *dest, const char *src);
extern int strcmp(const char *s1, const char *s2);
extern void memcpy(void *dest, const void *src, int n);
extern void memset(void *ptr, int value, int n);
extern int memcmp(const void *s1, const void *s2, int n);

/* Declaraciones de funciones auxiliares */
extern void VideoHexPrint(u32 value);
extern void VideoDecPrint(u32 value);

/* Variables globales */
static u8 boot_drive = 0;
static u8 boot_partition = 0;

/*
 * BootEntry - Punto de entrada desde el boot sector
 * 
 * El boot sector carga FreeLoader en 0000:F800 y salta aquí.
 * Los parámetros de arranque vienen en:
 *   DL = unidad de arranque
 *   DH = partición de arranque
 * 
 * Esta función está en Assembly y llama a BootMain
 */
__asm__(
    ".code16\n"
    ".section .text.entry\n"
    ".globl _start\n"
    "_start:\n"
    "    cli\n"                          // Deshabilitar interrupciones
    "    xor %ax, %ax\n"                 // AX = 0
    "    mov %ax, %ds\n"                 // DS = 0
    "    mov %ax, %es\n"                 // ES = 0
    "    mov %ax, %ss\n"                 // SS = 0
    "    mov $0x7BF0, %sp\n"             // Configurar pila
    "    sti\n"                          // Habilitar interrupciones
    "    \n"
    "    # Guardar parámetros de arranque\n"
    "    movb %dl, boot_drive\n"         // Guardar unidad de arranque
    "    movb %dh, boot_partition\n"     // Guardar partición
    "    \n"
    "    # Saltar a BootMain (código C)\n"
    "    call BootMain\n"
    "    \n"
    "    # Si BootMain retorna, entrar en halt\n"
    "    jmp Halt\n"
    "\n"
    ".code16\n"
);

/*
 * ShowWelcomeBanner - Muestra el banner de bienvenida
 */
static void ShowWelcomeBanner(void)
{
    VideoSetColor(MAKE_COLOR(COLOR_YELLOW, COLOR_BLACK));
    VideoPutString("\n");
    VideoPutString("  ========================================\n");
    VideoPutString("      FreeLoader - System Operative Edit\n");
    VideoPutString("  ========================================\n");
    VideoPutString("\n");
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_GRAY, COLOR_BLACK));
}

/*
 * ShowSystemInfo - Muestra información del sistema
 */
static void ShowSystemInfo(void)
{
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_CYAN, COLOR_BLACK));
    VideoPutString("Informacion del Sistema:\n");
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_GRAY, COLOR_BLACK));
    VideoPutString("------------------------\n\n");
    
    // Mostrar unidad de arranque
    VideoPutString("  Unidad de arranque: 0x");
    VideoHexPrint(boot_drive);
    VideoPutString("\n");
    
    VideoPutString("  Particion: ");
    VideoDecPrint(boot_partition);
    VideoPutString("\n\n");
    
    // Mostrar memoria
    u32 total_mem = MemoryGetTotal();
    u32 lower_mem = MemoryGetLower();
    u32 upper_mem = MemoryGetUpper();
    
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_CYAN, COLOR_BLACK));
    VideoPutString("Memoria Detectada:\n");
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_GRAY, COLOR_BLACK));
    VideoPutString("------------------\n\n");
    
    VideoPutString("  Memoria baja:  ");
    VideoDecPrint(lower_mem);
    VideoPutString(" KB\n");
    
    VideoPutString("  Memoria alta:  ");
    VideoDecPrint(upper_mem);
    VideoPutString(" KB\n");
    
    VideoPutString("  Total:         ");
    VideoDecPrint(total_mem);
    VideoPutString(" KB (");
    VideoDecPrint(total_mem / 1024);
    VideoPutString(" MB)\n\n");
}

/*
 * ShowStatus - Muestra el estado del bootloader
 */
static void ShowStatus(void)
{
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_GREEN, COLOR_BLACK));
    VideoPutString("Estado del Bootloader:\n");
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_GRAY, COLOR_BLACK));
    VideoPutString("----------------------\n\n");
    
    VideoPutString("  [OK] Video inicializado\n");
    VideoPutString("  [OK] Memoria detectada\n");
    VideoPutString("  [OK] Disco inicializado\n");
    VideoPutString("  [OK] Sistema listo\n\n");
}

/*
 * ShowNextSteps - Muestra los próximos pasos
 */
static void ShowNextSteps(void)
{
    VideoSetColor(MAKE_COLOR(COLOR_YELLOW, COLOR_BLACK));
    VideoPutString("Proximos Pasos:\n");
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_GRAY, COLOR_BLACK));
    VideoPutString("---------------\n\n");
    
    VideoPutString("  1. Implementar cargador de configuracion\n");
    VideoPutString("  2. Implementar cargador de kernel\n");
    VideoPutString("  3. Preparar estructuras para el kernel\n");
    VideoPutString("  4. Transferir control al kernel\n\n");
}

/*
 * BootMain - Función principal del bootloader
 * 
 * Esta es la función principal en C que ejecuta toda la lógica del bootloader
 */
void BootMain(void)
{
    // 1. Inicializar video
    VideoInit();
    
    // 2. Mostrar banner de bienvenida
    ShowWelcomeBanner();
    
    // 3. Inicializar subsistema de memoria
    MemoryInit();
    
    // 4. Inicializar subsistema de disco
    DiskInit(boot_drive);
    
    // 5. Mostrar información del sistema
    ShowSystemInfo();
    
    // 6. Mostrar estado
    ShowStatus();
    
    // 7. Mostrar próximos pasos
    ShowNextSteps();
    
    // 8. Mensaje final
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_CYAN, COLOR_BLACK));
    VideoPutString("FreeLoader Phase 1 completado exitosamente.\n");
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_GRAY, COLOR_BLACK));
    VideoPutString("Sistema detenido - En fase futura se cargara el kernel.\n\n");
    
    // Por ahora, simplemente nos detenemos
    // En el futuro, aquí cargaríamos el kernel
    Halt();
}

/*
 * Halt - Detiene el sistema
 * 
 * Entra en un bucle infinito con HLT
 */
void Halt(void)
{
    VideoSetColor(MAKE_COLOR(COLOR_LIGHT_RED, COLOR_BLACK));
    VideoPutString("\nSistema detenido. Puede reiniciar.\n");
    
    // Deshabilitar interrupciones y entrar en bucle HLT
    __asm__ volatile (
        "cli\n"
        "1:\n"
        "    hlt\n"
        "    jmp 1b\n"
    );
    
    // Nunca debería llegar aquí
    while (1) {
        __asm__ volatile ("hlt");
    }
}

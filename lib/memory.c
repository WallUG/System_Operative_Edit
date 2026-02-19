/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     Kernel Memory Management
 * COPYRIGHT:   Universidad de Guayaquil
 */

#include <types.h>

/*
 * HEAP_START debe estar DESPUES del fin del kernel image.
 * El kernel se carga en 0x00100000 (linker.ld). Sus secciones ocupan:
 *   .text   ~28KB  (EIPs del kernel llegan hasta ~0x107000)
 *   .rodata ~4KB
 *   .data   ~4KB
 *   .bss    ~310KB (incluye g_shadow[640*480] = 300KB)
 * Total kernel ~ 0x56000 bytes -> termina alrededor de 0x156000
 *
 * ANTES era 0x00100000 = MISMA DIRECCION que el inicio del kernel.
 * El primer malloc() sobreescribia la cabecera multiboot y el .text
 * del kernel con ceros -> comportamiento completamente indefinido.
 *
 * 0x00200000 (2MB) es seguro: deja >600KB de margen sobre el kernel
 * y coincide con el comentario de pmm.h ("0x200000-... frames libres").
 */
#define HEAP_SIZE  0x00600000  /* 6MB de heap del kernel */

/* _kernel_end es exportado por linker.ld como el primer byte libre
 * despues de todas las secciones del kernel (.text/.rodata/.data/.bss).
 * El heap empieza en el siguiente PAGE_ALIGN para que no haya overlap. */
extern uint32_t _kernel_end;
#define PAGE_ALIGN_UP(x) (((x) + 0xFFF) & ~0xFFFu)

static uint32_t heap_current = 0;   /* inicializado en heap_init() */
static uint32_t heap_end     = 0;

/* Debe llamarse UNA vez al inicio, antes del primer malloc() */
void heap_init(void)
{
    uint32_t start = PAGE_ALIGN_UP((uint32_t)&_kernel_end);
    /* Nunca empieza antes de 0x200000 por seguridad (evita solapar con kernel) */
    if (start < 0x00200000u) start = 0x00200000u;
    heap_current = start;
    heap_end     = start + HEAP_SIZE;
}

/**
 * malloc - Allocate memory
 * @size: Size in bytes to allocate
 * 
 * Returns: Pointer to allocated memory or NULL if failed
 */
void *malloc(size_t size)
{
    void *ptr;

    /* Si heap_init() no se llamo todavia, devolver NULL en lugar de
     * escribir en la direccion 0 (o en el kernel image). */
    if (heap_current == 0) return NULL;

    /* Align to 4 bytes */
    size = (size + 3) & ~3;
    
    /* Check if we have enough space */
    if (heap_current + size > heap_end) {
        return NULL;
    }
    
    ptr = (void *)heap_current;
    heap_current += size;
    
    return ptr;
}

/**
 * free - Free allocated memory
 * @ptr: Pointer to memory to free
 * 
 * Note: LIMITATION - This is a simple bump allocator implementation.
 * Memory is NOT actually freed or reused. This will lead to memory exhaustion
 * over time as allocations accumulate. A proper allocator (buddy system,
 * slab allocator, etc.) should be implemented for production use.
 * 
 * For Phase 1, this is acceptable since:
 * - Drivers are loaded once at boot
 * - Device objects persist for system lifetime
 * - Limited dynamic allocation occurs after initialization
 */
void free(void *ptr)
{
    /* Simple bump allocator - memory is never actually freed */
    /* TODO: Implement proper free list or buddy allocator */
    (void)ptr;
}

/**
 * memset - Fill memory with a constant byte
 * @ptr: Pointer to memory
 * @value: Value to set
 * @n: Number of bytes
 * 
 * Returns: Pointer to memory
 */
void *memset(void *ptr, int value, size_t n)
{
    unsigned char *p = (unsigned char *)ptr;
    unsigned char val = (unsigned char)value;
    
    for (size_t i = 0; i < n; i++) {
        p[i] = val;
    }
    
    return ptr;
}

/**
 * memcpy - Copy memory
 * @dest: Destination pointer
 * @src: Source pointer
 * @n: Number of bytes to copy
 * 
 * Returns: Destination pointer
 */
void *memcpy(void *dest, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

/**
 * memcmp - Compare memory
 * @s1: First memory block
 * @s2: Second memory block
 * @n: Number of bytes to compare
 * 
 * Returns: 0 if equal, <0 if s1 < s2, >0 if s1 > s2
 */
int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    
    return 0;
}

/**
 * strlen - Get string length
 * @str: String
 * 
 * Returns: Length of string
 */
size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

/**
 * strcmp - Compare strings
 * @s1: First string
 * @s2: Second string
 * 
 * Returns: 0 if equal, <0 if s1 < s2, >0 if s1 > s2
 */
int strcmp(const char *s1, const char *s2)
{
    size_t i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) {
            return s1[i] - s2[i];
        }
        i++;
    }
    return s1[i] - s2[i];
}

/**
 * strcpy - Copy string
 * @dest: Destination
 * @src: Source
 * 
 * Returns: Destination pointer
 */
char *strcpy(char *dest, const char *src)
{
    size_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

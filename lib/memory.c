/*
 * PROJECT:     System_Operative_Edit
 * LICENSE:     GPL-3.0
 * PURPOSE:     Kernel Memory Management
 * COPYRIGHT:   Universidad de Guayaquil
 */

#include <types.h>

/* Simple heap implementation */
#define HEAP_START 0x00100000  /* 1MB */
#define HEAP_SIZE  0x00F00000  /* 15MB */

static uint32_t heap_current = HEAP_START;
static uint32_t heap_end = HEAP_START + HEAP_SIZE;

/**
 * malloc - Allocate memory
 * @size: Size in bytes to allocate
 * 
 * Returns: Pointer to allocated memory or NULL if failed
 */
void *malloc(size_t size)
{
    void *ptr;
    
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
 * Note: This is a simple implementation that doesn't actually free memory
 * A real implementation would use a proper allocator (e.g., buddy system)
 */
void free(void *ptr)
{
    /* Simple implementation - just ignore free for now */
    /* In a real implementation, we would track allocations and reuse memory */
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

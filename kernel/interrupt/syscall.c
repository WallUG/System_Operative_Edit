#include "syscall.h"
#include "../mm/vmm.h"   /* para validación de punteros si lo necesitamos */
#include "../proc/process.h"
#include "../proc/scheduler.h"
#include "../drivers/video/vga/vga.h"    /* funciones VGA */
#include "../drivers/video/vga/vga_font.h" /* VgaDrawString */
#include "../drivers/input/ps2mouse.h" /* MOUSE_STATE */
#include <gui.h>           /* GUI_WINDOW, GUI_MOUSE_EVENT */
#include "../../include/libsys.h"  /* definiciones de SYS_MOUSE, etc. */
#include <types.h>

/* función de serial definida en idt.c */
extern void serial_puts(const char *s);

/* Tick counter para SYS_GET_TICK */
static volatile uint32_t g_ticks = 0;

uint32_t get_tick_count(void)
{
    return g_ticks;
}

/* Esta función es llamada por el handler de IRQ0 en cada tick. */
void syscall_tick_increment(void)
{
    g_ticks++;
    /* debug: escribir cada 256 ticks para verificar que el timer realmente
       dispara (no queremos spam cada tick). */
    if ((g_ticks & 0xFF) == 0) {
        serial_puts("[tick]\r\n");
    }
}

/* Validación sencilla de punteros de usuario. Acepta direcciones < 0x80000000. */
static int is_user_ptr(const void* p)
{
    return ((uint32_t)p < 0x80000000);
}

/* Copia bytes de un buffer de usuario a kernel; retorna 0 en éxito, -1 en fallo. */
static int copy_from_user(void* dest, const void* src, size_t len)
{
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < len; i++) {
        if (!is_user_ptr(s + i))
            return -1;
        d[i] = s[i];
    }
    return 0;
}

/* helper to print a 32-bit value in hex to serial */
void serial_print_hex(uint32_t v)
{
    char buf[11];
    buf[0] = '0'; buf[1] = 'x';
    static const char *hex = "0123456789ABCDEF";
    for (int i = 0; i < 8; i++) {
        buf[2 + i] = hex[(v >> ((7 - i) * 4)) & 0xF];
    }
    buf[10] = '\0';
    serial_puts(buf);
}

/* dump first few words from a stack pointer for debugging */
void dump_stack(uint32_t *sp, int count)
{
    for (int i = 0; i < count; i++) {
        serial_print_hex(sp[i]);
        serial_puts(" ");
    }
    serial_puts("\r\n");
}

/* Stub de syscall: preserva registros y invoca dispatcher. */
__attribute__((naked))
void syscall_entry(void)
{
    __asm__ volatile(
        "cli\n"
        /* save caller-saved registers (except eax) */
        "push %ebp\n"
        "mov %eax, %ebp\n"    /* store syscall number in ebp */
        "push %edi\n"
        "push %esi\n"
        "push %edx\n"
        "push %ecx\n"
        "push %ebx\n"

        /* syscall number is saved in ebp */

        /* save segments */
        "push %ds\n"
        "push %es\n"
        "push %fs\n"
        "push %gs\n"

        /* switch to kernel segments */
        "mov $0x10, %ax\n"
        "mov %ax, %ds\n"
        "mov %ax, %es\n"
        "mov %ax, %fs\n"
        "mov %ax, %gs\n"

        "/* push argumentos para cdecl: num,a,b,c,d,e */\n"
        "push %edi\n"   /* arg5 */
        "push %esi\n"   /* arg4 */
        "push %edx\n"   /* arg3 */
        "push %ecx\n"   /* arg2 */
        "push %ebx\n"   /* arg1 */
        "push %ebp\n"   /* syscall number saved earlier */

        "call syscall_dispatch\n"
        "add $24, %esp\n"  /* remove pushed args */

        /* restore segments */
        "pop %gs\n"
        "pop %fs\n"
        "pop %es\n"
        "pop %ds\n"

        /* restore registers */
        "pop %ebx\n"
        "pop %ecx\n"
        "pop %edx\n"
        "pop %esi\n"
        "pop %edi\n"
        "pop %ebp\n"

        /* debug: dump a few words from the current stack frame before
           we adjust for the interrupt return. This helps verify the
           return address that iret will pop. */
        "push $5\n"        /* count */
        "push %esp\n"      /* pointer to stack */
        "call dump_stack\n"
        "add $8, %esp\n"    /* skip SS+ESP from interrupt frame */
        "sti\n"
        "iret\n"
    );
}

uint32_t syscall_dispatch(uint32_t num, uint32_t a, uint32_t b,
                          uint32_t c, uint32_t d, uint32_t e)
{
    /* debug: imprimir llamada con argumentos (limitado para no spam) */
    {
        extern void serial_puts(const char*);
        char buf[128];
        int n = 0;
        buf[n++] = 'S'; buf[n++]='Y'; buf[n++]='S'; buf[n++]=':';
        buf[n++]='0'+((num/10)%10); buf[n++]='0'+(num%10);
        buf[n++]='(';
        /* simple hex for first two args */
        const char *hex = "0123456789ABCDEF";
        for (int i=0;i<2;i++){
            uint32_t v = (i==0?a:b);
            buf[n++]='0'; buf[n++]='x';
            for (int j=7;j>=0;j--) buf[n++]=hex[(v>>(j*4))&0xF];
            if (i==0) buf[n++]=',';
        }
        buf[n++]=')'; buf[n++]='\r'; buf[n++]='\n';
        buf[n]=0;
        serial_puts(buf);
    }
    uint32_t ret = 0;
    switch (num) {
    case SYS_EXIT:
        proc_exit(a);
        /* proc_exit no regresa */
        break;
    case SYS_YIELD:
        scheduler_yield();
        ret = 0;
        break;
    case SYS_DRAW_PIXEL:
        VgaPutPixel((INT)a, (INT)b, (UCHAR)c);
        ret = 0;
        break;
    case SYS_FILL_RECT:
        /* debug extra: print width/height */
        {
            serial_puts("FILL x="); serial_print_hex(a);
            serial_puts(" y="); serial_print_hex(b);
            serial_puts(" w="); serial_print_hex(c);
            serial_puts(" h="); serial_print_hex(d);
            serial_puts(" col="); serial_print_hex(e);
            serial_puts("\r\n");
        }
        VgaFillRect((INT)a, (INT)b, (INT)c, (INT)d, (UCHAR)e);
        ret = 0;
        break;
    case SYS_DRAW_STRING: {
        /* a=x, b=y, c=pointer, d=fg, e=bg */
        char buf[128];
        int i;
        if (!is_user_ptr((const void*)c)) {
            ret = (uint32_t)-1;
            break;
        }
        /* copiar caracter a caracter hasta null o saturacion */
        for (i = 0; i < (int)sizeof(buf)-1; i++) {
            const char *uc = (const char*)(c + i);
            if (!is_user_ptr(uc)) { ret = (uint32_t)-1; break; }
            buf[i] = *uc;  /* acceso directo - pagina debe ser PTE_USER */
            if (buf[i] == '\0') break;
        }
        if (ret != 0) break;
        buf[sizeof(buf)-1] = '\0';
        VgaDrawString((INT)a, (INT)b, buf, (UCHAR)d, (UCHAR)e);
        ret = 0;
        break;
    }
    case SYS_GET_TICK:
        ret = get_tick_count();
        break;
    case SYS_GET_MOUSE_STATE: {
        /* b = pointer to SYS_MOUSE */
        if (!is_user_ptr((const void*)b)) { ret = (uint32_t)-1; break; }
        SYS_MOUSE ms;
        {
            extern MOUSE_STATE* MouseGetState(void);
            MOUSE_STATE* cur = MouseGetState();
            if (cur) {
                ms.x = cur->x;
                ms.y = cur->y;
                ms.buttons = cur->buttons;
            } else {
                ms.x = ms.y = ms.buttons = 0;
            }
        }
        if (copy_from_user((void*)b, &ms, sizeof(ms)) != 0)
            ret = (uint32_t)-1;
        else
            ret = 0;
        break;
    }
    case SYS_GET_MOUSE_EVENT: {
        /* pop next event from GUI queue */
        GUI_MOUSE_EVENT evt;
        extern int GuiGetMouseEvent(GUI_MOUSE_EVENT*);
        if (GuiGetMouseEvent(&evt) != 0) {
            ret = (uint32_t)-1; /* no event */
            break;
        }
        if (!is_user_ptr((const void*)b)) { ret = (uint32_t)-1; break; }
        if (copy_from_user((void*)b, &evt, sizeof(evt)) != 0) {
            ret = (uint32_t)-1;
        } else {
            ret = 0;
        }
        break;
    }
    case SYS_SET_CURSOR_POS: {
        /* a=x, b=y */
        extern void CursorErase(int32_t, int32_t);
        extern void CursorDraw(int32_t, int32_t);
        static int prevx = -1, prevy = -1;
        if (prevx >= 0 && prevy >= 0) {
            CursorErase(prevx, prevy);
        }
        CursorDraw((int)a, (int)b);
        prevx = (int)a; prevy = (int)b;
        ret = 0;
        break;
    }
    case SYS_HIDE_CURSOR: {
        extern void CursorErase(int32_t, int32_t);
        /* simply erase at last known position */
        static int lastx = -1, lasty = -1;
        if (lastx >= 0 && lasty >= 0) CursorErase(lastx, lasty);
        ret = 0;
        break;
    }
    case SYS_SHOW_CURSOR: {
        /* nothing special, cursor will be drawn on next move */
        ret = 0;
        break;
    }
    case SYS_GUI_DRAW_DESKTOP: {
        extern void GuiDrawDesktop(void);
        GuiDrawDesktop();
        ret = 0;
        break;
    }
    case SYS_GUI_DRAW_TASKBAR: {
        extern void GuiDrawTaskbar(void);
        GuiDrawTaskbar();
        ret = 0;
        break;
    }
    case SYS_GUI_DRAW_WINDOW: {
        /* b = pointer to GUI_WINDOW */
        if (!is_user_ptr((const void*)b)) { ret = (uint32_t)-1; break; }
        GUI_WINDOW win;
        if (copy_from_user(&win, (const void*)b, sizeof(win)) != 0) {
            ret = (uint32_t)-1; break;
        }
        extern void GuiDrawWindow(const GUI_WINDOW*);
        GuiDrawWindow(&win);
        ret = 0;
        break;
    }
    case SYS_GUI_DRAW_WINDOW_TEXT: {
        /* a = pointer to GUI_WINDOW, b=rx, c=ry, d=pointer to string, e=fg */
        GUI_WINDOW win;
        if (!is_user_ptr((const void*)a) || !is_user_ptr((const void*)d)) { ret = (uint32_t)-1; break; }
        if (copy_from_user(&win, (const void*)a, sizeof(win)) != 0) { ret = (uint32_t)-1; break; }
        /* copy string to local buffer */
        char buf[128];
        int i;
        const char *src = (const char*)d;
        for (i = 0; i < (int)sizeof(buf)-1; i++) {
            if (!is_user_ptr(src + i)) { ret = (uint32_t)-1; break; }
            buf[i] = src[i];
            if (buf[i] == '\0') break;
        }
        if (ret != 0) break;
        buf[sizeof(buf)-1] = '\0';
        extern void GuiDrawWindowText(const GUI_WINDOW*, int, int, const char*, UCHAR);
        GuiDrawWindowText(&win, (int)b, (int)c, buf, (UCHAR)e);
        ret = 0;
        break;
    }
    case SYS_GET_PIXEL:
        /* a=x b=y */
        ret = VgaGetPixel((INT)a, (INT)b);
        break;
    case SYS_DEBUG: {
        /* a = pointer to nul-terminated string */
        serial_puts("[debug syscall]\r\n");
        if (!is_user_ptr((const void*)a)) { ret = (uint32_t)-1; break; }
        /* copiar max 128 bytes para evitar overflow */
        char buf[128];
        if (copy_from_user(buf, (const void*)a, sizeof(buf)-1) != 0) {
            ret = (uint32_t)-1;
            break;
        }
        buf[sizeof(buf)-1] = '\0';
        serial_puts(buf);
        ret = 0;
        break;
    }
    case SYS_DUMP_VRAM: {
        serial_puts("[dump syscall]\r\n");
        /* read each of the four planes separately by setting Read Map Select */
        for (int p = 0; p < 4; p++) {
            /* select read plane p */
            VgaWriteGraphicsController(4, (UCHAR)p);
            char linebuf[160];
            for (int row = 0; row < 16; row++) {
                /* prefix plane and row */
                serial_puts("pl");
                char digit = '0' + p;
                serial_puts((const char[]){digit,' ',0});
                serial_puts("row ");
                serial_print_hex(row);
                serial_puts(": ");
                PUCHAR fb = (PUCHAR)VGA_BASE_ADDRESS;
                for (int col = 0; col < 16; col++) {
                    UCHAR b = fb[row * (640/8) + col];
                    char h[3];
                    static const char *hex="0123456789ABCDEF";
                    h[0] = hex[(b>>4)&0xF]; h[1] = hex[b&0xF]; h[2]='\0';
                    serial_puts(h);
                    serial_puts(" ");
                }
                serial_puts("\r\n");
            }
        }
        /* restore read map to plane 0 for sanity */
        VgaWriteGraphicsController(4, 0x00);
        /* also dump first 8 entries of shadow row 0 */
        {
            serial_puts("[shadow row0]: ");
            static const char *hex="0123456789ABCDEF";
            for (int i = 0; i < 8; i++) {
                UCHAR c = VgaGetPixel(i, 0);
                char h[3];
                h[0] = hex[(c>>4)&0xF]; h[1] = hex[c&0xF]; h[2]='\0';
                serial_puts(h);
                serial_puts(" ");
            }
            serial_puts("\r\n");
        }
        ret = 0;
        break;
    }
    default:
        /* syscall desconocido */
        ret = (uint32_t)-1;
        break;
    }
    return ret;
}

#include "syscall.h"
#include <gui.h>           /* GUI_WINDOW, GUI_MOUSE_EVENT */
#include "../mm/vmm.h"   /* para validación de punteros y estructuras PTE */
#include "../proc/process.h"
#include "../proc/scheduler.h"
#include "../drivers/video/vga/vga.h"    /* funciones VGA */
#include "../drivers/video/vga/vga_font.h" /* VgaDrawString */
#include "../drivers/input/ps2mouse.h" /* MOUSE_STATE */
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
    uint32_t addr = (uint32_t)p;
    if (addr >= 0x80000000) return 0;
    /* verify page table entries have U bit set; protect from kernel-only addresses */
    uint32_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    /* walk two-level page tables */
    pde_t *pde = (pde_t*)((cr3 & ~0xFFF) + ((addr >> 22) * sizeof(pde_t)));
    if (!(*pde & PTE_PRESENT) || !(*pde & PTE_USER))
        return 0;
    pte_t *pte = (pte_t*)((*pde & ~0xFFF) + (((addr >> 12) & 0x3FF) * sizeof(pte_t)));
    if (!(*pte & PTE_PRESENT) || !(*pte & PTE_USER))
        return 0;
    return 1;
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
        /* debug: imprimir el valor retornado para verificar progreso */
        serial_puts("[sys_get_tick] ");
        serial_print_hex(ret);
        serial_puts("\r\n");
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
    case SYS_DEBUG: {
        /* b = pointer a cadena usuario */
        if (!is_user_ptr((const void*)b)) {
            serial_puts("[debug] user ptr invalid\r\n");
            ret = (uint32_t)-1;
            break;
        }
        /* simple copia y enviar a serial */
        char buf[128];
        int i;
        for (i = 0; i < (int)sizeof(buf)-1; i++) {
            char ch;
            if (copy_from_user(&ch, (const void*)(b + i), 1) != 0) {
                serial_puts("[debug] copy_from_user failed\r\n");
                ret = (uint32_t)-1;
                break;
            }
            buf[i] = ch;
            if (ch == '\0') break;
        }
        if (i == (int)sizeof(buf)-1) {
            /* unterminated? ensure null */
            buf[sizeof(buf)-1] = '\0';
        }
        serial_puts(buf);
        ret = 0;
        break;
    }
    case SYS_GET_MOUSE_EVENT: {
        /* b = pointer to SYS_MOUSE (user buffer) */
        if (!is_user_ptr((const void*)b)) { ret = (uint32_t)-1; break; }
        GUI_MOUSE_EVENT ev;
        int status = GuiGetMouseEvent(&ev);
        if (status == 0) {
            SYS_MOUSE ms = { ev.x, ev.y, ev.buttons };
            if (copy_from_user((void*)b, &ms, sizeof(ms)) != 0)
                ret = (uint32_t)-1;
            else
                ret = 0;
        } else {
            /* cola vacía */
            ret = (uint32_t)-1;
        }
        break;
    }
    case SYS_GUI_DRAW_DESKTOP:
        GuiDrawDesktop();
        ret = 0;
        break;
    case SYS_GUI_DRAW_TASKBAR:
        GuiDrawTaskbar();
        ret = 0;
        break;
    case SYS_GUI_DRAW_WINDOW: {
        /* argumentos: a=x, b=y, c=w, d=h, e=pointer title */
        if (!is_user_ptr((const void*)e)) { ret = (uint32_t)-1; break; }
        char title[128];
        for (int i = 0; i < (int)sizeof(title)-1; i++) {
            char ch;
            if (copy_from_user(&ch, (const void*)(e + i), 1) != 0) { ret = (uint32_t)-1; break; }
            title[i] = ch;
            if (ch == '\0') break;
        }
        title[127] = '\0';
        GUI_WINDOW win = { (INT)a, (INT)b, (INT)c, (INT)d, title, 1 };
        GuiDrawWindow(&win);
        ret = 0;
        break;
    }
    case SYS_GUI_DRAW_WINDOW_TEXT: {
        /* a=pointer win (user), b=rx, c=ry, d=pointer txt, e=fg */
        if (!is_user_ptr((const void*)a) || !is_user_ptr((const void*)d)) { ret = (uint32_t)-1; break; }
        GUI_WINDOW localWin;
        if (copy_from_user(&localWin, (const void*)a, sizeof(localWin)) != 0) { ret = (uint32_t)-1; break; }
        char txt[128];
        for (int i = 0; i < (int)sizeof(txt)-1; i++) {
            char ch;
            if (copy_from_user(&ch, (const void*)(d + i), 1) != 0) { ret = (uint32_t)-1; break; }
            txt[i] = ch;
            if (ch == '\0') break;
        }
        txt[127] = '\0';
        GuiDrawWindowText(&localWin, (INT)b, (INT)c, txt, (UCHAR)e);
        ret = 0;
        break;
    }
    case SYS_GUI_DRAW_BUTTON: {
        /* a=x,b=y,c=w,d=h,e=pressed,label in esi?? not handled here */
        /* For simplicity we ignore label and pressed state from params and assume
           the user will only draw buttons via taskbar helper, so we just call
           GuiDrawButton using registers directly. */
        GuiDrawButton((INT)a, (INT)b, (INT)c, (INT)d, NULL, (INT)e);
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

#!/bin/sh

# Colores para output (ANSI escape codes)
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuración
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ISO_PATH="${SCRIPT_DIR}/build/system_operative_edit.iso"
QEMU_LOG="qemu.log"
BUILD_SCRIPT="${SCRIPT_DIR}/scripts/build.sh"
CREATE_ISO_SCRIPT="${SCRIPT_DIR}/scripts/create-iso.sh"

# Valores por defecto
BUILD=1
RUN=1
DEBUG=0
MONITOR=0
CLEAN=0
KVM=0
SMP="1"
RAM="256M"
VERBOSE=0

# Procesar argumentos
while [ $# -gt 0 ]; do
    case $1 in
        -h|--help)
            echo "${CYAN}Uso: $0 [opciones]${NC}"
            echo ""
            echo "Opciones:"
            echo "  -h, --help        Muestra esta ayuda"
            echo "  -b, --build-only  Solo compila (no ejecuta QEMU)"
            echo "  -r, --run-only    Solo ejecuta QEMU (no compila)"
            echo "  -d, --debug       Ejecuta con QEMU en modo debug (GDB)"
            echo "  -m, --monitor     Abre monitor de QEMU (Ctrl+Alt+2)"
            echo "  -c, --clean       Limpia y recompila todo"
            echo "  -k, --kvm         Usa aceleración KVM (si está disponible)"
            echo "  -s, --smp <n>     Número de CPUs (ej: -s 2)"
            echo "  -R, --ram <size>  RAM en MB (ej: -R 512)"
            echo "  -v, --verbose     Log detallado de QEMU"
            echo ""
            echo "Ejemplos:"
            echo "  $0                # Compila y ejecuta normal"
            echo "  $0 -d             # Compila y ejecuta con debug"
            echo "  $0 -r -k          # Solo ejecuta con KVM"
            echo "  $0 -c -s 4 -R 1G  # Limpia, compila con 4 CPUs y 1GB RAM"
            exit 0
            ;;
        -b|--build-only)
            RUN=0
            shift
            ;;
        -r|--run-only)
            BUILD=0
            shift
            ;;
        -d|--debug)
            DEBUG=1
            shift
            ;;
        -m|--monitor)
            MONITOR=1
            shift
            ;;
        -c|--clean)
            CLEAN=1
            shift
            ;;
        -k|--kvm)
            KVM=1
            shift
            ;;
        -s|--smp)
            SMP="$2"
            shift 2
            ;;
        -R|--ram)
            RAM="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        *)
            echo "${RED}Error: Opción desconocida $1${NC}"
            exit 1
            ;;
    esac
done

# Función para verificar archivos
check_files() {
    if [ ! -f "$BUILD_SCRIPT" ]; then
        echo "${RED}Error: No se encuentra $BUILD_SCRIPT${NC}"
        exit 1
    fi
    
    if [ ! -f "$CREATE_ISO_SCRIPT" ]; then
        echo "${RED}Error: No se encuentra $CREATE_ISO_SCRIPT${NC}"
        exit 1
    fi
}

# Función para compilar
build() {
    echo "${YELLOW}=== Compilando sistema ===${NC}"
    
    if [ "$CLEAN" = 1 ]; then
        echo "${BLUE}Limpiando build anterior...${NC}"
        rm -rf "${SCRIPT_DIR}/build"
    fi
    
    echo "${CYAN}Ejecutando build.sh...${NC}"
    if ! sh "$BUILD_SCRIPT"; then
        echo "${RED}Error en la compilación${NC}"
        exit 1
    fi
    
    echo "${CYAN}Creando ISO...${NC}"
    if ! sh "$CREATE_ISO_SCRIPT"; then
        echo "${RED}Error creando la ISO${NC}"
        exit 1
    fi
    
    echo "${GREEN}✓ Compilación exitosa${NC}"
}

# Función para ejecutar QEMU
run_qemu() {
    echo "${YELLOW}=== Iniciando QEMU ===${NC}"
    
    # Verificar que la ISO existe
    if [ ! -f "$ISO_PATH" ]; then
        echo "${RED}Error: No se encuentra $ISO_PATH${NC}"
        echo "${YELLOW}Ejecuta primero la compilación: $0 --build-only${NC}"
        exit 1
    fi
    
    # Construir comando base de QEMU
    QEMU_CMD="qemu-system-i386"
    
    # Opciones básicas
    QEMU_CMD="${QEMU_CMD} -cdrom \"${ISO_PATH}\""
    QEMU_CMD="${QEMU_CMD} -m ${RAM}"
    QEMU_CMD="${QEMU_CMD} -smp ${SMP}"
    
    # Opciones de salida
    if [ "$VERBOSE" = 1 ]; then
        QEMU_CMD="${QEMU_CMD} -d int,cpu_reset,guest_errors -D ${QEMU_LOG}"
        echo "${BLUE}Log detallado en: $QEMU_LOG${NC}"
    else
        QEMU_CMD="${QEMU_CMD} -d int,cpu_reset,guest_errors -D ${QEMU_LOG}"
    fi
    
    # Opciones de debug
    if [ "$DEBUG" = 1 ]; then
        echo "${PURPLE}Modo DEBUG activado - GDB en puerto 1234${NC}"
        QEMU_CMD="${QEMU_CMD} -s -S"
    fi
    
    # Monitor QEMU
    if [ "$MONITOR" = 1 ]; then
        echo "${CYAN}Monitor QEMU activado (Ctrl+Alt+2)${NC}"
        QEMU_CMD="${QEMU_CMD} -monitor stdio"
    fi
    
    # Aceleración KVM
    if [ "$KVM" = 1 ]; then
        if [ -e /dev/kvm ]; then
            echo "${GREEN}Usando aceleración KVM${NC}"
            QEMU_CMD="${QEMU_CMD} -enable-kvm"
        else
            echo "${YELLOW}KVM no disponible, usando emulación${NC}"
        fi
    fi
    
    # Dispositivos útiles
    QEMU_CMD="${QEMU_CMD} -vga std"
    QEMU_CMD="${QEMU_CMD} -usb -device usb-tablet"
    
    if [ "$MONITOR" != 1 ]; then
        # Redirigir salida serial a archivo si no usamos monitor
        QEMU_CMD="${QEMU_CMD} -serial file:serial.log"
        echo "${BLUE}Salida serial en: serial.log${NC}"
    fi
    
    # Mostrar comando
    echo "${CYAN}Comando:${NC} $QEMU_CMD"
    echo "${YELLOW}Presiona Ctrl+Alt+G para liberar el mouse${NC}"
    echo "${GREEN}Iniciando...${NC}"
    echo ""
    
    # Ejecutar QEMU
    if [ "$MONITOR" = 1 ]; then
        eval "$QEMU_CMD"
    else
        eval "$QEMU_CMD" &
        QEMU_PID=$!
        echo "${BLUE}QEMU PID: $QEMU_PID${NC}"
        echo "${YELLOW}Presiona Ctrl+C para terminar${NC}"
        wait $QEMU_PID
    fi
}

# Función para mostrar resumen
show_summary() {
    echo ""
    echo "${GREEN}================================${NC}"
    echo "${GREEN}    System Operative Edit       ${NC}"
    echo "${GREEN}================================${NC}"
    echo "${CYAN}Configuración:${NC}"
    echo "  RAM: ${YELLOW}${RAM}${NC}"
    echo "  CPUs: ${YELLOW}${SMP}${NC}"
    
    if [ "$KVM" = 1 ]; then
        KVM_TEXT="Sí"
    else
        KVM_TEXT="No"
    fi
    echo "  KVM: ${YELLOW}${KVM_TEXT}${NC}"
    
    if [ "$DEBUG" = 1 ]; then
        DEBUG_TEXT="Sí"
    else
        DEBUG_TEXT="No"
    fi
    echo "  Debug: ${YELLOW}${DEBUG_TEXT}${NC}"
    
    echo "  Log: ${YELLOW}${QEMU_LOG}${NC}"
    echo ""
}

# Main
main() {
    echo "${PURPLE}=== System Operative Edit - Run Script ===${NC}"
    
    # Verificar archivos necesarios
    if [ "$BUILD" = 1 ]; then
        check_files
    fi
    
    # Mostrar resumen
    show_summary
    
    # Compilar si es necesario
    if [ "$BUILD" = 1 ]; then
        build
    fi
    
    # Ejecutar QEMU si es necesario
    if [ "$RUN" = 1 ]; then
        run_qemu
    else
        echo "${GREEN}✓ Build completado. Para ejecutar: $0 -r${NC}"
    fi
}

# Ejecutar main
main
#!/bin/bash

# Script para lanzar barridos de IOPS con fit_to_g5k_app_v6 usando la
# plataforma fit_to_g5k.xml. Recorre IoTs desde 1 hasta 5000 para mostrar
# crecimiento de IOPS por cantidad de IoTs, para cada config de fog servers.
#
# Uso:
#   ./run_iops_v6_sweep_v2.sh
#
# Variables opcionales (exportar antes de ejecutar):
#   NUM_MESSAGES   Número de mensajes por IoT (por defecto 1000)
#   PACKET_SIZE    Tamaño de paquete en bytes (por defecto 2048)
#   MAX_IOTS       Máximo número de IoTs a probar (por defecto 5000)

# No usar set -e para evitar terminaciones prematuras
set -u

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
REPO_ROOT="$SCRIPT_DIR"
BUILD_DIR="$REPO_ROOT/build"

APP="$BUILD_DIR/fit_to_g5k_app_v6"
PLATFORM_FILE="$REPO_ROOT/platforms/fit_to_g5k.xml"

NUM_MESSAGES=${NUM_MESSAGES:-1000}
PACKET_SIZE=${PACKET_SIZE:-2048}
MAX_IOTS=${MAX_IOTS:-5000}
FOG_SERVERS=(8 16 32 64)

# Puntos de prueba: desde 1 hasta MAX_IOTS con incremento adaptativo
# Usa una progresión que da más densidad en valores bajos
generate_iot_counts() {
  local max=$1
  local points=()
  
  # Puntos específicos para granularidad inicial
  points+=(1 2 5 10 20 50 100 200 500 1000 2000 5000)
  
  # Si MAX_IOTS es menor, filtrar
  local result=()
  for p in "${points[@]}"; do
    if [ "$p" -le "$max" ]; then
      result+=("$p")
    fi
  done
  
  echo "${result[@]}"
}

IOT_COUNTS=($(generate_iot_counts $MAX_IOTS))

# Validaciones básicas
if [ ! -x "$APP" ]; then
  echo "ERROR: No se encuentra el binario $APP. Ejecuta ./build.sh primero." >&2
  exit 1
fi

if [ ! -f "$PLATFORM_FILE" ]; then
  echo "ERROR: No se encuentra el archivo de plataforma $PLATFORM_FILE" >&2
  exit 1
fi

timestamp=$(date +%Y%m%d_%H%M%S)
RESULTS_DIR="$REPO_ROOT/iops_v6_scaling_${timestamp}"
mkdir -p "$RESULTS_DIR"

SUMMARY_FILE="$RESULTS_DIR/summary_table.txt"
CSV_FILE="$RESULTS_DIR/summary_table.csv"

echo "=========================================" | tee "$SUMMARY_FILE"
echo "  IOPS Growth Analysis - fit_to_g5k_app_v6" | tee -a "$SUMMARY_FILE"
echo "  Plataforma: $(basename "$PLATFORM_FILE")" | tee -a "$SUMMARY_FILE"
echo "  Mensajes por IoT: $NUM_MESSAGES" | tee -a "$SUMMARY_FILE"
echo "  Tamaño de paquete: $PACKET_SIZE bytes" | tee -a "$SUMMARY_FILE"
echo "  Rango de IoTs: 1 a ${MAX_IOTS}" | tee -a "$SUMMARY_FILE"
echo "  Puntos de prueba: ${#IOT_COUNTS[@]}" | tee -a "$SUMMARY_FILE"
echo "  Fog servers: ${FOG_SERVERS[*]}" | tee -a "$SUMMARY_FILE"
echo "  Resultados: $RESULTS_DIR" | tee -a "$SUMMARY_FILE"
echo "=========================================" | tee -a "$SUMMARY_FILE"
echo "" | tee -a "$SUMMARY_FILE"

# Encabezado CSV
echo "FogServers,IoTs,TotalOps,TotalIOPS,SimTime,OpsPerIoT" > "$CSV_FILE"

total_tests=$((${#FOG_SERVERS[@]} * ${#IOT_COUNTS[@]}))
current=0

for fogs in "${FOG_SERVERS[@]}"; do
  echo "=========================================" | tee -a "$SUMMARY_FILE"
  echo "FOG SERVERS: $fogs" | tee -a "$SUMMARY_FILE"
  echo "=========================================" | tee -a "$SUMMARY_FILE"
  printf "%-8s | %-12s | %-12s | %-12s | %-10s | %s\n" "IoTs" "Total Ops" "Total IOPS" "Ops/IoT" "Sim Time" "Growth" | tee -a "$SUMMARY_FILE"
  echo "---------|--------------|--------------|--------------|------------|----------" | tee -a "$SUMMARY_FILE"

  prev_iops=0
  for iots in "${IOT_COUNTS[@]}"; do
    current=$((current + 1))
    echo "[${current}/${total_tests}] Ejecutando: fogs=$fogs, iots=$iots (esto puede tardar varios minutos...)"

    log_file="$RESULTS_DIR/fogs${fogs}_iots${iots}.log"

    # Ejecutar sin || para evitar que set -e termine el script
    "$APP" "$PLATFORM_FILE" "$NUM_MESSAGES" "$fogs" "$iots" "$PACKET_SIZE" -d rr --ack --quiet > "$log_file" 2>&1
    exit_code=$?
    
    if [ $exit_code -ne 0 ]; then
      echo "  ADVERTENCIA: La simulación terminó con código $exit_code (ver $log_file)" | tee -a "$SUMMARY_FILE"
      echo "  Continuando con la siguiente prueba..." | tee -a "$SUMMARY_FILE"
      continue
    fi

    # Extraer métricas
    total_ops=$(grep "Total operations:" "$log_file" | tail -n1 | awk '{print $NF}')
    total_iops=$(grep "Global IOPS (total ops / worst time):" "$log_file" | tail -n1 | awk '{print $(NF-1)}')
    sim_time=$(grep "Simulated time:" "$log_file" | tail -n1 | awk '{print $(NF-1)}')

    # Validar que se extrajeron métricas válidas
    if [ -z "$total_ops" ] || [ -z "$total_iops" ] || [ -z "$sim_time" ]; then
      echo "  ADVERTENCIA: No se pudieron extraer métricas del log (ver $log_file)" | tee -a "$SUMMARY_FILE"
      echo "  Continuando con la siguiente prueba..." | tee -a "$SUMMARY_FILE"
      continue
    fi
    
    # Calcular operaciones por IoT
    ops_per_iot=$(awk "BEGIN {printf \"%.0f\", $total_ops / $iots}")
    
    # Calcular crecimiento porcentual respecto a la medición anterior
    if [ "$prev_iops" = "0" ] || [ "$prev_iops" = "" ]; then
      growth="baseline"
    else
      growth=$(awk "BEGIN {printf \"%.1f%%\", (($total_iops - $prev_iops) / $prev_iops) * 100}")
    fi
    prev_iops=$total_iops

    printf "%-8s | %-12s | %-12s | %-12s | %-10s | %s\n" "$iots" "$total_ops" "$total_iops" "$ops_per_iot" "$sim_time" "$growth" | tee -a "$SUMMARY_FILE"

    echo "$fogs,$iots,$total_ops,$total_iops,$sim_time,$ops_per_iot" >> "$CSV_FILE"
    
    echo "  ✓ Completado exitosamente"
  done

  echo "" | tee -a "$SUMMARY_FILE"
done

echo "=========================================" | tee -a "$SUMMARY_FILE"
echo "Sweep finalizado. Resultados en $RESULTS_DIR" | tee -a "$SUMMARY_FILE"
echo "=========================================" | tee -a "$SUMMARY_FILE"
echo "" | tee -a "$SUMMARY_FILE"
echo "Archivos generados:" | tee -a "$SUMMARY_FILE"
echo "  - Tabla resumen: $SUMMARY_FILE" | tee -a "$SUMMARY_FILE"
echo "  - Datos CSV: $CSV_FILE" | tee -a "$SUMMARY_FILE"
echo "  - Logs individuales: $RESULTS_DIR/fogs*_iots*.log" | tee -a "$SUMMARY_FILE"
echo "" | tee -a "$SUMMARY_FILE"

echo "Preview del resumen:" | tee -a "$SUMMARY_FILE"
head -n 50 "$SUMMARY_FILE"

exit 0

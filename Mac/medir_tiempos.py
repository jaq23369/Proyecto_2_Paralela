#!/usr/bin/env python3
"""Mide el tiempo de busqueda de la llave con bruteforce (version adaptada para Mac).

Para cada llave y numero de procesos corre el programa varias veces, lee el
"Tiempo de busqueda" que imprime el proceso 0 (MPI_Wtime) y saca el promedio.
Las combinaciones que tardarian demasiado no se corren, su tiempo se estima con
la velocidad medida (llaves por segundo con 1 proceso).

Uso
    make
    python3 medir_tiempos.py               # mide con 5 corridas por combinacion
    python3 medir_tiempos.py --runs 3
    python3 medir_tiempos.py --solo-tabla  # rehace tiempos.md desde resultados_tiempos.csv

Guarda todas las corridas en resultados_tiempos.csv y las tablas en tiempos.md
"""
import argparse
import csv
import os
import platform
import re
import signal
import statistics
import subprocess
import time

PROGRAMA = "./bruteforce"
TOTAL = 1 << 56
PROCS = [1, 2, 4, 8]
LIMITE_S = 60          # no se corre una combinacion si se estima que tarda mas que esto
TIMEOUT_S = 180        # tiempo maximo de una corrida antes de detenerla
CSV = "resultados_tiempos.csv"
TABLA = "tiempos.md"

# (nombre, llave). La primera se usa para medir la velocidad con 1 proceso.
# Las demas estan pensadas para caer en distintas partes de los rangos de cada proceso
LLAVES = [
    ("Chica", 1_000_000),
    ("Mediana", 8_000_000),
    ("Del grupo (2^54)", 1 << 54),
    ("2^55 + 4,000,000", (1 << 55) + 4_000_000),
    ("3*2^53 + 4,000,000", 3 * (1 << 53) + 4_000_000),
]


def posicion(llave, n):
    """Proceso que tiene la llave y cuantas llaves prueba antes de llegar a ella.
    Devuelve None si el programa nunca la prueba (se salta la ultima llave de cada rango)."""
    rango = TOTAL // n
    pid = min(llave // rango, n - 1)
    if pid < n - 1 and llave == rango * (pid + 1) - 1:
        return pid, None
    return pid, llave - pid * rango


def correr(n, llave):
    cmd = ["mpirun", "-np", str(n), PROGRAMA, str(llave)]
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                         text=True, start_new_session=True)
    try:
        out, _ = p.communicate(timeout=TIMEOUT_S)
    except subprocess.TimeoutExpired:
        os.killpg(p.pid, signal.SIGKILL)
        p.communicate()
        return None, "se paso del tiempo"
    t = re.search(r"Tiempo de busqueda = ([0-9.]+) s", out)
    k = re.search(r"Key = (-?\d+)", out)
    if not t or not k:
        return None, "salida inesperada"
    if int(k.group(1)) != llave:
        return None, "encontro otra llave (%s)" % k.group(1)
    return float(t.group(1)), "ok"


def medir(runs):
    filas = []
    velocidad = None
    for nombre, llave in LLAVES:
        for n in PROCS:
            _, antes = posicion(llave, n)
            if antes is None or (velocidad and antes / velocidad > LIMITE_S):
                print("%-20s np=%d  no se corre, tardaria demasiado" % (nombre, n))
                continue
            tiempos = []
            for r in range(runs):
                t, estado = correr(n, llave)
                filas.append(dict(llave_nombre=nombre, llave=llave, procesos=n, corrida=r + 1,
                                  tiempo_s="" if t is None else "%.6f" % t, estado=estado))
                print("%-20s np=%d  corrida %d  %s" % (nombre, n, r + 1,
                                                       estado if t is None else "%.6f s" % t))
                if t is not None:
                    tiempos.append(t)
            if velocidad is None and n == 1 and tiempos:
                velocidad = llave / statistics.mean(tiempos)
    with open(CSV, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=list(filas[0].keys()))
        w.writeheader()
        w.writerows(filas)


def fmt_tiempo(prom, desv):
    if prom < 0.001:
        return "%.0f ± %.0f µs" % (prom * 1e6, desv * 1e6)
    return "%.3f ± %.3f s" % (prom, desv)


def fmt_estimado(seg):
    if seg < 3600:
        return "%.0f s (estimado)" % seg
    if seg < 86400 * 365:
        return "%.1f días (estimado)" % (seg / 86400)
    return "%s años (estimado)" % format(round(seg / (365.25 * 86400)), ",")


def tablas():
    datos = {}
    runs = 0
    with open(CSV, encoding="utf-8") as f:
        for fila in csv.DictReader(f):
            if fila["estado"] != "ok":
                continue
            clave = (fila["llave_nombre"], int(fila["procesos"]))
            datos.setdefault(clave, []).append(float(fila["tiempo_s"]))
            runs = max(runs, int(fila["corrida"]))
    nombre0, llave0 = LLAVES[0]
    velocidad = llave0 / statistics.mean(datos[(nombre0, 1)])

    resumen = {}
    for nombre, llave in LLAVES:
        for n in PROCS:
            pid, antes = posicion(llave, n)
            ts = datos.get((nombre, n))
            if ts:
                resumen[(nombre, n)] = dict(pid=pid, antes=antes, prom=statistics.mean(ts),
                                            desv=statistics.stdev(ts) if len(ts) > 1 else 0.0)
            else:
                resumen[(nombre, n)] = dict(pid=pid, antes=antes,
                                            est=float("inf") if antes is None else antes / velocidad)

    L = []
    L.append("Medido el %s en una Mac %s (macOS %s), con %d corridas por combinación. "
             "El tiempo es el que imprime el proceso 0 con MPI_Wtime, desde que todos los procesos "
             "empiezan a buscar hasta que le llega la llave.\n"
             % (time.strftime("%d/%m/%Y"), platform.machine(), platform.mac_ver()[0], runs))
    L.append("Velocidad con 1 proceso, %s llaves por segundo.\n" % format(round(velocidad), ","))
    L.append("#### Tiempo promedio y desviación estándar\n")
    L.append("| Llave | Valor | " + " | ".join("%d proceso%s" % (n, "" if n == 1 else "s") for n in PROCS) + " |")
    L.append("|---|---|" + "---|" * len(PROCS))
    for nombre, llave in LLAVES:
        celdas = []
        for n in PROCS:
            d = resumen[(nombre, n)]
            if "prom" in d:
                celdas.append(fmt_tiempo(d["prom"], d["desv"]))
            elif d["est"] == float("inf"):
                celdas.append("nunca la encuentra")
            else:
                celdas.append(fmt_estimado(d["est"]))
        L.append("| %s | %s | %s |" % (nombre, format(llave, ","), " | ".join(celdas)))
    L.append("")
    # una tabla por numero de procesos con cada corrida y el promedio, que es el tiempo adecuado de cada llave
    for n in PROCS:
        L.append("#### Tiempo adecuado de cada llave con %d proceso%s\n" % (n, "" if n == 1 else "s"))
        L.append("| Llave | " + " | ".join("Corrida %d" % (r + 1) for r in range(runs))
                 + " | Promedio (tiempo adecuado) | Desviación estándar | Tiempo por llave probada |")
        L.append("|---|" + "---|" * (runs + 3))
        for nombre, llave in LLAVES:
            d = resumen[(nombre, n)]
            ts = datos.get((nombre, n))
            if ts:
                corr = [fmt_tiempo(t, 0).split(" ±")[0] + (" µs" if t < 0.001 else " s") for t in ts]
                corr += ["-"] * (runs - len(corr))
                prom = fmt_tiempo(d["prom"], 0).split(" ±")[0] + (" µs" if d["prom"] < 0.001 else " s")
                desv = ("%.0f µs" % (d["desv"] * 1e6)) if d["prom"] < 0.001 else ("%.3f s" % d["desv"])
                por_llave = ("%.3f µs" % (d["prom"] / d["antes"] * 1e6)) if d["antes"] else "-"
                L.append("| %s | %s | %s | %s | %s |" % (nombre, " | ".join(corr), prom, desv, por_llave))
            else:
                est = "nunca la encuentra" if d["est"] == float("inf") else fmt_estimado(d["est"])
                L.append("| %s | %s | %s | - | - |" % (nombre, " | ".join(["no se corrió"] * runs), est))
        # fila final, promedio de las llaves que si se midieron con este numero de procesos
        proms = [resumen[(nm, n)]["prom"] for nm, _ in LLAVES if "prom" in resumen[(nm, n)]]
        pls = [resumen[(nm, n)]["prom"] / resumen[(nm, n)]["antes"]
               for nm, _ in LLAVES if "prom" in resumen[(nm, n)] and resumen[(nm, n)]["antes"]]
        L.append("| Promedio con %d proceso%s | %s | %.3f s | - | %.3f µs |" % (
            n, "" if n == 1 else "s", " | ".join([""] * runs), statistics.mean(proms),
            statistics.mean(pls) * 1e6))
        L.append("")
    L.append("#### Proceso que tiene la llave y llaves que prueba antes de llegar a ella\n")
    L.append("| Llave | " + " | ".join("%d proceso%s" % (n, "" if n == 1 else "s") for n in PROCS) + " |")
    L.append("|---|" + "---|" * len(PROCS))
    for nombre, llave in LLAVES:
        celdas = []
        for n in PROCS:
            d = resumen[(nombre, n)]
            celdas.append("P%d, %s" % (d["pid"], "nunca" if d["antes"] is None else format(d["antes"], ",")))
        L.append("| %s | %s |" % (nombre, " | ".join(celdas)))
    L.append("")
    L.append("#### Speedup con respecto a 1 proceso\n")
    L.append("Es el tiempo con 1 proceso dividido entre el tiempo con n procesos. "
             "Cuando uno de los dos tiempos es estimado, el speedup también lo es.\n")
    L.append("| Llave | " + " | ".join("%d proceso%s" % (n, "" if n == 1 else "s") for n in PROCS) + " |")
    L.append("|---|" + "---|" * len(PROCS))
    for nombre, _ in LLAVES:
        b = resumen[(nombre, 1)]
        tb = b.get("prom", b.get("est"))
        celdas = []
        for n in PROCS:
            d = resumen[(nombre, n)]
            tn = d.get("prom", d.get("est"))
            if n == 1:
                celdas.append("1.00")
            elif tb == float("inf") or tn == float("inf"):
                celdas.append("-")
            else:
                s = tb / tn
                txt = "%.2f" % s if s < 1000 else "%.1e" % s
                celdas.append(txt if ("prom" in b and "prom" in d) else txt + " (estimado)")
        L.append("| %s | %s |" % (nombre, " | ".join(celdas)))
    with open(TABLA, "w", encoding="utf-8") as f:
        f.write("\n".join(L) + "\n")
    print("Tablas guardadas en %s" % TABLA)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--runs", type=int, default=5)
    ap.add_argument("--solo-tabla", action="store_true")
    args = ap.parse_args()
    if not args.solo_tabla:
        medir(args.runs)
    tablas()


if __name__ == "__main__":
    main()

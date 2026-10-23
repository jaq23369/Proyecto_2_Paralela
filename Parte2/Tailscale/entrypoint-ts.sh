#!/bin/bash
# Si se pasa TS_AUTHKEY, el contenedor entra a la VPN de Tailscale con su propia IP.
# Si no se pasa, solo arranca SSH como en la imagen base.
if [ -n "$TS_AUTHKEY" ]; then
  mkdir -p /var/lib/tailscale /var/run/tailscale
  tailscaled --state=/var/lib/tailscale/tailscaled.state >/var/log/tailscaled.log 2>&1 &
  for i in $(seq 1 20); do tailscale status >/dev/null 2>&1 && break; sleep 0.5; done
  tailscale up --authkey="$TS_AUTHKEY" --hostname="${TS_HOSTNAME:-$(hostname)}" --accept-dns=false
  echo "Tailscale IP: $(tailscale ip -4 | head -1)"
fi
exec /proyecto/entrypoint.sh "$@"

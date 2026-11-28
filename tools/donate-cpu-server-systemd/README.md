# daca@home server setup

The daca@home server runs two services under the non-sudo system account `daca`, with the work
path `/var/daca@home`:

| Service | Script | Port | Purpose |
|---|---|---|---|
| `daca-server.service` | `donate-cpu-server.py` | 8000 | socket protocol for `donate-cpu.py` clients; HTTP requests are redirected to port 80 |
| `daca-rest.service` | `donate-cpu-server-rest.py` | 80 | HTML reports and REST API |

`daca-rest.service` only gets the `CAP_NET_BIND_SERVICE` capability so that it can bind to port 80.

## Installation

```sh
# account without login and without sudo; the group 'daca' is shared with the maintainers
sudo groupadd -f daca
sudo useradd --system --gid daca --home-dir /var/daca@home --no-create-home --shell /usr/sbin/nologin daca

# scripts and virtual environment for the REST server
cp donate-cpu-server.py donate-cpu-server-rest.py donate-cpu-server-requirements.txt /var/daca@home/
python3 -m venv /var/daca@home/rest-venv
/var/daca@home/rest-venv/bin/pip install -r /var/daca@home/donate-cpu-server-requirements.txt

# ownership (group writable so that maintainers in the group 'daca' can edit packages.txt etc.)
sudo chown -R daca:daca /var/daca@home
sudo chmod -R g+rwX /var/daca@home
sudo find /var/daca@home -type d -exec chmod g+s {} +

# services
sudo cp daca-server.service daca-rest.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now daca-server daca-rest
```

## Updating a script

Copy the new script to `/var/daca@home/` and restart the service:

```sh
sudo systemctl restart daca-server   # donate-cpu-server.py
sudo systemctl restart daca-rest     # donate-cpu-server-rest.py
```

The logs are in the journal: `journalctl -u daca-server -u daca-rest -f`.

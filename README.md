# ProofService-Lpr-Printer
Proof-based service for remote printing

How-to use
----------
Config changes are applied only at service restart. If service was installed using deb package then auto restarter should be enabled and simple kill -9 SERVICE_PID will work.

There are 2 modes available (General\imposition_system):

 * profit (requires ProFIT)
 * files

#### Common params
```ini
[server]
# server port
port=8090
# printer group names via |
printers=zebra_ups|hp_net
# alias of default printer if "printer" parameter is not specified in the URL
default_printer=z_ups

[zebra_ups]
# alias through which the printer is available by "printer" parameter in the URL
alias=z_ups
# printer name for lpq, empty for default
name=Zebra_UPS
# printer host, empty for local
host=
# printer can print EPL
accepts_raw=true
# printer ccan print files
accepts_files=false

[hp_net]
alias=hp_laser
name=HP_LaserJet_Pro
host=
accepts_raw=false
accepts_files=true
```

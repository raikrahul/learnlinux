savedcmd_numa_zone_trace.mod := printf '%s\n'   numa_zone_trace.o | awk '!x[$$0]++ { print("./"$$0) }' > numa_zone_trace.mod


## Bip

Bip parses text and emits bleeps to let you hear what's happening. Great for analysis of
real time info like log files, network dumps or output of complex program.


## Building

```
make
```

Copy `bip.lua` to `~/.bip.lua` and edit the code to make the beeps match your input.


## Usage

Pipe anything to `bip`.

Default lua file is `~/.bip.lua`, or provide a file name to load on the command line.

Bip wil periodically reload the lua file when modified, so you can tweak your bips in
a running session without having to restart.


## Examples

### Tail your syslog:

```
journalctl -f | bip
```

### Interactive shell with sound. 

```
bash | bip
```

This will break some programs requiring a true tty though


### Live network inspection

Run tcpdump in real time and generate an unique pitch and pan for each source
and destination:

```
sudo tcpdump --immediate-mode -i enp8s0 -n -l | bip
```

with the following lua script:

```
function on_line(line)
   for src, dst in line:gmatch("IP (%S+) > (%S-):") do
      local i = hash(src) + hash(dst)
      local freq = 110 * math.pow(1.05946309, (i % 40) + 20)
      local pan = (i % 16) / 8 - 1.0
      bip(freq, 0.02, 1.0, pan)
   end
end
```

## Lua API

Functions available in your lua code

### Callbacks:

To be implemented by the user, this gets called for each line on stdin.

```on_line(line)```


### API functions


```
bip(freq, duration, gain, pan)

freq: Hz
duration: sec
gain: 0.1
pan: -1.0 .. 1.0
```

Play a beep with the given frequency, duration, gain and pan


```
hash(s)

s: string
```

Generates a simple hash integer from the given string.


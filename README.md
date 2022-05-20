
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

Tail your syslog:

```
journalctl -f | bip
```

Interactive shell with sound. (This will break some programs requiring a true
tty though)

```
bash | bip
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



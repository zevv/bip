
-- example bip.lua

function on_line(line)

	-- do your pattern matching here. This example below
	-- generates a generic beep depending on the line length
	
	local l = #line                                        
	local freq = 110 * math.pow(1.05946309, (l-10) % 60) 
	pan = (l % 16) / 8 - 1.0 
	bip(freq, 0.05, 1.0, pan)
	
end


-- friendly startup bip

bip(440, 0.05)

-- vi: ft=lua ts=3 sw=3

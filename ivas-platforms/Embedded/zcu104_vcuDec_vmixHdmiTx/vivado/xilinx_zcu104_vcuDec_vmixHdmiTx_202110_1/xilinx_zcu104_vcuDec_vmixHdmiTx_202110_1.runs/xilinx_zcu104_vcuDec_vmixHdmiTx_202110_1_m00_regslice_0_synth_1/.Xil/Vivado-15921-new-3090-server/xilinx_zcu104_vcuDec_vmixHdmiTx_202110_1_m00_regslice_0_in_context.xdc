set_property -quiet CLOCK_PERIOD_OOC_TARGET 3.333 [get_ports -no_traverse -quiet aclk]
set_property -quiet IS_IP_OOC_CELL TRUE [get_cells -of [get_ports -no_traverse -quiet aclk]]
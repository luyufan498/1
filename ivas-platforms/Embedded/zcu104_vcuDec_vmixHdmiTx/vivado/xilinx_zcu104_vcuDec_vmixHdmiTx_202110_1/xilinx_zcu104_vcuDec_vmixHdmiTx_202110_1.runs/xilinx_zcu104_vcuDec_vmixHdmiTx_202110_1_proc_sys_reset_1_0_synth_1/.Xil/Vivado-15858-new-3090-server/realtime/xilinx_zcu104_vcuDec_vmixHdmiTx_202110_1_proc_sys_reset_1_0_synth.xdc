set_property SRC_FILE_INFO {cfile:/home/catmouse/VVAS/ivas-platforms/Embedded/zcu104_vcuDec_vmixHdmiTx/vivado/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1.gen/sources_1/bd/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1/ip/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1_proc_sys_reset_1_0/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1_proc_sys_reset_1_0_ooc.xdc rfile:../../../../../xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1.gen/sources_1/bd/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1/ip/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1_proc_sys_reset_1_0/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1_proc_sys_reset_1_0_ooc.xdc id:1 order:EARLY scoped_inst:U0} [current_design]
set_property SRC_FILE_INFO {cfile:/home/catmouse/VVAS/ivas-platforms/Embedded/zcu104_vcuDec_vmixHdmiTx/vivado/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1.gen/sources_1/bd/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1/ip/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1_proc_sys_reset_1_0/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1_proc_sys_reset_1_0.xdc rfile:../../../../../xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1.gen/sources_1/bd/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1/ip/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1_proc_sys_reset_1_0/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1_proc_sys_reset_1_0.xdc id:2 order:EARLY scoped_inst:U0} [current_design]
set_property SRC_FILE_INFO {cfile:/home/catmouse/VVAS/ivas-platforms/Embedded/zcu104_vcuDec_vmixHdmiTx/vivado/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1.runs/xilinx_zcu104_vcuDec_vmixHdmiTx_202110_1_proc_sys_reset_1_0_synth_1/dont_touch.xdc rfile:../../../dont_touch.xdc id:3} [current_design]
set_property src_info {type:SCOPED_XDC file:1 line:55 export:INPUT save:INPUT read:FILTER_OUT_OF_CONTEXT} [current_design]
create_clock -period 3.333 -name slowest_sync_clk [get_ports slowest_sync_clk]
set_property src_info {type:SCOPED_XDC file:2 line:49 export:INPUT save:INPUT read:READ} [current_design]
set_false_path -to [get_pins [list U0/EXT_LPF/ACTIVE_LOW_AUX.ACT_LO_AUX/GENERATE_LEVEL_P_S_CDC.SINGLE_BIT.CROSS_PLEVEL_IN2SCNDRY_IN_cdc_to/D U0/EXT_LPF/ACTIVE_LOW_EXT.ACT_LO_EXT/GENERATE_LEVEL_P_S_CDC.SINGLE_BIT.CROSS_PLEVEL_IN2SCNDRY_IN_cdc_to/D]]
set_property src_info {type:XDC file:3 line:9 export:INPUT save:INPUT read:READ} [current_design]
set_property KEEP_HIERARCHY SOFT [get_cells U0]

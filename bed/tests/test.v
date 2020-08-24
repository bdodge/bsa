//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------

reg  [7:0]   cmd_reg,
             vert_size;

reg  [31:0]  prnt_ctrl_reg;

integer      i,j;

//==============================================================
//defines

parameter dly       =  100;
parameter prnt_ctrl = 32'hfa00_00e0;

//==============================================================
//assignments

assign data     = (dat_en)   ? prnt_ctrl_reg : 32'bz;

//==============================================================
//initial

initial begin
  dat_en        = 1'b0;
  end

always
  #p_dly pclk_out = ~pclk_out;

  
endmodule

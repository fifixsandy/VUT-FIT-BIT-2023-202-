-- cpu.vhd: Simple 8-bit CPU (BrainFuck interpreter)
-- Copyright (C) 2024 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Filip Novak <xnovakf00 AT stud.fit.vutbr.cz>
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru
 
   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(12 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- mem[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_RDWR  : out std_logic;                    -- cteni (1) / zapis (0)
   DATA_EN    : out std_logic;                    -- povoleni cinnosti
   
   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA <- stav klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna
   IN_REQ    : out std_logic;                     -- pozadavek na vstup data
   
   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- LCD je zaneprazdnen (1), nelze zapisovat
   OUT_INV  : out std_logic;                      -- pozadavek na aktivaci inverzniho zobrazeni (1)
   OUT_WE   : out std_logic;                      -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'

   -- stavove signaly
   READY    : out std_logic;                      -- hodnota 1 znamena, ze byl procesor inicializovan a zacina vykonavat program
   DONE     : out std_logic                       -- hodnota 1 znamena, ze procesor ukoncil vykonavani programu (narazil na instrukci halt)
 );
end cpu;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is

  signal pc_inc   : std_logic;
  signal pc_dec   : std_logic;
  signal pc_out   : std_logic_vector(12 downto 0) := (others => '0');

  signal ptr_inc  : std_logic;
  signal ptr_dec  : std_logic;
  signal ptr_out  : std_logic_vector(12 downto 0) := (others => '0');

  signal cnt_inc  : std_logic;
  signal cnt_dec  : std_logic;
  signal cnt_out  : std_logic_vector(12 downto 0) := (others => '0');


  signal tmp_ld   : std_logic;
  signal tmp_out  : std_logic_vector(7 downto 0); 

  signal mx1_sel  : std_logic;
  signal mx2_sel  : std_logic_vector(1 downto 0);

  type FSM_state is ( s,
                      sWFORSTART0,
                      sWFORSTART1,
                      sINIT,
                      sFETCH,
                      sDECODE,

                      sPTRINC,
                      sPTRDEC,

                      sCELLINC0,
                      sCELLINC1,
                      sCELLINC2,

                      sCELLDEC0,
                      sCELLDEC1,
                      sCELLDEC2,
                      
                      sPRINT0,
                      sPRINT1,

                      sLOAD0,
                      sLOAD1,

                      sTMPLOAD0,
                      sTMPLOAD1,

                      sTMPUNLOAD,

                      sLOOP0,
                      sLOOP1,
                      sLOOP2,
                      sLOOP3,

                      sLOOP4,
                      sLOOP5,
                      sLOOP6,
                      sLOOP7,

                      sDONE);
  signal pstate  : FSM_state;
  signal nstate  : FSM_state;

begin

 -- pri tvorbe kodu reflektujte rady ze cviceni INP, zejmena mejte na pameti, ze 
 --   - nelze z vice procesu ovladat stejny signal,
 --   - je vhodne mit jeden proces pro popis jedne hardwarove komponenty, protoze pak
 --      - u synchronnich komponent obsahuje sensitivity list pouze CLK a RESET a 
 --      - u kombinacnich komponent obsahuje sensitivity list vsechny ctene signaly. 

  PC: process(CLK, RESET)
  begin
    if RESET = '1' then
      pc_out <= (others => '0');
    elsif rising_edge(CLK) then
      if pc_inc = '1' then
        pc_out <= pc_out + 1;
      elsif pc_dec = '1' then
        pc_out <= pc_out - 1;
      end if;
    end if;
  end process;

  PTR: process(CLK, RESET)
  begin
    if RESET = '1' then
      ptr_out <= (others => '0');
    elsif rising_edge(CLK) then
      if ptr_inc = '1' then
        if ptr_out = "1111111111111" then  -- final address (this is for the PTR + 1 % 0x2000)
          ptr_out <= "0000000000000";      -- goes back to 0
        else
          ptr_out <= ptr_out + 1;          -- not end, just increment
        end if;
      elsif ptr_dec = '1' then
        if ptr_out = "0000000000000" then  -- first address (this is for the PTR - 1 % 0x2000)
          ptr_out <= "1111111111111";      -- goes to max adress
        else
          ptr_out <= ptr_out - 1;          -- not beg, just decrement
        end if;
      end if;
    end if;
  end process;

  CNT: process(CLK, RESET)
  begin
    if RESET = '1' then
      cnt_out <= (others => '0');
    elsif rising_edge(CLK) then
      if cnt_inc = '1' then
        cnt_out <= cnt_out + 1;
      elsif cnt_dec = '1' then
        cnt_out <= cnt_out - 1;
      else
        cnt_out <= cnt_out;
      end if;
    end if;
  end process;


  TMP: process(CLK, RESET)
  begin
    if RESET = '1' then
        tmp_out <= (others => '0'); 
    elsif rising_edge(CLK) then
      if tmp_ld = '1' then
        tmp_out <= DATA_RDATA;
      end if;
    end if;
  end process;

  MX1: process(ptr_out, pc_out, mx1_sel)
  begin
    case mx1_sel is
      when '0' => 
            DATA_ADDR <= ptr_out;
      when '1' =>
            DATA_ADDR <= pc_out;
      when others =>
            DATA_ADDR <= "0000000000000";
    end case;
  end process;

  MX2: process(IN_DATA, tmp_out, DATA_RDATA, mx2_sel)
  begin
    case mx2_sel is
      when "00" =>
            DATA_WDATA <= IN_DATA;
      when "01" =>
            DATA_WDATA <= tmp_out;
      when "10" =>
            DATA_WDATA <= DATA_RDATA - 1;
      when "11" => 
            DATA_WDATA <= DATA_RDATA + 1;
      when others =>
            null;
    end case;
  end process;

  FSM_STATE_REG: process(CLK, RESET)
  begin
    if RESET = '1' then
      pstate <= s;
    elsif rising_edge(CLK) and EN = '1' then
      pstate <= nstate;
    end if; 
  end process;

  FSM_NSTATE_LOG: process(pstate, DATA_RDATA, OUT_BUSY, IN_VLD)
  begin

    IN_REQ    <= '0';
    OUT_INV   <= '0';
    OUT_WE    <= '0';
    DATA_RDWR <= '0';
    DATA_EN   <= '0';
    READY     <= '0';
    DONE      <= '0';

    cnt_inc   <= '0';
    cnt_dec   <= '0';
    
    tmp_ld <= '0';
    
    ptr_inc <= '0';
    ptr_dec <= '0';

    pc_inc  <= '0';
    pc_dec  <= '0';

    mx1_sel <= '0';
    mx2_sel <= (others => '0');

    case pstate is
      when s =>
        nstate <= sWFORSTART0;
      when sWFORSTART0 =>
        mx1_sel <= '0';
        DATA_EN <= '1';
        DATA_RDWR <= '1';
        nstate <= sWFORSTART1;
      when sWFORSTART1 =>
        mx1_sel <= '0';
        DATA_EN <= '1';
        DATA_RDWR <= '1';
        if DATA_RDATA = X"40" then
          nstate <= sINIT;
        else
          ptr_inc <= '1';
          nstate <= sWFORSTART1;
        end if;
      when sINIT =>
        READY <= '1';
        DATA_EN <= '0';
        nstate <= sFETCH;
      when sFETCH =>
        mx1_sel <= '1';
        DATA_EN <= '1';
        DATA_RDWR <= '1';
        nstate <= sDECODE;
      when sDECODE =>
        case DATA_RDATA is
          when x"3E" => -- >
            nstate <= sPTRINC;

          when x"3C" => -- <
            nstate <= sPTRDEC;

           when x"2B" => -- +
            nstate <= sCELLINC0;

           when x"2D" => -- -
            nstate <= sCELLDEC0;

          when x"5B" => -- [
            nstate <= sLOOP0;

          when x"5D" => -- ]
            nstate <= sLOOP4;

          when x"24" => -- $
            nstate <= sTMPLOAD0;

          when x"21" => -- !
            nstate <= sTMPUNLOAD;

          when x"2E" => -- .
            nstate <= sPRINT0;

          when x"2C" => -- ,
            nstate <= sLOAD0;

          when x"40" => -- @
            nstate <= sDONE;
          
          when others => -- comments
            pc_inc <= '1'; -- move to next instruction
            nstate <= sFETCH; -- FETCH next instruction
        end case;

      when sPTRINC => -- >
        ptr_inc <= '1';
        pc_inc  <= '1';
        nstate  <= sFETCH;
      
      when sPTRDEC => -- <
        ptr_dec <= '1';
        pc_inc  <= '1';
        nstate  <= sFETCH;
    
      -- cases sCELLINC0 and sCELLINC2 perform logic for +
      when sCELLINC0 =>
        mx1_sel <= '0'; -- reading from adress PTR
        DATA_EN <= '1';
        DATA_RDWR <= '1'; -- reading, DATA_RDATA will be value on current cell
        nstate <= sCELLINC1;

      when sCELLINC1 =>
        mx2_sel <= "11"; -- we will chose the last option in mx2, meaning it will take DATA_RDATA and increment it
        DATA_EN <= '1'; 
        DATA_RDWR <= '0'; -- writing
        pc_inc <= '1';
        nstate <= sFETCH; -- perform everything in next tact and move to sFETCH for other instructions

      -- cases sCELLDEC0 and sCELLDEC1 perform logic for -
      when sCELLDEC0 =>
        mx1_sel <= '0';
        DATA_EN <= '1';
        DATA_RDWR <= '1';
        nstate <= sCELLDEC1;
      
      when sCELLDEC1 =>
        mx2_sel <= "10"; -- we will chose the second to last option in mx2, meaning it will take DATA_RDATA and decrement it
        DATA_EN <= '1';
        DATA_RDWR <= '0'; -- writing
        pc_inc <= '1';
        nstate <= sFETCH; -- perform everything in next tact and move to sFETCH for other instructions
      
      -- cases sPRINT0 and sPRINT1 deal with .
      when sPRINT0 =>
        DATA_EN <= '1';     -- in next tact, we will access memory
        DATA_RDWR <= '1';   -- accessing memory for read
        mx1_sel <= '0';     -- accessing memory on address given by ptr
        nstate <= sPRINT1; -- move to the next state to perform printing

      when sPRINT1 =>
        if OUT_BUSY = '1' then -- periphery is busy, we are not performing any printing, but going back into this state
          nstate <= sPRINT1;
        else -- periphery is free, we already have the value read in DATA_RDATA
          OUT_DATA <= DATA_RDATA; -- put value to OUT_DATA from already loaded DATA_RDATA
          OUT_WE <= '1'; -- set OUT_WE to 1 for one tact, when printing
          pc_inc <= '1'; -- move instruction
          nstate <= sFETCH; -- move back to state sFETCH and perform all given instrucions
        end if;

      -- cases sLOAD0 and sLOAD1 deal with logic of ,  
      when sLOAD0 =>
        IN_REQ <= '1';    -- requesting access
        nstate <= sLOAD1;
      
      when sLOAD1 =>
        if IN_VLD = '0' then -- input is not valid, we have to loop
          IN_REQ <= '1'; -- and again requesting access
          nstate <= sLOAD1;
        else -- input is valid
          DATA_EN <= '1'; 
          DATA_RDWR <= '0'; -- we will write
          mx2_sel <= "00";  -- selecting the IN_DATA way for data
          pc_inc <= '1';
          nstate <= sFETCH;
        end if;

      -- cases sTMPLOAD0 and sTMPLOAD1 deal with $
      when sTMPLOAD0 =>
        mx1_sel <= '0'; -- accessing mem[ptr]
        DATA_EN <= '1';
        DATA_RDWR <= '1'; -- we will read
        nstate <= sTMPLOAD1;
      
      when sTMPLOAD1 =>
        tmp_ld <= '1'; -- load into TMP
        pc_inc <= '1'; -- move an instruction
        nstate <= sFETCH;

      -- cases sTMPUNLOAD deals with !
      when sTMPUNLOAD =>
        mx2_sel <= "01"; -- chosing DATA_WDATA from TMP
        DATA_EN <= '1';
        DATA_RDWR <= '0'; -- we will write
        pc_inc <= '1';
        nstate <= sFETCH;
      
      -- cases sLOOP0 to sLOOP7 deal with looping with [ and ]
      when sLOOP0 =>
          pc_inc <= '1';
          DATA_EN <= '1';
          mx1_sel <= '0';
          DATA_RDWR <= '1';
          nstate <= sLOOP1;
        
      when sLOOP1 =>
        if DATA_RDATA = "00000000" then
          cnt_inc <= '1';
          nstate <= sLOOP2;
        else
          nstate <= sFETCH;
        end if;
      
      when sLOOP2 =>
        if cnt_out = "0000000000000" then
          nstate <= sFETCH;
        else
          mx1_sel <= '1';
          DATA_EN <= '1';
          DATA_RDWR <= '1';
          nstate <= sLOOP3;
        end if;
      
      when sLOOP3 =>
        if DATA_RDATA = x"5B" then
          cnt_inc <= '1';

        elsif DATA_RDATA = x"5D" then
          cnt_dec <= '1';
        end if;
        pc_inc <= '1';
        nstate <= sLOOP2;
      
      when sLOOP4 =>
        DATA_EN <= '1';
        mx1_sel <= '0';
        DATA_RDWR <= '1';
        nstate <= sLOOP5;
      
      when sLOOP5 =>
        if DATA_RDATA = "00000000" then
          pc_inc <= '1';
          nstate <= sFETCH;
        else
          cnt_inc <= '1';
          pc_dec <= '1';
          nstate <= sLOOP6;
        end if;
        
      when sLOOP6 =>
        if cnt_out = "0000000000000" then
          pc_inc <= '1';
          nstate <= sFETCH;
        else
          mx1_sel <= '1';
          DATA_EN <= '1';
          DATA_RDWR <= '1';
          nstate <= sLOOP7;
        end if;
      
      when sLOOP7 =>
        if DATA_RDATA = x"5D" then
          cnt_inc <= '1';
        elsif DATA_RDATA = x"5B" then
          cnt_dec <= '1';
        end if;
        pc_dec <= '1';
        nstate <= sLOOP6;
        
      -- case sDONE ends the program
      when sDONE =>
        DONE <= '1';
        READY <= '1';
        nstate <= sDONE;
      when others =>

      end case;
        
  end process;
end behavioral;


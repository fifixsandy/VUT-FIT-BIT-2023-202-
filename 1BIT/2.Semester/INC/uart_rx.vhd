-- uart_rx.vhd: UART controller - receiving (RX) side
-- Author(s): THIS PROJECT HAS BEEN STORED ON GITHUB FOR PERSONAL USE, IF THIS COMMENT IS HERE, THE PERSON THAT HAS SUBMITTED IT HAS STOLEN IT AND DIDNT EVEN OPEN IT


library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;



-- Entity declaration (DO NOT ALTER THIS PART!)
entity UART_RX is
    port(
        CLK      : in std_logic;
        RST      : in std_logic;
        DIN      : in std_logic;
        DOUT     : out std_logic_vector(7 downto 0);
        DOUT_VLD : out std_logic
    );
end entity;



-- Architecture implementation (INSERT YOUR IMPLEMENTATION HERE)
architecture behavioral of UART_RX is
    signal reg_din_q : std_logic := '0';
    signal clk_cnt_en : std_logic;
    signal clk_cnt_q : std_logic_vector(3 downto 0) := (others => '0');
    signal clk_cnt_end : std_logic;
    signal strt_cnt_en : std_logic;
    signal strt_cnt_q : std_logic_vector(2 downto 0) := (others => '0');
    signal strt_cnt_end : std_logic;
    signal bit_cnt_en : std_logic;
    signal bit_cnt_rst : std_logic;
    signal bit_cnt_q : std_logic_vector(2 downto 0) := (others => '0');
    signal bit_cnt_end : std_logic;
    signal bit_ce : std_logic;
    signal dc_en : std_logic := '0';
    signal dc_slc : std_logic_vector(2 downto 0) := (others => '0');
    signal reg_dout_q : std_logic_vector(7 downto 0) := (others => '0');
    signal reg_slow_q: std_logic;
begin


    REG_DIN: process(CLK) 
    begin
        if rising_edge(CLK) then
            reg_din_q <= DIN;
        end if;
    end process;
            
    CLK_CNT: process(CLK, clk_cnt_en)
    begin
        if rising_edge(CLK) then
            if clk_cnt_en = '1' then
                clk_cnt_q <= clk_cnt_q + 1;
            else
                clk_cnt_q <= (others => '0');
            end if;
        end if;
    end process;

    E1: clk_cnt_end <= '1' when clk_cnt_q = "1111" else '0';
    
    STRT_CNT: process(CLK, strt_cnt_en)
    begin
        if rising_edge(CLK) then
            if strt_cnt_en = '1' then
                strt_cnt_q <= strt_cnt_q + 1;
            else
                strt_cnt_q <= (others => '0');
            end if;
        end if;
    end process;

    E2: strt_cnt_end <= '1' when strt_cnt_q = "111" else '0';

    BIT_CNT: process(CLK, bit_cnt_en, bit_cnt_rst)
    begin
        if rising_edge(CLK) then
            if bit_cnt_en = '1' then
                bit_cnt_q <= bit_cnt_q + 1;
            elsif bit_cnt_rst = '1' then
                bit_cnt_q <= (others => '0');
            end if;
        end if;
    end process;

    E3: bit_cnt_end <= '1' when bit_cnt_q = "111" else '0';

    AND_1: bit_cnt_en <= '1' when clk_cnt_end = '1' and bit_ce = '1' else '0';

    AND_2: reg_slow_q <= '1' when clk_cnt_end = '1' and strt_cnt_end = '0' else '0';

    dc_slc <= bit_cnt_q;

    REG_SLOW: process(CLK)
    begin
        if rising_edge(CLK) then
            dc_en <= reg_slow_q;
        end if;
    end process;

    DC: process(CLK, dc_en, dc_slc, bit_cnt_rst)
    begin
        if rising_edge(CLK) then
            if dc_en = '1' then
                case dc_slc is
                    when "000" =>
                        reg_dout_q(0) <= reg_din_q;
                    when "001" =>
                        reg_dout_q(1) <= reg_din_q;
                    when "010" =>
                        reg_dout_q(2) <= reg_din_q;
                    when "011" =>
                        reg_dout_q(3) <= reg_din_q;
                    when "100" => 
                        reg_dout_q(4) <= reg_din_q;
                    when "101" =>
                        reg_dout_q(5) <= reg_din_q;
                    when "110" =>
                        reg_dout_q(6) <= reg_din_q;
                    when "111" =>
                        reg_dout_q(7) <= reg_din_q;
                    when others =>
                        null;
                end case;
            elsif bit_cnt_rst = '1' then
                reg_dout_q <= (others => '0');
            else
                null;
            end if;
        end if;
    end process;

    DOUT <= reg_dout_q;

    -- Instance of RX FSM
    fsm: entity work.UART_RX_FSM
    port map (
        CLK => CLK,
        RST => RST,
        DIN_FSM => reg_din_q,
        DATA_READ => clk_cnt_end,
        WORD_END => bit_cnt_end,
        START_MID => strt_cnt_end,

        START_BIT_CE => strt_cnt_en, 
        CLK_CE => clk_cnt_en, 
        BIT_CE => bit_ce,
        VLD => DOUT_VLD,
        BIT_RST => bit_cnt_rst
    );


end architecture;

-- uart_rx_fsm.vhd: UART controller - finite state machine controlling RX side
-- Author(s): THIS PROJECT HAS BEEN STORED ON GITHUB FOR PERSONAL USE, IF THIS COMMENT IS HERE, THE PERSON THAT HAS SUBMITTED IT HAS STOLEN IT AND DIDNT EVEN OPEN IT


library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;



entity UART_RX_FSM is
    port(
       CLK : in std_logic;
       RST : in std_logic;
       START_MID : in std_logic;
       DIN_FSM : in std_logic;
       DATA_READ : in std_logic;
       WORD_END : in std_logic;

       START_BIT_CE : out std_logic;
       CLK_CE : out std_logic;
       BIT_CE : out std_logic;
       VLD : out std_logic;
       BIT_RST : out std_logic
    );
end entity;


architecture behavioral of UART_RX_FSM is
    type ST is (IDLE, START_BIT, DATA_WAIT, DATA, STOP_BIT, VALIDITY);
    signal state : ST;
    signal next_state : ST; 
begin

    state_register: process(CLK, RST)
    begin
        if rising_edge(CLK) then
            if RST = '1' then
                state <= IDLE;
            else 
                state <= next_state;
            end if;
        end if;
    end process;

    next_state_logic: process(state, START_MID, DIN_FSM, DATA_READ, WORD_END)
    begin
        next_state <= state;
        case state is
            when IDLE => if DIN_FSM = '0' then
                                next_state <= START_BIT;
                            end if;
            when START_BIT => if START_MID = '1' and DIN_FSM = '1' then
                                    next_state <= IDLE;
                                elsif START_MID = '1' and DIN_FSM = '0' then
                                    next_state <= DATA_WAIT;
                                end if;
            when DATA_WAIT => if DATA_READ = '1' then
                                  next_state <= DATA;
                                end if;
            when DATA => if WORD_END = '1' then
                                next_state <= STOP_BIT;
                            end if;
            when STOP_BIT => if DATA_READ = '1' and DIN_FSM = '1' then
                                    next_state <= VALIDITY;
                                elsif  DATA_READ = '1' and DIN_FSM = '0' then
                                    next_state <= IDLE;
                                end if;
            when VALIDITY => next_state <= IDLE;
            when others => null;
        end case;
    end process;

    output_logic: process(state)
    begin
        START_BIT_CE    <= '0';
        CLK_CE          <= '1';
        BIT_CE          <= '0';
        VLD             <= '0';
        BIT_RST         <= '0';

        case state is
            when IDLE => 
                BIT_RST      <= '1';
                CLK_CE       <= '0';
            when START_BIT =>
                START_BIT_CE <= '1';
                CLK_CE       <= '0';
            when DATA_WAIT =>
            when DATA => 
                BIT_CE       <= '1';
            when STOP_BIT => 
            when VALIDITY => 
                VLD          <= '1';
                CLK_CE       <= '0';
        end case;
    end process;
end architecture;

library ieee;
use ieee.std_logic_1164.all;

entity repro2 is
  port (aclr : std_logic;
        clk : std_logic;
        din : std_logic;
        dout : out std_logic);
end;

architecture behav of repro2 is
  signal r : std_logic;
begin
  process (aclr, clk) is
    variable init : boolean := false;
  begin
    if not init then
      r <= '0';
      init := true;
    end if;

    if rising_edge (clk) then
      r <= din;
    end if;
    if aclr = '1' then
      r <= '0';
    end if;
  end process;

  dout <= r;
end behav;

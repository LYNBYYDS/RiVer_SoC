entity tb_cmp01 is
end tb_cmp01;

library ieee;
use ieee.std_logic_1164.all;

architecture behav of tb_cmp01 is
  signal l  : std_logic_vector(3 downto 0);
  signal r  : std_logic_vector(3 downto 0);
  signal eq : std_logic;
  signal ne : std_logic;
  signal lt : std_logic;
  signal le : std_logic;
  signal ge : std_logic;
  signal gt : std_logic;
begin
  cmp01_1: entity work.cmp01
    port map (
      l  => l,
      r  => r,
      eq => eq,
      ne => ne,
      lt => lt,
      le => le,
      ge => ge,
      gt => gt);

  process
  begin
    l <= x"5";
    r <= x"7";
    wait for 1 ns;
    assert eq = '0' severity failure;
    assert ne = '1' severity failure;
    assert lt = '1' severity failure;
    assert le = '1' severity failure;
    assert ge = '0' severity failure;
    assert gt = '0' severity failure;

    l <= x"a";
    r <= x"7";
    wait for 1 ns;
    assert eq = '0' severity failure;
    assert ne = '1' severity failure;
    assert lt = '0' severity failure;
    assert le = '0' severity failure;
    assert ge = '1' severity failure;
    assert gt = '1' severity failure;

    l <= x"9";
    r <= x"9";
    wait for 1 ns;
    assert eq = '1' severity failure;
    assert ne = '0' severity failure;
    assert lt = '0' severity failure;
    assert le = '1' severity failure;
    assert ge = '1' severity failure;
    assert gt = '0' severity failure;

    wait;
  end process;
end behav;


-- Copyright (C) 1996 Morgan Kaufmann Publishers, Inc

-- This file is part of VESTs (Vhdl tESTs).

-- VESTs is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by the
-- Free Software Foundation; either version 2 of the License, or (at
-- your option) any later version. 

-- VESTs is distributed in the hope that it will be useful, but WITHOUT
-- ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
-- FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
-- for more details. 

-- You should have received a copy of the GNU General Public License
-- along with VESTs; if not, write to the Free Software Foundation,
-- Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 

-- ---------------------------------------------------------------------
--
-- $Id: ch_03_ch_03_13.vhd,v 1.2 2001-10-24 23:30:59 paw Exp $
-- $Revision: 1.2 $
--
-- ---------------------------------------------------------------------

entity ch_03_13 is
end entity ch_03_13;

architecture test of ch_03_13 is

  signal count_out : integer;

begin

  process_3_4_b : process is
  begin

    -- code from book:

    for count_value in 0 to 127 loop
      count_out <= count_value;
      wait for 5 ns;
    end loop;

    -- end of code from book

    wait;
  end process process_3_4_b;

end architecture test;


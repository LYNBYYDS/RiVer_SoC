
-- Copyright (C) 2001 Bill Billowitch.

-- Some of the work to develop this test suite was done with Air Force
-- support.  The Air Force and Bill Billowitch assume no
-- responsibilities for this software.

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
-- $Id: tc600.vhd,v 1.3 2001-10-29 02:12:45 paw Exp $
-- $Revision: 1.3 $
--
-- ---------------------------------------------------------------------


--                 ****************************               --
-- Ported to VHDL 93 by port93.pl - Tue Nov  5 16:37:40 1996  --
--                 ****************************               --



--                 ****************************                   --
-- Reversed to VHDL 87 by reverse87.pl - Tue Nov  5 11:25:58 1996  --
--                 ****************************                    --



--                 ****************************               --
-- Ported to VHDL 93 by port93.pl - Mon Nov  4 17:36:18 1996  --
--                 ****************************               --



ENTITY c03s04b01x00p01n01i00600ent IS
END c03s04b01x00p01n01i00600ent;

ARCHITECTURE c03s04b01x00p01n01i00600arch OF c03s04b01x00p01n01i00600ent IS
  type time_vector      is array (natural range <>) of time;
  type time_vector_file is file of time_vector;
  signal    k : integer := 0;
BEGIN
  TESTING: PROCESS
    file filein    : time_vector_file open read_mode is "iofile.27";
    variable  v    : time_vector(0 to 3);
    variable  len   : natural;
  BEGIN
    for i in 1 to 100 loop
      assert(endfile(filein) = false) report"end of file reached before expected";
      read(filein,v,len);
      assert(len = 4) report "wrong length passed during read operation";
      if (v /= (1 ns,2 ns,3 ns,4 ns)) then
        k <= 1;
      end if;
    end loop;
    wait for 1 ns;
    assert NOT(k = 0)
      report "***PASSED TEST: c03s04b01x00p01n01i00600"
      severity NOTE;
    assert (k = 0)
      report "***FAILED TEST: c03s04b01x00p01n01i00600 - File reading operation (time_vector file type) failed."
      severity ERROR;
    wait;
  END PROCESS TESTING;

END c03s04b01x00p01n01i00600arch;

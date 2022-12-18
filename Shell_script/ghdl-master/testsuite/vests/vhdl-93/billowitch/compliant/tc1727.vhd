
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
-- $Id: tc1727.vhd,v 1.2 2001-10-26 16:29:43 paw Exp $
-- $Revision: 1.2 $
--
-- ---------------------------------------------------------------------

ENTITY c12s06b01x00p04n03i01727ent IS
END c12s06b01x00p04n03i01727ent;

ARCHITECTURE c12s06b01x00p04n03i01727arch OF c12s06b01x00p04n03i01727ent IS
  signal   B : BIT := '1';
BEGIN
  TESTING: PROCESS
    variable ShouldBeTime : TIME; 
  BEGIN
    B <= '1','0' after 10 ns;
    ShouldBeTime := NOW + 10 ns;
    wait on B;
    assert NOT(( NOW = ShouldBeTime ) and ( B = '0' ))
      report "***PASSED TEST: c12s06b01x00p04n03i01727"
      severity NOTE;
    assert (( NOW = ShouldBeTime ) and ( B = '0' ))
      report "***FAILED TEST: c12s06b01x00p04n03i01727 - As time passes, the current transaction is deleted from the projected output waveform of that driver and the new tra nsaction takes its place."
      severity ERROR;
    wait;
  END PROCESS TESTING;

END c12s06b01x00p04n03i01727arch;

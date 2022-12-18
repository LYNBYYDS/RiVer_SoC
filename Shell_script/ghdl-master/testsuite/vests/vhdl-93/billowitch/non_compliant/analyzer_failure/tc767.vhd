
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
-- $Id: tc767.vhd,v 1.2 2001-10-26 16:30:27 paw Exp $
-- $Revision: 1.2 $
--
-- ---------------------------------------------------------------------

ENTITY c01s01b01x02p07n01i00767ent_a IS
  port ( c1 : out    integer ;
         c2 : in    integer );
END c01s01b01x02p07n01i00767ent_a;

ARCHITECTURE c01s01b01x02p07n01i00767arch_a OF c01s01b01x02p07n01i00767ent_a IS

BEGIN
  c1 <= c2;
END c01s01b01x02p07n01i00767arch_a;


ENTITY c01s01b01x02p07n01i00767ent IS
  port ( p1 : in    integer ;
         p2 : in    integer );
END c01s01b01x02p07n01i00767ent;

ARCHITECTURE c01s01b01x02p07n01i00767arch OF c01s01b01x02p07n01i00767ent IS
  component c01s01b01x02p07n01i00767ent_b
    port ( c1 : out    integer ;
           c2 : in    integer );
  end component;
  for L : c01s01b01x02p07n01i00767ent_b use entity work.c01s01b01x02p07n01i00767ent_a(c01s01b01x02p07n01i00767arch_a);
BEGIN
  L :c01s01b01x02p07n01i00767ent_b port map (p1, p2);
  -- Failure_here
  TESTING: PROCESS
  BEGIN
    assert FALSE
      report "***FAILED TEST: c01s01b01x02p07n01i00767 - An actual of mode in cannot be associated with a formal of mode out."
      severity NOTE;
    wait;
  END PROCESS TESTING;

END c01s01b01x02p07n01i00767arch;

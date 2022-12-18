package pixel_pkg is
    generic(
        NBITS                       : natural;
        NBR_OF_CHROMA               : natural
    );

    subtype CHROMA is bit_vector(NBITS-1 downto 0);
    type TYPE_PIXEL is array (NBR_OF_CHROMA-1 downto 0) of CHROMA;

    function bit_vector_to_pixel
      (in_vector : in bit_vector(NBR_OF_CHROMA*NBITS-1 downto 0))
      return TYPE_PIXEL;
end package pixel_pkg;

package body pixel_pkg is

  function bit_vector_to_pixel
    ( in_vector : in bit_vector(NBR_OF_CHROMA*NBITS-1 downto 0))
    return TYPE_PIXEL is
    variable pixel : TYPE_PIXEL;
  begin
    for chroma in 0 to NBR_OF_CHROMA -1 loop
      pixel(chroma) := in_vector((chroma+1)*NBITS-1 downto chroma*NBITS);
    end loop;
    return pixel;
  end function bit_vector_to_pixel;
end package body pixel_pkg;


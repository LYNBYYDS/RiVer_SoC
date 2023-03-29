CC=g++
INCLUDE_PATH=/usr/local/systemc-2.3.3/include
LINK_PATH=/usr/local/systemc-2.3.3/lib-linux64
LINKER_ARGS=-rpath=/usr/local/systemc-2.3.3/lib-linux64
LIBS=-lsystemc -lm
C_ARGS=-I$(INCLUDE_PATH) -L$(LINK_PATH) -Wl,$(LINKER_ARGS) $(LIBS) -g

all: pred_branch_cache_tb

pred_branch_cache_tb: pred_branch_cache_tb.o pred_branch_cache_tb.o
	$(CC) $^ -o $@ $(C_ARGS)

%.o: %.cpp	
	$(CC) -c $^ $(C_ARGS)

clean:
	rm -f *.o pred_branch_cache_tb *.vcd
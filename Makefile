CPP = g++
AR = ar

INCLUDE = 
LIBS= 
C_ARGS = -g -Wall -D_FILE_OFFSET_BITS=64 -D__STDC_FORMAT_MACROS -fPIC

BINARY = libtqlparser.a
ALL_OBJS = SqlParser.yo SqlLex.lo tql_parser.o

%.o : %.cpp
	$(CPP) $(C_ARGS) $(INCLUDE) -c  $< -o $(patsubst %.cpp,%.o,$<)
%.o : %.cc
	$(CPP) $(C_ARGS) $(INCLUDE) -c  $< -o $(patsubst %.cc,%.o,$<)
%.o : %.c
	$(CPP) $(C_ARGS) $(INCLUDE) -c  $< -o $(patsubst %.c,%.o,$<)
%.yo : %.y
	lemon -m $<
	makeheaders $(patsubst %.y,%.c,$<)
	$(CPP) $(C_ARGS) $(INCLUDE) -c $(patsubst %.y,%.c,$<) -o $(patsubst %.y,%.yo,$<)
%.lo : %.l
	flex -i --header-file=$(patsubst %.l,%.h,$<) --outfile=$(patsubst %.l,%.c,$<) $<
	$(CPP) $(C_ARGS) $(INCLUDE) -c $(patsubst %.l,%.c,$<) -o $(patsubst %.l,%.lo,$<)
	
all : $(BINARY)

$(BINARY) : $(ALL_OBJS)
	rm -f $@
	ar rcs $@ $(ALL_OBJS)

clean:
	rm -f $(ALL_OBJS) $(BINARY)

print:
	@echo "print all vars"
	@echo "all objs:" $(ALL_OBJS)

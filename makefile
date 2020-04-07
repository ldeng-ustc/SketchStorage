DIR_OBJ = obj
DIR_BIN = bin
DIR_EVAL = eval

EXECS_NAME = trace_preprocessing trace_analysis
EXECS = $(patsubst %, ${DIR_BIN}/%, ${EXECS_NAME})

EVALS_NAME = flowradar_decoding_eval flowradar_caida_eval
EVALS = $(patsubst %, ${DIR_BIN}/${DIR_EVAL}/%, ${EVALS_NAME})

OBJS_NAME = MurmurHash3.o
OBJS = $(patsubst %, ${DIR_OBJ}/%, ${OBJS_NAME})

CC = g++
LIBS = -lpcap
CPPFLAGS = -g

.phony: all clean
all: ${EXECS} ${EVALS}

${DIR_BIN}/${DIR_EVAL}/%: ${DIR_EVAL}/%.cpp
	@mkdir -p ${DIR_BIN}/${DIR_EVAL}
	$(CC) $^ ${OBJS} -o $@ ${LIBS}

${DIR_BIN}/%: ${DIR_OBJ}/%.o
	@mkdir -p ${DIR_BIN}
	$(CC) $^ ${OBJS} -o $@ ${LIBS}

${DIR_OBJ}/%.o: %.cpp ${OBJS}
	@mkdir -p ${DIR_OBJ}
	$(CC) $(CPPFLAGS) -c  $< -o $@

clean:
	rm -r bin obj
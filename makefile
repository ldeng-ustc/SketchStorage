DIR_OBJ = ./obj
DIR_BIN = ./bin
DIR_EVAL = eval

EXECS_NAME = trace_preprocessing trace_analysis
EXECS = $(patsubst %, ${DIR_BIN}/%, ${EXECS_NAME})

EVALS_NAME = flowradar_decoding_eval flowradar_caida_eval
EVALS = $(patsubst %, ${DIR_BIN}/%, ${EVALS_NAME})

OBJS_NAME = MurmurHash3.o
OBJS = $(patsubst %, ${DIR_OBJ}/%, ${OBJS_NAME})

CC = g++
LIBS = -lpcap
CPPFLAGS = -Wall -g

.phony: all clean
all: ${EXECS} ${EVALS}

${DIR_BIN}/trace_analysis: trace_analysis.cpp ${OBJS}
	@mkdir -p ${DIR_BIN}
	$(CC) $(CPPFLAGS) $< -o $@ ${OBJS} ${LIBS}

${DIR_BIN}/trace_preprocessing: trace_preprocessing.cpp ${OBJS}
	@mkdir -p ${DIR_BIN}
	$(CC) $(CPPFLAGS) $< -o $@ ${OBJS} ${LIBS}

${DIR_BIN}/flowradar_decoding_eval: ${DIR_EVAL}/flowradar_decoding_eval.cpp ${OBJS}
	@mkdir -p ${DIR_BIN}
	$(CC) $(CPPFLAGS) $< -o $@ ${OBJS} ${LIBS}

${DIR_BIN}/flowradar_caida_eval: ${DIR_EVAL}/flowradar_caida_eval.cpp ${OBJS}
	@mkdir -p ${DIR_BIN}
	$(CC) $(CPPFLAGS) $< -o $@ ${OBJS} ${LIBS}

${DIR_OBJ}/%.o: %.cpp
	@mkdir -p ${DIR_OBJ}
	$(CC) $(CPPFLAGS) -c  $< -o $@

clean:
	rm -r bin obj
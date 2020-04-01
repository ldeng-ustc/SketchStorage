
.phony: all clean

all: trace_preprocessing trace_analysis ./evaluations/flowradar_decoding_eval

trace_analysis: trace_analysis.cpp packet.h flow.h trace.h
	g++ $< -o $@ -lpcap

trace_preprocessing: trace_preprocessing.cpp packet.h flow.h trace.h
	g++ $< -o $@ -lpcap

./evaluations/flowradar_decoding_eval: ./evaluations/flowradar_decoding_eval.cpp MurmurHash3.cpp packet.h flow.h trace.h flowradar.h
	g++  MurmurHash3.cpp $< -o $@ -lpcap


clean:
	rm trace_preprocessing trace_analysis ./evaluations/flowradar_decoding_eval

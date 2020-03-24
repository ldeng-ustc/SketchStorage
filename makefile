
.phony: all clean

all: trace_preprocessing trace_analysis

trace_analysis: trace_analysis.cpp packet.h flow.h trace.h
	g++ $< -o $@ -lpcap

trace_preprocessing: trace_preprocessing.cpp packet.h flow.h trace.h
	g++ $< -o $@ -lpcap

clean:
	rm trace_preprocessing trace_analysis

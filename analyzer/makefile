CC=g++ -std=gnu++14
VLROOT=/usr/local/lib/vlfeat-0.9.21/
IDIRS=-I/usr/local/include/opencv -I/usr/local/include/opencv2 -I$(VLROOT)
LDIRS=-L/usr/local/lib/ -L$(VLROOT)bin/glnxa64/ -LD_LIBRARY_PATH$(VLROOT)bin/glnxa64/
LINKS=-lvl -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_features2d -lboost_program_options
%.o: %.cpp
	$(CC) -c -o $@ $< $(IDIRS) $(LDIRS) $(LINKS)

segment: segment.o Segmenter.cpp VLFeatWrapper.cpp Clump.cpp
	export LD_LIBRARY_PATH=$(VLROOT)bin/glnxa64
	$(CC) -o segment segment.o $(IDIRS) $(LDIRS) $(LINKS)

clean:
	rm -f segment *.o
	make

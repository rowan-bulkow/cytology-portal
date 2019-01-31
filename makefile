CC=clang++ -std=gnu++14
VLROOT=/Users/rbulkow/vlfeat-0.9.21/
IDIRS=-I/usr/local/include/opencv4 -I$(VLROOT) -I/opt/local/opt/boost/include/
LDIRS=-L/usr/local/lib/ -L$(VLROOT)bin/maci64/ -L/usr/local/opt/boost/lib/ #-LD_LIBRARY_PATH$(VLROOT)bin/maci64/
LINKS=-lvl -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_features2d -lboost_program_options
%.o: %.cpp
	$(CC) -c -o $@ $< $(IDIRS) $(LDIRS) $(LINKS)

segment: segment.o Segmenter.cpp VLFeatWrapper.cpp Clump.cpp
	$(CC) -o segment segment.o $(IDIRS) $(LDIRS) $(LINKS)

clean:
	rm -f segment *.o
	make

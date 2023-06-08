todo: test hellotriangle helloviewport adaptviewport movingtriangle \
	spinningcube hellotexture hellotexture2 multitex multitex2 \
	spinningcube_withlight

LDLIBS=-lGL -lGLEW -lglfw -lm 

spinningcube_withlight: spinningcube_withlight.o textfile.o

clean:
	rm -f *.o *~

cleanall: clean
	rm -f test hellotriangle helloviewport adaptviewport movingtriangle \
		spinningcube hellotexture hellotexture2 multitex multitex2 \
		spinningcube_withlight

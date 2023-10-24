STUDENT_ID=3030749

SRCDIR = ./
CFILELIST = signals.c

RAWC = $(patsubst %.c,%,$(addprefix $(SRCDIR), $(CFILELIST)))

all: signals

test: all
	./signals 10 out.txt
	cat out.txt

signals: signals.c
	gcc -g $^ -o $@

signal_tester: signal_tester.c
	gcc -g $^ -o $@

clean:
	rm -f *.o signals
	rm -rf $(STUDENT_ID)-signals-lab

zip: clean
#	create temp dir
	mkdir $(STUDENT_ID)-signals-lab
#	get all the c files to be .txt for archiving
	$(foreach file, $(RAWC), cp $(file).c $(file)-c.txt;)
#	move the necessary files into the temp dir
	cp signals.c Makefile $(STUDENT_ID)-signals-lab/
	mv *-c.txt $(STUDENT_ID)-signals-lab/
	zip -r $(STUDENT_ID)-signals-lab.zip $(STUDENT_ID)-signals-lab
	rm -rf $(STUDENT_ID)-signals-lab

.PHONY: clean zip

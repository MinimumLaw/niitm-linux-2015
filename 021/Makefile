#
#
#
ESIZE=$(shell awk 'BEGIN{srand();printf("%d",255*rand()+1)}')
ENUMS=$(shell awk 'BEGIN{srand();printf("%d",1024*1024*rand()+1)}')

mmapsort: main
	mv main mmapsort

main: main.c

clean:
	rm -f mmapsort main test.bin

run: mmapsort
	echo Make file with ${ENUMS} records of ${ESIZE} bytes each
	dd if=/dev/urandom of=test.bin bs=${ESIZE} count=${ENUMS}
	echo Starting sorting....
	./mmapsort test.bin ${ESIZE}
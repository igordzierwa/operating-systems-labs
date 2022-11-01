CC = gcc
CFLAGS = -Wall

all: program 4bytes1000records 4bytes3000records 512bytes1000records 512bytes3000records 4096bytes1000records 4096bytes3000 8192bytes1000records 8192bytes3000records

program:
	$(CC) $(CFLAGS) -o program.o main.c

4bytes1000records:
	echo "Liczba rekordów: 1000, Długość rekordu: 4" >> wyniki.txt
	echo "Czas generowania:" >> wyniki.txt
	./program.o generate file1.txt 1000 4 >> wyniki.txt
	echo "Pomiar czasu dla funkcji COPY przy użyciu funkcji SYS" >> wyniki.txt
	./program.o copy file1.txt file1_sys.txt 1000 4 sys >> wyniki.txt
	echo "Pomiar czasu dla funkcji COPY przy użyciu funkcji LIB" >> wyniki.txt
	./program.o copy file1.txt file1_lib.txt 1000 4 lib >> wyniki.txt
	echo "Pomiar czasu dla funkcji SORT przy użyciu funkcji SYS" >> wyniki.txt
	./program.o sort file1_sys.txt 1000 4 sys >> wyniki.txt
	echo "Pomiar czasu dla funkcji SORT przy użyciu funkcji LIB" >> wyniki.txt
	./program.o sort file1_lib.txt 1000 4 lib >> wyniki.txt
	echo "" >> wyniki.txt

4bytes3000records:
	echo "Liczba rekordów: 3000, Długość rekordu: 4" >> wyniki.txt
	echo "Czas generowania:" >> wyniki.txt
	./program.o generate file2.txt 3000 4 >> wyniki.txt
	echo "Pomiar czasu dla funkcji COPY przy użyciu funkcji SYS" >> wyniki.txt
	./program.o copy file2.txt file2_sys.txt 3000 4 sys >> wyniki.txt
	echo "Pomiar czasu dla funkcji COPY przy użyciu funkcji LIB" >> wyniki.txt
	./program.o copy file2.txt file2_lib.txt 3000 4 lib >> wyniki.txt
	echo "Pomiar czasu dla funkcji SORT przy użyciu funkcji SYS" >> wyniki.txt
	./program.o sort file2_sys.txt 3000 4 sys >> wyniki.txt
	echo "Pomiar czasu dla funkcji SORT przy użyciu funkcji LIB" >> wyniki.txt
	./program.o sort file2_lib.txt 3000 4 lib >> wyniki.txt
	echo "" >> wyniki.txt

512bytes1000records:
	echo "Liczba rekordów: 1000, Długość rekordu: 512" >> wyniki.txt
	echo "Czas generowania:" >> wyniki.txt
	./program.o generate file3.txt 1000 512 >> wyniki.txt
	echo "Pomiar czasu dla funkcji COPY przy użyciu funkcji SYS" >> wyniki.txt
	./program.o copy file3.txt file3_sys.txt 1000 512 sys >> wyniki.txt
	echo "Pomiar czasu dla funkcji COPY przy użyciu funkcji LIB" >> wyniki.txt
	./program.o copy file3.txt file3_lib.txt 1000 512 lib >> wyniki.txt
	echo "Pomiar czasu dla funkcji SORT przy użyciu funkcji SYS" >> wyniki.txt
	./program.o sort file3_sys.txt 1000 512 sys >> wyniki.txt
	echo "Pomiar czasu dla funkcji SORT przy użyciu funkcji LIB" >> wyniki.txt
	./program.o sort file3_lib.txt 1000 512 lib >> wyniki.txt
	echo "" >> wyniki.txt

512bytes3000records:
	echo "Liczba rekordów: 3000, Długość rekordu: 512" >> wyniki.txt
	echo "Czas generowania:" >> wyniki.txt
	./program.o generate file4.txt 3000 512 >> wyniki.txt
	echo "Pomiar czasu dla funkcji COPY przy użyciu funkcji SYS" >> wyniki.txt
	./program.o copy file4.txt file4_sys.txt 3000 512 sys >> wyniki.txt
	echo "Pomiar czasu dla funkcji COPY przy użyciu funkcji LIB" >> wyniki.txt
	./program.o copy file4.txt file4_lib.txt 3000 512 lib >> wyniki.txt
	echo "Pomiar czasu dla funkcji SORT przy użyciu funkcji SYS" >> wyniki.txt
	./program.o sort file4_sys.txt 3000 512 sys >> wyniki.txt
	echo "Pomiar czasu dla funkcji SORT przy użyciu funkcji LIB" >> wyniki.txt
	./program.o sort file4_lib.txt 3000 512 lib >> wyniki.txt
	echo "" >> wyniki.txt

4096bytes1000records:
	echo "Liczba rekordów: 1000, Długość rekordu: 4096" >> wyniki.txt
	echo "Czas generowania:" >> wyniki.txt
	./program.o generate file5.txt 1000 4096 >> wyniki.txt
	echo "Pomiar czasu dla funkcji COPY przy użyciu funkcji SYS" >> wyniki.txt
	./program.o copy file5.txt file5_sys.txt 1000 4096 sys >> wyniki.txt
	echo "Pomiar czasu dla funkcji COPY przy użyciu funkcji LIB" >> wyniki.txt
	./program.o copy file5.txt file5_lib.txt 1000 4096 lib >> wyniki.txt
	echo "Pomiar czasu dla funkcji SORT przy użyciu funkcji SYS" >> wyniki.txt
	./program.o sort file5_sys.txt 1000 4096 sys >> wyniki.txt
	echo "Pomiar czasu dla funkcji SORT przy użyciu funkcji LIB" >> wyniki.txt
	./program.o sort file5_lib.txt 1000 4096 lib >> wyniki.txt
	echo "" >> wyniki.txt

4096bytes3000:
	echo "Liczba rekordów: 3000, Długość rekordu: 4096" >> wyniki.txt
	echo "Czas generowania:" >> wyniki.txt
	./program.o generate file6.txt 3000 4096 >> wyniki.txt
	echo "Pomiar czasu dla funkcji COPY przy użyciu funkcji SYS" >> wyniki.txt
	./program.o copy file6.txt file6_sys.txt 3000 4096 sys >> wyniki.txt
	echo "Pomiar czasu dla funkcji COPY przy użyciu funkcji LIB" >> wyniki.txt
	./program.o copy file6.txt file6_lib.txt 3000 4096 lib >> wyniki.txt
	echo "Pomiar czasu dla funkcji SORT przy użyciu funkcji SYS" >> wyniki.txt
	./program.o sort file6_sys.txt 3000 4096 sys >> wyniki.txt
	echo "Pomiar czasu dla funkcji SORT przy użyciu funkcji LIB" >> wyniki.txt
	./program.o sort file6_lib.txt 3000 4096 lib >> wyniki.txt
	echo "" >> wyniki.txt

8192bytes1000records:
	echo "Liczba rekordów: 1000, Długość rekordu: 8192" >> wyniki.txt
	echo "Czas generowania:" >> wyniki.txt
	./program.o generate file7.txt 1000 8192 >> wyniki.txt
	echo "Pomiar czasu dla funkcji COPY przy użyciu funkcji SYS" >> wyniki.txt
	./program.o copy file7.txt file7_sys.txt 1000 8192 sys >> wyniki.txt
	echo "Pomiar czasu dla funkcji COPY przy użyciu funkcji LIB" >> wyniki.txt
	./program.o copy file7.txt file7_lib.txt 1000 8192 lib >> wyniki.txt
	echo "Pomiar czasu dla funkcji SORT przy użyciu funkcji SYS" >> wyniki.txt
	./program.o sort file7_sys.txt 1000 8192 sys >> wyniki.txt
	echo "Pomiar czasu dla funkcji SORT przy użyciu funkcji LIB" >> wyniki.txt
	./program.o sort file7_lib.txt 1000 8192 lib >> wyniki.txt
	echo "" >> wyniki.txt

8192bytes3000records:
	echo "Liczba rekordów: 3000, Długość rekordu: 8192" >> wyniki.txt
	echo "Czas generowania:" >> wyniki.txt
	./program.o generate file8.txt 3000 8192 >> wyniki.txt
	echo "Pomiar czasu dla fsunkcji COPY przy użyciu funkcji SYS" >> wyniki.txt
	./program.o copy file8.txt file8_sys.txt 3000 8192 sys >> wyniki.txt
	echo "Pomiar czasu dla funkcji COPY przy użyciu funkcji LIB" >> wyniki.txt
	./program.o copy file8.txt file8_lib.txt 3000 8192 lib >> wyniki.txt
	echo "Pomiar czasu dla funkcji SORT przy użyciu funkcji SYS" >> wyniki.txt
	./program.o sort file8_sys.txt 3000 8192 sys >> wyniki.txt
	echo "Pomiar czasu dla funkcji SORT przy użyciu funkcji LIB" >> wyniki.txt
	./program.o sort file8_lib.txt 3000 8192 lib >> wyniki.txt
	echo "" >> wyniki.txt

clean:
	rm -f *.txt *.o

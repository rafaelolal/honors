NVCC = nvcc
# NVFLAGS = -O3 -g -arch=native -Xcompiler -Wall
NVFLAGS = --extended-lambda -Xcompiler -Wall

prog: main.cu
	$(NVCC) $(NVFLAGS) -o $@ main.cu -lm

clean:
	rm -f prog

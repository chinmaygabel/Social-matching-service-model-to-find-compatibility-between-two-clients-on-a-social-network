all: central.cpp serverT.cpp serverS.cpp serverP.cpp clientA.cpp clientB.cpp
	g++ -o serverC central.cpp

	g++ -o serverT serverT.cpp

	g++ -o serverS serverS.cpp
	
	g++ -o serverP serverP.cpp	

	g++ -o clientA clientA.cpp

	g++ -o clientB clientB.cpp


clean:
	rm -f clientA clientB serverC serverS serverP serverT

.PHONY: serverC
serverC: 
	./serverC

.PHONY: serverT
serverT:
	./serverT

.PHONY: serverS
serverS:
	./serverS

.PHONY: serverP
serverP:
	./serverP
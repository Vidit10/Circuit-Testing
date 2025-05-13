#include <stdio.h>
#include <stdbool.h>

// Valid gate types are - and, nand, or, nor, xor, xnor, not, buff

bool andFunction(bool gate1, bool gate2){
	bool result;
	if(gate1==gate2){
		if(gate1=1){
			result=1;
			return result;
		}
	}
	result=0;
	return result;
}

bool orFunction(bool gate1, bool gate2){
	bool result;
	if(gate1==gate2){
		if(gate1=0){
			result=0;
			return result;
		}
	}
	result=1;
	return result;
}

bool notFunction(bool gate){
	bool result;
	if(gate==1){
		result=0;
		return result;
	}
	result=1;
	return result;
}

bool nandFunction(bool gate1, bool gate2){
	return notFunction(andFunction(gate1, gate2));
}

bool norFunction(bool gate1, bool gate2){
	return notFunction(orFunction(gate1, gate2));
}

bool xorFunction(bool gate1, bool gate2){
	bool result;
	if(gate1==gate2){
		result=0;
		return result;
	}
	result=1;
	return result;
}

bool xnorFunction(bool gate1, bool gate2){
	return notFunction(xorFunction(gate1, gate2));
}

bool buffFunction(bool gate){
	return gate;
}

int main(){
	// Testing gates
	/*
	bool gate1 = 1;
	bool gate2 = 0;
	printf("Gate-1: %d; Gate-2: %d\n", gate1, gate2);
	printf("AND gate: %d\n", andFunction(gate1, gate2));
	printf("OR gate: %d\n", orFunction(gate1, gate2));
	printf("NOT gate for gate1: %d\n", notFunction(gate1));
	printf("NAND gate: %d\n", nandFunction(gate1, gate2));
	printf("NOR gate: %d\n", norFunction(gate1, gate2));
	printf("XOR gate: %d\n", xorFunction(gate1, gate2));
	printf("XNOR gate: %d\n", xnorFunction(gate1, gate2));
	printf("BUFF gate for gate1: %d\n", buffFunction(gate1));
	*/
	return 0;
}

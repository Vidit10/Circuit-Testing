#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Gate types
typedef enum {
    AND, NAND, OR, NOR, XOR, XNOR, NOT, BUFF, INPUT, OUTPUT, BRANCH
} GateType;

// Node structure for the linked list
typedef struct Node {
    int id;
    char name[50];
    GateType type;
    int numInputs;
    struct Node **inputs;
    int numOutputs;
    struct Node **outputs;
    bool value;
    bool visited;
    int pathCount;
} Node;

// Circuit structure
typedef struct {
    Node **nodes;
    int nodeCount;
    Node **inputs;
    int inputCount;
    Node **outputs;
    int outputCount;
} Circuit;

// Function to create a new node
Node* createNode(int id, const char* name, GateType type) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->id = id;
    strncpy(newNode->name, name, 49);
    newNode->name[49] = '\0';
    newNode->type = type;
    newNode->numInputs = 0;
    newNode->inputs = NULL;
    newNode->numOutputs = 0;
    newNode->outputs = NULL;
    newNode->value = false;
    newNode->visited = false;
    newNode->pathCount = 0;
    return newNode;
}

// Function to add an input to a node
void addInput(Node* node, Node* input) {
    node->numInputs++;
    node->inputs = (Node**)realloc(node->inputs, node->numInputs * sizeof(Node*));
    node->inputs[node->numInputs - 1] = input;
    input->numOutputs++;
    input->outputs = (Node**)realloc(input->outputs, input->numOutputs * sizeof(Node*));
    input->outputs[input->numOutputs - 1] = node;
}

// Function to initialize a circuit
Circuit* initCircuit() {
    Circuit* circuit = (Circuit*)malloc(sizeof(Circuit));
    circuit->nodes = NULL;
    circuit->nodeCount = 0;
    circuit->inputs = NULL;
    circuit->inputCount = 0;
    circuit->outputs = NULL;
    circuit->outputCount = 0;
    return circuit;
}

// Function to add a node to the circuit
void addNode(Circuit* circuit, Node* node) {
    circuit->nodeCount++;
    circuit->nodes = (Node**)realloc(circuit->nodes, circuit->nodeCount * sizeof(Node*));
    circuit->nodes[circuit->nodeCount - 1] = node;
    if (node->type == INPUT) {
        circuit->inputCount++;
        circuit->inputs = (Node**)realloc(circuit->inputs, circuit->inputCount * sizeof(Node*));
        circuit->inputs[circuit->inputCount - 1] = node;
    }
    if (node->type == OUTPUT) {
        circuit->outputCount++;
        circuit->outputs = (Node**)realloc(circuit->outputs, circuit->outputCount * sizeof(Node*));
        circuit->outputs[circuit->outputCount - 1] = node;
    }
}

// Function to find a node by name
Node* findNode(Circuit* circuit, const char* name) {
    for (int i = 0; i < circuit->nodeCount; i++) {
        if (strcmp(circuit->nodes[i]->name, name) == 0) {
            return circuit->nodes[i];
        }
    }
    return NULL;
}

// Function to parse the Verilog file and build the circuit
Circuit* parseVerilogFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file: %s\n", filename);
        exit(1);
    }
    Circuit* circuit = initCircuit();
    char line[256];
    // Parse input, output, and wire declarations
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "input")) {
            char* token = strtok(line + 5, ",;");
            while (token) {
                while (*token == ' ' || *token == '\t' || *token == '\n') token++;
                if (*token) {
                    Node* node = createNode(circuit->nodeCount, token, INPUT);
                    addNode(circuit, node);
                }
                token = strtok(NULL, ",;");
            }
        }
        if (strstr(line, "output")) {
            char* token = strtok(line + 6, ",;");
            while (token) {
                while (*token == ' ' || *token == '\t' || *token == '\n') token++;
                if (*token) {
                    Node* node = createNode(circuit->nodeCount, token, OUTPUT);
                    addNode(circuit, node);
                }
                token = strtok(NULL, ",;");
            }
        }
        if (strstr(line, "wire")) {
            char* token = strtok(line + 4, ",;");
            while (token) {
                while (*token == ' ' || *token == '\t' || *token == '\n') token++;
                if (*token) {
                    Node* node = createNode(circuit->nodeCount, token, BRANCH);
                    addNode(circuit, node);
                }
                token = strtok(NULL, ",;");
            }
        }
        // Stop after declarations
        if (strstr(line, ";")) break;
    }
    // Parse gate declarations
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "endmodule")) break;
        char type[16], inst[32], out[32], in1[32], in2[32];
        if (sscanf(line, "%15s %31s (%31[^,], %31[^,], %31[^)]);", type, inst, out, in1, in2) == 5) {
            GateType gateType;
            if (strcmp(type, "nand") == 0) gateType = NAND;
            else if (strcmp(type, "and") == 0) gateType = AND;
            else if (strcmp(type, "or") == 0) gateType = OR;
            else if (strcmp(type, "nor") == 0) gateType = NOR;
            else if (strcmp(type, "xor") == 0) gateType = XOR;
            else if (strcmp(type, "xnor") == 0) gateType = XNOR;
            else if (strcmp(type, "not") == 0) gateType = NOT;
            else if (strcmp(type, "buff") == 0) gateType = BUFF;
            else continue;
            Node* outputNode = findNode(circuit, out);
            if (!outputNode) {
                outputNode = createNode(circuit->nodeCount, out, gateType);
                addNode(circuit, outputNode);
            }
            outputNode->type = gateType;
            Node* inputNode1 = findNode(circuit, in1);
            if (!inputNode1) {
                inputNode1 = createNode(circuit->nodeCount, in1, BRANCH);
                addNode(circuit, inputNode1);
            }
            addInput(outputNode, inputNode1);
            Node* inputNode2 = findNode(circuit, in2);
            if (!inputNode2) {
                inputNode2 = createNode(circuit->nodeCount, in2, BRANCH);
                addNode(circuit, inputNode2);
            }
            addInput(outputNode, inputNode2);
        }
    }
    fclose(file);
    return circuit;
}

// Example main function
int main() {
    Circuit* circuit = parseVerilogFile("c1908 (2).v");
    printf("Parsed circuit with %d nodes, %d inputs, %d outputs.\n", circuit->nodeCount, circuit->inputCount, circuit->outputCount);
    // Further processing or simulation can be added here
    // Free memory as needed
    return 0;
}

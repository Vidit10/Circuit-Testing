#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Gate types
typedef enum {
    AND,
    NAND,
    OR,
    NOR,
    XOR,
    XNOR,
    NOT,
    BUFF,
    INPUT,
    OUTPUT,
    BRANCH
} GateType;

// Node structure for the linked list
typedef struct Node {
    int id;                 // Node identifier
    char name[50];          // Node name
    GateType type;          // Type of gate
    int numInputs;          // Number of inputs to this gate
    struct Node **inputs;   // Array of pointers to input nodes
    int numOutputs;         // Number of outputs from this node
    struct Node **outputs;  // Array of pointers to output nodes
    bool value;             // Current value of the node
    bool visited;           // Flag for traversal algorithms
    int pathCount;          // For path counting algorithm
} Node;

// Circuit structure
typedef struct {
    Node **nodes;           // Array of all nodes
    int nodeCount;          // Total number of nodes
    Node **inputs;          // Array of input nodes
    int inputCount;         // Number of input nodes
    Node **outputs;         // Array of output nodes
    int outputCount;        // Number of output nodes
} Circuit;

// Gate functions from functional_codes.c
bool andFunction(bool gate1, bool gate2) {
    if (gate1 == 1 && gate2 == 1) {
        return 1;
    }
    return 0;
}

bool orFunction(bool gate1, bool gate2) {
    if (gate1 == 0 && gate2 == 0) {
        return 0;
    }
    return 1;
}

bool notFunction(bool gate) {
    return !gate;
}

bool nandFunction(bool gate1, bool gate2) {
    return notFunction(andFunction(gate1, gate2));
}

bool norFunction(bool gate1, bool gate2) {
    return notFunction(orFunction(gate1, gate2));
}

bool xorFunction(bool gate1, bool gate2) {
    if (gate1 == gate2) {
        return 0;
    }
    return 1;
}

bool xnorFunction(bool gate1, bool gate2) {
    return notFunction(xorFunction(gate1, gate2));
}

bool buffFunction(bool gate) {
    return gate;
}

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
    if (!node->inputs) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    node->inputs[node->numInputs - 1] = input;
    
    // Also add this node as an output to the input node
    input->numOutputs++;
    input->outputs = (Node**)realloc(input->outputs, input->numOutputs * sizeof(Node*));
    if (!input->outputs) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    input->outputs[input->numOutputs - 1] = node;
}

// Function to initialize a circuit
Circuit* initCircuit() {
    Circuit* circuit = (Circuit*)malloc(sizeof(Circuit));
    if (!circuit) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    
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
    if (!circuit->nodes) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    circuit->nodes[circuit->nodeCount - 1] = node;
    
    // If it's an input node, add it to the inputs array
    if (node->type == INPUT) {
        circuit->inputCount++;
        circuit->inputs = (Node**)realloc(circuit->inputs, circuit->inputCount * sizeof(Node*));
        if (!circuit->inputs) {
            printf("Memory allocation failed\n");
            exit(1);
        }
        circuit->inputs[circuit->inputCount - 1] = node;
    }
    
    // If it's an output node, add it to the outputs array
    if (node->type == OUTPUT) {
        circuit->outputCount++;
        circuit->outputs = (Node**)realloc(circuit->outputs, circuit->outputCount * sizeof(Node*));
        if (!circuit->outputs) {
            printf("Memory allocation failed\n");
            exit(1);
        }
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
    char name[50], type[50], input1[50], input2[50];
    
    // Skip comments and module declaration
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "input") || strstr(line, "output") || strstr(line, "wire")) {
            break;
        }
    }
    
    // Parse input declarations
    if (strstr(line, "input")) {
        char* token = strtok(line + 6, ","); // Skip "input "
        while (token != NULL) {
            // Remove any trailing semicolons or whitespace
            char* semicolon = strchr(token, ';');
            if (semicolon) *semicolon = '\0';
            
            // Trim whitespace
            while (*token == ' ' || *token == '\t' || *token == '\n') token++;
            char* end = token + strlen(token) - 1;
            while (end > token && (*end == ' ' || *end == '\t' || *end == '\n')) *end-- = '\0';
            
            if (strlen(token) > 0) {
                Node* node = createNode(circuit->nodeCount, token, INPUT);
                addNode(circuit, node);
            }
            
            token = strtok(NULL, ",");
        }
        
        // Get the next line
        fgets(line, sizeof(line), file);
    }
    
    // Parse output declarations
    if (strstr(line, "output")) {
        char* token = strtok(line + 7, ","); // Skip "output "
        while (token != NULL) {
            // Remove any trailing semicolons or whitespace
            char* semicolon = strchr(token, ';');
            if (semicolon) *semicolon = '\0';
            
            // Trim whitespace
            while (*token == ' ' || *token == '\t' || *token == '\n') token++;
            char* end = token + strlen(token) - 1;
            while (end > token && (*end == ' ' || *end == '\t' || *end == '\n')) *end-- = '\0';
            
            if (strlen(token) > 0) {
                Node* node = createNode(circuit->nodeCount, token, OUTPUT);
                addNode(circuit, node);
            }
            
            token = strtok(NULL, ",");
        }
        
        // Get the next line
        fgets(line, sizeof(line), file);
    }
    
    // Parse wire declarations
    if (strstr(line, "wire")) {
        char* token = strtok(line + 5, ","); // Skip "wire "
        while (token != NULL) {
            // Remove any trailing semicolons or whitespace
            char* semicolon = strchr(token, ';');
            if (semicolon) *semicolon = '\0';
            
            // Trim whitespace
            while (*token == ' ' || *token == '\t' || *token == '\n') token++;
            char* end = token + strlen(token) - 1;
            while (end > token && (*end == ' ' || *end == '\t' || *end == '\n')) *end-- = '\0';
            
            if (strlen(token) > 0) {
                // Create wire nodes (these will be connected later)
                Node* node = createNode(circuit->nodeCount, token, BRANCH);
                addNode(circuit, node);
            }
            
            token = strtok(NULL, ",");
        }
    }
    
    // Parse gate declarations
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "endmodule")) {
            break;
        }
        
        // Skip empty lines and comments
        if (strlen(line) <= 1 || line[0] == '/' || line[0] == '\n') {
            continue;
        }
        
        // Parse the gate declaration
        // Format: nand NAND2_1 (N10, N1, N3);
        if (sscanf(line, "%s %s (%[^,], %[^,], %[^)]);", type, name, name, input1, input2) == 5) {
            // Extract the output node name (first parameter)
            char output[50];
            sscanf(line, "%*s %*s (%[^,],", output);
            
            // Find or create the output node
            Node* outputNode = findNode(circuit, output);
            if (!outputNode) {
                outputNode = createNode(circuit->nodeCount, output, BRANCH);
                addNode(circuit, outputNode);
            }
            
            // Determine the gate type
            GateType gateType;
            if (strcmp(type, "nand") == 0) {
                gateType = NAND;
            } else if (strcmp(type, "and") == 0) {
                gateType = AND;
            } else if (strcmp(type, "or") == 0) {
                gateType = OR;
            } else if (strcmp(type, "nor") == 0) {
                gateType = NOR;
            } else if (strcmp(type, "xor") == 0) {
                gateType = XOR;
            } else if (strcmp(type, "xnor") == 0) {
                gateType = XNOR;
            } else if (strcmp(type, "not") == 0) {
                gateType = NOT;
            } else if (strcmp(type, "buff") == 0) {
                gateType = BUFF;
            } else {
                printf("Unknown gate type: %s\n", type);
                continue;
            }
            
            // Update the output node's type
            outputNode->type = gateType;
            
            // Find or create the input nodes and connect them
            Node* inputNode1 = findNode(circuit, input1);
            if (!inputNode1) {
                inputNode1 = createNode(circuit->nodeCount, input1, BRANCH);
                addNode(circuit, inputNode1);
            }
            
            Node* inputNode2 = findNode(circuit, input2);
            if (!inputNode2) {
                inputNode2 = createNode(circuit->nodeCount, input2, BRANCH);
                addNode(circuit, inputNode2);
            }
            
            // Connect the inputs to the output
            addInput(outputNode, inputNode1);
            addInput(outputNode, inputNode2);
        }
    }
    
    fclose(file);
    return circuit;
}

// Function to evaluate a node based on its inputs and gate type
bool evaluateNode(Node* node) {
    if (node->type == INPUT) {
        return node->value; // Input values are set externally
    }
    
    if (node->numInputs == 0) {
        return false; // No inputs, default to false
    }
    
    bool result = false;
    
    switch (node->type) {
        case AND:
            result = andFunction(node->inputs[0]->value, node->inputs[1]->value);
            break;
        case NAND:
            result = nandFunction(node->inputs[0]->value, node->inputs[1]->value);
            break;
        case OR:
            result = orFunction(node->inputs[0]->value, node->inputs[1]->value);
            break;
        case NOR:
            result = norFunction(node->inputs[0]->value, node->inputs[1]->value);
            break;
        case XOR:
            result = xorFunction(node->inputs[0]->value, node->inputs[1]->value);
            break;
        case XNOR:
            result = xnorFunction(node->inputs[0]->value, node->inputs[1]->value);
            break;
        case NOT:
            result = notFunction(node->inputs[0]->value);
            break;
        case BUFF:
            result = buffFunction(node->inputs[0]->value);
            break;
        case BRANCH:
            result = node->inputs[0]->value; // Pass through the input value
            break;
        case OUTPUT:
            if (node->numInputs > 0) {
                result = node->inputs[0]->value; // Pass through the input value
            }
            break;
        default:
            printf("Unknown gate type for node %s\n", node->name);
            break;
    }
    
    return result;
}

// Function to simulate the circuit
void simulateCircuit(Circuit* circuit, bool* inputValues) {
    // Set input values
    for (int i = 0; i < circuit->inputCount; i++) {
        circuit->inputs[i]->value = inputValues[i];
    }
    
    // Reset visited flags
    for (int i = 0; i < circuit->nodeCount; i++) {
        circuit->nodes[i]->visited = false;
    }
    
    // Topological evaluation
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < circuit->nodeCount; i++) {
            Node* node = circuit->nodes[i];
            if (node->type == INPUT) {
                continue; // Skip input nodes, they're already set
            }
            
            bool allInputsVisited = true;
            for (int j = 0; j < node->numInputs; j++) {
                if (!node->inputs[j]->visited) {
                    allInputsVisited = false;
                    break;
                }
            }
            
            if (allInputsVisited && !node->visited) {
                bool newValue = evaluateNode(node);
                if (newValue != node->value) {
                    node->value = newValue;
                    changed = true;
                }
                node->visited = true;
            }
        }
    }
}

// Function to count paths in the circuit
int countPaths(Circuit* circuit) {
    // Initialize path counts
    for (int i = 0; i < circuit->nodeCount; i++) {
        circuit->nodes[i]->pathCount = 0;
        circuit->nodes[i]->visited = false;
    }
    
    // Set path count for input nodes to 1
    for (int i = 0; i < circuit->inputCount; i++) {
        circuit->inputs[i]->pathCount = 1;
        circuit->inputs[i]->visited = true;
    }
    
    // Topological traversal for path counting
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < circuit->nodeCount; i++) {
            Node* node = circuit->nodes[i];
            if (node->visited) {
                continue;
            }
            
            bool allInputsVisited = true;
            for (int j = 0; j < node->numInputs; j++) {
                if (!node->inputs[j]->visited) {
                    allInputsVisited = false;
                    break;
                }
            }
            
            if (allInputsVisited) {
                // Calculate path count as sum of input path counts
                for (int j = 0; j < node->numInputs; j++) {
                    node->pathCount += node->inputs[j]->pathCount;
                }
                node->visited = true;
                changed = true;
            }
        }
    }
    
    // Sum path counts for output nodes
    int totalPaths = 0;
    for (int i = 0; i < circuit->outputCount; i++) {
        totalPaths += circuit->outputs[i]->pathCount;
    }
    
    return totalPaths;
}

// Function to print circuit details
void printCircuit(Circuit* circuit) {
    printf("Circuit Details:\n");
    printf("Total nodes: %d\n", circuit->nodeCount);
    printf("Input nodes: %d\n", circuit->inputCount);
    printf("Output nodes: %d\n", circuit->outputCount);
    
    printf("\nInput Nodes:\n");
    for (int i = 0; i < circuit->inputCount; i++) {
        printf("  %s (ID: %d)\n", circuit->inputs[i]->name, circuit->inputs[i]->id);
    }
    
    printf("\nOutput Nodes:\n");
    for (int i = 0; i < circuit->outputCount; i++) {
        printf("  %s (ID: %d)\n", circuit->outputs[i]->name, circuit->outputs[i]->id);
    }
    
    printf("\nAll Nodes and Connections:\n");
    for (int i = 0; i < circuit->nodeCount; i++) {
        Node* node = circuit->nodes[i];
        printf("Node: %s (ID: %d, Type: ", node->name, node->id);
        
        switch (node->type) {
            case AND: printf("AND"); break;
            case NAND: printf("NAND"); break;
            case OR: printf("OR"); break;
            case NOR: printf("NOR"); break;
            case XOR: printf("XOR"); break;
            case XNOR: printf("XNOR"); break;
            case NOT: printf("NOT"); break;
            case BUFF: printf("BUFF"); break;
            case INPUT: printf("INPUT"); break;
            case OUTPUT: printf("OUTPUT"); break;
            case BRANCH: printf("BRANCH"); break;
            default: printf("UNKNOWN"); break;
        }
        printf(")\n");
        
        printf("  Inputs (%d):", node->numInputs);
        for (int j = 0; j < node->numInputs; j++) {
            printf(" %s", node->inputs[j]->name);
        }
        printf("\n");
        
        printf("  Outputs (%d):", node->numOutputs);
        for (int j = 0; j < node->numOutputs; j++) {
            printf(" %s", node->outputs[j]->name);
        }
        printf("\n\n");
    }
}

// Function to free the circuit memory
void freeCircuit(Circuit* circuit) {
    if (!circuit) return;
    
    for (int i = 0; i < circuit->nodeCount; i++) {
        Node* node = circuit->nodes[i];
        if (node) {
            if (node->inputs) free(node->inputs);
            if (node->outputs) free(node->outputs);
            free(node);
        }
    }
    
    if (circuit->nodes) free(circuit->nodes);
    if (circuit->inputs) free(circuit->inputs);
    if (circuit->outputs) free(circuit->outputs);
    free(circuit);
}

int main() {
    // Parse the Verilog file
    Circuit* circuit = parseVerilogFile("c432 (1).v");
    
    // Print circuit details
    printCircuit(circuit);
    
    // Count paths in the circuit
    int pathCount = countPaths(circuit);
    printf("Total number of paths in the circuit: %d\n", pathCount);
    
    // Simulate the circuit with all inputs set to 1
    bool inputValues[5] = {1, 1, 1, 1, 1};
    simulateCircuit(circuit, inputValues);
    
    // Print output values
    printf("\nOutput values after simulation (all inputs = 1):\n");
    for (int i = 0; i < circuit->outputCount; i++) {
        printf("%s: %d\n", circuit->outputs[i]->name, circuit->outputs[i]->value);
    }
    
    // Free memory
    freeCircuit(circuit);
    
    return 0;
}
